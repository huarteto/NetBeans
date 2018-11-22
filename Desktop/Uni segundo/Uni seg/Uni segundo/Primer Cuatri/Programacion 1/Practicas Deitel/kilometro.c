/* calcular litros kilometros etc
 */
 
#include "stdio.h"


int litros;
int kilometros;
int kilometro_litro= 0;
int kilometros_litros_total=0;
int contador = 0;
int main() {
	while(kilometros != -1){
	printf("%s", "Introducir los kilometros recorridos: ");
	scanf("%d", &kilometros);
	printf("%s", "Introducir los litros usados: ");
	scanf("%d", &litros);
	kilometro_litro = kilometros/litros;
	printf("kilometros_litro: %d\n", kilometro_litro);
	contador++;

	kilometros_litros_total = kilometros_litros_total + kilometros/litros;
	printf("kilometros_litros_total: %d\n", kilometros_litros_total);
	
}

return 0;
}
