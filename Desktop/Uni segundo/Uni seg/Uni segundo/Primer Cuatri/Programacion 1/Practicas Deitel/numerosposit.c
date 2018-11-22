#include <stdio.h>
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
	scanf("%s", (dato)->nombre);   
	printf("Introduce la nota: ");
	scanf("%f", &(dato)->nota);
}


}
