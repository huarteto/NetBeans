/*************************************************************************
*																		 *
* ESCUELA UNIVERSITARIA POLITECNICA DE TERUEL							 *
* GRADO EN Ingenieria Informática										 *
* ASIGNATURA: Programación I 											 *
* CURSO: 2014-2015														 *
* 																		 *
* PRACTICA 1_5													         *
* 																		 *
* Fichero : triangulo.c	 		                                 		 *
* Autor : Alejandro Fernandez Rodriguez							         *
* Fecha : 15/10/2014 													 *
* 	                                                                     *
* 	dibuja un triagulo de astericos de altura introducida por teclado.   *
*                  														 *
*************************************************************************/

#include "stdio.h"

int altura;
int c1;
int c2;

int main(void){
	printf("%s","introduce altura: ");
	scanf("%d", &altura);
	c1=1;c2=1;
		
		while(c2<=altura){
			c1=1;
			
			while(c1<=c2){
				printf("%s","*");
				c1++;
			}
		printf("\n");
		c2++;
		}
  printf("\n");
  return 0;
}
