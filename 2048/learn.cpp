#include <cstdio>
#include <cstring>
#include "board.hpp"
#include "attr.hpp"

#define learnTimes 20000
#define learnSpeed (0.02)

typedef int (board::*move)(bool);

int attrN;
Attr attr[10];

struct record
{
	board s1,s2;
	int earned;
};

record rec[1000];

bool load(const char* filename, Attr attr[], int& attrN) {
	FILE* fin = fopen(filename, "rb");
	if(!fin){
		return false;
	}
	fscanf(fin,"%d",&attrN);
	for(int i=0; i<attrN; i++){
		fscanf(fin,"%d",&attr[i].slotNum);
		fscanf(fin,"%d",&attr[i].position);
		for(int j=0; j<(1<<(attr[i].slotNum<<2)); j++){
			fscanf(fin,"%f",&attr[i].data[j]);
		}
	}
	return true;
}
bool save(const char* filename, Attr attr[], int& attrN) {
	FILE* fout = fopen(filename, "wb");
	if(!fout){
		return false;
	}
	fscanf(fout,"%d",&attrN);
	for(int i=0; i<attrN; i++){
		fprintf(fout,"%d\n",attr[i].slotNum);
		fprintf(fout,"%d\n",attr[i].position);
		for(int j=0; j<(1<<(attr[i].slotNum<<2)); j++){
			fprintf(fout,"%f\n",attr[i].data[j]);
		}
	}
	return true;
}

int main(int argc, char* argv[]) {
	char in[2048], out[2048];
	if( argc==3 ) {
		strcpy(in, argv[1]);
		strcpy(out, argv[2]);
	} else {
		return 1;
	}

	if( !load(in, attr, attrN) ) {
		printf("file open failed.\n");
		return 1;
	}
	srand(time(NULL));
	genMap();
	move moveArr[4];
	moveArr[0]=&board::up;
	moveArr[1]=&board::down;
	moveArr[2]=&board::left;
	moveArr[3]=&board::right;
	float acc=0;
	for(int T=0; T<learnTimes; T++){
		board b;
		b.init();
		int score=0;
		int step=0;
		do{
			rec[step].s1=b;
			board newb[4];
			int earnScore[4];
			for(int i=0; i<4; i++){
				newb[i]=b;
				earnScore[i]=(newb[i].*moveArr[i])(false);
			}
			int tar=-1;
			int tmpScore=-1;
			for(int i=0; i<4; i++){
				if(earnScore[i]!=-1){
					float tmp=earnScore[i]+getScore(newb[i],attr,attrN);
					if(tmpScore<tmp){
						tar=i;
						tmpScore=tmp;
					}
				}
			}
			if(tar==-1){
				//printf("%d\n",step);
				break;
			}else{
				score+=earnScore[tar];
				b=newb[tar];
				rec[step].s2=b;
				rec[step++].earned=earnScore[tar];
			}
			if(step==1000){
				printf("play too long\n");
				b.print();
				break;
			}
			b.genCell();
		}while(1);
		for(int i=step-1; i>=0; i--){
			float dif=getScore(rec[i].s2, attr, attrN)+rec[i].earned-getScore(rec[i].s1, attr, attrN);
			updateAttr(rec[i].s1, attr, attrN, dif*learnSpeed);
		}
		acc+=score;
		if(T%100==99){
			printf("times: %d\tscore: %f\n",T,acc/100);
			acc=0;
		}
	}
	if( !save(out, attr, attrN) ) {
		printf("file open failed.\n");
		return 1;
	}
	return 0;

}
