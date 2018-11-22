/* Parametros por referencia punterosssss
 */
#include "stdio.h"

int pedro(int valor) {
	valor= valor +10;
	
return valor;
}
int punteros(int *valor){
	*valor = *valor +10;
	return *valor;
}
	
