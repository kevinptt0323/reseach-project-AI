#ifdef _WIN32
#define _hypot hypot
#endif

#include "board.hpp"
#include "attr.hpp"
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <vector>
#include <map>

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


struct statistic
{
	int tupleCnt[20];
	int gen[5];
	int learnSpeed[50];
};

data arr[600];

vector<Attr> zeroAttr[100];
map<int,int> mmap;
int tupleNum;
int position[20];

statistic states[5];
int division[5]={0,10000,20000,40000,10000000};

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

bool cmp(const data &a, const data &b)
{
	return a.avg<b.avg;
}

int main(int argc, char* argv[]) {
	char in[2048];
	vector<Attr> attr;
	tupleNum=1;
	for(int ID=1; ID<=60; ID++){
		sprintf(in,"zero%d.dat",ID);
		if( !load(in, zeroAttr[ID]) ) {
			fprintf(stderr, "file open failed.\n");
			return 1;
		}
		for(int j=0; j<(int)zeroAttr[ID].size(); j++){
			if(mmap.count(zeroAttr[ID][j].position)==0){
				position[tupleNum]=zeroAttr[ID][j].position;
				mmap[zeroAttr[ID][j].position]=tupleNum++;
			}
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
	sort(arr,arr+540,cmp);
	int divCnt[5]={0};
	for(int i=0; i<540; i++){
		int tar=-1,ID=arr[i].set.ID;
		for(int j=0; j<5; j++)
			if(arr[i].avg>division[j])
				tar=j;
		divCnt[tar]++;
		for(int j=0; j<(int)zeroAttr[ID].size(); j++)
			states[tar].tupleCnt[mmap[zeroAttr[ID][j].position]]++;
		states[tar].gen[arr[i].set.gen]++;
		states[tar].learnSpeed[arr[i].set.speed]++;
	}
	for(int i=0; i<5; i++)
		printf("%.2f ",1.0*divCnt[i]/540);
	puts("");
	int tupleSum[5];
	for(int i=1; i<tupleNum; i++){
		tupleSum[i]=0;
		for(int j=0; j<5; j++){
			tupleSum[i]+=states[j].tupleCnt[i];
		}
	}
	for(int i=0; i<5; i++){
		for(int j=1; j<tupleNum; j++)
			printf("%.2f ",1.0*states[i].tupleCnt[j]/tupleSum[j]);
		puts("");
		for(int j=1; j<=3; j++)
			printf("%d ",states[i].gen[j]);
		puts("");
		printf("%d %d %d\n",states[i].learnSpeed[5],states[i].learnSpeed[10],states[i].learnSpeed[40]);
		puts("");
	}
	for(int i=1; i<tupleNum; i++){
		int pos[10];
		int slotNum=0;
		for(int j=0; j<3 || (position[i]>>(j<<2)); j++)
			slotNum++;
		for(int j=0; j<slotNum; j++){
			pos[j]=(position[i]>>(j<<2))&0xf;
		}
		board b;
		memset(b.row,0,sizeof(b.row));
		for(int j=0; j<slotNum; j++){
			b.setCell(pos[j]>>2,pos[j]&0x3,j+1);
		}
		printf("tuple ID: %d\n",i);
		b.print();
	}
	return 0;
}
