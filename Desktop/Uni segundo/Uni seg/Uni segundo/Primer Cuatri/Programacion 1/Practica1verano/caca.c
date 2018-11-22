#include "stdio.h"

int num;
int i;
int primo=0;


int main (void){
	printf("escribir numero: ");
	scanf("%d", &num);
	
		for(i=1;i<=num;i++){
			if(num%i==0){
				primo++;
			}
		}
		if(primo==2){
			printf(" numero  primo");
		}else{
			printf("numero no primo");
		}
	
printf("\n");
return 0;
}
