#ifdef _WIN32
#define _hypot hypot
#endif
#ifdef BSNIPERS
#include "board-bsnipers.hpp"
#endif
#ifdef KEVINPTT
#include "board-kevinptt.hpp"
#endif
#include <cstdio>

int main() {
	board b;
	b.init();
	b.print();
	char str[10];
	while( ~scanf("%s", str) ) {
		switch(str[0]) {
			case 'a': b.left(true); break;
			case 'd': b.right(true); break;
			case 'w': b.up(true); break;
			case 's': b.down(true); break;
		}
		b.print();
		puts("");
	}
	return 0;
}
