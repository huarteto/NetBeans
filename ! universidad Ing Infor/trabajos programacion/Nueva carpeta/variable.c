/*************************************************************************
*																		 *
* ESCUELA UNIVERSITARIA POLITECNICA DE TERUEL							 *
* GRADO EN Ingenieria Informática										 *
* ASIGNATURA: Programación 1											 *
* CURSO: 2013/2014 														 *
* 																		 *
* PRACTICA N. 2														     *
* 																		 *
* Fichero : triangulo.c	 		                                         *
* Autor : Álvaro Huarte Albás								             *
* Fecha : 19/10/2013 													 *
* 																		 *
* Breve descripción: A través de este programa intercambiamos            *
* variables.                                                             *
*		 					 					                         *
* 																		 *
*************************************************************************/
/* variable.c */

#include "simpleio.h"

int variableA, variableB, variableC;

int main(void) {

	escribir_texto("Variable A: ");
	leer_entero(variableA);
	escribir_texto("Variable B: ");
	leer_entero(variableB);
	
//  Calculos
	
	if (variableA > variableB) {
		escribir_texto_nl("Las variables aparecen intercambiadas");
		escribir_nl();
		variableC = variableA, variableA = variableB, variableB = variableC;
		escribir_texto("Variable A: "); escribir_entero(variableA);
		escribir_nl();
		escribir_texto("Variable B: "); escribir_entero(variableB);
		escribir_nl();
		}
		else {
		escribir_texto_nl("Las variables no aparecen intercambiadas");
		}

 return 0;
}
