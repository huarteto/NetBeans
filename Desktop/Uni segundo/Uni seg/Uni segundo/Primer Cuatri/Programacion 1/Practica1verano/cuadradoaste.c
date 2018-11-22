/* cuadradoaste.c
 * Dibujar un cuadrado de asteriscos
 */
#include "stdio.h"
int numero;
int lado1;
int lado2;
int main (void) {
	printf("Introducir lado del cuadrado:" );
	scanf("%d", &numero);
	lado1= 1;
	lado2= 1;
		while(lado2 <= numero) {
			lado1=1;
		
			while(lado1 <= numero) {
				printf("%s", "*");
				lado1++;
			}
		printf("\n");
		lado2++;
		}
  printf("\n");
	
return 0;	
}
