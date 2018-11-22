 #include <stdio.h>
 #include <stdlib.h>
 #include <time.h>
 //Introduce los datos iniciales comunes para los dos juegos.
void  introducir_datos(char nombreDeUsuari[15],char apellido[15], float *saldoInicia){
	
	 printf("Introducir nombre: ");
	 scanf("%s", nombreDeUsuari);
	 
	 printf("Introducir apellidos: ");
	 scanf("%s", apellido);
	 
	  
	 printf("Introducir saldo inicial: ");
	 scanf("%f", saldoInicia);
	 
}
// Las funcion de cuando el jugador elige apostar por impar
void jugada_impar(float *saldoInicial){
	float dineroApostado;
	int numeroRuleta;
	int hora = time(NULL);
	srand(hora);
	printf("Tu saldo actual es: %0.2f", *saldoInicial);
	printf("Cuanto quieres apostar?:\n");
	scanf("%f", &dineroApostado);
	while(dineroApostado>*saldoInicial){
		printf("Te has pasado de saldo\n");
		printf("Cuanto quieres apostar?:\n ");
		scanf("%f", &dineroApostado);
   }
	numeroRuleta= rand()%36;
	
	
	if(numeroRuleta%2!=0) {
		*saldoInicial=*saldoInicial+dineroApostado;
		printf("Has ganado\n");
		printf("Tu saldo actual es: %0.2lf\n", *saldoInicial);
   }
   else {
	   *saldoInicial=*saldoInicial-dineroApostado;
	   printf("Has perdido\n");
	   printf("Tu saldo actual es: %0.2lf\n", *saldoInicial);
   }   
}
// Las funcion de cuando el jugador elige apostar por par
void jugadas_par(float *saldoInicial) {
  float dineroApostado;
	int numeroRuleta;
	int hora = time(NULL);
	srand(hora);
	printf("Tu saldo actual es: %0.2f", *saldoInicial);
	printf("Cuanto quieres apostar?:\n ");
	scanf("%f", &dineroApostado);
	numeroRuleta= rand()%36;
	printf("%d", numeroRuleta);
	
	if(numeroRuleta%2==0) {
		*saldoInicial=*saldoInicial+dineroApostado;
		printf("Has ganado\n");
		printf("Tu saldo actual es: %0.2lf\n", *saldoInicial);
   }
   else {
	   *saldoInicial=*saldoInicial-dineroApostado;
	   printf("Has perdido\n");
	   printf("Tu saldo actual es: %0.2lf\n", *saldoInicial);
   }   
} 
// Las funcion de cuando el jugador elige apostar por numero
void jugada_numero( float *saldoInicial) {
	int numeroJugador;
	int dineroApostado;
	int numeroRuleta;
	int hora = time(NULL);
	srand(hora);
	numeroRuleta= rand()%36;
	printf("Cuanto quieres apostar?: ");
	scanf("%d", &dineroApostado);
	printf("Introduce tu numero: ");
	scanf("%d", &numeroJugador);
	while(numeroJugador>36) {
		printf("Introduce un numero entre el 1 y el 36 ");
		printf("Introduce tu numero: ");
		scanf("%d", &numeroJugador);
	}
	if(numeroJugador==numeroRuleta){
	   dineroApostado=dineroApostado*36;
	   *saldoInicial=*saldoInicial+dineroApostado*36;
	   printf("Has ganado\n");
	   printf("Tu saldo actual es: %0.2lf\n", *saldoInicial);
    }
	else{
		*saldoInicial=*saldoInicial-dineroApostado;
		printf("Has perdido\n");
		printf("Tu saldo actual es: %0.2lf\n", *saldoInicial);
    }

}
// Funcion que permite decidir dentro de la ruleta a que tipo de juego quieres jugar.		
void ruleta(float *saldoInicial){
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
		  jugada_impar(saldoInicial);
      }
       if(opcion==2) {
		  jugada_impar(saldoInicial);
	   } 
	     if(opcion==3) {
		  jugada_numero(saldoInicial);
	   } 
      if(saldoInicial==0) {
		  terminado=1;
		  printf(" Tu saldo se ha agotado");
	  }	  	
   
   }
}
// Funcion que reparte las cartas al jugador y a la banca.
int    pedir_cartas (int *palo) {
	    int hora = time(NULL);
	    int cartaJugador;
	    srand(hora);
	    cartaJugador= (rand()%10)+1;
	    *palo= (rand()%4)+1;
	    		switch(*palo){
				case 1: printf("Carta: %d de copas\n",cartaJugador);break;
				case 2: printf("Carta: %d de bastos\n",cartaJugador);break;
				case 3: printf("Carta: %d de oros\n",cartaJugador);break;
				case 4: printf("Carta: %d de espadas\n",cartaJugador);break;
				}
	   return(cartaJugador);
}
// Funcion que comprueba el numero de cartas del jugador y de la banca para saber cual es el ganador.
void comprobar ( float saldoInicial, float saldoJugado){
	int sumaCartas;
	double saldoGanado=0;
	double saldoPerdido=0;
	if (sumaCartas == 21){
		printf("Ganastes la partida\n\n");
		saldoGanado = saldoJugado * 21;
		printf("Saldo ganado: %0.2lf euros\n",saldoGanado);
		saldoInicial = saldoInicial + saldoGanado;
		printf("Saldo que te queda: %0.2lf euros\n\n",saldoInicial);
	}	
	if (sumaCartas>21){
		printf("Perdistes la partida\n");
		printf("La suma se pasa de los 21\n\n");
		saldoPerdido = saldoJugado * 1;
		printf("Saldo perdido: %0.2lf euros\n",saldoPerdido);
		saldoInicial = saldoInicial - saldoPerdido;
		printf("Saldo que te queda: %0.2lf euros\n\n",saldoInicial);			
	}	
}

// Es la función del blackjack
void    blackjack(float *saldoInicial) {
	    float saldoJugado;
	    int jugar;
	    int salida=0;
	    int palo;
	    int numeroCarta = 0;
	    int suma = 0;
	    int sumaCrupier=0;
	    int sumaDeVuelta;
	    printf("Cuanto quieres apostar?:\n");
	    scanf("%f", &saldoJugado);
	    
	    while(saldoJugado>*saldoInicial){
			printf("Te has pasado de saldo\n");
			printf("Cuanto quieres apostar?: ");
			scanf("%f", &saldoJugado);
        }
        printf("Tus cartas son:");
         while (sumaCrupier<21) {
			 sumaDeVuelta=pedir_cartas(&palo);
			 sumaCrupier=sumaCrupier + sumaDeVuelta;
	    }	
	    while(jugar !=2 && salida==0) {
			
			numeroCarta = pedir_cartas(&palo);
			
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
	   comprobar(*saldoInicial, saldoJugado);
  }
	
 int main (void){
	float saldoInicial;
	int opcion;
	char nombreDeUsuario[15];
	char apellido[15];
	
	introducir_datos(nombreDeUsuario,apellido,&saldoInicial);
	
	printf("A que juego quieres jugar? Ruleta[1] Blackjack[2] ");
	scanf("%d", &opcion); 
	switch (opcion) {
		case 1: ruleta(&saldoInicial);
		break;
		case 2: blackjack(&saldoInicial);
		break;
	}
    
    return 0;
}

