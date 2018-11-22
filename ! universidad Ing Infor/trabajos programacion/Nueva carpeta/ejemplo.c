#include <stdio.h>

#define LADO_BALDOSA 30 // todas las medidas en cm.

// variables globales

int largoHab;
int anchoHab;
int numBaldosas;
int precioBaldosa;
 // programa principal
int main(void) {
 // Presentación y lectura de datos
printf("Cálculo Baldosas\n");
printf("Largo y ancho de la habitación (en cm.): ");
scanf("%d %d", &largoHab, &anchoHab);
printf("Precio de la baldosa: ");
scanf("%d", &precioBaldosa);
 // Cálculo número de baldosas necesarias, por exceso
numBaldosas = (largoHab + LADO_BALDOSA - 1) / LADO_BALDOSA  // largo
(anchoHab + LADO_BALDOSA - 1) / LADO_BALDOSA; // ancho
 // Resultados
printf("Total de baldosas: %d\n", numBaldosas);
printf("Coste total aproximado: %d\n", numBaldosas  precioBaldosa);
return 0;
}
