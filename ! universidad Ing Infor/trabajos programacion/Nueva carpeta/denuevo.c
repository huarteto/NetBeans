
#include "simpleio.h"

int a, b, c, d, e, f, x, y,;

int main(void)  {
  escribir_texto_nl("resolucion de un sistema de ecuaciones");
  // lectura de datos
  escribir_texto("introduce coeficiente a: ") leer_entero(a);
  escribir_texto("introduce coeficiente b: ") leer_entero(b);
  escribir_texto("introduce coeficiente c: ") leer_entero(c);
  escribir_texto("introduce coeficiente d: ") leer_entero(d);
  escribir_texto("introduce coeficiente e: ") leer_entero(e);
  escribir_texto("introduce coeficiente f: ") leer_entero(f);
  
  // escribir ecuacion
  escribir_entero(a);
  escribir_texto("x");
  escribir_texto(" x ");
  escribir_entero(b);
  escribir_texto("y");
  escribir_texto(" = ");
  escribir_entero_nl(c);
  escribir_entero(d);
  escribir_texto("x");
  escribir_texto(" + ");
  escribir_entero(e);
  escribir_texto("y");
  escribir_texto(" = ");
  escribir_entero_nl(f);
  
  // calculos y resultado
  escribir_texto("x");
  escribir_texto(" = ");
  escribir_entero(c*e-b*f/(a*e-b*d));
  
  escribir_texto("y");
  escribir_texto(" = ");
  escribir_entero(a*f-c*d/(a*e-b*d));
}
  
