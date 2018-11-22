#include <stdio.h>

struct lista {
	char nombre [30];
	int dia_naci;
	int mes_naci;
	int anio_naci;
}datos[3];


void introducir(struct lista *datos);
void leer(struct lista *datos);
void modificar(struct lista *datos);

int main() {
	introducir(datos);
	modificar(datos);
	
return 0;
}

void introducir(struct lista *datos){
	int i;
	for (i=0; i<3; i++) {
		printf("Introduce el nombre: ");
		scanf("%s", (datos[i]).nombre);
		printf("Introduce el dia: ");
		scanf("%d", &(datos[i]).dia_naci);
		printf("Introduce el mes: ");
		scanf("%d", &(datos[i]).mes_naci);
		printf("Introduce el anio: ");
		scanf("%d", &(datos[i]).anio_naci);
			
	}
}

void leer(struct lista *datos){
	int j;
	for(j=0;j<3;j++){
	printf("Nombre: %s || Dia: %d || Mes: %d || Anio: %d\n ", (datos[j]).nombre, (datos[j]).dia_naci, (datos[j]).mes_naci, (datos[j]).anio_naci);	
    
    }
}

void modificar(struct lista *datos){
	int h;
	printf("Introduce el año de nacimiento alumnos de los cuales quieres modificar:\n");
	for(h=0;h<1;h++){
		if((datos[h]).anio_naci == 1995){
			(datos[h]).mes_naci = 6;
			printf("Mes: %d\n", (datos[h]).mes_naci);
	    }
    }
}


