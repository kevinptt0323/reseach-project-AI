#include <cstdio>
#include <cstring>
#include "board.hpp"
#include "attr.hpp"

#include <vector>

#include <time.h>

using namespace std;


typedef int (board::*MoveFunc)(bool);


struct record
{
	board s1,s2;
	int earned;
};

vector<record> rec;

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
bool save(const char* filename, vector<Attr> &attr) {
	FILE* fout = fopen(filename, "wb");
	if(!fout){
		return false;
	}
	int attrN=attr.size();
	fwrite(&attrN,sizeof(int),1,fout);
	for(int i=0; i<attrN; i++){
		fwrite(&attr[i].slotNum,sizeof(int),1,fout);
		fwrite(&attr[i].position,sizeof(int),1,fout);
		fwrite(&(*attr[i].data),sizeof(float),1<<(attr[i].slotNum<<2),fout);
		delete[] attr[i].data;
	}
	fclose(fout);
	attr.clear();
	return true;
}


int test(vector<Attr> &attr){
	MoveFunc moveArr[4];
	moveArr[0]=&board::up;
	moveArr[1]=&board::down;
	moveArr[2]=&board::left;
	moveArr[3]=&board::right;
	float acc=0;
	int maxscore=0,maxstep=0;
	int goal=0;
	for(int T=0; T<100; T++){
		board b;
		b.init();
		int score=0;
		int step=1;
		do{
			board newb[4];
			int earnScore[4];
			bool die=true;
			int tar=-1;
			int tmpScore;
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
			for(int i=tar+1; i<4; i++){
				if(earnScore[i]!=-1){
					float tmp=earnScore[i]+getScore(newb[i],attr);
					if(tmpScore<tmp){
						tar=i;
						tmpScore=tmp;
					}
				}
			}
			score+=earnScore[tar];
			b=newb[tar];
			b.genCell();
		}while(1);
		acc+=score;
		if(maxstep<step)
			maxstep=step;
		if(maxscore<score)
			maxscore=score;
		for(int i=0; i<16; i++){
			if(rec[step-2].s2.getCell(i>>2,i&3)>9){
				goal++;
				break;
			}
		}
		//printf("times: %d\tscore: %f\tmaxstep: %d\tmaxscore: %d\t%d\n",T+1,acc/100,maxstep,maxscore,goal);
	}
	return goal;
}


void learn(vector<Attr> &attr, int learnTimes, double learnSpeed){
	MoveFunc moveArr[4];
	moveArr[0]=&board::up;
	moveArr[1]=&board::down;
	moveArr[2]=&board::left;
	moveArr[3]=&board::right;
	rec=vector<record>(1000);
	float acc=0;
	int maxscore=0,maxstep=0;
	int goal=0;
	for(int T=0; T<learnTimes; T++){
		board b;
		b.init();
		int score=0;
		int step=1;
		do{
			board newb[4];
			int earnScore[4];
			bool die=true;
			int tar=-1;
			int tmpScore;
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
			for(int i=tar+1; i<4; i++){
				if(earnScore[i]!=-1){
					float tmp=earnScore[i]+getScore(newb[i],attr);
					if(tmpScore<tmp){
						tar=i;
						tmpScore=tmp;
					}
				}
			}
			score+=earnScore[tar];
			b=newb[tar];
			rec[step-1].s2=b;
			rec[step-1].earned=earnScore[tar];
			rec[step++].s1=b;
			if(step==int(rec.size())){
				rec.resize(rec.size()<<1);
			}
			b.genCell();
		}while(1);
		for(int i=step-2; i>0; i--){
			float s1=getScore(rec[i].s1, attr);
			float s2=getScore(rec[i].s2, attr);
			if(i==step-2)
				s2=-100;
			float dif=s2+rec[i].earned-s1;
			updateAttr(rec[i].s1, attr, dif*learnSpeed);
		}
		acc+=score;
		if(maxstep<step)
			maxstep=step;
		if(maxscore<score)
			maxscore=score;
		for(int i=0; i<16; i++){
			if(rec[step-2].s2.getCell(i>>2,i&3)>9){
				goal++;
				break;
			}
		}
		if(T%100==99){
			printf("times: %d\tscore: %f\tmaxstep: %d\tmaxscore: %d\t%d\n",T+1,acc/100,maxstep,maxscore,goal);
			goal=0;
			maxstep=0;
			maxscore=0;
			acc=0;
		}
	}
}

int main(int argc, char* argv[]) {
	genMap();
	vector<Attr> attr;
	int learnTimes = 10000;
	double learnSpeed = 0.01;
	char in[2048], out[2048];
	if( argc>=3 ) {
		strcpy(in, argv[1]);
		strcpy(out, argv[2]);
		if( argc>=4 )
			learnTimes = atoi(argv[3]);
		if( argc>=5 )
			learnSpeed = atof(argv[4]);
	} else {
		fprintf(stderr, "error: %s <input> <output> <learnTimes> <learnSpeed>\n", argv[0]);
		return 1;
	}
	if( !load(in, attr) ) {
		fprintf(stderr, "file open failed.\n");
		return 1;
	}
	srand(time(NULL));
	learn(attr, learnTimes, learnSpeed);
	if( !save(out, attr) ) {
		printf("file open failed.\n");
		return 1;
	}
	return 0;

}
