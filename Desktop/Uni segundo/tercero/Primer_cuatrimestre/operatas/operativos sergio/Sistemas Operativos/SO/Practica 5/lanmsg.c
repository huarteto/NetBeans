// lanmsg

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("Mal, debes introducir dos parametros x, y. (x < y)\n");
		return(-1);
	}
	
	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	
	if(x > y) {
		printf("Mal uso, debes introducir dos parametros x, y. (x < y)\n");
		return (-1);
	}
	
	int hijo = fork();
	printf("Se crea un hijo\n");
	if(hijo == 0) {
		while(1) {
			duerme(x);
			printf("El hijo esta molestando\n");
			fflush(stdout);
		}
	}
	else {
		duerme(y);
		printf("A matar a %d\n", hijo);
		kill(hijo, SIGKILL);
		printf("El padre se deshace del hijo\n");
	}
	return 0;
}

void (*syshandler)();
void handler(int signal) {
}

int duerme(int segundos) {
	int alarma;
	syshandler = signal(SIGALRM, handler);
	alarma = alarm(segundos);
	pause();
	return 0;
}
