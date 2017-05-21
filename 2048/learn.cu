#include <cstdio>
#include <cstring>
#include "board.hpp"
#include "attr.hpp"

#include <vector>

#include <time.h>

#include <cuda.h>
#include <curand_kernel.h>

#define INF 0x71227122

#define gpuErrchk(ans) { gpuAssert((ans),__FILE__,__LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line)
{
	if (code != cudaSuccess){
		fprintf(stderr, "error: %s %s %d\n", cudaGetErrorString(code), file, line);
	}

}

using namespace std;

/*
 * all variable with suffix _d means it's a device memory(global memory)
 * all variable with suffix _ds means it's a device share memory
 */


typedef int (board::*MoveFunc)(bool);
// record the two continuous states in the game
class record {
public:
	record() {}
	record(board _s1, board _s2, int _earned) :s1(_s1), s2(_s2), earned(_earned) {}
	board s1, s2;
	int earned;
};

class record_d {
public:
	int s[2][4];
	int earned;
};

int recProgress_h[10005][3];

const int recordSize = 50000;		// should larger than any possible game's length
const int attrNumLimit = 4;			// array size
const int attrNum = 4;				// actual usage
const int attrSlotNum = 4;			// the number of slots in any attribute
const int attrDataSize = attrSlotNum*4;// the table size of attribute
const int branchSizeLimit = 1000;	// the number of random seed
const int branchSize = 208;			// the maximum number of thread (the number of rec array) should be larger than the maximum number of block GPU can run simultaneously

vector<record> rec;

// read in attribute data
bool load(const char* filename, vector<Attr> &attr) {
	FILE* fin = fopen(filename, "rb");
	if (!fin){
		return false;
	}
	int attrN;
	fread(&attrN, sizeof(int), 1, fin);
	attr.resize(attrN);
	for (int i = 0; i<attrN; i++){
		fread(&attr[i].slotNum, sizeof(int), 1, fin);
		fread(&attr[i].position, sizeof(int), 1, fin);
		attr[i].data = new float[1 << (attr[i].slotNum << 2)];
		fread(&(*attr[i].data), sizeof(float), 1 << (attr[i].slotNum << 2), fin);
	}
	fclose(fin);
	return true;
}
// write out attribute data
bool save(const char* filename, vector<Attr> &attr) {
	FILE* fout = fopen(filename, "wb");
	if (!fout){
		return false;
	}
	int attrN = attr.size();
	fwrite(&attrN, sizeof(int), 1, fout);
	for (int i = 0; i<attrN; i++){
		fwrite(&attr[i].slotNum, sizeof(int), 1, fout);
		fwrite(&attr[i].position, sizeof(int), 1, fout);
		fwrite(&(*attr[i].data), sizeof(float), 1 << (attr[i].slotNum << 2), fout);
		delete[] attr[i].data;
	}
	fclose(fout);
	attr.clear();
	return true;
}

void deepCopy(vector<Attr> &dst, vector<Attr> &src) {
	dst.clear();
	dst.resize(src.size());
	for (int i = 0; i<(int)src.size(); i++){
		dst[i].slotNum = src[i].slotNum;
		dst[i].position = src[i].position;
		dst[i].data = new float[1 << (src[i].slotNum << 2)];
		memcpy(dst[i].data, src[i].data, sizeof(float)*(1 << (src[i].slotNum << 2)));
	}
}

void attrDestroy(vector<Attr> &attr) {
	for (int i = 0; i<(int)attr.size(); i++){
		delete[] attr[i].data;
	}
	attr.clear();
}

// initialize random seed
__forceinline__ __global__ void initrand(curandState_t *randState, int *randSeed_d)
{
	int blockID = blockIdx.x;
	curand_init(randSeed_d[blockID], 0, 0, &randState[blockID]);
}

/*
 * high      low bit
 *  0  1  2  3
 *  4  5  6  7
 *  8  9  10 11
 *  12 13 14 15
 */

