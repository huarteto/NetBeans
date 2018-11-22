/* ejercicio 1*/
#include <stdio.h>
/* En este caso solo usamos la funcion main, declaramos una variable llamada a como
 * un entero, y gracias al if comprobamos si es o no multiplo de 5.Si los es nos
 * imprime por pantalla Es multimo sino No lo es.*/
int main(){
	int a; 
	printf("Introduce un numero: ");
	scanf("%d", &a);
	if((a % 5)==0){
		printf("El numero es multiplo\n");
	}
	else{
		printf("No es multiplo\n");
    }
return 0;
}
