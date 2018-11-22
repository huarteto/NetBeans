#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX 1024

int main(int argc, char *argv[]) {
	
	char buffer [MAX];
	int leer;
	int descriptor;
	
	printf("%d\n", argc);
	
	if(argc > 1) {
		descriptor = open(argv[1], O_RDWR);
		lseek(descriptor, 0, SEEK_END);
		
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
