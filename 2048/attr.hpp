#ifndef _ATTR_HPP_
#define _ATTR_HPP_


#include "board-kevinptt.hpp"

using namespace std;

#define ATTR_DATA_SIZE (1<<24)

struct Attr
{
	int slotNum;
	int position;
	float *data;	//ATTR_DATA_SIZE
};

float _getScore(const board &b, Attr attr[], int attrN)
{
	float ret=0;
	for(int i=0; i<attrN; i++){
		int id=0;
		for(int j=0; j<attr[i].slotNum; j++){
			int pos=(attr[i].position>>(j<<2))&0xf;
			id|=b.getCell(pos>>2,pos&0x3)<<(j<<2);
		}
		ret+=attr[i].data[id];
	}
	return ret;
}

float getScore(board b, Attr attr[], int attrN)
{
	float ret=0;
	ret+=_getScore(b,attr,attrN);
	b.mirrorUD();
	ret+=_getScore(b,attr,attrN);
	b.mirrorLR();
	ret+=_getScore(b,attr,attrN);
	b.mirrorUD();
	ret+=_getScore(b,attr,attrN);
	b.trans();
	ret+=_getScore(b,attr,attrN);
	b.mirrorUD();
	ret+=_getScore(b,attr,attrN);
	b.mirrorLR();
	ret+=_getScore(b,attr,attrN);
	b.mirrorUD();
	ret+=_getScore(b,attr,attrN);
	return ret;
}

void _updateAttr(const board &b, Attr attr[], int attrN, float val)
{
	for(int i=0; i<attrN; i++){
		int id=0;
		for(int j=0; j<attr[i].slotNum; j++){
			int pos=(attr[i].position>>(j<<2))&0xf;
			id|=b.getCell(pos>>2,pos&0x3)<<(j<<2);
		}
		attr[i].data[id]+=val;
	}
}
void updateAttr(board b, Attr attr[], int attrN, float val)
{
	_updateAttr(b,attr,attrN,val);
	b.mirrorUD();
	_updateAttr(b,attr,attrN,val);
	b.mirrorLR();
	_updateAttr(b,attr,attrN,val);
	b.mirrorUD();
	_updateAttr(b,attr,attrN,val);
	b.trans();
	_updateAttr(b,attr,attrN,val);
	b.mirrorUD();
	_updateAttr(b,attr,attrN,val);
	b.mirrorLR();
	_updateAttr(b,attr,attrN,val);
	b.mirrorUD();
	_updateAttr(b,attr,attrN,val);
}

#endif