// get a single cell in a board
__forceinline__ __device__ int getCell_d(int *board_ds, int x, int y)
{
	return (board_ds[x] >> ((3 - y) << 2)) & 0xf;
}
// set a single cell in a board
__forceinline__ __device__ void setCell_d(int *board_ds, int x, int y, int val)
{
	board_ds[x] = (board_ds[x] & (0xffff ^ (0xf << ((3 - y) << 2)))) | (val << ((3 - y) << 2));
}

// generate a random number ( should be called only when there is at least one empty cell )
__forceinline__ __device__ void genCell_d(int *board_ds, curandState_t *randState)
{
	unsigned long long pos = 0;
	int cnt = 0;
	for (unsigned long long i = 0; i<16; i++)		// collect all empty positions
		if (!getCell_d(board_ds, (int)(i >> 2), (int)(i & 0x3)))
			pos |= i << (cnt++ << 2);
	if (cnt != 0){
		int num = ((curand(randState) & 0x7fffffff) % 10 == 0) ? 2 : 1; //2:4 = 9:1
		int tar = (curand(randState) & 0x7fffffff) % cnt;
		tar = (pos >> (tar << 2)) & 0xf;
		setCell_d(board_ds, tar >> 2, tar & 0x3, num);
	}
}

// push a row from right to left, which is higher bit
__forceinline__ __device__ void pushLeft_d(int &n) {
	if ((n & 0xf0) == 0)
		n = (n & 0xff00) | ((n & 0xf) << 4);
	if ((n & 0xf00) == 0)
		n = (n & 0xf000) | ((n & 0xff) << 4);
	if ((n & 0xf000) == 0)
		n <<= 4;
}
// push a board from right to left and return the moving score
__forceinline__ __device__ int left_d(int &board_ds) {
	int ori = board_ds;
	int scoreSum = 0;
	pushLeft_d(board_ds);
	if ((board_ds & 0xf000) && ((board_ds ^ (board_ds << 4)) & 0xf000) == 0){
		scoreSum += (2 << ((board_ds & 0xf000) >> 12));
		board_ds = (board_ds & 0xf0ff) + 0x1000;
	}
	if ((board_ds & 0xf00) && ((board_ds ^ (board_ds << 4)) & 0xf00) == 0){
		scoreSum += (2 << ((board_ds & 0xf00) >> 8));
		board_ds = (board_ds & 0xff0f) + 0x100;
	}
	if ((board_ds & 0xf0) && ((board_ds ^ (board_ds << 4)) & 0xf0) == 0){
		scoreSum += (2 << ((board_ds & 0xf0) >> 4));
		board_ds = (board_ds & 0xfff0) + 0x10;
	}
	pushLeft_d(board_ds);
	if (ori == board_ds)
		return -1;
	else
		return scoreSum;
}

// make a row flip horizontally
__forceinline__ __device__ void mirrorLR_d(int &n)
{
	n = ((n & 0xf) << 12) | ((n & 0xf0) << 4) | ((n & 0xf00) >> 4) | (n >> 12);
}
// make a row flip diagonally. the complicate assignment is to avoid bank comflict, which may not be necessary to that complicate.
__forceinline__ __device__ void diagonal_d(int *board_ds, int attrID)
{
	int tmp = 0;
	tmp |= ((board_ds[(attrID + 0) & 0x3] >> (attrID << 2)) & 0xf) << (((attrID + 0) & 0x3) << 2);
	tmp |= ((board_ds[(attrID + 1) & 0x3] >> (attrID << 2)) & 0xf) << (((attrID + 1) & 0x3) << 2);
	tmp |= ((board_ds[(attrID + 2) & 0x3] >> (attrID << 2)) & 0xf) << (((attrID + 2) & 0x3) << 2);
	tmp |= ((board_ds[(attrID + 3) & 0x3] >> (attrID << 2)) & 0xf) << (((attrID + 3) & 0x3) << 2);
	board_ds[attrID] = tmp;
}
// make a row flip anti-diagonally. the complicate assignment is to avoid bank comflict, which may not be necessary to that complicate.
__forceinline__ __device__ void adiagonal_d(int *board_ds, int attrID)
{
	int tmp = 0;
	tmp |= ((board_ds[(attrID + 0) & 0x3] >> (attrID << 2)) & 0xf) << (((7 - attrID - 0) & 0x3) << 2);
	tmp |= ((board_ds[(attrID + 1) & 0x3] >> (attrID << 2)) & 0xf) << (((7 - attrID - 1) & 0x3) << 2);
	tmp |= ((board_ds[(attrID + 2) & 0x3] >> (attrID << 2)) & 0xf) << (((7 - attrID - 2) & 0x3) << 2);
	tmp |= ((board_ds[(attrID + 3) & 0x3] >> (attrID << 2)) & 0xf) << (((7 - attrID - 3) & 0x3) << 2);
	board_ds[3 - attrID] = tmp;
}

