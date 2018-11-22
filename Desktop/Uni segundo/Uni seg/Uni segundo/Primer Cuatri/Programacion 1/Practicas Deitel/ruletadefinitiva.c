/*
Al empezar, el programa debe pedir el nombre y apellidos al usuario y su saldo inicial 
(dinero que va a Juego de la Ruleta I apellidos al usuario,
 y su saldo inicial (dinero que va a utilizar para jugar). */
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void datos_iniciales (char nombre[30], char apellido[30], float *saldoInicio) {
	printf("Introduce tu nombre:");
	scanf("%s", nombre);
    printf("Introduce tu apellido:");
    scanf("%s", apellido);
    printf("Introduce tu saldo de Inicio: ");
    scanf("%f", saldoInicio);
	
}
void jugar_par(float *saldoInicio) {
	float saldoApostado;
	int numeroRulet;
	int hora = time(NULL);
	srand(hora);
	printf("Tu saldo actual es: %f ", *saldoInicio);
	printf("Introduce el dinero que quieres apostar: ");
	scanf("%f", &saldoApostado);
	
	numeroRulet= rand()%36;
	
	if(numeroRulet %2 ==0){
	    printf(" HAS GANADO\n");
		*saldoInicio = *saldoInicio + saldoApostado*2;
		printf(" Tu saldo actual es: %f", *saldoInicio);
	}
	else{
		printf(" HAS PERDIDO\n");
		*saldoInicio = *saldoInicio - saldoApostado;
		printf(" Tu saldo actual es: %f", *saldoInicio);
		
    }
}
void jugar_impar(float *saldoInicio) {
	float saldoApostado;
	int numeroRulet;
	int hora = time(NULL);
	srand(hora);
	printf("Tu saldo actual es: %f", *saldoInicio);
	printf("¿Cuanto dinero quieres apostar: ?");
	scanf("%f", &saldoApostado);
	while(saldoApostado > *saldoInicio){
		printf("No tienes suficiente saldo amigo");
		printf("Vuelve a introducir la cantidad a apostar:");
		scanf("%f", &saldoApostado);
	}
	
	numeroRulet = rand()%36;
	if(numeroRulet %2 != 0){
		printf(" HAS GANADO");
		*saldoInicio = *saldoInicio + saldoApostado*2;
		printf("Tu saldo actual es: %f", *saldoInicio);
	}
	else{
		printf(" HAS PERDIDO");
		*saldoInicio = *saldoInicio - saldoApostado;
		printf("Tu saldo actual es: %f", *saldoInicio);
	}	
}
void jugar_numero( float *saldoInicio){
	float saldoApostado;
	int numeroRulet;
	int numeroJugador;
	int hora = time(NULL);
	srand(hora);
	printf("Cuanto quieres apostar: ");
	scanf("%f", &saldoApostado);
	while(saldoApostado > *saldoInicio){
		printf("No tienes suficiente saldo amigo");
		printf("Vuelve a introducir la cantidad a apostar:");
		scanf("%f", &saldoApostado);
	}
	printf("Introduce el numero al que quieres apostar: ");
	scanf("%d", &numeroJugador);
	numeroRulet = rand()%36;
	if(numeroRulet == numeroJugador){
		printf(" MENUDA SUERTE, HAS GANADO\n");
		*saldoInicio = *saldoInicio + saldoApostado*36;
		printf(" Tu saldo actual es: %f", *saldoInicio);
	}
	else{
		printf(" MENUDA PENA, HAS PERDIDO\n");
		*saldoInicio = *saldoInicio - saldoApostado;
		printf(" Tu saldo actual es: %f", *saldoInicio);
    }
}
void ruletica(float *saldoInicio){
	int terminado=0;
	int opcion;
	
	while(terminado==0){
	   printf("A que quieres apostar?:\n");
	   printf("Salida del juego es 0\n");
	   printf("Impar es 1\n");
	   printf("Par es 2\n");
	   printf("Numero es 3\n");
	   
	   scanf("%d", &opcion);
	   if(opcion==0){
		   terminado=1;
		   printf("Gracias por jugar........");
	   } 
	  if(opcion==1) {
		  jugar_impar(saldoInicio);
      }
       if(opcion==2) {
		  jugar_impar(saldoInicio);
	   } 
	     if(opcion==3) {
		  jugar_numero(saldoInicio);
	   } 
      if(saldoInicio==0) {
		  terminado=1;
		  printf(" Tu saldo se ha agotado");
	  }	  	
   
   }
}

int carticas (int *palo) {
	int cartaJugador;
	int hora = time(NULL);
	srand(hora);
	cartaJugador = (rand()%10) +1;
	*palo = (rand()%4) +1;
	switch(*palo) {
		case 1: printf(" %d de oros", cartaJugador); break;
		case 2: printf(" %d de copas", cartaJugador); break;
		case 3: printf(" %d de espadas", cartaJugador);break;
		case 4: printf(" %d de basttos", cartaJugador);break;
	}
	return (cartaJugador);
}

void comprobacion (float *saldoInicio, float saldoApostado){
	int sumaCartas;
	float saldoGanado = 0;
	float saldoPerdido = 0;
	if(sumaCartas == 21){
		printf(" Ostia que has ganado tio\n");
		saldoGanado = saldoGanado + saldoApostado;
		*saldoInicio = *saldoInicio + saldoGanado;
		printf("Tu saldo actual es: %f", *saldoInicio); 
	}	
	if(sumaCartas > 21){
		printf(" Ostia que has perdido tio\n");
		saldoPerdido = saldoPerdido + saldoApostado;
		*saldoInicio = *saldoInicio - saldoPerdido;
		printf("Tu saldo actual es: %f", *saldoInicio); 
	}	
	
}
void    blackjack(float *saldoInicio) {
	    float saldoApostado;
	    int jugar;
	    int salida=0;
	    int palo;
	    int numeroCarta = 0;
	    int suma = 0;
	    int sumaCrupier=0;
	    int sumaDeVuelta;
	    printf("Cuanto quieres apostar?:\n");
	    scanf("%f", &saldoApostado);
	    
	    while(saldoApostado>*saldoInicio){
			printf("Te has pasado de saldo\n");
			printf("Cuanto quieres apostar?: ");
			scanf("%f", &saldoApostado);
        }
        printf("Tus cartas son:");
         while (sumaCrupier<21) {
			 sumaDeVuelta = carticas(&palo);
			 sumaCrupier=sumaCrupier + sumaDeVuelta;
	    }	
	    while(jugar !=2 && salida==0) {
			
			numeroCarta = carticas(&palo);
			
			suma = suma + numeroCarta;
            
			printf("\nTu suma es: %d\n", suma);
           
			if (jugar!=2){
				printf("Quieres otra carta?(Si[1] No[2]): ");
				scanf("%d", &jugar);
		    }
			if (suma>21) {		
				salida=1;
			}		
	   }
	   comprobacion(saldoInicio, saldoApostado);
  }

int main() {
	char nombre[30];
	char apellido[30];
	float saldoInicio;
	int opcion;
	
    datos_iniciales(nombre, apellido, &saldoInicio);
    printf("A que juego quieres jugar ruleta [1] y el blackjack[2]: ");
    scanf("%d", &opcion);
    if(opcion==1){
		ruletica(&saldoInicio);
	}
    if(opcion ==2){
		blackjack(&saldoInicio);
    }
return 0;
}
