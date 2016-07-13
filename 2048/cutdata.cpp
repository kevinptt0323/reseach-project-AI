#include <cstdio>
#include <cstring>
#include "attr.hpp"

#include <vector>

#include <time.h>

using namespace std;



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
		//fwrite(&(*attr[i].data),sizeof(float),1<<(attr[i].slotNum<<2),fout);
		delete[] attr[i].data;
	}
	fclose(fout);
	attr.clear();
	return true;
}

int main(int argc, char* argv[]) {
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
	}else{
		if( !load(in, attr) ) {
			fprintf(stderr, "file open failed.\n");
			return 1;
		}

		if( !save(in, attr) ) {
			fprintf(stderr,"file open failed.\n");
			return 1;
		}
	}
	return 0;

}