// get the score of a board from attribute
__forceinline__ __device__ void getScore_d(float *attr_d, int *attrPosition_ds, float *getScore_ds, int *board_ds, int attrID)
{
	int id = 0;
	for (int j = 0; j<attrSlotNum; j++){
		int pos = (attrPosition_ds[attrID] >> (j << 2)) & 0xf;
		id |= getCell_d(board_ds, pos >> 2, pos & 0x3) << (j << 2);
	}
	getScore_ds[attrID] = attr_d[attrID*(1 << attrDataSize) + id];
}
// update the score of a board
__forceinline__ __device__ void updateAttr_d(float *attr_d, int *attrPosition_ds, int *board_ds, int attrID, float val)
{
	int id = 0;
	for (int j = 0; j<attrSlotNum; j++){
		int pos = (attrPosition_ds[attrID] >> (j << 2)) & 0xf;
		id |= getCell_d(board_ds, pos >> 2, pos & 0x3) << (j << 2);
	}
	atomicAdd(&attr_d[attrID*(1 << attrDataSize) + id], val);
}

// the main loop of learning
__global__ void run_d(float *attr_d, int *attrPosition_d, record_d *rec_d, curandState_t *randState, float learnSpeed, int *learnCnt_d, int *recProgress_d)
{
	bool learn = learnSpeed > 1e-6;
	__shared__ int board_ds[8][4][4];	// 8 way, 4 direction, 4 row
	__shared__ int earnScore_ds[4][4];	// 4 direction, 4 row
	__shared__ float getScore_ds[8][4][attrNumLimit];	// 8 way, 4 direction, 4 attr
	__shared__ int attrPosition_ds[attrNumLimit];	// 4 attr
	__shared__ curandState_t randState_ds;
	__shared__ int tar_ds;
	__shared__ int activeID;
	// initialization
	int attrID = threadIdx.x % 4 + (threadIdx.x / 4 / 4 / 8) * 4;
	int directID = threadIdx.x / 4 % 4;
	int wayID = threadIdx.x / 4 / 4 % 8;
	if (attrID < 4) {
		board_ds[wayID][directID][attrID] = 0;
	}
	if (wayID == 0 && directID == 0)
		attrPosition_ds[attrID] = attrPosition_d[attrID];
	__syncthreads();
	int score = 0;
	int step = 1;
	if (threadIdx.x == 0){
		int oldcnt = atomicAdd(&(learnCnt_d[1]), 1);
		activeID = oldcnt%branchSize;
		randState_ds = randState[oldcnt % 1000];
		genCell_d(board_ds[0][0], &randState_ds);
		genCell_d(board_ds[0][0], &randState_ds);
	}
	__syncthreads();
	do{
		// build 4 direction once
		if (wayID == 0 && attrID<4){
			if (directID>0){
				board_ds[0][directID][attrID] = board_ds[0][0][attrID];
			}
			// up; down; left; right;
			if (directID == 0){
				adiagonal_d(board_ds[0][0], attrID);
				diagonal_d(board_ds[0][1], attrID);
				mirrorLR_d(board_ds[0][3][attrID]);
			}
			// push all four boards to left and get their score
			earnScore_ds[directID][attrID] = left_d(board_ds[0][directID][attrID]);
			if (attrID == 0){	// checking if the move is legal
				int unchange = 0;
				int tmpsum = 0;
				for (int i = 0; i<4; i++){
					if (earnScore_ds[directID][i] == -1){
						unchange++;
					}
					else{
						tmpsum += earnScore_ds[directID][i];
					}
				}
				earnScore_ds[directID][0] = unchange == 4 ? -1 : tmpsum;
			}
			if (directID == 0){
				adiagonal_d(board_ds[0][0], attrID);
				diagonal_d(board_ds[0][1], attrID);
				mirrorLR_d(board_ds[0][3][attrID]);
			}
		}
		__syncthreads();
		// copy for 8 way
		if (wayID>0 && attrID<4){
			board_ds[wayID][directID][attrID] = board_ds[0][directID][attrID];
		}
		__syncthreads();
		// flip 8 to different directions
		if (attrID<4){
			if (wayID == 1){
				mirrorLR_d(board_ds[wayID][directID][attrID]);
			}
			else if (wayID == 2){
				board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
			}
			else if (wayID == 3){
				mirrorLR_d(board_ds[wayID][directID][attrID]);
				board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
			}
			else if (wayID == 4){
				diagonal_d(board_ds[wayID][directID], attrID);
			}
			else if (wayID == 5){
				diagonal_d(board_ds[wayID][directID], attrID);
				mirrorLR_d(board_ds[wayID][directID][attrID]);
			}
			else if (wayID == 6){
				diagonal_d(board_ds[wayID][directID], attrID);
				board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
			}
			else if (wayID == 7){
				diagonal_d(board_ds[wayID][directID], attrID);
				mirrorLR_d(board_ds[wayID][directID][attrID]);
				board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
			}
		}
		__syncthreads();
		// calculate getscore
		if (earnScore_ds[directID][0] != -1){
			getScore_d(attr_d, attrPosition_ds, getScore_ds[wayID][directID], board_ds[wayID][directID], attrID);
			if ((wayID & 0x1) == 0 && (attrID & 0x3) == 0){
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID][directID][attrID + 1];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID][directID][attrID + 2];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID][directID][attrID + 3];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 0];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 1];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 2];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 3];
			}
		}
		__syncthreads();
		if (earnScore_ds[directID][0] != -1){
			if (wayID == 0 && attrID == 0){
				float tmpScore = 0;
				for (int i = 0; i<8; i += 2){
					for (int j = 0; j<attrNum; j += 4){
						tmpScore += getScore_ds[i][directID][j];
					}
				}
				getScore_ds[0][directID][0] = tmpScore + earnScore_ds[directID][0];
			}
		}
		__syncthreads();
		// choose the best move
		if (wayID == 0 && directID == 0 && attrID == 0){
			tar_ds = -1;
			float tarScore = -100;
			for (int i = 0; i<4; i++){
				if (earnScore_ds[i][0] != -1){
					if (tar_ds == -1 || getScore_ds[0][i][0]>tarScore){
						tar_ds = i;
						tarScore = getScore_ds[0][i][0];
					}
				}
			}
		}
		__syncthreads();
		if (tar_ds == -1) break;		// break out the main loop if die
		if (wayID == 0 && directID == 0 && attrID<4){		// record the game states
			if (learn) {
				rec_d[activeID*recordSize + step - 1].s[1][attrID] = board_ds[0][tar_ds][attrID];
				if (attrID == 0){
					rec_d[activeID*recordSize + step - 1].earned = earnScore_ds[tar_ds][0];
				}
				rec_d[activeID*recordSize + step++].s[0][attrID] = board_ds[0][tar_ds][attrID];
			}
			board_ds[0][0][attrID] = board_ds[0][tar_ds][attrID];
		}
		else{
			step++;
		}
		__syncthreads();
		if (threadIdx.x == 0){
			score += earnScore_ds[tar_ds][0];
			genCell_d(board_ds[0][0], &randState_ds);
		}
		__syncthreads();
	} while (1);
	if (learn) {
		float dif;
		// update the last state
		// copy for 8 way
		if (directID == 0 && attrID<4){
			board_ds[wayID][directID][attrID] = rec_d[activeID*recordSize + step - 2].s[1][attrID];
		}
		__syncthreads();
		if (directID == 0 && attrID<4){
			if (wayID == 1){
				mirrorLR_d(board_ds[wayID][directID][attrID]);
			}
			else if (wayID == 2){
				board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
			}
			else if (wayID == 3){
				mirrorLR_d(board_ds[wayID][directID][attrID]);
				board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
			}
			else if (wayID == 4){
				diagonal_d(board_ds[wayID][directID], attrID);
			}
			else if (wayID == 5){
				diagonal_d(board_ds[wayID][directID], attrID);
				mirrorLR_d(board_ds[wayID][directID][attrID]);
			}
			else if (wayID == 6){
				diagonal_d(board_ds[wayID][directID], attrID);
				board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
			}
			else if (wayID == 7){
				diagonal_d(board_ds[wayID][directID], attrID);
				mirrorLR_d(board_ds[wayID][directID][attrID]);
				board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
			}
		}
		__syncthreads();
		// calculate getscore
		if (directID == 0){
			getScore_d(attr_d, attrPosition_ds, getScore_ds[wayID][directID], board_ds[wayID][directID], attrID);
			if ((wayID & 0x1) == 0 && (attrID & 0x3) == 0){
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID][directID][attrID + 1];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID][directID][attrID + 2];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID][directID][attrID + 3];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 0];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 1];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 2];
				getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 3];
			}
		}
		__syncthreads();
		if (directID == 0){
			if (wayID == 0 && attrID == 0){		// could be parallel for wayID
				float tmpScore = 0;
				for (int i = 0; i<8; i += 2){
					for (int j = 0; j<attrNum; j += 4){
						tmpScore += getScore_ds[i][directID][j];
					}
				}
				getScore_ds[0][directID][0] = tmpScore;
			}
		}
		__syncthreads();
		if (directID == 0){
			dif = 0 - getScore_ds[0][0][0];
			updateAttr_d(attr_d, attrPosition_ds, board_ds[wayID][directID], attrID, dif*learnSpeed);
		}
		__syncthreads();
		// update all the state from the end of game to the beginning
		for (int i = step - 2; i>0; i--){
			float s[2];
			// copy for 8 way
			if (directID<2 && attrID<4){
				board_ds[wayID][directID][attrID] = rec_d[activeID*recordSize + i].s[directID][attrID];
			}
			__syncthreads();
			if (directID<2 && attrID<4){
				if (wayID == 1){
					mirrorLR_d(board_ds[wayID][directID][attrID]);
				}
				else if (wayID == 2){
					board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
				}
				else if (wayID == 3){
					mirrorLR_d(board_ds[wayID][directID][attrID]);
					board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
				}
				else if (wayID == 4){
					diagonal_d(board_ds[wayID][directID], attrID);
				}
				else if (wayID == 5){
					diagonal_d(board_ds[wayID][directID], attrID);
					mirrorLR_d(board_ds[wayID][directID][attrID]);
				}
				else if (wayID == 6){
					diagonal_d(board_ds[wayID][directID], attrID);
					board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
				}
				else if (wayID == 7){
					diagonal_d(board_ds[wayID][directID], attrID);
					mirrorLR_d(board_ds[wayID][directID][attrID]);
					board_ds[wayID][directID][attrID] = board_ds[wayID][directID][3 - attrID];
				}
			}
			__syncthreads();
			// calculate getscore
			if (directID<2){
				getScore_d(attr_d, attrPosition_ds, getScore_ds[wayID][directID], board_ds[wayID][directID], attrID);
				if ((wayID & 0x1) == 0 && (attrID & 0x3) == 0){
					getScore_ds[wayID][directID][attrID] += getScore_ds[wayID][directID][attrID + 1];
					getScore_ds[wayID][directID][attrID] += getScore_ds[wayID][directID][attrID + 2];
					getScore_ds[wayID][directID][attrID] += getScore_ds[wayID][directID][attrID + 3];
					getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 0];
					getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 1];
					getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 2];
					getScore_ds[wayID][directID][attrID] += getScore_ds[wayID + 1][directID][attrID + 3];
				}
			}
			__syncthreads();
			if (directID<2){
				if (wayID == 0 && attrID == 0){		// could be parallel for wayID
					float tmpScore = 0;
					for (int i = 0; i<8; i += 2){
						for (int j = 0; j<attrNum; j += 4){
							tmpScore += getScore_ds[i][directID][j];
						}
					}
					getScore_ds[0][directID][0] = tmpScore;
				}
			}
			__syncthreads();
			if (directID == 0){
				s[0] = getScore_ds[0][0][0];
				s[1] = getScore_ds[0][1][0];
				dif = s[1] + rec_d[activeID*recordSize + i].earned - s[0];
				updateAttr_d(attr_d, attrPosition_ds, board_ds[wayID][directID], attrID, dif*learnSpeed);
			}
			__syncthreads();
		}
	}
	if (threadIdx.x == 0){
		// restore the random seed
		int oldcnt = atomicAdd(&(learnCnt_d[2]), 1);
		randState[oldcnt % 1000] = randState_ds;
		// record the learning result
		oldcnt = atomicAdd(&(learnCnt_d[0]), 1);
		atomicAdd(&recProgress_d[oldcnt / 1000 * 3], score);
		atomicMax(&recProgress_d[oldcnt / 1000 * 3 + 1], score);
		atomicAdd(&recProgress_d[oldcnt / 1000 * 3 + 2], step);
	}
}

