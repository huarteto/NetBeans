#include <stdlib.h>  /*fichero con declaracion de funciones srand,rand*/
#define M 5			/*n�mero de tablas a ordenar*/
#define N 10		/*tama�o tabla a ordenar*/

int T[N];			/*tabla de datos a ordenar*/

extern
void ordena(int *tabla, int numero_elem);		/*declara funci�n externa de ordenaci�n*/

int main () {
	int i, j, k;

	k = 1 << 30;		/*k=2^30*/
	srand(23456);		/*semilla n�meros pseudo-aleatorios*/

	for (i = 0; i < M; i++) {		/*M ordenaciones a realizar*/
		for (j = 0; j < N; j++) {
			T[j] = rand() - k;		/*n�meros enteros aleatorios en [-2^30,2^30-1]*/
		}
		ordena(T,N);		/*llamada a funci�n de ordenaci�n*/
	}
	while (1);		/*bucle infinito de finalizaci�n*/
}

	