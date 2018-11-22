/*ejercicio 3 */



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/* Esta es una funcion del programa llamada carticasA la cual lo que hace es 
 * crear una carta aleatoria del uno al 10 con su palo correspondiente para el jugador A, para ello utilizamos
 * la instruccion rand, por último la instruccion nos devuelve con el return el valor de dicha carta.*/
 
int carticasA (int *palo) {
	int cartaJugadorA;
	int hora = time(NULL);
	srand(hora);
	cartaJugadorA = (rand()%10) +1;
	*palo = (rand()%4) +1;
	switch(*palo) {
		case 1: printf(" %d de oros", cartaJugadorA); break;
		case 2: printf(" %d de copas", cartaJugadorA); break;
		case 3: printf(" %d de espadas", cartaJugadorA);break;
		case 4: printf(" %d de bastos", cartaJugadorA);break;
	}
	return (cartaJugadorA);
}
/* Esta funcion hace practicamente lo mismo que la funcion anterior pero para
 * el jugador B devolviendonos su carta con el return*/
 
 
int carticasB (int *palo){
	int cartaJugadorB;
	int hora = time(NULL);
	srand(hora);
	cartaJugadorB = (rand()%10) +1;
	*palo = (rand()%4) +1;
	switch(*palo) {
		case 1: printf(" %d de oros", cartaJugadorB); break;
		case 2: printf(" %d de copas", cartaJugadorB); break;
		case 3: printf(" %d de espadas", cartaJugadorB);break;
		case 4: printf(" %d de bastos", cartaJugadorB);break;
	}
	return (cartaJugadorB);
}

/* Esta funcion se llama comprobacion la cual declaramos las variables sumaA y sumaB 
 * y tambien llamamos a las cartas de la otra funcion. Con un while reccorremos varias 
 * veces que realize la comprobacion de que carta es mas grande, si son iguales suma 
 * un punto a cada uno de los jugadores y si una es mas grande que la otra al jugador 
 * que posea la carta mas alta. Esto se produce hasta que el while detecta que una de las dos
 * sumas o las dos(empate) ha llegado a 5 entonces procedemos con la comprobacion
 * que dependiendo quien haya ganado lo imprime por pantalla junto con el resueltado
 * de ambos.Ivan al imprimir la pantalla he modificado algo de codigo que la verdad
 * nose que he hecho pero me imprime oomo un bucle infinito directamente.*/

void comprobacion(){
	int sumaA;
	int sumaB;
	int cartaA;
	int cartaB;
	
	while(sumaA !=5 || sumaB != 5){
		if( cartaA == cartaB){
		printf("Las dos cartas son iguales !!!EMPATE!!\n");
		sumaA++;
		sumaB++;
    }
    if(cartaA > cartaB){
		printf("El jugador A a GANADO¡¡¡\n");
		sumaA++;
	}
	if(cartaB < cartaA){
		printf("El jugador B a GANADO!!!!\n");
		sumaB++;
	}
  } 
	if(sumaA ==5){
		printf("Se termino el juego!!!!\n");
		printf("la suma del JugadorA es: %d", sumaA);
		printf("la suma del JugadorB es: %d", sumaB);
		printf("El jugador ganador es el Jugador A!!!!");
	}
	if(sumaB == 5){
		printf("Se termino el juego!!!!\n");
		printf("la suma del JugadorA es: %d", sumaA);
		printf("la suma del JugadorB es: %d", sumaB);
		printf("El jugador ganador es el Jugador B!!!!");
	}
	if(sumaA ==5 && sumaB == 5){
		printf("Se termino el juego!!!!!\n");
		printf("Tenemos un empate entre los dos jugadores!!!");
	}
    
 }

/*Esta es una funcion llamada juego la cual realiza una funcion simple
 * declaramos dos variables que igualamos al resultado de las dos funciones
 * de antes una para cada jugador,y las imprimimos por pantalla a traves del
 *"printf", luego y la parte mas importante llamamos a la funcion comprobacion.
 * No he podido terminar le codigo completamente por lo que en la captura de pantalla 
 * solo aparece lo que hace la funcion juego ya que la funcion comprobacion no he podido
 * terminarla*/
void juego(){
	int palo;
	int cartaA;
	int cartaB;
	cartaA = carticasA(&palo);
	cartaB = carticasB(&palo);
	printf("La carta del jugador A es: %d\n", cartaA);
	printf("La carta del jugador B es: %d\n", cartaB);
	comprobacion();
}

/* Esta es la funcion del main la cual lo unico que hace es imprimir por pantalla que vamos
 * a empezar el juego ademas de llamar a la funcion juego que contiene a todas las demas*/ 
int main() {
	printf("Vamos a empezar el juego!!!\n");
    juego();
return 0;	
}

