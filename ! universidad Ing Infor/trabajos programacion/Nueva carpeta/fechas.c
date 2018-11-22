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
* Breve descripción: A través de este programa escribimos una fecha      *
* pudiendo elegir entre 5 formatos diferentes.                           *
*		 					 					                         *
* 																		 *
*************************************************************************/
/* fechas.c */

#include "simpleio.h"

int dia, mes, anio, variable_formato, bisiesto;

int main(void) {
		escribir_texto("Escribe el dia: ");
		leer_entero(dia);
		escribir_texto("Escribe el mes: ");
		leer_entero(mes);
		escribir_texto("Escribe el anio: ");
		leer_entero(anio);
		escribir_nl();
		
//  Conocer si el año es bisiesto
	bisiesto=0;
	if ((anio%4) !=0){
		bisiesto=0;
	}
	if ((anio%100)==0) {
		bisiesto=0;
	}
	if (((anio%100)==0) && ((anio%400)==0)) {
		bisiesto=1;
	}

//  Conocer los dias dependiendo del mes que escribas.

switch (mes) {
	case 1: if ((dia<=0) && (dia>31)) {
				escribir_texto("El dia escrito no existe");
			}
			break;
	case 2: if (bisiesto==1) {
					if (!((dia>0) && (dia<=29))) {
						escribir_texto_nl("El dia escrito no existe");
					}
					break;
				}
				else {
					if (!((dia>0) && (dia<=28))) {
						escribir_texto_nl("El dia escrito no existe");
					}
					break;
				}
	case 3: if (!((dia>0) && (dia<=31))) {
					escribir_texto_nl("El dia escrito no existe");
				}
				break;
	case 4: if (!((dia>0) && (dia<=30))) {
					escribir_texto_nl("El dia escrito no existe");
				}
				break;
	case 5: if (!((dia>0) && (dia<=31))) {
					escribir_texto_nl("El dia escrito no existe");
				}
				break;
	case 6: if (!((dia>0) && (dia<=30))) {
					escribir_texto_nl("El dia escrito no existe");
				}
				break;
	case 7: if (!((dia>0) && (dia<=31))) {
					escribir_texto_nl("El dia escrito no existe");
				}
				break;
	case 8: if (!((dia>0) && (dia<=31))) {
					escribir_texto_nl("El dia escrito no existe");
				}
				break;
	case 9: if (!((dia>0) && (dia<=30))) {
					escribir_texto_nl("El dia escrito no existe");
				}
				break;
	case 10: if (!((dia>0) && (dia<=31))) {
					escribir_texto_nl("El dia escrito no existe");
				}
				break;
	case 11: if (!((dia>0) && (dia<=30))) {
					escribir_texto_nl("El dia escrito no existe");
				}
				break;
	case 12: if (!((dia>0) && (dia<=31))) {
					escribir_texto_nl("El dia escrito no existe");
				}
				break;
		default: escribir_texto_nl("El mes escrito no existe");		
	}

if (dia<1 || dia>31 || mes<1 || mes>12) {
			escribir_texto_nl("Los valores escritos no son correctos");
			escribir_nl();
		}
			else {

//  Formatos
	escribir_texto_nl("Elige el formato");
	escribir_nl();
	escribir_texto_nl("1 -> dd de mes de aaaa");
	escribir_texto_nl("2 -> dd/mm/aaaa");
	escribir_texto_nl("3 -> dd-mm-aaaa");
	escribir_texto_nl("4 -> mm/dd/aaaa");
	escribir_texto_nl("5 -> mm-dd-aaaa");
	escribir_texto("Escribir formato elegido: ");
	leer_entero(variable_formato);
	escribir_nl();
	
switch (variable_formato) {
	case 1: 
		escribir_entero(dia);
		escribir_texto(" de ");
		switch (mes) {
			case 1:
				escribir_texto("Enero");break;
			case 2:
				escribir_texto("Febrero");break;
			case 3:
				escribir_texto("Marzo");break;
			case 4:
				escribir_texto("Abril");break;
			case 5:
				escribir_texto("Mayo");break;
			case 6:
				escribir_texto("Junio");break;
			case 7:
				escribir_texto("Julio");break;
			case 8:
				escribir_texto("Agosto");break;
			case 9:
				escribir_texto("Septiembre");break;
			case 10:
				escribir_texto("Octubre");break;
			case 11:
				escribir_texto("Noviembre");break;
			case 12:
				escribir_texto("Diciembre");break;
			}
		escribir_texto(" de ");
		escribir_entero(anio);
		escribir_nl();break;
		
	case 2:
		escribir_entero(dia);
		escribir_texto("/");
		escribir_entero(mes);
		escribir_texto("/");
		escribir_entero(anio);
		escribir_nl();break;
		
	case 3:
		escribir_entero(dia);
		escribir_texto("-");
		escribir_entero(mes);
		escribir_texto("-");
		escribir_entero(anio);
		escribir_nl();break;
	
	case 4:
		escribir_entero(mes);
		escribir_texto("/");
		escribir_entero(dia);
		escribir_texto("/");
		escribir_entero(anio);
		escribir_nl();break;
		
	case 5:
		escribir_entero(mes);
		escribir_texto("-");
		escribir_entero(dia);
		escribir_texto("-");
		escribir_entero(anio);
		escribir_nl();break;
		
	return 0;
		}
	}
}
