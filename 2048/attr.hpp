#ifndef _ATTR_HPP_
#define _ATTR_HPP_

#include "board-kevinptt.hpp"

#include <vector>

using namespace std;

#define ATTR_DATA_SIZE (1<<24)

struct Attr
{
	int slotNum;
	int position;
	float *data;	//ATTR_DATA_SIZE
};

float _getScore(const board &b, vector<Attr> &attr)
{
	float ret=0;
	for(int i=0; i<(int)attr.size(); i++){
		int id=0;
		for(int j=0; j<attr[i].slotNum; j++){
			int pos=(attr[i].position>>(j<<2))&0xf;
			id|=b.getCell(pos>>2,pos&0x3)<<(j<<2);
		}
		ret+=attr[i].data[id];
	}
	return ret;
}

float getScore(board b, vector<Attr> &attr)
{
	float ret=0;
	ret+=_getScore(b,attr);
	b.mirrorUD();
	ret+=_getScore(b,attr);
	b.mirrorLR();
	ret+=_getScore(b,attr);
	b.mirrorUD();
	ret+=_getScore(b,attr);
	b.trans();
	ret+=_getScore(b,attr);
	b.mirrorUD();
	ret+=_getScore(b,attr);
	b.mirrorLR();
	ret+=_getScore(b,attr);
	b.mirrorUD();
	ret+=_getScore(b,attr);
	return ret;
}

void _updateAttr(const board &b, vector<Attr> &attr, float val)
{
	for(int i=0; i<(int)attr.size(); i++){
		int id=0;
		for(int j=0; j<attr[i].slotNum; j++){
			int pos=(attr[i].position>>(j<<2))&0xf;
			id|=b.getCell(pos>>2,pos&0x3)<<(j<<2);
		}
		attr[i].data[id]+=val;
	}
}
void updateAttr(board b, vector<Attr> &attr, float val)
{
	_updateAttr(b,attr,val);
	b.mirrorUD();
	_updateAttr(b,attr,val);
	b.mirrorLR();
	_updateAttr(b,attr,val);
	b.mirrorUD();
	_updateAttr(b,attr,val);
	b.trans();
	_updateAttr(b,attr,val);
	b.mirrorUD();
	_updateAttr(b,attr,val);
	b.mirrorLR();
	_updateAttr(b,attr,val);
	b.mirrorUD();
	_updateAttr(b,attr,val);
}

#endif
