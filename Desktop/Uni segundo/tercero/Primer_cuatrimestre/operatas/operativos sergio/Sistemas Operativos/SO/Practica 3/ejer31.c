// ejer31.c

#include <sys/types.h>
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
	int pidh;
	printf("\n Inicio prueba \n");
	fflush(stdout);
	
	if ((pidh = fork()) == 0) {
		fprintf(stderr, "\n\t Soy el hijo: %d\n", getpid());
		fprintf(stderr, "\n\t fork() me devuelve: %d\n", pidh);
		sleep(10);
		exit(1);
	}
		
	if (wait(NULL) == -1) syserr("wait");
	
	fprintf(stdout, "\n\t Soy el padre: %d\n", getpid());
	fprintf(stdout, "\n\t fork() me devuelve: %d\n", pidh);
	exit(0);
}
