

#include "stdio.h"

int medida;
int c1;
int c2;

int main(void){

	printf("introducir lado del cuadrado: ");
	scanf("%d", &medida);
	c1=1;c2=1;
 
		while(c2<=medida){
			c1=1;
		
			if(c2%2!=0){
		
				while(c1<=medida){
					if(c1%2==0){
						printf("%s","B");
						c1++;
				}else{
					printf("%s","N");
					c1++;
				}
			
				}
					printf("\n");
					c2++;
				}else{
					while(c1<=medida){
						if(c1%2==0){
							printf("%s","N");
							c1++;
						}else{
							printf("%s","B");
							c1++;
						}			
					}
					printf("\n");
					c2++;
		
				}	
		}
  printf("\n");
  return 0;
	
}
