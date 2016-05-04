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
int mirrorMap[65536];	// 1234 -> 4321
int SmirrorMap[65536];	// 1234 -> 2143

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
	for(int i=0; i<65536; ++i)
		SmirrorMap[i] = ((i&0xf)<<4) | ((i&0xf0)>>4) | ((i&0xf00)<<4) | ((i&0xf000)>>4);
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
		genCell();
		genCell();
	}
	void left(bool next=false) {
		bool change = false;
		for(int i=0; i<4; ++i) {
			if (row[i] != leftMap[row[i]])
				change = true;
			row[i] = leftMap[row[i]];
		}
		if (next && change)
			genCell();
	}
	void right(bool next=false) {
		bool change = false;
		for(int i=0; i<4; ++i) {
			if (row[i] != rightMap[row[i]])
				change = true;
			row[i] = rightMap[row[i]];
		}
		if (next && change)
			genCell();
	}
	void up(bool next=false) {
		trans();
		left(next);
		trans();
	}
	void down(bool next=false) {
		trans();
		right(next);
		trans();
	}
	void clockwise()
	{
		trans();
		mirrorLR();
	}
	void C_clockwise()
	{
		mirrorLR();
		trans();
	}
	inline void mirrorLR() {		//left right mirrorLR
		for(int i=0; i<4; ++i) {
			row[i] = mirrorMap[row[i]];
		}
	}
	inline void swap(int r1,int r2,int pos)
	{
		row[r2]^=row[r1]&pos;
		row[r1]^=row[r2]&pos;
		row[r2]^=row[r1]&pos;
	}
	void trans()
	{
		for(int i=2; i<4; i++)
			row[i]=mirrorMap[row[i]];
		swap(0,3,0xff);
		swap(1,2,0xff);
		for(int i=2; i<4; i++)
			row[i]=mirrorMap[row[i]];
		for(int i=1; i<4; i+=2)
			row[i]=SmirrorMap[row[i]];
		swap(0,1,0xff0);
		swap(2,3,0xf00f);
		for(int i=1; i<4; i+=2)
			row[i]=SmirrorMap[row[i]];
	}
	void print() {
		puts("---------------------------------");
		for(int i=0; i<4; ++i) {
			printf("|");
			for(int j=0; j<4; ++j) {
				int k = getCell(i, j);
				if (k)
					printf(" %5d |", 1<<k);
				else
					printf("       |");
			}
			puts("");
		}
		puts("---------------------------------");
	}
	int row[4];
	inline int _empty() {
		int empty = 0;
		for(int i=0; i<16; ++i)
			empty |= (getCell(i>>2, i&0x3)==0)<<i;
		return empty;
	}
	inline bool die() {
		return _empty()==0;
	}
	inline void genCell() {
		int empty = _empty(), ret, num;
		if (empty!=0) {
			while(1) {
				ret = rand()&0xf;
				num = ((rand()%10)==0)+1; // 2:4 = 9:1
				if (empty&(1<<ret)) {
					setCell(row, ret>>2, ret&0x3, num);
					return;
				}
			}
		}
	}
	bool movable()
	{
		board tmp=*this;
		tmp.left();
		if(tmp!=*this)
			return true;
		tmp.right();
		if(tmp!=*this)
			return true;
		tmp.up();
		if(tmp!=*this)
			return true;
		tmp.down();
		if(tmp!=*this)
			return true;
		return false;
	}
	inline int getCell(int x, int y) {
		return (row[x]>>((3-y)<<2)) & 0xf;
	}
	inline void setCell(int arr[], int x, int y, int val) {
		arr[x] = (arr[x] & (0xffff^(0xf<<((3-y)<<2)))) | (val<<((3-y)<<2));
	}
	inline bool operator!=(const board& rhs) const
	{
		for(int i=0; i<4; i++)
			if(row[i]!=rhs.row[i])
				return true;
		return false;
	}
	inline bool operator==(const board& rhs) const
	{
		for(int i=0; i<4; i++)
			if(row[i]!=rhs.row[i])
				return false;
		return true;
	}
};

#endif
