/* numeroprimoo.c
 * Averiguar si n es un numero primo o no
 */
#include "stdio.h"
int numero;
int i;
int primo = 0;
int main (void) {
	printf("%s\n", "Escribe el numero: ");
	scanf("%d", &numero);
	
	for(i=1;i<=numero;i++){
			if(numero%i==0){
				primo++;
			}
	
    }
  if (numero==0  ) {
	  printf("%s\n", "El numero es primo");
  }
  else {
	  printf("%s\n", "El numero no es primo");
  }
  printf("\n");

  return 0;
}
