21. Que lea una cadena y diga cuantas vocales hay.

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    int sum=0,x;
    char frase[50];
   
    printf("Introduzca una frase: ");
    gets(frase);
   
    for(x = 0;x < 50;x++)
    {
      switch (frase[x])
      {
           case 'a':
                sum++;
                break;
           case 'e':
                sum++;
                break;
           case 'i':
                sum++;
                break;
           case 'o':
                sum++;
                break;
           case 'u':
                sum++;
                break;            
           default:
                break;
      }
     
    }





20. Que lea una cadena y la muestre al revés.

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    int indice,x;
    char frase[50];
   
    printf("Introduzca una frase: ");
    gets(frase);
   
    for(x = 0;x < 50;x++)
    {
      if (frase[x]=='\0')
      {
       indice=x;
       break;
      }
    }
   
    printf("La frase al reves es: \n\n");
   
    for(x = indice-1;x >=0;x--)
    {
      printf("%c",frase[x]);
    }
   
    printf("\n\n");
   
    system("PAUSE");     
    return 0;
}



14. Que rellene un array con 20 números y luego busque un número concreto. 

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    int  i,x=0,vector[20], n=20, dato, centro,inf=0,sup=n-1;
  
    for (i=0;i<20;i++){
        printf("Escriba un número");
        scanf("%d",&vector[i]);
    }
  
    printf("Escriba el número a buscar");
    scanf("%d",&dato);
  
    while(inf<=sup)
    {
     centro=(sup+inf)/2;
     if (vector[centro]==dato)
     {
        printf("Existe\n");
        x=1;
        break;
     } 
     else if(dato < vector [centro] )
     {
        sup=centro-1;
     }
     else
     {
       inf=centro+1;
     }
    }
   
    if (x==0)
    {
       printf("No existe\n");     
    }
  
    system("PAUSE");     
    return 0;

}






13. Que mediante un array almacene números tanto positivos como negativos y los muestre ordenados.

#include <stdio.h>
#include <stdlib.h>

int main()
{
  float aux, numeros[10];
  int i,j,n=10;

  for (i=0;i<n;i++){
                    printf("Escriba un número");
                    scanf("%f",&numeros[i]);
      }

  for(i=0;i<n-1;i++)
  {
   for(j=i+1;j<n;j++)
   {
       if(numeros[i]<numeros[j])
       {
           aux=numeros[i];
           numeros[i]=numeros[j];
           numeros[j]=aux;
       }
   }
  }
 
  for (i=n-1;i>=0;i--){
      printf("%f\n",numeros[i]);
  }
     
  system("PAUSE");     
  return 0;
}






#include <stdio.h>
05
06 void cuboPorReferencia( int *ptrN ); /* prototipo */
07
08 int main()
09 {
10 int numero = 5; /* inicializa numero */
11
12 printf( “El valor original de numero es %d”, numero );
13
14 /* pasa la dirección de numero a cuboPorReferencia */
15 cuboPorReferencia( &numero );
16
17 printf( “\nEl nuevo valor de numero es %d\n”, numero );
18
19 return 0; /* indica terminación exitosa
} /* fin de main */
22
23 /* calcula el cubo de *ptrN; modifica la variable numero en main */
24 void cuboPorReferencia( int *ptrN )
25 {
26 *ptrN = *ptrN * *ptrN * *ptrN; /* cubo de *ptrN */
27 } /* fin de la función cuboPorReferencia */
