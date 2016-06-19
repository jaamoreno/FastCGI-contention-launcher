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
  	


/*printf("HTTP/1.1 301 Moved Permanently\n");	*/


/*printf("Location: http://www.gfi.es/\n");*/
  	
		printf("Content-type: text/html\n");
/*		printf("Content-type: text/plain; charset=iso-8859-1\n"); */
		printf("Set-Cookie: CGISD=1003233060481211252927427906;PATH=/;HttpOnly;secure;DOMAIN=172.28.170.92;\n");
		printf("Content-type: text/html\n");		
		printf("\n");
		printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
		
/*		
		printf("<html>\n");
		printf("<head>\n");
		printf("	<title>Titulo</title>\n");
		printf("</head>\n");
		printf("<body>\n");
		printf("</body>\n");
		printf("</html>\n");  	  	  						
	*/	
		printf("\n<HTML><HEAD><TITLE>echoCGI3</TITLE></HEAD><BODY>\n");
		

		
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

/*		sleep(slp);   */
/*		sleep(45);   */
		return 0;        

}


