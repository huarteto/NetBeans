#include <stdio.h>

struct empleado{
	char nombre[30];
	char sexo[10];
	float sueldo;
}infor[3];

void cojer(struct empleado *infor);
void sacar(struct empleado *infor);
void comparar(struct empleado *infor);

int main(){
	cojer(infor);
	sacar(infor);
	comparar(infor);
return 0;
}

void cojer(struct empleado *infor){
	int i;
	for(i=0;i<3;i++){
		printf("Introduce el nombre del empleado:");
		scanf("%s", (infor[i]).nombre);
		printf("Introduce el sexo del empleado:");
		scanf("%s", (infor[i]).sexo);
		printf("Introduce el sueldo del empleado:");
		scanf("%f", &(infor[i]).sueldo);
	}
}
void sacar(struct empleado *infor){
	int j;
	for(j=0;j<3;j++){	
		printf("Nombre: %s || Sexo: %s || Sueldo: %f\n", (infor[j]).nombre, (infor[j]).sexo, (infor[j]).sueldo);
    }
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
}

