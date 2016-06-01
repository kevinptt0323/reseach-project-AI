#include <stdio.h>

#include "board-kevinptt.hpp"
#include "attr.hpp"


int attrN;
Attr attr[10];

int main()
{
	freopen("zero.dat","w",stdout);
	printf("1\n");
	printf("6\n");
	printf("%d\n",(0x0)|(0x1<<4)|(0x2<<8)|(0x3<<12)|(0x4<<16)|(0x5<<20));
	for(int i=0; i<1<<24; i++){
		printf("0.0\n");
	}
	return 0;
	printf("6\n");
	printf("%d\n",(0x0)|(0x1<<4)|(0x2<<8)|(0x3<<12)|(0x4<<16)|(0x8<<20));
	for(int i=0; i<1<<24; i++){
		printf("0.0\n");
	}
	return 0;
}

