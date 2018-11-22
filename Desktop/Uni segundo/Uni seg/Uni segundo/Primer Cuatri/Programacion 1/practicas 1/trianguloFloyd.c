/*************************************************************************
*																		 *
* ESCUELA UNIVERSITARIA POLITECNICA DE TERUEL							 *
* GRADO EN Ingenieria Informática										 *
* ASIGNATURA: Programación I 											 *
* CURSO: 2014-2015														 *
* 																		 *
* PRACTICA 1_7													         *
* 																		 *
* Fichero : trianguloFloyd.c	 		                                 *
* Autor : Alejandro Fernandez Rodriguez							         *
* Fecha : 15/10/2014 													 *
* 	                                                                     *
* dibuja el triangulo de floid de altura introducida por teclado.        *
*                  														 *
*************************************************************************/

#include "stdio.h"

int altura;
int c1;
int c2;
int c3;

int main(void){
	
 printf("%s","introduce altura: ");
 scanf("%d", &altura);
 c1=1;c2=1;c3=1;
	while(c2<=altura){
		c1=1;
		while(c1<=c2){
			printf("%d",c3);
			c1++;
			c3++;
			
		}
	printf("\n");
	c2++;
    }
  printf("\n");
  return 0;
}
