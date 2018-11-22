// duerme.c

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void (*syshandler)();

void handler(int signal) {
	printf("Estamos en la rutina de servicio\n");
}
	
int duerme(int segundos) {
	
	int alarma;
	syshandler = signal(SIGALRM, handler);
	alarma = alarm(segundos);
	pause();
	return 0;
}

int main(int argc, char *argv[]) {
	printf("Entramos, espera 5 segundos y veras.\n");
	duerme(5);
	return 0;
}



