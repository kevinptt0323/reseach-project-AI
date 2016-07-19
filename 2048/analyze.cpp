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
double tupleAvg[20];

statistic states[5];
int division[5]={0,10000,20000,40000,10000000};

double pairAvg[20][20];
int pairAvgCnt[20][20];

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
	for(int i=0; i<tupleNum; i++)
		for(int j=0; j<tupleNum; j++){
			pairAvg[i][j]=0;
			pairAvgCnt[i][j]=0;
		}
	sort(arr,arr+540,cmp);
	
	for(int i=0; i<540; i++){
		for(int j=0; j<(int)zeroAttr[arr[i].set.ID].size(); j++){
			for(int k=j+1; k<(int)zeroAttr[arr[i].set.ID].size(); k++){
				int a,b;
				a=mmap[zeroAttr[arr[i].set.ID][j].position];
				b=mmap[zeroAttr[arr[i].set.ID][k].position];
				pairAvg[a][b]+=arr[i].avg;
				pairAvg[b][a]+=arr[i].avg;
				pairAvgCnt[a][b]++;
				pairAvgCnt[b][a]++;
			}
		}
		if(zeroAttr[arr[i].set.ID].size()==1){
			int a=mmap[zeroAttr[arr[i].set.ID][0].position];
			pairAvg[a][a]+=arr[i].avg;
			pairAvgCnt[a][a]++;
		}
		if(0 && arr[i].set.gen==3 && arr[i].set.speed==5){
			printf("%f\t",arr[i].avg);
			int tarID=arr[i].set.ID;
			for(int i=0; i<(int)zeroAttr[tarID].size(); i++){
				printf("%d ",mmap[zeroAttr[tarID][i].position]);
			}
			puts("");
		}
	}
	for(int i=1; i<tupleNum; i++){
		printf("%d\n",i);
		for(int j=1; j<tupleNum; j++){
			if(pairAvgCnt[i][j]==0)
				printf("%2d ------\t",j);
			else
				printf("%2d %6.0f\t",j,pairAvg[i][j]/pairAvgCnt[i][j]);
		}
		puts("");
	}
	int divCnt[5]={0};
	for(int i=0; i<tupleNum; i++){
		tupleAvg[i]=0;
	}
	for(int i=0; i<5; i++){
		for(int j=0; j<13; j++)
			states[i].tupleCnt[j]=0;
		for(int j=0; j<5; j++)
			states[i].gen[j]=0;
		for(int j=0; j<50; j++)
			states[i].learnSpeed[j]=0;
	}
	for(int i=0; i<540; i++){
		int tar=-1,ID=arr[i].set.ID;
		for(int j=0; j<5; j++)
			if(arr[i].avg>division[j])
				tar=j;
		divCnt[tar]++;
		for(int j=0; j<(int)zeroAttr[ID].size(); j++){
			states[tar].tupleCnt[mmap[zeroAttr[ID][j].position]]++;
			tupleAvg[mmap[zeroAttr[ID][j].position]]+=arr[i].avg;
		}
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
	for(int i=1; i<tupleNum; i++){
		tupleAvg[i]/=tupleSum[i];
		printf("tuple ID:%d\t average score:%f\n",i,tupleAvg[i]);
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
	for(int I=1; 0 && I<=60; I++){
		int tarID=I;
		printf("tuple %d:\n",tarID);
		for(int i=0; i<(int)zeroAttr[tarID].size(); i++){
			printf("%d ",mmap[zeroAttr[tarID][i].position]);
		}
		puts("");
	}
	return 0;
}
