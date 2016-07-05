#ifdef _WIN32
#define _hypot hypot
#endif

#include "board.hpp"
#include "attr.hpp"
#include <cstdio>

bool load(const char* filename, vector<Attr> &attr) {
	FILE* fin = fopen(filename, "rb");
	if(!fin){
		return false;
	}
	int attrN;
	fread(&attrN,sizeof(int),1,fin);
	attr.resize(attrN);
	for(int i=0; i<attrN; i++){
		fread(&attr[i].slotNum,sizeof(int),1,fin);
		fread(&attr[i].position,sizeof(int),1,fin);
		attr[i].data=new float[1<<(attr[i].slotNum<<2)];
		fread(&(*attr[i].data),sizeof(float),1<<(attr[i].slotNum<<2),fin);
	}
	fclose(fin);
	return true;
}

int main(int argc, char* argv[]) {
	genMap();
	board b;
	b.init();
	b.print();
	int sum=0;
	if( argc==2 ) {
		char in[2048];
		strcpy(in, argv[1]);
		vector<Attr> attr;
		if( !load(in, attr) ) {
			fprintf(stderr, "file open failed.\n");
			return 1;
		}

	} else {
		char str[10];
		while( ~scanf("%s", str) ) {
			switch(str[0]) {
				case 'a': printf("total score: %d\n",sum+=b.left(true)); break;
				case 'd': printf("total score: %d\n",sum+=b.right(true)); break;
				case 'w': printf("total score: %d\n",sum+=b.up(true)); break;
				case 's': printf("total score: %d\n",sum+=b.down(true)); break;
			}
			b.print();
			puts("");
		}
	}
	return 0;
}
