#include <stdio.h>
#include <unistd.h>
#define MAX 80

int main(){
	char buffer [MAX];
	int leer;
	do {
		leer= read(0, buffer, MAX);
		write(1, buffer, leer);
	}
	while(leer > 0);
}
