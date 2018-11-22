/* Calcular el precio con iba
 * y sin iva
 */
 
#include "stdio.h"

double precio;
double iva;
double precioIva;

int main(void){
	printf("introducir precio: ");
	scanf("%lf", &precio);
 
	iva= precio * 0.21;
	precioIva=(precio * 0.21) + precio;
 
	printf("%0.2lf", precioIva);
	printf("\n");
 
 return 0;
}

