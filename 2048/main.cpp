#ifdef _WIN32
#define _hypot hypot
#endif

#include "board.hpp"
#include "attr.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <vector>
#include <random>

using namespace std;

typedef int (board::*MoveFunc)(bool);

bool load(const char* filename, vector<Attr> &attr) {
	FILE* fin = fopen(filename, "rb");
	if(!fin){
		return false;
	}
	int attrN;
	fread(&attrN,sizeof(int),1,fin);
	attr.resize(attrN);
	for(int i=0; i<attrN; i++){
		fread(&attr[i].slotNum,sizeof(int),1,fin);
		fread(&attr[i].position,sizeof(int),1,fin);
		attr[i].data=new float[1<<(attr[i].slotNum<<2)];
		fread(&(*attr[i].data),sizeof(float),1<<(attr[i].slotNum<<2),fin);
	}
	fclose(fin);
	return true;
}

int walk(board &b, vector<Attr> &attr, int times)
{
	MoveFunc moveArr[4];
	moveArr[0]=&board::up;
	moveArr[1]=&board::down;
	moveArr[2]=&board::left;
	moveArr[3]=&board::right;
	int earned=0;
	for(int T=0; T<times; T++){
		if(T!=0)
			b.genCell();
		board newb[4];
		int earnScore[4], tar=-1;
		float tarScore, tmpScore;
		for(int i=0; i<4; i++){
			newb[i]=b;
			earnScore[i]=(newb[i].*moveArr[i])(false);
			if(earnScore[i]!=-INF){
				tmpScore = earnScore[i]+getScore(newb[i],attr);
				if(tar==-1 || tmpScore>tarScore){
					tar=i;
					tarScore = tmpScore;
				}
			}
		}
		if(tar==-1)
			break;
		earned+=earnScore[tar];
		b=newb[tar];
	}
	return earned;
}

int main(int argc, char* argv[]) {
	genMap();
	board b;
	{
#ifdef _WIN32
		srand(time(0));
#else
		random_device rd;
		minstd_rand RNG(rd());
		uniform_int_distribution<int> uid(0, 32768);
		srand(uid(RNG));
#endif
	}
	if( argc>=2 ) {
		char in[2048];
		strcpy(in, argv[1]);
		vector<Attr> attr;
		if( !load(in, attr) ) {
			fprintf(stderr, "file open failed.\n");
			return 1;
		}
		MoveFunc moveArr[4];
		moveArr[0]=&board::up;
		moveArr[1]=&board::down;
		moveArr[2]=&board::left;
		moveArr[3]=&board::right;
		int round = 1;
		if (argc==3) round = atoi(argv[2]);
		long long sum = 0;
		int maxScore = 0;
		for(int _=1; _<=round; _++) {
			b.init();
			if (round==1) b.print();
			int score = 0;
			do{
				board newb[4];
				int earnScore[4], tar=-1;
				float tarScore, tmpScore;
				for(int i=0; i<4; i++){
					newb[i]=b;
					earnScore[i]=(newb[i].*moveArr[i])(false);
					if(earnScore[i]!=-INF){
						tmpScore = earnScore[i]+getScore(newb[i],attr);
						if(tar==-1 || tmpScore>tarScore){
							tar=i;
							tarScore = tmpScore;
						}
					}
				}
				if(tar==-1)
					break;
				score+=(b.*moveArr[tar])(false);
				b.genCell();
			} while(1);
			sum += score;
			maxScore = max(maxScore, score);
			if (round==1) {
				b.print();
				printf("%d\n",score);
			}
		}
		printf("Max Score:\t%d\n", maxScore);
		printf("Average Score:\t%.2f\n", 1.*sum/round);
		for(int i=0; i<(int)attr.size(); i++) {
			delete[] attr[i].data;
		}
	} else {
		int score=0;
		b.init();
		b.print();
		char str[10];
		while( ~scanf("%s", str) ) {
			switch(str[0]) {
				case 'a': printf("total score: %d\n",score+=b.left(true)); break;
				case 'd': printf("total score: %d\n",score+=b.right(true)); break;
				case 'w': printf("total score: %d\n",score+=b.up(true)); break;
				case 's': printf("total score: %d\n",score+=b.down(true)); break;
			}
			b.print();
			puts("");
		}
	}
	return 0;
}
