
/*************************************************************************
 *                                                                       *
 *   ESCUELA UNIVERSITARIA POLITECNICA DE TERUEL                         *
 *   GRADO EN INGENIERIA INFORMÁTICA                                     *
 *   ASIGNATURA: PROGRAMACIÓN I                                          *
 *   CURSO: 2013/2014                                                    *
 *                                                                       *
 *   PRACTICA 3 Ejercicio 4                                              *
 *                                                                       *
 *   Autor   : Alejandro fernandez                                       *
 *                                                                       *
 *************************************************************************/
 /*piramide.c
  introduciendo la altura de una piramide por el teclado, dibuja la piramide en funcion de la altura introducida*/


#include "simpleio.h"
int altura, fila, columna;

int main(void){

//Presentacion de los datos
escribir_texto("Altura piramide: ");
leer_entero(altura);

//Calculos y representacion final
	while (altura <= 1) {
		escribir_texto("Altura piramide: ");
		leer_entero(altura);}
	
	for(fila=1; fila<=altura; fila++){    
		for(columna=1; columna<=altura-fila; columna++){
			escribir_texto(" ");}
		
		for(columna=1;columna<=2*fila-1;columna++){ 
			escribir_texto("*");}
			escribir_nl();
	}
}


