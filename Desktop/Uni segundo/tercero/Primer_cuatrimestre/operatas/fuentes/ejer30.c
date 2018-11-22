/*
 *              LUIS CARLOS APARICIO CARDIEL
 *
 *                Sistemas Operativos I
 *              Ingenieria Tecnica en Informatica de Gestion
 *
 *              ejer30.c
 *
 */


#include <stdio.h>

int 
main()
{
	int             pid;

	if (fork() == 0) {
		pid = getppid();
		printf("\n pid padre antes = %d\n", pid);

		sleep(10);	/* damos tiempo a que muera el padre */

		pid = getppid();
		printf("\n pid padre despues = %d\n", pid);

		exit(1);
	}
	sleep(5);
	exit(0);
}
