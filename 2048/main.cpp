#ifdef _WIN32
#define _hypot hypot
#endif

#include "board.hpp"
#include <cstdio>

int main() {
	board b;
	b.init();
	b.print();
	char str[10];
	int sum=0;
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
	return 0;
}
