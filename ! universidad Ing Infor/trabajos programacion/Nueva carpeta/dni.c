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
* Breve descripción: A través de este programa desciframos cual          *
* es la letra que corresponde a cada número del DNI, dividiendo          *
* el número por 23.                                                      *
*		 					 					                         *
*************************************************************************/

/* dni.c */

#include "simpleio.h"

int dni, division;

int main(void) {
	escribir_texto("Escribir el numero de dni: ");
	leer_entero(dni);
	
//  Calculos
	division = dni % 23;
	escribir_texto("Aparecera la letra del dni: ");
	
	switch (division) {
	case 0:
		escribir_texto_nl("T");break;
	case 1:
		escribir_texto_nl("R");break;
	case 2:
		escribir_texto_nl("W");break;
	case 3:	
		escribir_texto_nl("A");break;
	case 4:
		escribir_texto_nl("G");break;
	case 5:
		escribir_texto_nl("M");break;
	case 6:
		escribir_texto_nl("Y");break;
	case 7:
		escribir_texto_nl("F");break;
	case 8:
		escribir_texto_nl("P");break;
	case 9:
		escribir_texto_nl("D");break;
	case 10:
		escribir_texto_nl("X");break;
	case 11:
		escribir_texto_nl("B");break;
	case 12:
		escribir_texto_nl("N");break;
	case 13:
		escribir_texto_nl("J");break;
	case 14:
		escribir_texto_nl("Z");break;
	case 15:
		escribir_texto_nl("S");break;
	case 16:
		escribir_texto_nl("Q");break;
	case 17:
		escribir_texto_nl("V");break;
	case 18:
		escribir_texto_nl("H");break;
	case 19:
		escribir_texto_nl("L");break;
	case 20:
		escribir_texto_nl("C");break;
	case 21:
		escribir_texto_nl("K");break;
	case 22:
		escribir_texto_nl("E");break;
	
	return 0;
	}	
}
