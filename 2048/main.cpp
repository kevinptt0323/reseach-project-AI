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
			case 'a': b.left(); break;
			case 'd': b.right(); break;
			case 'w': b.up(); break;
			case 's': b.down(); break;
		}
		b.print();
		puts("");
	}
	return 0;
}
