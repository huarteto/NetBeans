/* segundos.c 
 * Convierte seg. en horas, minutos y segundos.
 */
#include "stdio.h"

#define SEG_DIA 60*60*24
#define SEG_HORA 60*60
#define SEG_MIN  60

int segundos;
int main(void) {
printf("%s\n", "Introduce segundos: ");
scanf("%d",&segundos);

printf(segundos/SEG_DIA);
escribir_texto_nl(" dia/s");
segundos =segundos % SEG_DIA;

escribir_entero(segundos/SEG_HORA);
escribir_texto_nl(" hora/s");
segundos =segundos % SEG_HORA;

escribir_entero(segundos/SEG_MIN);
escribir_texto_nl(" minuto/s");
segundos =segundos % SEG_MIN;

escribir_entero(segundos % SEG_MIN);
escribir_texto_nl(" segundos/s");

return 0;
}
