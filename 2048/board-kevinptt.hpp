#ifndef _BOARD_HPP_
#define _BOARD_HPP_

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

int leftMap[65536];
int rightMap[65536];
int mirrorMap[65536];

int pushLeft(int n) {
	if ((n&0xf0)==0)
		n = (n&0xff00) | ((n&0xf)<<4);
	if ((n&0xf00)==0)
		n = (n&0xf000) | ((n&0xff)<<4);
	if ((n&0xf000)==0)
		n <<= 4;
	return n;
}

void genMap() {
	for(int i=0; i<65536; ++i)
		mirrorMap[i] = ((i&0xf)<<12) | ((i&0xf0)<<4) | ((i&0xf00)>>4) | (i>>12);
	for(int i=65535, i2; i; --i) {
		i2 = pushLeft(i);
		if ( (i2&0xf000) && ((i2^(i2<<4))&0xf000) == 0)
			i2 = (i2&0xf0ff) + 0x1000;
		if ( (i2& 0xf00) && ((i2^(i2<<4))& 0xf00) == 0)
			i2 = (i2&0xff0f) + 0x100;
		if ( (i2&  0xf0) && ((i2^(i2<<4))&  0xf0) == 0)
			i2 = (i2&0xfff0) + 0x10;
		i2 = pushLeft(i2);
		leftMap[i] = i2;
	}
	printf("%x %x\n", leftMap[0x2200], leftMap[0x2210]);
	for(int i=65535; i; --i)
		rightMap[mirrorMap[i]] = mirrorMap[leftMap[i]];
}

class board {
public:
	board() {
		srand(time(0));
		genMap();
	}
	~board() {
	}
	void init() {
		memset(row, 0, sizeof(row));
		genCeil();
		genCeil();
	}
	void left() {
		bool change = false;
		for(int i=0; i<4; ++i) {
			if (row[i] != leftMap[row[i]])
				change = true;
			row[i] = leftMap[row[i]];
		}
		if (change)
			genCeil();
	}
	void right() {
		bool change = false;
		for(int i=0; i<4; ++i) {
			if (row[i] != rightMap[row[i]])
				change = true;
			row[i] = rightMap[row[i]];
		}
		if (change)
			genCeil();
	}
	void up() {
		reflex();
		left();
		reflex();
	}
	void down() {
		reflex();
		right();
		reflex();
	}
	void mirror() {
		for(int i=0; i<4; ++i) {
			row[i] = mirrorMap[row[i]];
		}
	}
	void reflex() {
		int row2[4];
		for(int i=0; i<4; ++i) row2[i] = row[i];
		for(int i=0; i<4; ++i) {
			row[i] = 0;
			for(int j=0; j<4; ++j)
				row[i] |= getCeil(row2, j, i)<<((3-j)<<2);
		}
	}
	void print() {
		puts("---------------------------------");
		for(int i=0; i<4; ++i) {
			printf("|");
			for(int j=0; j<4; ++j) {
				int k = getCeil(row, i, j);
				if (k)
					printf(" %5d |", 1<<k);
				else
					printf("       |");
			}
			printf("\t%04x", row[i]);
			puts("");
		}
		puts("---------------------------------");
	}
private:
	int row[4];
	inline int _empty() {
		int empty = 0;
		for(int i=0; i<16; ++i)
			empty |= (getCeil(row, i>>2, i&0x3)==0)<<i;
		return empty;
	}
	inline bool die() {
		return _empty()==0;
	}
	inline void genCeil() {
		int empty = _empty(), ret, num;
		printf("%x\n", empty);
		if (empty!=0) {
			while(1) {
				ret = rand()&0xf;
				num = ((rand()%10)==0)+1; // 2:4 = 9:1
				if (empty&(1<<ret)) {
					setCeil(row, ret>>2, ret&0x3, num);
					return;
				}
			}
		}
	}
	inline int getCeil(int arr[], int x, int y) {
		return (arr[x]>>((3-y)<<2)) & 0xf;
	}
	inline void setCeil(int arr[], int x, int y, int val) {
		arr[x] = (arr[x] & (0xffff^(0xf<<((3-y)<<2)))) | (val<<((3-y)<<2));
	}
};

#endif
