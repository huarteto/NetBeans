/*************************************************************************
 *                                                                       *
 *   ESCUELA UNIVERSITARIA POLITECNICA DE TERUEL                         *
 *   GRADO EN INGENIERIA INFORMÁTICA                                     *
 *   ASIGNATURA: PROGRAMACIÓN I                                          *
 *   CURSO: 2013/2014                                                    *
 *                                                                       *
 *   PRACTICA 3 Ejercicio 1                                              *
 *                                                                       *
 *   Autor   : Alejandro fernandez rodriguez	                         *
 *                                                                       *
 *************************************************************************/
 /*media-mayor-menor.c
  lee una serie de enteros ,calcula su media y obtiene el mayor y el menor*/
  
 #include "simpleio.h"

int mayor,	menor,	numero, suma,	media,	n;


int main(void) {
 escribir_texto("introducir numero: ");
 leer_entero(numero);
 mayor = numero; menor = numero;
 suma = numero;
 n = 0;
 
 
 while(numero!=0) {
 escribir_texto("introducir numero: ");
 leer_entero(numero);
 n = n + 1;
 suma = suma + numero;
 
  if(numero==0) {
	  
	  media = suma / n;
  }
 
  if(numero>mayor) {
  mayor = numero;
  }
  else if(numero<menor && numero!=0) {
  menor = numero;
  }
  }
  
  
	  
	escribir_texto("mayor= "); escribir_entero_nl(mayor);
	escribir_texto("menor= "); escribir_entero_nl(menor);
	escribir_texto("media= "); escribir_entero_nl(media);
	
return 0;

	
}	
