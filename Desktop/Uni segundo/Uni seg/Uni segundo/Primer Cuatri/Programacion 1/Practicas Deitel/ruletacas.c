/* Realizar la ruleta de un casino
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


	

void  introducir_datos(char nombre, char apellido, float saldoInicial ) {
	
    
    printf("%s", "Introduce tu nombre: ");
	scanf("%s", &nombre);
	
	printf("%s", "Introduce tu apellido: ");
    scanf("%s", &apellido);

   	printf("%s", "Introduce tu saldo inicial: ");
	scanf("%f", &saldoInicial);
}
void jugar_impar(float saldoInicial) {
	
	float dineroApostado;
	int numeroRuleta;
	int hora = time(NULL);
	srand(hora);
	
	printf("Tu saldo actual es: %0.2f", saldoInicial);
	printf("Cuanto quieres apostar: ");
	scanf("%2f", &dineroApostado);
	if(dineroApostado > saldoInicial) {
		printf(" No tienes suficiente dinero\n ");
		printf(" Vuelve a introducir cuanto quieres apostar\n: ");
		scanf("%2f", &dineroApostado);
	}
	numeroRuleta = rand()%36;
	if(numeroRuleta%2!= 0) {
		saldoInicial = saldoInicial + dineroApostado*2;
		printf("Has ganado\n ");
		printf("Tu saldo actual es: %0.21f", saldoInicial);
	}else{
		saldoInicial = saldoInicial - dineroApostado;
		printf(" Has perdido\n");
		printf("Tu saldo actual es: %0.21f", saldoInicial);
	}	
}

void jugar_par(float saldoInicial){
	float dineroApostado;
	int numeroRuleta;
	int hora = time(NULL);
	srand(hora);
	
	printf("Tu saldo actual es: %0.2f", saldoInicial);
	printf("Cuanto quieres apostar: ");
	scanf("%2f", &dineroApostado);
	if(dineroApostado > saldoInicial) {
		printf(" No tienes suficiente dinero\n ");
		printf(" Vuelve a introducir cuanto quieres apostar\n: ");
		scanf("%2f", &dineroApostado);
	}
	numeroRuleta = rand()%36;
	if(numeroRuleta%2== 0) {
		saldoInicial = saldoInicial + dineroApostado*2;
		printf("Has ganado\n ");
		printf("Tu saldo actual es: %0.21f", saldoInicial);
	}else{
		saldoInicial = saldoInicial - dineroApostado;
		printf(" Has perdido\n");
		printf("Tu saldo actual es: %0.21f", saldoInicial);
	}	
}

void numero(float saldoInicial) {
	float dineroApostado;
	int numeroRuleta;
	int numeroJugador;
	int hora = time(NULL);
	srand(hora);
	
	printf("Tu saldo actual es: %0.21f", saldoInicial);
	printf("Cuanto dinero quieres apostar: ");
	scanf("%f", &dineroApostado);
	if(dineroApostado > saldoInicial) {
		printf(" No tienes suficiente dinero\n ");
		printf(" Vuelve a introducir cuanto quieres apostar\n: ");
		scanf("%2f", &dineroApostado);
	}
	    printf("Introduce tu numero:");
	    scanf("%d", &numeroJugador);
	    if(numeroJugador < 1 && numeroJugador > 36){
			printf(" El numero debe ser positivo y ser menor que 36");
			printf("Introduce el numero otra vez: ");
			scanf("%d", &numeroJugador);
	    }
		numeroRuleta = rand()%36;
		if(numeroRuleta == numeroJugador){
			saldoInicial = saldoInicial + dineroApostado*36;
			printf(" Has ganado\n");
			printf("Tu saldo actual es: %0.21f", saldoInicial);
		}else{
			saldoInicial = saldoInicial - dineroApostado;
			printf(" Has perdido\n");
			printf("Tu saldo actual es: %0.21f", saldoInicial);
		}
}	

void ruleta(float saldoInicial) {
	int terminado = 0;
	int opcion;
	while( terminado == 0){
		printf("¿A que quieres jugar?\n");
		printf("Impar es 1\n");
		printf("Par es 2\n");
		printf("Numero es 3\n");
		printf("Salir es 0\n");
		scanf("%d", &opcion);
		
		if(opcion == 0){
			terminado = 1;
			printf("Gracias por jugar......");
		}
		if(opcion == 1){
		    jugar_impar(saldoInicial);
	    }
	    if(opcion == 2){
			jugar_par(saldoInicial);
		}
		if(opcion == 3){
		    numero(saldoInicial);
	    }
	     if(saldoInicial==0) {
		  terminado=1;
		  printf(" Tu saldo se ha agotado");
	     }	  	
  }	
}
	
int pedir_carticas(int palo){
	int cartaJugador;
	int hora = time(NULL);
	srand(hora);
	cartaJugador = rand()%10;
	switch(palo) {
		case 1: printf("La carta es de oros: %d\n", cartaJugador);break;
		case 2: printf("La carta es de copas: %d\n", cartaJugador);break;
		case 3: printf("La carta es de espadas: %d\n", cartaJugador);break;
		case 4: printf("La carta es de bastos: %d\n", cartaJugador);break;
	}
	return (cartaJugador);
}

void comprobacion(int sumaCartas,float saldoInicial,float dineroApostado) {
	
	if(sumaCartas == 21){
		saldoInicial = saldoInicial + dineroApostado*21;
		printf("ENHORABUENA HAS GANADO\n");
		printf("Tu saldo actual es: %f", saldoInicial);
	}
	if(sumaCartas > 21){
		saldoInicial = saldoInicial - dineroApostado;
		printf("Lo sentimos has perdido\n");
		printf("Tu saldo actual es: %f", saldoInicial);
	}
	
}

void black_jack(float saldoInicial) {
	
	float saldoJugado;
	int sumaCrupier;
	int sumaDeVuelta;
	int jugar;
	int salida;
	int suma;
	int palo;
	int numeroCarta;
	printf("Cuanto quieres apostar: ");
	scanf("%f", &saldoJugado);
	if(saldoJugado > saldoInicial){
		printf("Lo sentimos no posee tanto dinero: ");
		printf("Introduzca la cantidad otra vez:");
		scanf("%f", &saldoJugado);
    }
     printf("Tus cartas son:");
         while (sumaCrupier<21) {
			 sumaDeVuelta=pedir_carticas(palo);
			 sumaCrupier=sumaCrupier + sumaDeVuelta;
	    }	
	    while(jugar !=2 && salida==0) {
			
			numeroCarta = pedir_carticas(palo);
			
			suma = suma + numeroCarta;
            
			printf("Tu suma es: %d\n", suma);
           
			if (jugar!=2){
				printf("Quieres otra carta?(Si[1] No[2]): ");
				scanf("%d", &jugar);
		    }
			if (suma>21) {		
				salida=1;
			}		
	   }
	   comprobacion(suma, saldoInicial, saldoJugado);
  }

int main (void){
		
    char nombre;
	char apellido;
	float saldoInicial;
	int opcion=1;
	
	introducir_datos(nombre, apellido, saldoInicial);
	
	printf("A que juego quieres jugar? Ruleta[1] Blackjack[2] ");
	scanf("%d", &opcion); 
	switch (opcion) {
		case 1: ruleta(saldoInicial);
		case 2: black_jack(saldoInicial);
	}
    
    return 0;
 
    return 0;
}


