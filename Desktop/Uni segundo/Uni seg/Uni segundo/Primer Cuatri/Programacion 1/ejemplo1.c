/* suma.c 
 * Lee dos enteros y calcula la suma.
 */
#include "stdio.h"
int entero1, entero2;
int suma;
int main (void) {
printf("%s\n", " Suma de dos numeros ");
// lectura de los Datos
printf("%s\n", " Numero 1: ");
scanf("%d", &entero1);
printf("%s\n", " Numero 2: ");
scanf("%d", &entero2);

//Calculo de las cuentas
suma = entero1 + entero2;
printf("%d", entero1);
printf("%s\n", "+");
printf("%d", entero2);
printf("%s\n", "=");
printf("%d", suma);
return 0;
}
