#include <cstdio>
#include <cstring>
#include "board.hpp"
#include "attr.hpp"

#include <vector>

#include <time.h>

using namespace std;


typedef int (board::*MoveFunc)(bool);


class record {
public:
	record() {}
	record(board _s1, board _s2, int _earned):s1(_s1), s2(_s2), earned(_earned) {}
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

void deepCopy(vector<Attr> &dst, vector<Attr> &src) {
	dst.clear();
	dst.resize(src.size());
	for(int i=0; i<(int)src.size(); i++){
		dst[i].slotNum=src[i].slotNum;
		dst[i].position=src[i].position;
		dst[i].data=new float[1<<(src[i].slotNum<<2)];
		memcpy(dst[i].data,src[i].data,sizeof(float)*(1<<(src[i].slotNum<<2)));
	}
}

void attrDestroy(vector<Attr> &attr) {
	for(int i=0; i<(int)attr.size(); i++){
		delete[] attr[i].data;
	}
	attr.clear();
}

double run(vector<Attr> &attr, int times, float learnSpeed = 0){
	bool learn = learnSpeed > 1e-6;
	MoveFunc moveArr[4];
	moveArr[0]=&board::up;
	moveArr[1]=&board::down;
	moveArr[2]=&board::left;
	moveArr[3]=&board::right;
	double acc=0;
	int maxscore=0, maxstep=0;
	int goal=0;
	for(int T=0; T<times; T++){
		board b, b_pre;
		b.init();
		b_pre = b;
		int score=0;
		int step=1;
		rec.clear();
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
			if(tar==-1) break;
			if( learn ) {
				rec.emplace_back(b_pre, newb[tar], earnScore[tar]);
				++step;
				b_pre = newb[tar];
				//rec[step-1].s2=newb[tar];
				//rec[step-1].earned=earnScore[tar];
				//rec[step++].s1=newb[tar];
				//if(step==int(rec.size())){
				//	rec.resize(rec.size()<<1);
				//}
			}
			b=newb[tar];
			score+=earnScore[tar];
			b.genCell();
		}while(1);
		if( learn ) {
			for(int i=rec.size()-1; i>0; i--){
				float s1=getScore(rec[i].s1, attr);
				float s2=getScore(rec[i].s2, attr);
				if(i==rec.size()-1)
					s2=0;
				float dif=s2+rec[i].earned-s1;
				updateAttr(rec[i].s1, attr, dif*learnSpeed);
			}
		}
		acc+=score;
		if(maxscore<score)
			maxscore=score;
		/*
		if( learn ) {
			if(maxstep<step)
				maxstep=step;
			for(int i=0; i<16; i++){
				if(rec[step-2].s2.getCell(i>>2,i&3)>9){
					goal++;
					break;
				}
			}
		}*/
		if( learn && T%100 == 99 ){
			//printf("times: %d\tscore: %f\tmaxstep: %d\tmaxscore: %d\t%d\n",T+1,acc/100,maxstep,maxscore,goal);
			printf("%10d,%10.3f,%10d\n",T+1,acc/100,maxscore);
			goal=0;
			maxstep=0;
			maxscore=0;
			acc=0;
		}
	}
	return acc/times;
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
	srand(time(NULL));
	int ID;
	if(sscanf(in,"zero%d.dat",&ID)!=1){
		if( !load(in, attr) ) {
			fprintf(stderr, "file open failed.\n");
			return 1;
		}
		run(attr, learnTimes, learnSpeed);
		if( !save(out, attr) ) {
			fprintf(stderr,"file open failed.\n");
			return 1;
		}
	}else{
		if( !load(in, attr) ) {
			fprintf(stderr, "file open failed.\n");
			return 1;
		}
		for(int gen=1; gen<=1; gen++){	//generation
			if( 0 && gen!=1 )
				if( !load(in, attr) ) {
					fprintf(stderr, "file open failed.\n");
					return 1;
				}
			sprintf(out,"LR%d-%d-%dk-%6.4f.dat",ID,gen,learnTimes/1000,learnSpeed);
			run(attr, learnTimes, learnSpeed);
			if(gen==1)
				if( !save(out, attr) ) {
					fprintf(stderr,"file open failed.\n");
					return 1;
				}
			strcpy(in,out);
		}
	}
	return 0;

}
