#ifndef _BOARD_KEVINPTT_HPP_
#define _BOARD_KEVINPTT_HPP_

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#define INF 0x71227122
using namespace std;

typedef unsigned long long ull;

int leftMap[65536];
int rightMap[65536];
int mirrorMap[65536];	// 1234 -> 4321
int SmirrorMap[65536];	// 1234 -> 2143
int moveLeftScore[65536];
int moveRightScore[65536];

int pushLeft(int n) {
	if ((n&0xf0)==0)
		n = (n&0xff00) | ((n&0xf)<<4);
	if ((n&0xf00)==0)
		n = (n&0xf000) | ((n&0xff)<<4);
	if ((n&0xf000)==0)
		n <<= 4;
	return n;
}

void genMap() {
	for(int i=0; i<65536; ++i)
		mirrorMap[i] = ((i&0xf)<<12) | ((i&0xf0)<<4) | ((i&0xf00)>>4) | (i>>12);
	for(int i=0; i<65536; ++i)
		SmirrorMap[i] = ((i&0xf)<<4) | ((i&0xf0)>>4) | ((i&0xf00)<<4) | ((i&0xf000)>>4);
	for(int i=65535, i2; i; --i) {
		int scoreSum=0;
		i2 = pushLeft(i);
		if ( (i2&0xf000) && ((i2^(i2<<4))&0xf000) == 0){
			scoreSum += (2<<((i2&0xf000)>>12));
			i2 = (i2&0xf0ff) + 0x1000;
		}
		if ( (i2& 0xf00) && ((i2^(i2<<4))& 0xf00) == 0){
			scoreSum += (2<<((i2&0xf00)>>8));
			i2 = (i2&0xff0f) + 0x100;
		}
		if ( (i2&  0xf0) && ((i2^(i2<<4))&  0xf0) == 0){
			scoreSum += (2<<((i2&0xf0)>>4));
			i2 = (i2&0xfff0) + 0x10;
		}
		i2 = pushLeft(i2);
		moveLeftScore[i] = scoreSum;
		leftMap[i] = i2;
	}
	for(int i=65535; i; --i){
		moveRightScore[mirrorMap[i]] = moveLeftScore[i];
		rightMap[mirrorMap[i]] = mirrorMap[leftMap[i]];
	}
}

class board {
private:
	int row[4];
public:
	board() {
		row[0] = row[1] = row[2] = row[3] = 0;
	}
	~board() {
	}
	void init() {
		row[0] = row[1] = row[2] = row[3] = 0;
		genCell();
		genCell();
	}
	int left(bool next=false) {
		int ret=0;
		bool change = false;
		for(int i=0; i<4; ++i) {
			if (row[i] != leftMap[row[i]]) {
				change = true;
				ret+=moveLeftScore[row[i]];
				row[i] = leftMap[row[i]];
			}
		}
		if (next && change)
			genCell();
		if(!change)
			return -INF;
		else
			return ret;
	}
	int right(bool next=false) {
		int ret=0;
		bool change = false;
		for(int i=0; i<4; ++i) {
			if (row[i] != rightMap[row[i]]) {
				change = true;
				ret+=moveRightScore[row[i]];
				row[i] = rightMap[row[i]];
			}
		}
		if (next && change)
			genCell();
		if(!change)
			return -INF;
		else
			return ret;
	}
	int up(bool next=false) {
		int ret;
		trans();
		ret=left(next);
		trans();
		return ret;
	}
	int down(bool next=false) {
		int ret;
		trans();
		ret=right(next);
		trans();
		return ret;
	}
	void clockwise()
	{
		trans();
		mirrorLR();
	}
	void C_clockwise()
	{
		mirrorLR();
		trans();
	}
	inline void mirrorLR() {		//left right mirror
		for(int i=0; i<4; ++i) {
			row[i] = mirrorMap[row[i]];
		}
	}
	inline void mirrorUD() {		//upside down mirror
		std::swap(row[0],row[3]);
		std::swap(row[1],row[2]);
	}
	inline void swap(int r1,int r2,int pos)
	{
		row[r2]^=row[r1]&pos;
		row[r1]^=row[r2]&pos;
		row[r2]^=row[r1]&pos;
	}
	void trans()
	{
		for(int i=2; i<4; i++)
			row[i]=mirrorMap[row[i]];
		swap(0,3,0xff);
		swap(1,2,0xff);
		for(int i=2; i<4; i++)
			row[i]=mirrorMap[row[i]];
		for(int i=1; i<4; i+=2)
			row[i]=SmirrorMap[row[i]];
		swap(0,1,0xff0);
		swap(2,3,0xf00f);
		for(int i=1; i<4; i+=2)
			row[i]=SmirrorMap[row[i]];
	}
	void print() const{
		puts("---------------------------------");
		for(int i=0; i<4; ++i) {
			printf("|");
			for(int j=0; j<4; ++j) {
				int k = getCell(i, j);
				if (k)
					printf(" %5d |", 1<<k);
				else
					printf("       |");
			}
			puts("");
		}
		puts("---------------------------------");
	}
	inline void genCell() {
		ull pos=0;
		int cnt=0;
		for(ull i=0; i<16; i++)
			if(!getCell(i>>2,i&0x3))
				pos|=i<<(cnt++<<2);
		if(cnt!=0){
			int num=(rand()%10==0)?2:1; //2:4 = 9:1
			int tar=rand()%cnt;
			tar=(pos>>(tar<<2))&0xf;
			setCell(tar>>2,tar&0x3,num);
		}
	}
	inline int getCell(int x, int y) const {
		return (row[x]>>((3-y)<<2)) & 0xf;
	}
	inline void setCell(int x, int y, int val) {
		row[x] = (row[x] & (0xffff^(0xf<<((3-y)<<2)))) | (val<<((3-y)<<2));
	}
	inline bool operator!=(const board& rhs) const
	{
		for(int i=0; i<4; i++)
			if(row[i]!=rhs.row[i])
				return true;
		return false;
	}
	inline bool operator==(const board& rhs) const
	{
		for(int i=0; i<4; i++)
			if(row[i]!=rhs.row[i])
				return false;
		return true;
	}
};

#endif
