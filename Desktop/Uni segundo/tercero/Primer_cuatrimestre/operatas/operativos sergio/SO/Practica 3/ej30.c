#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main() {
	int pid;
	if (fork() == 0) {
		pid = getppid();
		printf("\n pid padre antes = %d\n", pid);
		sleep(10); /* damos tiempo a que muera el padre */
		pid = getppid();
		printf("\n pid padre despues = %d\n", pid);
		exit(1);
	}
	sleep(40);
	exit(0);
}
