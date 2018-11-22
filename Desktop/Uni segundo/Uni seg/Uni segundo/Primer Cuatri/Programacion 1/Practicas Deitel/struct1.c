/*) Un programa que pida datos de 8 personas: nombre, dia de nacimiento, 
 * mes de nacimiento, y año de nacimiento (que se deben almacenar en una tabla de structs). 
 * Después deberá repetir lo siguiente: preguntar un número de mes y
 *  mostrar en pantalla los datos de las personas que cumplan los años durante ese mes.
 * Terminará de repetirse cuando se teclee 0 como número de mes. */

#include <stdio.h>

struct lista {
	char nombre [30];
	int dia_naci;
	int mes_naci;
	int anio_naci;
}datos[3];

void mes(struct lista *datos);
void introducir(struct lista *datos);
void borrar(struct lista *datos);
void modificar(struct lista *datos);
int main (){
	introducir(datos);
	mes(datos);
	borrar(datos);
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

void mes(struct lista *datos) {
	int mes_naci_escrito;
	int j;
	printf("Introduce el mes temporal: ");
	scanf("%d", &mes_naci_escrito);
	if(mes_naci_escrito == 0){
		printf("Hasta otra\n");
	}else{
		for(j=0; j<3; j++) {
			if((datos[j]).mes_naci == mes_naci_escrito){
				printf("Nombre: %s || Dia: %d || Mes: %d || Anio: %d\n ", (datos[j]).nombre, (datos[j]).dia_naci, (datos[j]).mes_naci, (datos[j]).anio_naci);	
		    }
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

void modificar(struct lista *datos){
	int u;
	for(u=0;u<1;u++){
		printf("El dato a modificar es:\n");
		printf("Año: %d", (datos[u]).anio_naci);
		(datos[u]).anio_naci = 1998;
		printf("Aqui tenemos el año modificado\n ");
		printf("Año: %d",(datos[u]).anio_naci);
		
	}	
}
