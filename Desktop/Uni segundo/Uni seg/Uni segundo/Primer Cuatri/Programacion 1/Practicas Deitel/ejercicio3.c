/* Ejercicio 3 */

#include <stdio.h>
/*Aqui hemos creado una estructura o struct llamada datos la cual almacena el nombre,
 apellidos, fecha de nacimiento y la lista de tareas de un empleado.Como solo nos piden un empleado 
 inicializamos la info que vamos a usar como puntero en las funciones en 1*/
struct datos{
	char nombre[30];
	char apellidos[50];
	char fecha_nac[30];
	char lista_tareas[40];
}info[1];

void introducirlos(struct datos *info);
void llamarlos(struct datos *info);

int main(){
	introducirlos(info);
	llamarlos(info);
	
return 0;
}
/* Esta es la primera funcion llamada introducirlos la cual relaiza un puntero a info del struct, 
 * se inicializa en un contador llamado i, en este caso ocmo solo no spiden un empleado no seria necesario usarlo
 * ni escribirlo pero si te piden varios se utiliza para que se repita tantas veces como quieras gracias al for
 * Una vez dentro del for tenemos las Instrucciones printf y scanf que imprimen y llaman a la variable que elegimos
 * en cada uno de ellos(nombre,apellidos etc...).*/
void introducirlos(struct datos *info){
	int i;
	for(i=0;i<1;i++){
		printf("Introduce el nombre del empleado:");
		scanf("%s",(info[i]).nombre);
		printf("Introduce el apellido del empleado:");
		scanf("%s", (info[i]).apellidos);
		printf("Introduce la fecha de nacimiento del empleado:");
		scanf("%s", (info[i]).fecha_nac);
		printf("Introduce la liasta de tareas del empleado:");
		scanf("%s", (info[i]).lista_tareas);
    }
}	
/* Esta es la segunda funcion llamada "llamarlos", que como su propio nombre indica una vez hemos 
 * introducido todos los datos con la anterior funcion, esta los llama para que aparezcan en pantalla.
 * Tambien tiene otro contador llamado j que sirve para lo mismo que he explicado en la anterior funcion
 * y en este caso no seria necesario usarlo.*/
 
void llamarlos(struct datos *info){
	int j;
	for(j=0;j<1;j++){
		printf("|| Nombre: %s\n || Apellidos: %s\n || Fecha nacimiento: %s\n || Lista de Tareas: %s\n", (info[j]).nombre, (info[j]).apellidos, (info[j]).fecha_nac, (info[j]).lista_tareas);
    }
}
