/* DEITEL numero mas pequeño y
 * numero mas grande
 */

#include <stdio.h>
int numero1;
int numero2;
int main () {
printf("%s", "Escribe un numero: ");
scanf ("%d", &numero1);
printf("%s", "Escribe un numero: ");
scanf ("%d", &numero2);

if(numero1 == numero2) {
	printf("%s", "Ambos son iguales\n");
}
if(numero1 < numero2) {
	printf("%s", "Numero1 es el mayor\n" );
}
if(numero2 < numero1) {
	printf("%s", "Numero2 es el mayor\n") ;
}




return 0;
}
