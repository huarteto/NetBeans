// ejer.331.c

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
	int i, maxveces;
	maxveces = 100;
	if (fork() == 0) {
		for (i = 0; i < maxveces; i++) {
			fprintf(stderr, "+++");
			sleep(1);
		}
		printf("\n Soy +++ y me muero \n");
		exit(0);
	}
	
	if (fork() == 0) {
		for (i = 0; i < maxveces; i++) {
			fprintf(stderr, "---");
			sleep(1);
		}
		printf("\n Soy --- y me muero \n");
		exit(0);
	}
	wait(NULL);	
	wait(NULL);
	exit(0);
}
