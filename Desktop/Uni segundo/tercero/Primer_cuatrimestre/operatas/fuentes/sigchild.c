 
#include <stdio.h>
#include <string.h>
#include <signal.h>

void captura();


int main()
{
	int hijo;
	int k;

	//signal(SIGCHLD, SIG_IGN);
	signal (SIGCHLD, captura);

	if (fork() == 0)
	{
		printf("\n Soy el hijo %d \n", getpid()); fflush(stdout);
		sleep(3);
		exit(0);
	}

	k = 20;
	do
	{
		k = sleep(k);
	} while (k > 0);

	//hijo = wait(NULL);
	//printf("\n %d Mi hijo ha finalizado: %d \n", k, hijo); fflush(stdout);

}

void captura()
{
	int hijo;

	fprintf(stderr, "\n Captura \n");
	hijo = wait(NULL);
	printf("\n Mi hijo ha finalizado: %d \n", hijo); fflush(stdout);
} 	 
 
