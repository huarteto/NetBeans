#include <stdio.h>



void  introducir_datos(char nombre, char apellido, float saldoInicial ) {
	
    
    printf("%s", "Introduce tu nombre: ");
	scanf("%s", &nombre);
	
	printf("%s", "Introduce tu apellido: ");
    scanf("%s", &apellido);

   	printf("%s", "Introduce tu saldo inicial: ");
	scanf("%f", &saldoInicial);
}
