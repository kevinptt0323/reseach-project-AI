#ifdef _WIN32
#define _hypot hypot
#endif

#include "board.hpp"
#include "attr.hpp"
#include <cstdio>
#include <cstdlib>
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
		int earnScore[4], tmpScore, tar=-1;
		bool die=true;
		for(int i=0; i<4; i++){
			newb[i]=b;
			earnScore[i]=(newb[i].*moveArr[i])(false);
			if(earnScore[i]!=-1){
				die=false;
				if(tar==-1){
					tar=i;
					tmpScore=earnScore[i]+getScore(newb[i],attr);
				}
			}
		}
		if(die)
			break;
		for(int i=tar; i<4; i++){
			if(earnScore[i]!=-1){
				float tmp=earnScore[i]+getScore(newb[i],attr);
				if(tmpScore<tmp){
					tar=i;
					tmpScore=tmp;
				}
			}
		}
		earned+=earnScore[tar];
		b=newb[tar];
	}
	return earned;
}

int main(int argc, char* argv[]) {
	genMap();
	board b;
	{
		random_device rd;
		minstd_rand RNG(rd());
		uniform_int_distribution<int> uid(0, 32768);
		srand(uid(RNG));
	}
	b.init();
	b.print();
	int sum=0;
	if( argc==2 ) {
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
		int score=0;
		do{
			board newb[4];
			int earnScore[4], tmpScore, tar=-1;
			bool die=true;
			for(int i=0; i<4; i++){
				newb[i]=b;
				earnScore[i]=(newb[i].*moveArr[i])(false);
				if(earnScore[i]!=-1){
					die=false;
					if(tar==-1){
						tar=i;
						tmpScore=earnScore[i]+getScore(newb[i],attr);
					}
				}
			}
			if(die)
				break;
			for(int i=tar; i<4; i++){
				if(earnScore[i]!=-1){
					float tmp=earnScore[i]+walk(newb[i], attr, 0); // should at least one
					tmp+=getScore(newb[i],attr);
					if(tmpScore<tmp){
						tar=i;
						tmpScore=tmp;
					}
				}
			}
			score+=(b.*moveArr[tar])(false);
			b.genCell();
		}while(1);
		b.print();
		printf("%d\n",score);
	} else {
		char str[10];
		while( ~scanf("%s", str) ) {
			switch(str[0]) {
				case 'a': printf("total score: %d\n",sum+=b.left(true)); break;
				case 'd': printf("total score: %d\n",sum+=b.right(true)); break;
				case 'w': printf("total score: %d\n",sum+=b.up(true)); break;
				case 's': printf("total score: %d\n",sum+=b.down(true)); break;
			}
			b.print();
			puts("");
		}
	}
	return 0;
}
