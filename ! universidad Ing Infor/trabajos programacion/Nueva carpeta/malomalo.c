algoritmo MayorMenor;
variables
entero n, número, mayor, menor;
principio
escribir_texto("Cantidad números: ");
leer_entero(n);
leer_entero(número); Cantidad números: 4
mayor = número; menor = número;
mientras que (n > 1) hacer
leer_entero(número);
si ( número > mayor) entonces
12
6
1
100
y ) M 100
mayor = número;
sino si (número < menor) entonces
menor = número;
fsi;
Mayor: Menor: 1
n = n – 1;
fmq;
escribir_texto("Mayor: "); escribir_entero(mayor);
escribir_texto("Menor: "); escribir_entero(menor);
fin.
