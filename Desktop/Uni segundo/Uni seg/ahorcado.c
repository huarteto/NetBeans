#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_PALABRAS 100
#define MAS_FALLOS 4
#define MAX_LONG_PAL 32
#define MASCARA '*'

char palabras[MAX_PALABRAS][MAX_LONG_PAL];
char jugada[MAX LONG PAL], jugada[MAX_LONG_PAL];
int i, numPalabras = 0, fallos = 0;
void leerPalabra(char *c, char palabra[], FILE *f) {
int i = 0;
while(isspace(*c)) *c = fgetc(f);
while (isalpha(*c)) {
palabra[i++] = *c; *c = fgetc(f);
}
palabra[i] = ' palabra[i] = '\0'; // final de cadena 0'; // final de cadena
}

int leerPalabras(char palabras[][MAX_LONG_PAL], char nf[],int *np){
char palabra[MAX_LONG_PAL]; int i = 0; char c;
FILE *f = fopen(nf, "r");
if (f == NULL) {
Vectores en C (5)
if (f == NULL) {
puts("Error al abrir el fichero");
return 0;
}
while ((c = fgetc( )) ) { f)) != EOF) {
leerPalabra(&c, palabras[i++], f); (*np)++;
}
fclose(f);
return 1;
}

void mostrarFallos(char palabra[], char jugada[]) {
int c;
for (c = 0; c < strlen(jugada); (jugada); c++)
if (jugada[c] == palabra [c]) putchar(jugada[c]);
else putchar(MASCARA);
putchar('\n');
}
int otraPartida() {
char seguir[MAX_LONG_PAL];
puts("Otra partida (S/N): "); gets(seguir);
return ((st c p r m (segu , S ) 0) || ir, "S") == 0) ||
(strcmp(seguir, "s") == 0));
}
