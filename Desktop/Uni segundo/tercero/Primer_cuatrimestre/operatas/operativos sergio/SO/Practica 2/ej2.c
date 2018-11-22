#include <stdio.h>
#include <unistd.h>
#define MAX 1024

int main(int argc, char *argv[]) {
	
	char buffer [MAX];
	int leer;
	int descriptor;
	
	printf("%d\n", argc);
	
	if(argc > 1) {
		descriptor = creat(argv[1], 0700);
		do {
			leer = read(0, buffer, 80);
			write(descriptor, buffer, leer);
		}
		while(leer > 0);
	}
	
	else {
		do {
			leer = read(0, buffer, 80);
			write(descriptor, buffer, leer);
		}
		while(leer > 0);
	}
	
	if (leer == -1) {
		printf("Ha habido un error\n");
	}
	close(descriptor);
}
