#define long 28	
int main () 
{
	int serie [] = {1,2,4,6,8,7,9};
	char resul = 0;
	int suma = 0, j = 0;
	do {
		suma = suma + serie[j];
		j++;
	} while (j < 7);
	resul = suma;
return 0;
}
