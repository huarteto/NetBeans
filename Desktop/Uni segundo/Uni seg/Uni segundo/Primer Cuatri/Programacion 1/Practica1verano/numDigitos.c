
/*************************************************************************
*																		 *
* ESCUELA UNIVERSITARIA POLITECNICA DE TERUEL							 *
* GRADO EN Ingenieria Informática										 *
* ASIGNATURA: Programación I 											 *
* CURSO: 2014-2015														 *
* 																		 *
* PRACTICA 1_2													         *
* 																		 *
* Fichero : numDigitos.c	 		                                     *
* Autor : Alejandro Fernandez Rodriguez							         *
* Fecha : 15/10/2014 													 *
* 	                                                                     *
* 	calcula el numero de digitos de un entero introducido por teclado.   *
*                  														 *
*************************************************************************/

#include "stdio.h"

int numero;
int contador=0;

int main(void){

	printf("introducir numero: ");
	scanf("%d", &numero);  

		while(numero>=1){
			numero=numero/10;
			contador++;
		}
  
  
  printf("%d %s", contador,"digitos");
  printf("\n");
  return 0;
}

