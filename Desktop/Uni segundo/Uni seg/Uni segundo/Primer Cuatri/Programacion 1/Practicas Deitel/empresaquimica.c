/* empresa quimcica
 */
 
#include "stdio.h"
int ventas;
int salario_semanal;
int salario_total;

int main () {
	salario_semanal = 200;
	while(ventas != -1) {
		printf("%s", "Introduzca las ventas totales: ");
		scanf("%d", &ventas);
		salario_total = salario_semanal + (ventas * 9)/100;
		printf("El salario total es: %d\n", salario_total);
        if(ventas == -1) {
			printf("%s", "Gracias\n" );
	    }

}


return 0;
}
