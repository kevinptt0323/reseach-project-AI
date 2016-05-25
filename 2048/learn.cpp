#include <cstdio>
#include <cstring>
#include "board.hpp"
#include "attr.hpp"

bool load(const char* filename, Attr &attr) {
	FILE* fin = fopen(filename, "rb");
	return true;
}

int main(int argc, char* argv[]) {
	char in[2048], out[2048];
	if( argc==3 ) {
		strcpy(in, argv[1]);
		strcpy(out, argv[2]);
	} else {
		return 1;
	}

	Attr attr;
	if( !load(in, attr) ) {
	}

}
