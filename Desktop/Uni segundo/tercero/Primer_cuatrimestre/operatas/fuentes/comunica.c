/*
 *              LUIS CARLOS APARICIO CARDIEL
 *
 *                Sistemas Operativos I
 *              Ingenieria Tecnica en Informatica de Gestion
 *
 *               	comunica.c
 */

#include <stdio.h>
#include <string.h>
#include "error.h"

int 
main()
{
	char            string[] = "Hola Alumnos de la EUPT \n";

	char            buf[256];
	int             count, i;

	int             to_par[2], to_chil[2];


	pipe(to_par);
	pipe(to_chil);

	if (fork() == 0) {
		//El proceso hijo se ejecuta aqui;
		close(0);
		dup(to_chil[0]);
		close(1);
		dup(to_par[1]);

		close(to_chil[0]);
		close(to_chil[1]);
		close(to_par[0]);
		close(to_par[1]);

		for (;;) {
			if ((count = read(0, buf, sizeof(buf))) == 0)
				exit(0);
			else {
				write(1, buf, count);
				write(2, buf, count);
			}
		}
	}
	//El proceso padre se ejecuta aqui;
	close(1);
	dup(to_chil[1]);
	close(0);
	dup(to_par[0]);

	close(to_par[0]);
	close(to_par[1]);
	close(to_chil[0]);
	close(to_chil[1]);

	for (i = 0; i < 10; i++) {
		write(1, string, strlen(string));
		write(2, string, strlen(string));
		read(0, buf, sizeof(buf));
	}

	exit(0);
}
