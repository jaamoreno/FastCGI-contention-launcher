#ifndef START_H
#define START_H

#include <time.h>
#include <stdint.h> /* para linux*/

/*fast cgi */
#include "fcgi_stdio.h"
#include "fcgi_config.h"
#include "fcgiapp.h"		

/* parsear ficheros.ini */		
#include "ini.h" 

/*memoria compartida*/
#include "api_shm.h"

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define START_VERSION	"1.2"	/* version del binario 2014-10-22 */
#define MAXPATH 256
/* #define MAXBUF 4096 */
#define MAXBUF 16384
#define MAX_ENV  512
#define MAX_ARG   64
#define MAX_ABT  100
#define MAX_SD  	32

	/* valores por defecto */
#define	DEFAULTTRACELEVEL  100		/*nivel de traza */
#define	DEFAULTCHILDTIMEOUT  15		/*time out para los hijos*/	
#define	DEFAULTLOGNAME "fcgi"		/*fichero de logs */
#define	DEFAULTSTATISTICSNAME "statistics"  /*fichero de estadisticas */
#define DEFAULTCGIPROGRAM "ls"		/* cgi a lanzar */
#define DEFAULTCONFIGFILE "start.cfg"	/*ini por defecto  JM */
#define DEFAULTMAXPROCESS 	10	/*numero maximo de procesos por defecto*/
#define DEFAULTMAXBUSYPROCESS 10
#define DEFAULTMAXSOFTBUSYPROCESS 5
#define DEFAULTIPCKEY	12345
#define DEFAULTIPCDIR "/usr/sbin/portmap" /*fichero por defecto para generar la key*/
#define DEFAULTTRACE	"Off" 


 /* niveles de traza */
#define ERRORL 10
#define WARNL  20
#define INFOL  30
#define DEBUGL 40

/*tipos de fecha*/
#define TFICHERO 0	/* tipo fichero*/
#define TTRAZA 	 1	/* tipo traza*/
#define TEPOCH   2	/* tipo EPOCH */
#define TEPOCHMS 3	/* tipo EPOCH en milisegundos*/

/* retorno de la ejecucion CGI*/
#define OK			0
#define TIMEOUT			9
#define ERRORGEN			15				 /*el hijo da error*/
#define MAXHARDPROCESS  99   /*error cuando se llega al limite duro de procesos ocupados*/
#define MAXSOFTPROCESS  66   /*error cuando se llega al limite blando de procesos ocupados*/

#define CONTENT_TYPE_TAG	"Content-type: "
#define NOSESIONTEXT 			"_SD=SD"

#define TRACEON "On"
#define TRACEOFF "Off"

#define exists(filename) (!access(filename, F_OK))  /*comprobar si existe un fichero*/
 

typedef struct
{
	char logName[MAXPATH];	/*nombre del fichero de log */
	char logFile[MAXPATH];	/*nombre del fichero de log con mascara de fecha */
	char cgiProgram[MAXPATH];	/* cgi a lanzar */
	char statisticsName[MAXPATH]; /*nombre del fichero de estadisticas*/
	char statisticsFile[MAXPATH]; /*nombre del fichero de estadisticas con mascara de fecha */	
	int traceLevel;	/* nivel de traza */
	int childTimeOut; /*time out para los hijos*/	
	int maxProcess; /*numero de procesos*/			
	int maxBusyProcess; /*limite de procesos ocupados*/		
	int maxSoftBusyProcess; /*limite de 1er nivel de procesos ocupados */
	long IPCKEY;				/*clave ipc para la memoria compartida*/	
	char paginaErrorGenerico [MAXPATH]; /*url de la pagina de error generico + timeout*/
	char paginaErrorSoft [MAXPATH]; /*url de la pagina de error al superar el limite soft*/
	char paginaErrorHard [MAXPATH]; /*url de la pagina de error al superar el limite hard*/
	char stats[MAXPATH]; 		/*flag de activacion de las estadisticas*/
} configuration;


typedef struct
{
	int tPos;									/*posicion del proceso en la tabla de shm*/
	dat_proceso dProcess;			/*datos del proceso SHM*/
	char fInicio[MAXPATH];							/*fecha de inicio en formato EPOCH*/
	char hInicio[MAXPATH];							/*hora de inicio en milisegundos*/
	char fFin[MAXPATH];								/*fecha de fin en formato EPOCH*/		
	char hFin[MAXPATH];								/*hora final en milisegundos*/	
	int nProcesosReg;						/*numero de procesos registrados en el momento de la ejecucion*/
	int status;								/*estado de la ultima ejecucion */
} fcgiProcess;

/*void traza(char const *msg,...); */
void trazaProceso(fcgiProcess p);
void traza(int level,char const *msg,...);
void readConfig();
void readConfigSignal();
char* getDate(int type);				/*funcion que devuelve la fecha actual*/
long getDateNum(int type);				/*funcion que devuelve la fecha en formato numerico*/
int checkBusyProcess();					/*funcion que comprueba si hemos llegado al limite de procesos*/
int getBusyProcess();						/*funcion que devuelve el numero de procesos ocupados*/
int tieneSesion();							/*funcion que comprueba si una peticion ya tiene sesion */

/* void getIn();									*/	/*funcion que lee el stdin y lo guarda en entrada */ 
int getIn();
void errorPage (int error);			/*funcion que recibe un codigo de error y muestra la pagina correspondiente*/
void incrementaContPro(int pos,	long pid,int contador);/* funcion que incrementa y actualiza el contador que recibe como parametro*/
char* getStatisticsFileName();


#endif
