
/*************************************************************************
 *                                                                       *
 *   ESCUELA UNIVERSITARIA POLITECNICA DE TERUEL                         *
 *   GRADO EN INGENIERIA INFORMÁTICA                                     *
 *   ASIGNATURA: PROGRAMACIÓN I                                          *
 *   CURSO: 2013/2014                                                    *
 *                                                                       *
 *   PRACTICA 3 Ejercicio 2                                              *
 *                                                                       *
 *   Autor   : Alejandro fernandez rodriguez	                         *
 *                                                                       *
 *************************************************************************/
 /*division.c
  realiza una division unicamente utilizando la operacion resta*/
 
 #include "simpleio.h"

int dividendo,	divisor,	resto,	 cociente;

int main(void) {
	escribir_texto("introduce dividendo: ");
	leer_entero(dividendo);
	escribir_texto("introduce divisor: ");
	leer_entero(divisor);
	cociente=0;


	
	
	
//calculos
	dividendo =  dividendo - divisor;
	cociente = cociente + 1;
	while(dividendo>=divisor) {
		dividendo = dividendo - divisor;
		cociente++;
		
		
		
		if(dividendo < divisor) {
			
			escribir_texto("cociente: ");escribir_entero_nl(cociente);
			resto = dividendo;
			escribir_texto("resto: ");escribir_entero(resto);
			escribir_nl();
			
			
		}
	}
		
	
			
	
	  
	
return 0;
}	
	
