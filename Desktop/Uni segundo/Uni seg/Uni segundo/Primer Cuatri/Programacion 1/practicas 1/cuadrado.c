
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
		
			while(c1<=medida){
				printf("%s","*");
				c1++;
			}
		
		printf("\n");
		c2++;
		}
  printf("\n");
  return 0;
	
}
