/* Medias de 10 alumnos
 */
 
#include "stdio.h" 

int main () {
int total = 0;
int promedio;
int contador = 1;
int calificacion;

while (contador<= 10) {
	printf("%s", "Escribe una calificacion: ");
	scanf("%d", &calificacion);
	
	total = total + calificacion;
	contador++;
}
    printf("%s", "El promedio es: ");
    promedio = total/10;
    
return 0;
}
