/*Ejemplo practico 3
 */
#include "stdio.h"

int main() {
int contador = 1;
int aprobados = 0;
int suspensos = 0;
int resultado;

while(contador<=10) {
	printf("%s", "Introducir resultado examen: ");
    scanf("%d", &resultado);
    contador++;
    
    
    if(aprobados == 1){
	   aprobados = aprobados +1;
	}else{
		suspensos = suspensos +1;
	}
}
	scanf("%d", &aprobados);
	scanf("%d", &suspensos);
	
	
    if(aprobados>=8) {
		printf("%s","Objetivo cumplido\n");
	}else {
		printf("%s", "No se ha cumplido\n");
    
	}	


return 0;


}
