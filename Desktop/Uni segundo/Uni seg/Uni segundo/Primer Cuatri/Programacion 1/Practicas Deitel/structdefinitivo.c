/*#include <stdio.h>

struct datoss{
	char nombre[30];
	int edad;
	float sueldo;
	
}dts[3];

void introducirlos(struct datoss *dts);
void llamarlos(struct datoss *dts);
void buscarlos(struct datoss *dts);
void modificarlos(struct datoss *dts);


int main(){
	introducirlos(dts);
	llamarlos(dts);
	buscarlos(dts);
	modificarlos(dts);

return 0;
}

void introducirlos(struct datoss *dts){
	int i;
	for(i=0;i<3;i++){
		printf("Introduce el nombre del cliente:");
		scanf("%s",(dts[i]).nombre);
		printf("Introduce la edad del cliente:");
		scanf("%d", &(dts[i]).edad);
		printf("Introduce el sueldo del cliente:");
		scanf("%f", &(dts[i]).sueldo);
    }
}	

void llamarlos(struct datoss *dts){
	int j;
	for(j=0;j<3;j++){
		printf("Nombre: %s || Edad %d || Sueldo: %f\n ", (dts[j]).nombre, (dts[j]).edad, (dts[j]).sueldo);
    }
}
void buscarlos(struct datoss *dts){
	int mes_naci_escrito;
	int m;
	printf("Introduce el mes temporal: ");
	scanf("%d", &mes_naci_escrito);
	if(mes_naci_escrito == 0){
		printf("Hasta otra\n");
	}else{
		for(m=0; m<3; m++) {
			if((dts[m]).edad == mes_naci_escrito){
				printf("Nombre: %s || Edad: %d || Saldo: %f\n ", (dts[m]).nombre, (dts[m]).edad, (dts[m]).sueldo);	
		    }
	    }
    }  
}
void modificarlos(struct datoss *dts){
	int h;
	printf("Introduce el anio de nacimiento alumnos de los cuales quieres modificar:\n");
	for(h=0;h<1;h++){
		if((dts[h]).edad == 1995){
			(dts[h]).edad = 6;
			printf("Edad: %d", (dts[h]).edad);
	    }
    }
}

void borrar(struct lista *datos){
	int h;
	for(h=0;h<3;h++){
	printf("Los datos a borrar son:\n ");
	printf("Dia de nacicmiento: %d",(datos[h]).dia_naci);
    }
    (datos[h]).dia_naci = 0;
}
void comparar(struct empleado *infor){
	float mayor = 0.0;
	float menor = 9999.0;
	int h;
	int pmay;
	int pmen;
	if((infor[h]).sueldo > mayor){
		mayor = (infor[h]).sueldo;
		h=pmay;
	}
	if((infor[h]).sueldo < menor){
		menor = (infor[h]).sueldo;
		h=pmen;
	}
	printf("\n\n %s es el empleado con MAYOR sueldo\n", (infor[pmay]).nombre);
	printf("\n\n %s es el empleado con MENOR sueldo\n", (infor[pmen]).nombre);
	
	//A cada elemento de esta estructura (nombre, apellido, teléfono, edad) se le denomina campo o miembro,
	 y al conjunto de los datos se le denomina registro.
	 char c;
	 while( (c = getchar())!= '\n' )  //buffer como funcionan
	 	putchar( c );
}
*/	 
	 #include <stdio.h>

struct estructura_amigo {
	char nombre[30];
	char apellido[40];
	char telefono[10];
	int edad;
	};

main()
{
	struct estructura_amigo *p_amigo;

	p_amigo = &amigo;

	/* Introducimos los datos mediante punteros */
	printf("Nombre: ");
	gets(p_amigo->nombre);
	printf("Apellido: ");
	gets(p_amigo->apellido);
	printf("Edad: ");
	scanf( "%i", &p_amigo->edad );

	/* Mostramos los datos */
	printf( "Mi amigo %s ", p_amigo->nombre );
	printf( "%s tiene ", p_amigo->apellido );
	printf( "%i años.\n", p_amigo->edad );
}     
	
/*
//Ahora veamos el ejemplo anterior de cómo introducir datos en un array de estructuras mediante punteros:


#include <stdio.h>

#define ELEMENTOS	3

struct estructura_amigo {
	char nombre[30];
	char apellido[40];
	char telefono[10];
	int edad;
	};

main()
{
	struct estructura_amigo amigo[ELEMENTOS], *p_amigo;
	int num_amigo;

	p_amigo = amigo;	/* apunta al primer elemento del array */

	/* Introducimos los datos mediante punteros */
	for( num_amigo=0; num_amigo<ELEMENTOS; num_amigo++ ) {
		printf( "\nDatos del amigo %i:\n", num_amigo+1 );

		printf( "Nombre: " );
		gets(amigo[num_amigo].nombre);
		printf( "Apellido: " );
		gets(amigo[num_amigo].apellido);
		printf( "Teléfono: " );
		gets(amigo[num_amigo].telefono);
		printf( "Edad: " );
		scanf( "%i", &amigo[num_amigo].edad );

		while(getchar()!= '\n');	/* Vacía el buffer de entrada */

		p_amigo++;			/* Siguiente elemento */

	}

	/* Imprimimos los datos */
	p_amigo = amigo;

	for( num_amigo=0; num_amigo<ELEMENTOS; num_amigo++ ) {

		printf( "Mi amigo %s ", p_amigo->nombre );
		printf( "%s tiene ", p_amigo->apellido );
		printf( "%i años ", p_amigo->edad );
		printf( "y su teléfono es el %s.\n" , p_amigo->telefono );

		p_amigo++;  
	}
}




//En el ejemplo siguiente se usa una función llamada suma que calcula cual será la edad 20 años más tarde
//(simplemente suma 20 a la edad). Esta función toma como argumento la variable estructura arg_amigo.
 // Cuando se ejecuta el programa llamamos a suma desde main 
  //y en esta variable se copia el contenido de la variable amigo.
#include <stdio.h>

struct estructura_amigo {
	char nombre[30];
	char apellido[40];
	char telefono[10];
	int edad;
	};

struct estructura_amigo amigo = {
	"Juanjo",
	"López",
	"983403367",
	30
	};

int suma( struct estructura_amigo );

main()
{
	printf( "%s tiene ", amigo.apellido );
	printf( "%i años ", amigo.edad );
	printf( "y dentro de 20 años tendrá %i.\n", suma(amigo) );
}

int suma( struct estructura_amigo arg_amigo )
{
	return arg_amigo.edad + 20;
}
