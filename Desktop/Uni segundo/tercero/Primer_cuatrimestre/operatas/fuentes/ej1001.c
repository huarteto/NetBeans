/*
 *	 	LUIS CARLOS APARICIO CARDIEL
 *
 *                Sistemas Operativos I
 *		Ingenieria Tecnica en Informatica de Gestion
 *
 *	      	ej1001.c
 *
 *		Shell elemental con bucle de lectura de comandos con parametros
 *		Uso: [arre / soo] [comando] [lista parmaetros]
 */

#include <stdio.h>
#include <string.h>
#include "error.h"

#define BUFSIZE 1024
#define TRUE 1
#define FALSE 0

int 
main()
{
	static char     s[BUFSIZE];
	char           *argt[BUFSIZE];
	int             i, parate, pid, pidaux;

	while (1) {
		do {
			fprintf(stderr, "_$ ");
			gets(s);
		} while (s[0] == '\0');

		argt[0] = strtok(s, " ");


		if (0 == strcmp(argt[0], "quit")) {
			write(2, "logout \n", 8);
			exit(0);
		}
		for (i = 1; (argt[i] = strtok(NULL, " \n")) != NULL; i++);


		if (0 == strcmp(argt[0], "soo"))
			parate = TRUE;
		else if (0 == strcmp(argt[0], "arre"))
			parate = FALSE;
		else {
			printf("\n Mande?");
			continue;
		}

		switch (pid = fork()) {
		case -1:	/* error */
			{
				fprintf(stderr, "\n No se puede crear un nuevo proceso\n");
				syserr("fork");
			} break;

		case 0:	/* proceso hijo */
			{
				if (!parate)
					sleep(3);	/* par que se note mas */
				execvp(argt[1], &argt[1]);
				fprintf(stderr, "\n No se puede ejectuar %s \n, argt[1]");
				syserr("execvp");
			} break;

		default:	/* proceso padre */
			{
				if (parate) {
					while (pid != (pidaux = wait(NULL))) {
						if (pidaux == -1) {
							write(2, "\n Muy raro. Bye\n", 15);
							exit(1);
						}
					}
				}
			}
		}		/* switch */
	}			/* while  */
}				/* main  */
