#include <stdio.h>
#include <stdlib.h>


struct lista {
    char nombre[50];
    float nota;
}dato[3];
void recoger(struct lista *dato);
void escribir(struct lista *dato);
void buscar (struct lista *dato);
int main() {
	recoger(dato);
	escribir(dato);
	buscar(dato);	
return 0;
}

void recoger (struct lista *dato) {
	int i=0;
	
	for(i=1; i<=3; i++){
	printf("Introduce el nombre: ");
	scanf("%s", (dato[i]).nombre);   
	printf("Introduce la nota: ");
	scanf("%f", &(dato[i]).nota);
}


}
void escribir(struct lista *dato){
	int j;
	for(j=1; j<=3; j++){
 printf("Nombre: %s  || Nota: %f \n ", (dato[j]).nombre, (dato[j]).nota);
}
}

void buscar (struct lista *dato) {
	int h; // contador;
	char nombreTemporal;
	printf("Introduce el nombre a buscar: ");
	scanf("%s", &nombreTemporal);
	for(h=1;h<=3;h++){
		if((dato[h]).nombre == nombreTemporal){
			printf("Nombre: %s || Nota: %f \n ", (dato[h]).nombre,(dato[h]).nota);
		}		
	}	
}

