#include <stdlib.h>
#include <stdio.h>


#include <unistd.h>

extern char **environ;

int main(int argc, char *argv[])
{			
    int count = 0;
    int i,nbytes;
    int slp;    
char entrada[1024];
    
  	/* initialize random seed: */
  	srand (time(NULL));    
  	

		
		slp = rand() % 12 + 3;  	
    
		printf("Content-type: text/plain; charset=iso-8859-1\r\n"
		"\r\n");
		
		printf("CGI - Tiempo aleatorio: [%d] <br>\n",slp);
		
		printf("CGI - Lista de argumentos: <br>\n");		
    
		for(i = 1; i < argc; i++)
		{
			printf("%s <br>\n",argv[i]);
		}	
		
		printf("CGI - Lista de variables de entorno: <br>\n");		
		for(i = 0; environ[i] != NULL; i++)
		{			
			printf("%s <br>\n", environ[i]);
			
		}
		
		/*mostramos stdin*/  
/* JAM PRUEBAS
		if ( (nbytes = read(STDIN_FILENO, entrada, sizeof(entrada)-1)) > 0)
		{
			 printf("Stdin: <br>\n");
	 		 while( (nbytes = read(STDIN_FILENO, entrada, sizeof(entrada)-1)) > 0)
			{
				entrada[nbytes] = 0;
				printf("%s",entrada);
			}	
		}
		else
			printf("<H1> NO STDIN </HI>");
*/
		while ( (nbytes = read( fileno(stdin), entrada, sizeof(entrada)-1 )) >0 )
		if (nbytes)
		{
			entrada[nbytes]='\0';
			printf("nbytes[%d]\n", nbytes);
			printf("entrada[%s]\n", entrada);
		}
		else
			printf("no hay entrada\n");
	

/*		sleep(slp);  */
		sleep(5); 
		return 0;        

}
