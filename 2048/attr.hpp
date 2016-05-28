#include "board-kevinptt.hpp"

#define ATTR_NUM 10

int attr_n;

struct Attr
{
	int slot_num;
	int position;
	int data[1<<24];
};

Attr attr[ATTR_NUM];

float _score(const board &b)
{
	float ret=0;
	for(int i=0; i<attr_n; i++){
		int id=0;
		for(int j=0; j<attr[i].slot_num; j++){
			int pos=(attr[i].position>>(j<<2))&0xf;
			id|=b.getCell(pos>>2,pos&0x3)<<(j<<2);
		}
		ret+=attr[i].data[id];
	}
	return ret;
}

float score(board b)
{
	float ret=0;
	ret+=_score(b);
	b.mirrorUD();
	ret+=_score(b);
	b.mirrorLR();
	ret+=_score(b);
	b.mirrorUD();
	ret+=_score(b);
	b.trans();
	ret+=_score(b);
	b.mirrorUD();
	ret+=_score(b);
	b.mirrorLR();
	ret+=_score(b);
	b.mirrorUD();
	ret+=_score(b);
	return ret;
}

void _updateAttr(const board &b, float val)
{
	for(int i=0; i<attr_n; i++){
		int id=0;
		for(int j=0; j<attr[i].slot_num; j++){
			int pos=(attr[i].position>>(j<<2))&0xf;
			id|=b.getCell(pos>>2,pos&0x3)<<(j<<2);
		}
		attr[i].data[id]+=val;
	}
}
void updateAttr(board b, float val)
{
	_updateAttr(b,val);
	b.mirrorUD();
	_updateAttr(b,val);
	b.mirrorLR();
	_updateAttr(b,val);
	b.mirrorUD();
	_updateAttr(b,val);
	b.trans();
	_updateAttr(b,val);
	b.mirrorUD();
	_updateAttr(b,val);
	b.mirrorLR();
	_updateAttr(b,val);
	b.mirrorUD();
	_updateAttr(b,val);
}

