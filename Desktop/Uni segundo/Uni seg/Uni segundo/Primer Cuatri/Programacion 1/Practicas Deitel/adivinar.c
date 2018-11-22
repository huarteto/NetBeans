/*  Escribir un programa en C para adivinar un número entre 1 y 100 que
previamente se ha definido como una constante. El programa irá pidiendo
números al usuario y, siempre que dicho número no coincida con el número
secreto, le indicará si el número introducido es mayor o menor que el número
secreto que tiene que adivinar. Al final, el programa indicará la cantidad de
intentos que se han necesitado para adivinar el número. Si el número de intentos
es menor que 5 se mostrará “Enhorabuena!”. Si es un valor entre 5 y 10 se
mostrará el mensaje “No está mal”. Si el número de intentos es mayor que 10 se
mostrará el mensaje “Debe practicar más”
*/
#include "stdio.h"
#define SECRETO 54

int main(){
int n;
int intentos = 0;

	do{
		printf("%s", "Introduce numero:");
		scanf("%d", &n);
		intentos ++;
		if(SECRETO > n) {
			printf("%s", "El numero que has dicho es menor\n");
			}else if(SECRETO < n) {
				printf("%s", "El numero que has dicho es mayor\n");
				}
				if(SECRETO == n){
					printf("%s", "Ole tu polla ahi\n");
			    }
		}while(n!= SECRETO);
		
		  
		if(intentos < 5){
			printf("%s", "Enhorabuena\n");
		}else  if(intentos >= 5 && intentos <=10){
			printf("%s" ,"No esta mal\n");
		}else {
			printf("%s", "Debe practicar mas\n");
		}  

return 0;
}