double run(vector<Attr> &attr, int times, float learnSpeed = 0, int *learnCnt_d = NULL, int *recProgress_d = NULL){
	/*
	bool learn = learnSpeed > 1e-6;
	MoveFunc moveArr[4];
	moveArr[0]=&board::up;
	moveArr[1]=&board::down;
	moveArr[2]=&board::left;
	moveArr[3]=&board::right;
	int maxscore=0, maxstep=0;
	int goal=0;
	*/
	double acc = 0;

	float *attr_d;
	int *attrPosition_d;
	curandState_t *randState;
	int *randSeed_d;
	record_d *rec_d;

	cudaMalloc((void**)&attr_d, (1 << attrDataSize)*attr.size()*sizeof(float));
	cudaMalloc((void**)&attrPosition_d, attr.size()*sizeof(int));
	cudaMalloc((void**)&randState, branchSizeLimit*sizeof(curandState_t));
	cudaMalloc((void**)&randSeed_d, branchSizeLimit*sizeof(int));
	cudaMalloc((void**)&rec_d, branchSize*recordSize*sizeof(record_d));

	int randSeed_h[1000];
	srand(time(NULL));
	for (int i = 0; i<branchSizeLimit; i++){
		randSeed_h[i] = rand();
	}
	cudaMemcpy(randSeed_d, randSeed_h, branchSizeLimit*sizeof(int), cudaMemcpyHostToDevice);
	initrand << <branchSizeLimit, 1 >> >(randState, randSeed_d);

	for (int i = 0; i<attr.size(); i++){
		cudaMemcpy(attr_d + i*(1 << attrDataSize), attr[i].data, (1 << attrDataSize)*sizeof(float), cudaMemcpyHostToDevice);
		cudaMemcpy(attrPosition_d + i, &attr[i].position, sizeof(int), cudaMemcpyHostToDevice);
	}
	// in compute capability 2.0, the grid X dimension has to be lower than 65535
	dim3 gridsize(10000, times / 10000);
	run_d << <gridsize, (8 * 4)*attr.size() >> >(attr_d, attrPosition_d, rec_d, randState, learnSpeed, learnCnt_d, recProgress_d);
	for (int i = 0; i<attr.size(); i++){
		cudaMemcpy(attr[i].data, attr_d + i*(1 << attrDataSize), (1 << attrDataSize)*sizeof(float), cudaMemcpyDeviceToHost);
	}

	cudaFree(attr_d);
	cudaFree(attrPosition_d);
	cudaFree(randState);
	cudaFree(randSeed_d);
	cudaFree(rec_d);
	return acc / times;
}

