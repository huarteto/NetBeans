/* Cuenta cliente
 */
 
 
#include "stdio.h"
int numero_cuenta;
int saldo_mes;
int total_cargos;
int creditos;
int limite_credito;
int nuevo_saldo;

int main() {
while(numero_cuenta != -1){
	 printf("%s", "Introduzca el numero de cuenta: ");
	 scanf("%d", &numero_cuenta);
	 printf("%s", "Introduzca el saldo inicial: ");
	 scanf("%d", &saldo_mes);
     printf("%s", "Introduzca el total de cargos: ");
	 scanf("%d", &total_cargos);
     printf("%s", "Introduzca el total de creditos: ");
	 scanf("%d", &creditos);
	 printf("%s", "Introduzca el limite de credito: ");
	 scanf("%d", &limite_credito);
	 nuevo_saldo = saldo_mes + total_cargos - creditos;
	 printf("El nuevo saldo es: %d\n", nuevo_saldo);
	 if( nuevo_saldo > limite_credito) {
		 printf("%s", "Limite de credito excedido, te jodes kio\n ");
     }
}
return 0;
}
