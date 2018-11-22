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
* Breve descripción: A través de este programa representamos             *
* graficamente una serie de puntos que ha marcado el alumno.             *
*		 					 					                         *
*************************************************************************/
/* triangulo.c */

#include "simpleio.h"

int Lado_1, Lado_2, Lado_3;

//  Lectura de datos

int main(void) {
	escribir_texto("Lado 1: ");
	leer_entero(Lado_1);
	escribir_texto("Lado 2: ");
	leer_entero(Lado_2);
	escribir_texto("Lado 3: ");
	leer_entero(Lado_3);

//  Calculos
	if (Lado_1==Lado_2 && Lado_2==Lado_3 && Lado_1==Lado_3)
		escribir_texto_nl("El triangulo es equilatero.");
		else if (Lado_1!=Lado_2 && Lado_2!=Lado_3 && Lado_3!=Lado_1)
			escribir_texto_nl("El triangulo es escaleno.");
			else 
				escribir_texto_nl("El triangulo es isosceles.");
	return 0;	
}
