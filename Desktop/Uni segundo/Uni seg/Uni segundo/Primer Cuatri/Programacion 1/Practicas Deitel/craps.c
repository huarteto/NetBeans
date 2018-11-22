#include <stdio.h>

struct empleado{
	char nombre[30];
	char sexo[10];
	float sueldo;
};

int main(){
	struct empleado infor[3];
	int i;
	float mayor = 0.0;
	float menor = 9999.0;
	int pmay;
	int pmen;
	
	for(i=0;i<3;i++){
		printf("Introduce el nombre del empleado:");
		scanf("%s", (infor[i]).nombre);
		printf("Introduce el sexo del empleado:");
		scanf("%s", (infor[i]).sexo);
		printf("Introduce el sueldo del empleado:");
		scanf("%f", &(infor[i]).sueldo);
	}
	for(i=0;i<3;i++){	
		printf("Nombre: %s || Sexo: %s || Sueldo: %f\n", (infor[i]).nombre, (infor[i]).sexo, (infor[i]).sueldo);
    }
    if((infor[i]).sueldo > mayor){
		mayor = (infor[i]).sueldo;
		i=pmay;
	}
	if((infor[i]).sueldo < menor){
		menor = (infor[i]).sueldo;
		i=pmen;
	}
	printf("\n\n %s es el empleado con MAYOR sueldo\n", (infor[pmay]).nombre);
	printf("\n\n %s es el empleado con MENOR sueldo\n", (infor[pmen]).nombre);
return 0;
}
