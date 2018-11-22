// sincronos.c

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


void nadahandler();
int main() {
	int i, pid_p, pid_h;
	pid_p = getpid();
	signal(SIGUSR1, nadahandler);
	
	if((pid_h = fork()) == 0) {
		while(1) {
			pause();
			signal(SIGUSR1, nadahandler);
			printf("\n Hijo %d", pid_p);
			kill(pid_p, SIGUSR1);
		}
	}
	else {
		for(i = 1; i < 1000; i++) {
			printf("\n Padre %d", i);
			kill(pid_h, SIGUSR1);
			pause();
			signal(SIGUSR1, nadahandler);
		}
		
		kill(pid_h, SIGKILL);
	}
	exit(0);
}

void nadahandler() {
}
