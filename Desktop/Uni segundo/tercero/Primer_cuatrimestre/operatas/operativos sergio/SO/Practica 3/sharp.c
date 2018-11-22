#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define MAX 128


int main(int argc, char *argv[]) {
	char *orden1[MAX];
	char *orden2[MAX];
	int i = 1, j = 0;
	
	while(strcmp(argv[i], "+")) {
		orden1[i - 1] = argv[i];
		i++;
	}
	
	orden1[i] = 0;
	i++;
	
	while(i < argc) {
		orden2[j] = argv[i];
		printf("%s\n", orden2[j]);
		i++;
		j++;
	}
	
	j++;
	orden2[i] = 0;
	
	if (fork() == 0) {
		execvp(orden1[0], &orden1[0]);
		exit(1);
	}
	
	if (fork() == 0) {
		execvp(orden2[0], &orden2[0]);
		exit(1);
		
	}
	wait(NULL);	
	wait(NULL);
	exit(0);
}
