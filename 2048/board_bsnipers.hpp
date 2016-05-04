#ifndef _BOARD_HPP_
#define _BOARD_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>

using namespace std;

typedef unsigned long long ull;

ull moveLeft[65536];
ull moveRight[65536];
ull transpose[65536];

int moveLscore[65536];
int moveRscore[65536];

void genMap()
{
	ull upgrade[16];
	upgrade[0]=0;
	for(int i=1; i<16; i++)
		upgrade[i]=(i+1)&0xf;
	for(ull i=0; i<65536; i++){
		int score=0;
		ull tmp=i,ip=i;
		int cnt=3;
		while(!(tmp&0xf) && cnt--)
			tmp>>=4;
		cnt=2;
		while(!(tmp&0xf0) && cnt--)
			tmp=(tmp&0xf)|(tmp>>8<<4);
		cnt=1;
		while(!(tmp&0xf00) && cnt--)
			tmp=(tmp&0xff)|(tmp>>12<<8);
		if((tmp&0xf)==(tmp>>4&0xf)){
			score+=(1<<(tmp&0xf))&0xfffe;
			tmp=upgrade[tmp&0xf]+(tmp>>8<<4);
		}
		if((tmp&0xf0)==(tmp>>4&0xf0)){
			score+=(1<<((tmp&0xf0)>>4))&0xfffe;
			tmp=(tmp&0xf)+(upgrade[tmp>>4&0xf]<<4)+(tmp>>12<<8);
		}
		if((tmp&0xf00)==(tmp>>4&0xf00)){
			score+=(1<<((tmp&0xf00)>>8))&0xfffe;
			tmp=(tmp&0xff)+(upgrade[tmp>>8&0xf]<<8);
		}
		moveLeft[i]=tmp;
		ip=((ip&0xf)<<12)+((ip&0xf0)<<4)+((ip&0xf00)>>4)+((ip&0xf000)>>12);
		tmp=((tmp&0xf)<<12)+((tmp&0xf0)<<4)+((tmp&0xf00)>>4)+((tmp&0xf000)>>12);
		moveRight[ip]=tmp;
		tmp=(i&0xf)|((i>>4&0xf)<<16)|((i>>8&0xf)<<32)|((i>>12&0xf)<<48);
		transpose[i]=tmp;
		moveLscore[i]=score;
		moveRscore[ip]=score;
	}
}

class board
{
public:
	board()
	{
		srand(time(0));
		genMap();
	}
	~board()
	{
	}
	void init()
	{
		bmap=0;
		genCell();
		genCell();
	}
	void left(bool next=false)
	{
		ull tmp=bmap;
		bmap=moveLeft[bmap&0xffff]
			|(moveLeft[bmap>>16&0xffff]<<16)
			|(moveLeft[bmap>>32&0xffff]<<32)
			|(moveLeft[bmap>>48&0xffff]<<48);
		/*
		bmap=moveLeft[bmap&0xffff]
			|(moveLeft[(bmap&0xffff0000ull)>>16]<<16)
			|(moveLeft[(bmap&0xffff00000000ull)>>32]<<32)
			|(moveLeft[(bmap&0xffff000000000000ull)>>48]<<48);
		*/
		if(next && (tmp != bmap))
			genCell();
	}
	void right(bool next=false)
	{
		ull tmp=bmap;
		bmap=moveRight[bmap&0xffff]
			|(moveRight[bmap>>16&0xffff]<<16)
			|(moveRight[bmap>>32&0xffff]<<32)
			|(moveRight[bmap>>48&0xffff]<<48);
		/*
		bmap=moveRight[bmap&0xffff]
			|(moveRight[(bmap&0xffff0000ull)>>16]<<16)
			|(moveRight[(bmap&0xffff00000000ull)>>32]<<32)
			|(moveRight[(bmap&0xffff000000000000ull)>>48]<<48);
		*/
		if(next && (tmp != bmap))
			genCell();
	}
	void up(bool next=false)
	{
		trans();
		left(next);
		trans();
	}
	void down(bool next=false)
	{
		trans();
		right(next);
		trans();
	}
	void clockwise()
	{
		mirrorUD();
		trans();
	}
	void C_clockwise()
	{
		trans();
		mirrorUD();
	}
	inline void mirrorUD()	// upside down mirror
	{
		bmap=((bmap&0xffff)<<48)
				|((bmap>>16&0xffff)<<32)
				|((bmap>>32&0xffff)<<16)
				|(bmap>>48&0xffff);
	}
	void trans()
	{
		bmap=transpose[bmap&0xffff]
				|(transpose[bmap>>16&0xffff]<<4)
				|(transpose[bmap>>32&0xffff]<<8)
				|(transpose[bmap>>48&0xffff]<<12);
	}
	void print()
	{
		puts("---------------------------------");
		ull tmp=bmap;
		for(int i=0; i<4; i++){
			printf("|");
			for(int j=0; j<4; j++){
#ifdef _WIN32
				printf(" %5I64u |",1ull<<(tmp&0xf)>>1<<1);
#else
				printf(" %5llu |",1ull<<(tmp&0xf)>>1<<1);
#endif
				tmp>>=4;
			}
			puts("");
		}
		puts("---------------------------------");
	}
	inline void genCell()
	{
		ull pos=0;
		ull cnt=0;
		for(ull i=0; i<16; i++)
			if(!(bmap&(0xfull<<(i<<2))))
				pos|=i<<(cnt++<<2);
		if(cnt!=0){
			ull num=(rand()%10==0)?2:1; //2:4 = 9:1
			int tar=rand()%cnt;
			bmap|=num<<(((pos>>(tar<<2))&0xf)<<2);
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
	inline int getCell(int x,int y)
	{
		return (bmap>>((x*4+y)<<2))&0xf;
	}
	inline void setCell(int x,int y,int val)
	{
		bmap|=val<<((x*4+y)<<2);
	}
	inline bool operator!=(const board& rhs) const
	{
		return bmap!=rhs.bmap;
	}
	inline bool operator==(const board& rhs) const
	{
		return bmap==rhs.bmap;
	}
	ull bmap;
};



#endif