int main(int argc, char* argv[]) {
	genMap();
	vector<Attr> attr;
	int learnTimes = 10000;
	double learnSpeed = 0.01;
	char in[2048], out[2048];
	if (argc >= 3) {
		strcpy(in, argv[1]);
		strcpy(out, argv[2]);
		if (argc >= 4)
			learnTimes = atoi(argv[3]);
		if (argc >= 5)
			learnSpeed = atof(argv[4]);
	}
	else {
		fprintf(stderr, "error: %s <input> <output> <learnTimes> <learnSpeed>\n", argv[0]);
		return 1;
	}
	srand(time(NULL));
	pair<clock_t, clock_t> real;
	real.first = clock();
	if (!load(in, attr)) {
		fprintf(stderr, "file open failed.\n");
		return 1;
	}
	cudaDeviceProp DeviceProp;
	cudaGetDeviceProperties(&DeviceProp, 0);
	double freq = DeviceProp.clockRate;
	freq *= 1000;
	int learnCnt_h[3] = { 0, 0, 0 };
	int *learnCnt_d;
	int *recProgress_d;
	for (int i = 0; i<10005; i++){
		recProgress_h[i][0] = 0,
			recProgress_h[i][1] = 0;
		recProgress_h[i][2] = 0;
	}
	cudaMalloc((void**)&learnCnt_d, sizeof(int));
	cudaMalloc((void**)&recProgress_d, 10005 * 3 * sizeof(int));
	cudaMemcpy(learnCnt_d, &learnCnt_h, 3 * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(recProgress_d, recProgress_h, 10005 * 3 * sizeof(int), cudaMemcpyHostToDevice);
	pair<clock_t, clock_t> run_t;
	run_t.first = clock();
	run(attr, learnTimes, learnSpeed, learnCnt_d, recProgress_d);
	run_t.second = clock();
	cudaMemcpy(recProgress_h, recProgress_d, 10005 * 3 * sizeof(int), cudaMemcpyDeviceToHost);
	FILE *recfile = fopen("rec.csv", "w");
	long long sumRec2 = 0;
	for (int i = 0; i<learnTimes / 1000; i++){
		fprintf(recfile, "%10d,%10.3f,%10d,%10d\n", (i + 1) * 1000, (double)recProgress_h[i][0] / 1000, recProgress_h[i][1], recProgress_h[i][2]);
		sumRec2 += recProgress_h[i][2];
	}
	printf("Speed: %.2f steps/s\n", 1.*sumRec2 / (run_t.second - run_t.first) *CLOCKS_PER_SEC);
	printf("Training time: %.3f s\n", 1.*(run_t.second - run_t.first) / CLOCKS_PER_SEC);
	fclose(recfile);
	cudaFree(learnCnt_d);
	cudaFree(recProgress_d);
	if (!save(out, attr)) {
		fprintf(stderr, "file open failed.\n");
		return 1;
	}
	real.second = clock();
	printf("Real time: %.3f s\n", 1.*(real.second - real.first) / CLOCKS_PER_SEC);
	return 0;

}
