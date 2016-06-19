#include <stdlib.h>
#include <stdio.h>


#include <unistd.h>

extern char **environ;

#define MAX_SIZE_QUERY_STRING 8192
#define lengthBuffer(x) (sizeof(x)/sizeof(x[0]))

int main(int argc, char *argv[])
{			
    int count = 0;
    int i,nbytes;
    int slp;    char entrada[1024];

char	QUERY_STRING[MAX_SIZE_QUERY_STRING];
    
  	/* initialize random seed: */
  	srand (time(NULL));    
  		
		printf(""
/*			"<HTTP>" 		*/
/*			"HTTP/1.0 200 OK"	*/
			"Content-Type: text/html; charset=iso-8859-1\n" 
			"Set-Cookie: CGISD=1003215043181313342170929109;PATH=/;HttpOnly;secure;DOMAIN=194.140.1.49;\n"

/*			"</HTTP>\n"  */
			""); 
					
		
		printf("\n<HTML><HEAD><TITLE>echoCGI2</TITLE></HEAD><BODY>\n");
		

		
		slp = rand() % 12 + 3;  	
    
/*		printf("Content-type: text/plain; charset=iso-8859-1\r\n"
		"\r\n"); */
		
		printf("CGI - Tiempo aleatorio: [%d] <br>\n",slp);
		
		printf("CGI - Lista de argumentos: total = [%d] argv[0]=%s<br>\n",argc,argv[0]);		
    
		for(i = 1; i < argc; i++)
		{
			printf("argv[%d]=[%s] <br>\n",i,argv[i]);
		}	
		
		printf("CGI - Lista de variables de entorno: <br>\n");		
		for(i = 0; environ[i] != NULL; i++)
		{			
			printf("%s <br>\n", environ[i]);
			
		}
		
		/*mostramos stdin*/  
		fgets(QUERY_STRING, lengthBuffer(QUERY_STRING), stdin);
		printf("QUERY_STRING=[%s]\n",QUERY_STRING);
	
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



		printf("</BODY></HTML>");

/*		sleep(slp);  */
		sleep(1); 
		return 0;        

}
