#include "board-kevinptt.hpp"

using namespace std;

#define ATTR_NUM 10

struct Attr
{
	int slot_num;
	int position;
	int data[1<<24];
};

float _score(const board &b, Attr attr[], int attrN)
{
	float ret=0;
	for(int i=0; i<attrN; i++){
		int id=0;
		for(int j=0; j<attr[i].slot_num; j++){
			int pos=(attr[i].position>>(j<<2))&0xf;
			id|=b.getCell(pos>>2,pos&0x3)<<(j<<2);
		}
		ret+=attr[i].data[id];
	}
	return ret;
}

float score(board b, Attr attr[], int attrN)
{
	float ret=0;
	ret+=_score(b,attr,attrN);
	b.mirrorUD();
	ret+=_score(b,attr,attrN);
	b.mirrorLR();
	ret+=_score(b,attr,attrN);
	b.mirrorUD();
	ret+=_score(b,attr,attrN);
	b.trans();
	ret+=_score(b,attr,attrN);
	b.mirrorUD();
	ret+=_score(b,attr,attrN);
	b.mirrorLR();
	ret+=_score(b,attr,attrN);
	b.mirrorUD();
	ret+=_score(b,attr,attrN);
	return ret;
}

void _updateAttr(const board &b, Attr attr[], int attrN, float val)
{
	for(int i=0; i<attrN; i++){
		int id=0;
		for(int j=0; j<attr[i].slot_num; j++){
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

