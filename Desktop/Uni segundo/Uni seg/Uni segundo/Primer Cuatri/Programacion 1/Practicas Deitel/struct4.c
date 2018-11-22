#include <stdio.h>

struct datos {
	char nombre[40];
	char pais[25];
};
struct atleta {
	char deporte[30];
	struct datos pers;
	int medallas;
}ats[3];

void sacatela(struct atleta *ats);
void sacar(struct atleta *ats);
void buscar(struct atleta *ats);
int main () {
	sacatela(ats);
	buscar(ats);
return 0;
}

void sacatela(struct atleta *ats){
	int i;
	for(i=0;i<3;i++){
		printf("Introduce el nombre del atleta:");
		scanf("%s",(ats[i]).pers.nombre);
		printf("Introduce el pais del atleta:");
		scanf("%s", (ats[i]).pers.pais);
		printf("Introduce el deporte que hace:");
		scanf("%s", (ats[i]).deporte);
    }
}
void sacar(struct atleta *ats){
	int j;
	for(j=0;j<3;j++){
		printf("Nombre: %s", (ats[j]).pers.nombre);
	}
}
void buscar(struct atleta *ats){
	char nombre_temporal;
	int h;
	printf("Introduce el nombre del atleta a buscar:");
	scanf("%s", &nombre_temporal);
	for(h=0;h<3;h++){
		if(((ats[h]).pers.nombre) == nombre_temporal){
			printf("Nombre: %s", (ats[h]).pers.nombre);
			printf("Pais: %s", (ats[h]).pers.pais);
			printf("Deporte: %s", (ats[h]).deporte);	
		}
	 }
}
