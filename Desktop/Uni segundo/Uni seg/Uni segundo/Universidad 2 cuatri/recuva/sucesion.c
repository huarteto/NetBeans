/*************************************************************************
 *                                                                       *
 *   ESCUELA UNIVERSITARIA POLITECNICA DE TERUEL                         *
 *   GRADO EN INGENIERIA INFORMÁTICA                                     *
 *   ASIGNATURA: PROGRAMACIÓN I                                          *
 *   CURSO: 2013/2014                                                    *
 *                                                                       *
 *   PRACTICA 3 Ejercicio 3                                              *
 *                                                                       *
 *   Autor   : Alejandro fernandez                                       *
 *                                                                       *
 *************************************************************************/
 /*sucesion.c
  calcula n terminos de una sucesion*/

#include "simpleio.h"

int n, i, resultado;

int main(void) {
	escribir_texto_nl("Sucesion: (-1)^(n + 1) * (n^2 - (n + 2))");
	escribir_texto("Numero de terminos: ");
	leer_entero(n);
	
	if (n!=0) {
		for(i=1;i<=n;i++) {
			resultado = (pow(-1,i+1))*((pow(i,2))-(i+2));
			escribir_entero(resultado);
			escribir_texto(" ");
		}
	}
	escribir_nl();
return 0;
}
