/* Realizar la ruleta de un casino
 */
 
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void  introducir_datos(char nombre, char apellido, float saldoInicial ) {
	
    
    printf("%s", "Introduce tu nombre: ");
	scanf("%s", &nombre);
	
	printf("%s", "Introduce tu apellido: ");
    scanf("%s", &apellido);

   	printf("%s", "Introduce tu saldo inicial: ");
	scanf("%f", &saldoInicial);
}

void jugar_impar(float saldoInicial) {
	
	float dineroApostado;
	printf("Tu saldo actual es: %0.2f", saldoInicial);
	printf("Cuanto quieres apostar: ");
	scanf("%2f", &dineroApostado);
	if(dineroApostado > saldoInicial) {
		printf(" No tienes suficiente dinero\n ");
		printf(" Vuelve a introducir cuanto quieres apostar: ");
		scanf("%2f", &dineroApostado);
	}
		
	
}
