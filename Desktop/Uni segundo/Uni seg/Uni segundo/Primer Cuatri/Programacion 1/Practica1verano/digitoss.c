/* digitoss.c
 * saber cuantos digitos tiene dicho numero
 */
#include "stdio.h"

int numero;
int contador = 0;

int main (void) {
	printf("%s\n", " Escribe el numero: ");
	scanf("%d", &numero);
	
	while (numero>=1) {
		numero = numero/10;
		contador++;
	}
	  printf("%d %s", contador,"digitos");
      printf("\n");
	
	
	
return 0;
 }
