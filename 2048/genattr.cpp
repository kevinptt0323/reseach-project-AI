#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board-kevinptt.hpp"
#include "attr.hpp"

#define FRWERROR(str,num) if(int(str)!=int(num)) puts("error");

#define GENSET 0
#define SETSIZE 13
#define NORAND 1

int attrN;
Attr attr[10];

unsigned long long ini[30]={(0x0<<0)|(0x1<<4)|(0x2<<8)|(0x3<<12),
							(0x1<<0)|(0x2<<4)|(0x4<<8)|(0x5<<12),
							(0x2<<0)|(0x4<<4)|(0x5<<8)|(0x6<<12),
							(0x2<<0)|(0x3<<4)|(0x4<<8)|(0x5<<12),
							(0x0<<0)|(0x2<<4)|(0x3<<8)|(0x4<<12),
							(0x0<<0)|(0x1<<4)|(0xb<<8)|(0xf<<12),
							(0x4<<0)|(0x5<<4)|(0x6<<8)|(0x7<<12),
							(0x0<<0)|(0x1<<4)|(0x2<<8)|(0x3<<12)|(0x4<<16)|(0x5<<20),
							(0x0<<0)|(0x1<<4)|(0x2<<8)|(0x3<<12)|(0x4<<16)|(0x6<<20),
							(0x0<<0)|(0x1<<4)|(0x2<<8)|(0x3<<12)|(0x4<<16)|(0x7<<20),
							(0x0<<0)|(0x1<<4)|(0x2<<8)|(0x3<<12)|(0x4<<16)|(0x8<<20),
							(0x0<<0)|(0x1<<4)|(0x2<<8)|(0x4<<12)|(0x5<<16)|(0x6<<20),
							(0x0<<0)|(0x1<<4)|(0x2<<8)|(0x7<<12)|(0xb<<16)|(0xf<<20),
							(0x0<<0)|(0x1<<4)|(0x2<<8)|(0xb<<12)|(0xe<<16)|(0xf<<20),
							(0x4<<0)|(0x5<<4)|(0x6<<8)|(0x8<<12)|(0x9<<16)|(0xa<<20)
							};


int main()
{
	char name[50];
	int seed;
	printf("this program will generate 10~30 random attributes with zero data.\n");
	printf("please input the file name(string without space) and seed(int).\n");
	scanf("%s %d",name,&seed);
	srand(seed);
#if GENSET
	FILE *out;
#else
	FILE *out=fopen(name,"wb");
#endif
	int n;
#if GENSET
	n=SETSIZE;//meaningless
#elif NORAND
	printf("input attr n: ");
	scanf("%d",&n);
#else
	n=rand()%20+10;
#endif

	attr[0].data=new float[ATTR_DATA_SIZE];
	for(int i=0; i<ATTR_DATA_SIZE; i++)
		attr[0].data[i]=0;
#if GENSET
	for(int T=1; T<=SETSIZE+50; T++){
		sprintf(name,"zero%d.dat",T);
		out=fopen(name,"wb");
		if(T<=SETSIZE)
			n=1;
		else
			n=rand()%3+2;
		int onehot[SETSIZE]={0};
		for(int i=0; i<n; i++){
			int tar=rand()%SETSIZE;
			if(onehot[tar]==1)
				i--;
			else
				onehot[tar]=1;
		}
		FRWERROR(fwrite(&n,sizeof(int),1,out),1)
		for(int i=0; i<SETSIZE; i++){
			if( (T<=SETSIZE && i+1!=T) || (T>SETSIZE && onehot[i]==0) ) continue;
			int slotNum=0;
			for(int j=0; j<3 || (ini[i]>>(j<<2)); j++)
				slotNum++;
			attr[0].slotNum=slotNum;
			attr[0].position=ini[i];
			FRWERROR(fwrite(&attr[0].slotNum,sizeof(int),1,out),1)
			FRWERROR(fwrite(&attr[0].position,sizeof(int),1,out),1)
			FRWERROR(fwrite(&(*attr[0].data),sizeof(float),1<<(slotNum<<2),out),1<<(slotNum<<2))
		}
		fclose(out);
	}
#else
	FRWERROR(fwrite(&n,sizeof(int),1,out),1)
	printf("%d attribute\n",n);
	for(int i=1; i<=n; i++){
		int slotNum=4;
		int arr[10];
#if NORAND
		int tar;
		printf("input %dth attr: ", i);
		scanf("%d",&tar);
		slotNum=0;
		for(int j=0; j<3 || (ini[tar]>>(j<<2)); j++)
			slotNum++;
		attr[0].slotNum=slotNum;
		attr[0].position=ini[tar];
		for(int j=0; j<slotNum; j++){
			arr[j]=(ini[tar]>>(j<<2))&0xf;
		}
#else
		for(int j=0; j<slotNum; j++){
			arr[j]=rand()%16;
			for(int k=0; k<j; k++)
				if(arr[j]==arr[k]){
					j--;
					break;
				}
		}
		attr[0].slotNum=slotNum;
		attr[0].position=0;
		for(int j=0; j<slotNum; j++){
			attr[0].position|=arr[j]<<(j<<2);
		}
#endif
		board b;
		for(int j=0; j<slotNum; j++){
			b.setCell(arr[j]>>2,arr[j]&0x3,j+1);
		}
		b.print();
		FRWERROR(fwrite(&attr[0].slotNum,sizeof(int),1,out),1)
		FRWERROR(fwrite(&attr[0].position,sizeof(int),1,out),1)
		FRWERROR(fwrite(&(*attr[0].data),sizeof(float),1<<(slotNum<<2),out),1<<(slotNum<<2))
	}
	fclose(out);
#endif
	return 0;
}

