#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board-kevinptt.hpp"
#include "attr.hpp"

#define FRWERROR(str,num) if(int(str)!=int(num)) puts("error");


int attrN;
Attr attr[10];


int main()
{
	char name[50];
	int seed;
	printf("this program will generate 10~30 random attributes with zero data.\n");
	printf("please input the file name(string without space) and seed(int).\n");
	scanf("%s %d",name,&seed);
	srand(seed);
	FILE *out=fopen(name,"wb");
	int n;
	n=rand()%20+10;
	FRWERROR(fwrite(&n,sizeof(int),1,out),1)
	printf("%d attribute\n",n);
	for(int i=0; i<ATTR_DATA_SIZE; i++)
		attr[0].data[i]=0;
	for(int i=0; i<n; i++){
		int slotNum=4;
		int arr[10];
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
		board b;
		memset(b.row,0,sizeof(b.row));
		for(int j=0; j<slotNum; j++){
			b.setCell(arr[j]>>2,arr[j]&0x3,j+1);
		}
		b.print();
		FRWERROR(fwrite(&attr[0].slotNum,sizeof(int),1,out),1)
		FRWERROR(fwrite(&attr[0].position,sizeof(int),1,out),1)
		FRWERROR(fwrite(&attr[0].data,sizeof(float),1<<(slotNum<<2),out),1<<(slotNum<<2))
	}
	fclose(out);
	return 0;
}

