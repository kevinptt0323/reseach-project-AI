#ifdef _WIN32
#define _hypot hypot
#endif

#include "board.hpp"
#include "attr.hpp"
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <vector>

using namespace std;


struct setting
{
	int ID,gen;
	int speed; // multple by 1000
};

struct data
{
	setting set;
	vector<Attr> attr;
	vector<int> score;
	double avg;
};

data arr[600];

vector<Attr> zeroAttr[100];

char str[100000];

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
	}
	fclose(fin);
	return true;
}



int main(int argc, char* argv[]) {
	char in[2048];
	vector<Attr> attr;
	for(int ID=1; ID<=60; ID++){
		sprintf(in,"zero%d.dat",ID);
		if( !load(in, zeroAttr[ID]) ) {
			fprintf(stderr, "file open failed.\n");
			return 1;
		}
	}
	FILE* res=fopen("result.csv","r");
	fgets(str,100000,res);
	char *ptr=strtok(str,",");
	for(int i=0; i<540; i++){
		sscanf(ptr,"LR%d-%d-%*d-0.%d.dat",&arr[i].set.ID,&arr[i].set.gen,&arr[i].set.speed);
		ptr=strtok(NULL,",");
		arr[i].avg=0;
	}
	for(int i=0; i<1000; i++){
		fgets(str,100000,res);
		ptr=strtok(str,",");
		for(int j=0; j<540; j++){
			int sco;
			sscanf(ptr,"%d",&sco);
			ptr=strtok(NULL,",");
			arr[j].score.push_back(sco);
			arr[j].avg+=sco;
		}
	}
	for(int i=0; i<540; i++){
		arr[i].avg/=1000;
	}
	return 0;
}
