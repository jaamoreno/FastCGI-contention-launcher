#ifndef CHECKFASTCGI_H
#define CHECKFASTCGI_H

#if defined(__linux__)
#include <stdint.h> 
#endif

#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

// JAM INI porting linux
#include <sys/types.h>
#include <sys/stat.h>
// JAM FIN porting linux

#include "api_shm.h"
#include "control_shm.h"

#define MAXPATH 256
#define MAXBUF 1024

#define MAX_SHM		25
#define MAX_PID		25
#define MAX_SEED_NAME	250
#define  EOL  		'\n'

#define CODCONFIGURACION 1
#define CODNOSUPERVISOR  2
#define CODNOSHM         3
#define CODNOSEM         4
#define CODNOCGI         5
#define CODNOSTARTLOG    6
#define CODNOSTARTSTA    7

#define exists(filename) (!access(filename, F_OK))  /* -- comprobacion existencia fichero -- */
#define vOn 1
#define vOff 0

#define retorno(r) (verbose==vOff?return r)

typedef struct
{
	char logName[MAXPATH];			/* nombre del fichero de log */
	char logFile[MAXPATH];			/* nombre del fichero de log con mascara de fecha */
	char cgiProgram[MAXPATH];		/* cgi a lanzar */
	char statisticsName[MAXPATH];		/* nombre del fichero de estadisticas */
	char statisticsFile[MAXPATH];		/* nombre del fichero de estadisticas con mascara de fecha */	
	int traceLevel;				/* nivel de traza */
	int childTimeOut; 			/* time out para los hijos */	
	int maxProcess; 			/* numero de procesos */			
	int maxBusyProcess; 			/* limite de procesos ocupados */		
	int maxSoftBusyProcess; 		/* limite de 1er nivel de procesos ocupados */
	long IPCKEY;				/* clave ipc para la memoria compartida */	
	char paginaErrorGenerico [MAXPATH]; 	/* url de la pagina de error generico + timeout */
	char paginaErrorSoft [MAXPATH]; 	/* url de la pagina de error al superar el limite soft */
	char paginaErrorHard [MAXPATH]; 	/* url de la pagina de error al superar el limite hard */
	char stats[MAXPATH]; 			/* flag de activacion de las estadisticas */
} startConfiguration;

typedef struct
{
	int ListenPort;
	int SupervisorInterval;
	int Instalations;
	key_t Ipckey[MAX_SHM];
	char IpcFile[MAX_SHM][MAX_SEED_NAME];
	char Name[MAX_SHM][MAXPATH];
	int tamanio[MAX_SHM];
	int max_procesos[MAX_SHM];
	int maxBusyProcess[MAX_SHM];
	int maxSoftBusyProcess[MAX_SHM];
	int timeKill;	
	char pidFile[MAXPATH];
	char FicheroTraza[MAXPATH];
} supConfiguration;

#endif
