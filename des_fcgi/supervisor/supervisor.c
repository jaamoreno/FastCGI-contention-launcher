
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <errno.h>

#include "tcputil.h"
#include "arqtraza.h"
#include "api_shm.h" 
#include "procesos.h"

#include "supervisor.h"


/*
** Array de sockets encargados de atender
** las peticiones desde clientes tcp. 
*/
sockinfo_t sockets[MAX_OPENED_SOCKETS];

/*
** Para terminar el bucle de SocketsEventLoop.
*/
int stop_loop = 0;

int supHistorico;	/*indica si es un supervisor de historico*/

static int port;

static configuration config;

static int contador_lectura;
static int timeKill;
static char pidFile[MAX_PID_FILE];
static char FicheroTraza[MAX_PID_FILE];
static char FichSesiones[MAX_PID_FILE];
static char ColTotSes[MAX_PID_FILE];

static 	tabla_shm *tabla[MAX_SHM];

char buffer[MAX_BUFFER]="";

static int flagrefresco = 0;

static char configuracion[MAX_PATH];	/* nombre del fichero de configuracion */

int maxpOcupados = 0; 	/*maximo numero alcanzado de procesos ocupados*/

/*****************************************************************************/
void main(int argc, char *argv[])
/*****************************************************************************/
{
   int  highest_sd = 0;
   int  i, p, ret, sd_aux, contcfg;   
   struct sigaction act,oact;

   long interval;
   char strport[MAXPORT_LEN + 1];
   char *strinterval;
   char fichtrzname[MAXFILETRZ_LEN + 1];
   

   
   /*
   ** Trazas.
   */
   if (argc > 1)
   {
	if( !strcmp(argv[1], "-V") )
	{
		printf("supervisor version: %s\n", SUPERVISOR_VERSION);
		return;	
	}
	else
		strcpy(configuracion, argv[1]);
   }
   else
	strcpy(configuracion, CONFIG_FILE);

   readConfigTraza(configuracion);
  
   sprintf(fichtrzname, "%.*s.log", MAXFILETRZ_LEN - strlen(".log"), FicheroTraza);

   ret=ARQTrazaIni(fichtrzname);
   if (ret != 0)
	return;
 
   /*
   ** Capturar la senyal de interrupcion.
   */
   ARQLog(ALL_TRACE, "%s::Capturando senial de interrupcion.", argv[0]);
   signal(SIGTERM, CleanUp);
   signal(SIGUSR2, flagPropagaRefresco);
   signal(SIGPIPE, SIG_IGN);
   
   /*
   ** Leer fichero de configuracion
   */

   readConfig(configuracion);
   
   /* comprobamos si es supervisor para historicos */
   supHistorico = (config.SupervisorInterval == INTERVALOHISTORICO);
   if (supHistorico)
   {
   	ARQLog(INF_TRACE, "Supervisor configurado PARA HISTORICOS.");
   	/*definimos el intervalo a un numero muy granda para que el bucle no consuma cpu*/
   	interval = MAX_INTERVAL;
   }
   else
   {
   	ARQLog(INF_TRACE, "Intervalo del supervisor definido con valor %d.", config.SupervisorInterval);
   	interval = config.SupervisorInterval;   	
   }   


   if (!supHistorico)
   {
   	if (config.Instalations > 0)
   	{
		contcfg=0;
		while (contcfg < config.Instalations)
		{
			if(readConfigPidfile(config.IpcFile[contcfg]))
			{
				ARQLog(ERR_TRACE, "readConfigPidfile::no se pudo abrir el fichero de configuracion %s", config.IpcFile[contcfg]);
				/* salir  */
				return ;
			}
			contcfg++;
		}
   	}   
   }
   
   /*Generar fichero pid*/
   EscribePidFile(pidFile);   
  
   ARQLog(BAS_TRACE, "supervisor version: %s", SUPERVISOR_VERSION); 
   ARQLog(BAS_TRACE, "Puerto de escucha: %d", config.ListenPort);

   port=config.ListenPort;
   sprintf(strport, "%.*d", MAXPORT_LEN, config.ListenPort);

   /* Fichero de sesiones (solo se utiliza en supervisores NO de historicos) */
   if (!supHistorico)
	 {
		   readConfigFichSesiones(configuracion);
	  	 if(strcmp(FichSesiones, ""))
	    		  ARQLog(BAS_TRACE, "Fichero de sesiones activas: %s", FichSesiones);
	   	 else
	      		ARQLog(BAS_TRACE, "No se ha establecido el fichero de sesiones activas");
	     
	   	 /* Columna total sesiones (dentro del fichero) */
	   	 readConfigColTotSes(configuracion);
	   	 if(strcmp(ColTotSes, ""))
	    	  	ARQLog(BAS_TRACE, "Columna de total para las sesiones activas: %s", ColTotSes);
	   	 else
	    	  	ARQLog(BAS_TRACE, "No se ha establecido ninguna columna de total para las sesiones activas");
   }

   /*
   ** Limpia el array de sockets inicializandolos a 0.
   */
   bzero(sockets, MAX_OPENED_SOCKETS*sizeof(sockinfo_t));
   
   /*
   ** Nos preparamos para escuchar peticiones de clientes.
   */

   if(!TCPSetupListenSocket(&sd_aux, strport))
	 {
      ARQLog(ERR_TRACE, "%s::Error en llamada a  TCPSetupListenSocket.", argv[0]);
      CleanUp(SIGINT);
      exit(EXIT_FAILURE);
   }

   /*
   ** Funcion que registra un socket asignandole una accion. Es decir cuando
   ** a dicho socket llegue un mensaje se ejecutara la accion que tiene
   ** asignada.
   */
   RegisterSocket(sd_aux, &highest_sd, ACCEPT_CALL);
   
   ARQLog(BAS_TRACE,
          "Socket de escucha para clientes establecido: ls = %d, high_sd = %d.",sd_aux, highest_sd);
   
   /* 
   ** Esperar eventos, en los sockets creados, y resolverlos.
   */
   ret = SocketsEventLoop(highest_sd, &interval);

   /*
   ** En un momento dado, el bucle de eventos finalizara devolviendo
   ** un determinado codigo de retorno que sera chequeado.
   */
   switch(ret) 
   {
      case EVENT_LOOP_ERROR:
	 			ARQLog(ERR_TRACE, "%s::Error interno del bucle de eventos.", argv[0]);
	 			break;
	 
      case EVENT_LOOP_STOPPED:
	 			ARQLog(ALL_TRACE, "%s::Bucle de eventos finalizado con normalidad...", argv[0]);
	 			break;
	 
      case EXEC_CANT_RECIVE:
	 			ARQLog(ERR_TRACE, "%s::Error al recibir un mensaje desde un cliente.", argv[0]);
	 			break;
	 
      case EXEC_CANT_SEND:
	 			ARQLog(ERR_TRACE, "%s::Error al mandar un mensaje desde un cliente.", argv[0]);
	 			break;
	 
      case EXEC_CANT_ACCEPT:
	 			ARQLog(ERR_TRACE, "%s::Error al aceptar una llamada desde un cliente.", argv[0]);
	 			break;
	 
      case EXEC_UNKNOW_ACTION:
	 			ARQLog(ERR_TRACE, "%s::Error por accion desconocida.", argv[0]);
	 			break;
   }
   
   /*
   ** Cerramos los sockets abiertos.
   */
   for(i = 0; i<MAX_OPENED_SOCKETS; i++) 
   {
      if(sockets[i].opened) {
         /*
         ** close(sockets[i].sd);
         */
         TCPDisconnectSocket(sockets[i].sd);
      }
   }
} /* main */


/* tratamiento de la configuracion */
static int handler(void* user, const char* section, const char* name, const char* value)
{
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	char str[250];
	char * pch;
	int indice=0;
	int nivel=0;

	configuration *pconfig = (configuration*)user;

	if (MATCH("General", "ListenPort")) 
		pconfig->ListenPort = atoi(value);
	else if (MATCH("General", "SupervisorInterval")) 
		pconfig->SupervisorInterval = atoi(value);
	else if (MATCH("General", "Instalations")) 
		pconfig->Instalations = atoi(value);
	else if (MATCH("General", "PidFile")) 
	{
		strcpy(pidFile, value);
	}	
	else if (MATCH("General", "RutaHistoricos")) 
	{
		strcpy(pconfig->rutaHistoricos, value);
	}		
	else if (MATCH("General", "TimeOutKill")) 
	{
		timeKill=atoi(value);
	}
	else if (MATCH("General", "TraceLevel")) 
	{
		nivel = atoi(value);
		TrzDefNivel(nivel);
		ARQLog(ALL_TRACE, "parseIni::Nivel de trazas = %d", nivel);
	}
	else if (MATCH("Instalations", "StartConfigFiles")) 
	{
		strcpy(str, value);
		pch = strtok (str,",");
		while (pch != NULL)
		{
			strcpy(pconfig->IpcFile[indice], pch);
			/*	pconfig->Ipckey[indice]=ftok(config.IpcFile[indice],SEED);  detectado error, PROBAR*/
			pconfig->Ipckey[indice]=ftok(pconfig->IpcFile[indice],SEED);
			ARQLog(ALL_TRACE, "parseIni::ipc generada = %ld, %x, %d",(int)config.Ipckey[indice],(int)config.Ipckey[indice], errno);
			pch = strtok (NULL,",");
			indice++;
		}	
	}
	else if (MATCH("Instalations", "Names")) 
	{
		strcpy(str, value);
		indice=0;
		pch = strtok (str,",");
		while (pch != NULL)
		{
			strcpy(pconfig->Name[indice], pch);
			pch = strtok (NULL,",");
			indice++;
		}	
	}	
	else
	{
		/* 2-oct-2013 se retira la traza 
	     		ARQLog(ERR_TRACE, "parseIni::Clave no usada [%s] %s = %s", section, name, value);
		*/
		return 1;
	}

	return 0; 
}

static int handler_tamanio(void* user, const char* section, const char* name, const char* value)
{
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	char str[250];
	char * pch;
	int indice=0;
	dat_proceso dprocesos;
	dat_global dglobal;
	long tam;
	long tamanio;

	configuration *pconfig = (configuration*)user;
	
	if (MATCH("config", "maxProcess")) 
	{
		tam=atoi(value);
		tamanio=tam * sizeof(dprocesos);
		tamanio=tamanio+sizeof(dglobal);
		pconfig->tamanio[contador_lectura] = tamanio;
		pconfig->max_procesos[contador_lectura]=tam;
		/*contador_lectura++;*/
	}
	else if (MATCH("config", "maxBusyProcess")) 
	{				
		pconfig->maxBusyProcess[contador_lectura] = atoi(value);
	}
	else if (MATCH("config", "maxSoftBusyProcess")) 
	{				
		pconfig->maxSoftBusyProcess[contador_lectura] = atoi(value);
	}
	else
	{
		/* 2-oct-2013 se retira la traza 
                   ARQLog(ERR_TRACE, "handler_tamanio::Clave no usada [%s] %s = %s", section, name, value);
                */
		return 1;
	}

	return 0; 
}

static int handler_traza(void* user, const char* section, const char* name, const char* value)
{
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	char str[250];
	char * pch;
	int indice=0;
	int nivel=0;

	configuration *pconfig = (configuration*)user;

	if (MATCH("General", "FicheroTraza"))
			strcpy(FicheroTraza, value);
	else
			return 1;
	
	return 0;
}

static int handler_FichSesiones(void* user, const char* section, const char* name, const char* value)
{
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	char str[250];
	char *pch;
	int indice=0;
	int nivel=0;

	configuration *pconfig = (configuration*)user;

	if (MATCH("General", "FicheroSesiones"))
			strcpy(FichSesiones, value);
	else
			return 1;
	
	return 0;
}

static int handler_ColTotSes(void* user, const char* section, const char* name, const char* value)
{
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	char str[250];
	char *pch;
	int indice=0;
	int nivel=0;

	configuration *pconfig = (configuration*)user;

	if (MATCH("General", "IdCampoSesiones"))
			strcpy(ColTotSes, value);
	else
			return 1;
	
	return 0;
}

int readConfig(char* configFile)
{	     

	if (ini_parse(configFile, handler, &config) < 0)   /* la funcion parseIni es la que tiene la implementacion del parseo */
			return 1;
	else 	
			return 0;
}

int readConfigPidfile(char* configFile)
{	     
	if (ini_parse(configFile, handler_tamanio, &config) < 0)   /* la funcion parseIni es la que tiene la implementacion del parseo */
			return 1;
	else 	
			return 0;
}

int readConfigTraza(char* configFile)
{	     
	/* funciona pra establecer fichero de traza */
	if (ini_parse(configFile, handler_traza, &config) < 0)   
			return 1;
	else 	
			return 0;
}

int readConfigFichSesiones(char* configFile)
{	     
	/* funciona para establecer el fichero de sesiones */
	if (ini_parse(configFile, handler_FichSesiones, &config) < 0)   
			return 1;
	else 	
			return 0;
}

int readConfigColTotSes(char* configFile)
{	     
	/* funciona para establecer el fichero de sesiones */
	if (ini_parse(configFile, handler_ColTotSes, &config) < 0)   
			return 1;
	else 	
			return 0;
}

/* funcion que repasa la memoria compartida */
int checkSHM(void)
{
	int i;
	int ret;
	key_t key;
	long tamanio;
	int shm_creada;

	dat_configuracion conf;
	
	
	for( i=0; i < config.Instalations; i++)
	{
		key=config.Ipckey[i];
		tamanio=config.tamanio[i];
		
		if (tabla[i] == NULL)
		{
			conf.max_procesos=config.max_procesos[i];
			conf.clave=key;			
			ret=crea_tabla(key,conf);

			if (ret != 0)
			{
				ARQLog(INF_TRACE, "checkSHM::ERROR en lectura de memoria compartida");
				return 1;
			}
		}
		
		tabla[i] = con_tabla_sinSem(0);
		if (tabla[i] == NULL)
			ARQLog(ERR_TRACE, "checkSHM::ERROR en limpiado de procesos %d",ret);
		else
		{
			ret=Limpia_procesos_tabla(key,timeKill,tabla[i]);
			if (ret)
			{
				ARQLog(ERR_TRACE, "checkSHM::ERROR en limpiado de procesos %d",ret);
				return 1;
			}
		}
	}

	return 0;
}


/* Funcion para recuperar el num. de sesiones activas a través de un columna (var. "ColTotSes") de 
   un fichero de texto informado en la variable "FichSesiones". 
   En caso de error, la función devuelve 0
*/
int RecuperarTotSesiones(void)
{
	 FILE *fp;
	 char str[LINE_LEN];
	 char *pch;
	 int  iPos=1, iPos2, iSalida=0;
	 long size;
	 
	 if(strcmp(FichSesiones,""))
	 {
			fp = fopen(FichSesiones,"rb");
			if(fp != NULL) 
			{ 
					if(fgets(str, LINE_LEN, fp) != NULL) 
					{
							if(strstr(str, ColTotSes))
							{   /* obtener posicion relativa (columna) */
 									pch = strtok (str," ");
									while( pch != NULL )
  								{
											if(!strcmp(pch, ColTotSes))
													break;
											iPos++;
    									pch = strtok(NULL, " ");
  								}								
								
		              fseek(fp, 0, SEEK_END);  // non-portable
		              size=ftell(fp);
		              size = size-LINE_LEN;
		              fseek(fp, size, SEEK_SET);
		              
		              while( fgets(str, LINE_LEN, fp) )
		              {   /* lectura en lineas */
											iPos2=1;		              	
			                pch = strtok(str," ");
											iSalida = atoi(pch);
											
											while( pch != NULL )
  										{
  											  if(iPos == iPos2)
  											  		break;
  											  iPos2++;
  											  pch = strtok (NULL, " ");
  											  iSalida = atoi(pch);
	 										}						
		              }
							}
		      }
	 		}
	 		fclose(fp);      	
	 }

   return iSalida;
}


int ConsultaHistorico(int sd, char *fechaIni, char* fechaFin)
{
	size_t result;
	
	int i;
	long j;
	FILE *fp=NULL;
	char bufferSalida[MAXBUF]="";	
	
	char newFechaIni[MAX_FECHAX+1];
	char newFechaFin[MAX_FECHAX+1];
	char hFile[MAX_PATH];
	
	memset(bufferSalida, '\0', MAXBUF);
	memset(newFechaIni,'\0',MAX_FECHAX+1);
	memset(newFechaFin,'\0',MAX_FECHAX+1);		
	
	/*quitamos los 2 primero caracteres de la fecha*/
	strcpy(newFechaIni,fechaIni+2);
	strcpy(newFechaFin,fechaFin+2);
			
	/*ponemos la cabecera del xml*/
	if(!TCPSendMsg(sd, XML_HEADER, strlen(XML_HEADER)))
	{
		ARQLog(ERR_TRACE, "ExecuteAction::Error en envio xml(TCPSendMsg).");
		return 1;
	}
	
	/*buscamos los fichero por cada una de las intalaciones*/
	for( i=0; i < config.Instalations; i++)
	{
		ARQLog(ALL_TRACE, "Instalacion [%s]",config.Name[i]);
		
		ARQLog(ALL_TRACE, "newFechaIni [%ld]",atol(newFechaIni));
		ARQLog(ALL_TRACE, "newFechaFin [%ld]",atol(newFechaFin));
		
		sprintf(bufferSalida,"<servidor nombre=\"%s\">\n",config.Name[i]);
		if(!TCPSendMsg(sd, bufferSalida, strlen(bufferSalida)))
		{
			ARQLog(ERR_TRACE, "ExecuteAction::Error en envio xml(TCPSendMsg).");
			return 1;
		}
		
		for (j=atol(newFechaIni); j<=atol(newFechaFin); j++)
		{
			/*obtenemos el nombre del archivo de historico*/
			sprintf(hFile, "%s/%s.%s.%d", config.rutaHistoricos, TAG_FICH_HIST, config.Name[i], j);
			ARQLog(ALL_TRACE, "fichero historico [%s]",hFile);
			if (exists(hFile))				
			{
				fp=fopen(hFile,"rb");
				
 				while(1)
				{
					result= fread(bufferSalida, 1,sizeof(bufferSalida), fp);
					/* ARQLog(ALL_TRACE, " ##### Result=[%d] sizeof=[%d] strlen=[%d] buffer= [%s]",result,sizeof(bufferSalida),strlen(bufferSalida),bufferSalida);  */
					/* Envio de la cabecera de respuesta al cliente TCP. */	 
					if(!TCPSendMsg(sd, (char*)bufferSalida, result))
					{
						ARQLog(ERR_TRACE, "ExecuteAction::Error en envio xml(TCPSendMsg).");
						return 1;
					}
					
					memset(bufferSalida, '\0', MAXBUF);
					if (result!=MAXBUF)
						break;
				}			  										
				fclose(fp);
			}
			else
				ARQLog(ALL_TRACE, "fichero historico [%s] no encontrado",hFile);
		}
		
		sprintf(bufferSalida,"</servidor>\n");
		if(!TCPSendMsg(sd, bufferSalida, strlen(bufferSalida)))
		{
			ARQLog(ERR_TRACE, "ExecuteAction::Error en envio xml(TCPSendMsg).");
			return 1;
		}		
	}
	
	/*ponemos el pie del xml*/
	if(!TCPSendMsg(sd, XML_TAIL, strlen(XML_TAIL)))
	{
		ARQLog(ERR_TRACE, "ExecuteAction::Error en envio xml(TCPSendMsg).");
		return 1;
	}
	
	ARQLog(ALL_TRACE, "ExecuteAction::Cabecera enviada correctamente.");				 		
	return 0;	
}


int ConsultaOnline(int sd)
{
	char aux[250]="";
	int i=0, j=0, max_peticiones, num_peticiones, max_procesos=0, numSesiones=0;
	long totalEjecutadas, totalSLimit, totalHLimit, totalTimeout, totalDown;	
	
	strcpy(buffer,0);
	strcat(buffer,XML_HEADER);
		
	for( i=0; i<config.Instalations; i++)
	{
		if (tabla[i] != NULL)
		{
			totalEjecutadas=0;
			totalSLimit=0;
			totalHLimit=0;
			totalTimeout=0;
			totalDown=0;
			
	 		max_procesos=tabla[i]->dglobal.num_max_procesos;
	 		for( j=0; j < max_procesos; j++)
			{
	     			if (tabla[i]->dproceso[j].pid > 0)
	     			{
	        			totalEjecutadas+=tabla[i]->dproceso[j].pEjecutadasOK;
	         			totalSLimit+=tabla[i]->dproceso[j].pSLimit;
	         			totalHLimit+=tabla[i]->dproceso[j].pHLimit;
	         			totalTimeout+=tabla[i]->dproceso[j].pTimeout;
	        			totalDown+=tabla[i]->dproceso[j].pDown;
	        			/* fix overflow */
	        			if (totalEjecutadas < 0) 
						totalEjecutadas = -totalEjecutadas - LONG_MAX;
	         			if (totalSLimit < 0)
						totalSLimit = -totalSLimit - LONG_MAX;
	         			if (totalHLimit < 0)
						totalHLimit = -totalHLimit - LONG_MAX;
	         			if (totalTimeout < 0)
						totalTimeout = -totalTimeout - LONG_MAX;
	        			if (totalDown < 0)
						totalDown = -totalDown - LONG_MAX;
	     			}
	 		}
	
			totalEjecutadas+=tabla[i]->dglobal.totalEjecutadas;
			totalSLimit+=tabla[i]->dglobal.totalSLimit;
			totalHLimit+=tabla[i]->dglobal.totalHLimit;
			totalTimeout+=tabla[i]->dglobal.totalTimeout;
			totalDown+=tabla[i]->dglobal.totalDown;
			/* fix overflow */
       			if (totalEjecutadas < 0) 
				totalEjecutadas = -totalEjecutadas - LONG_MAX;
       			if (totalSLimit < 0)
				totalSLimit = -totalSLimit - LONG_MAX;
       			if (totalHLimit < 0)
				totalHLimit = -totalHLimit - LONG_MAX;
       			if (totalTimeout < 0)
				totalTimeout = -totalTimeout - LONG_MAX;
      			if (totalDown < 0)
				totalDown = -totalDown - LONG_MAX;

			/*consulta peticiones ocupadas*/
			consulta_peticiones(0, max_peticiones, &num_peticiones);			
				
			if (num_peticiones>maxpOcupados)
				maxpOcupados=num_peticiones;
					
			/*consultar numero de sesiones*/
			numSesiones = RecuperarTotSesiones();
					
			/* montar xml a enviar */
			sprintf(aux,"<servidor nombre=\"%s\">\n",config.Name[i]);
			strcat(buffer,aux);
			sprintf(aux,"<maxProcesos>%d</maxProcesos>\n", tabla[i]->dglobal.num_max_procesos);
			strcat(buffer,aux);
			sprintf(aux,"<hLimit>%ld</hLimit>\n",config.maxBusyProcess[i]);			
			strcat(buffer,aux);
			sprintf(aux,"<sLimit>%d</sLimit>\n",config.maxSoftBusyProcess[i]);
			strcat(buffer,aux);
			sprintf(aux,"<pRegistrados>%d</pRegistrados>\n",tabla[i]->dglobal.num_proc_registrados);
			strcat(buffer,aux);
			sprintf(aux,"<pOcupados>%ld</pOcupados>\n",num_peticiones);
			strcat(buffer,aux);
			sprintf(aux,"<maxpOcupados>%d</maxpOcupados>\n",maxpOcupados);
			strcat(buffer,aux);				
			sprintf(aux,"<cSesiones>%d</cSesiones>\n",numSesiones);
			strcat(buffer,aux);				
				
			
			if( strcmp(tabla[i]->dglobal.fecha_ult_reset,"") )
				sprintf(aux,"<fReset>%s</fReset>\n",tabla[i]->dglobal.fecha_ult_reset);
			else
				sprintf(aux,"<fReset>%s</fReset>\n","0");  
			
			strcat(buffer,aux);
			sprintf(aux,"<cEjecutadas>%ld</cEjecutadas>\n",totalEjecutadas);
			strcat(buffer,aux);
			sprintf(aux,"<csLimit>%ld</csLimit>\n",totalSLimit);
			strcat(buffer,aux);
			sprintf(aux,"<chLimit>%ld</chLimit>\n",totalHLimit);
			strcat(buffer,aux);
			sprintf(aux,"<cTimeout>%ld</cTimeout>\n",totalTimeout);
			strcat(buffer,aux);
			sprintf(aux,"<cDown>%ld</cDown>\n",totalDown);
			strcat(buffer,aux);
			sprintf(aux,"</servidor>\n");
			strcat(buffer,aux);						
		}
	}  /* fin for */
	
	strcat(buffer, XML_TAIL);
	
	/*
	** Envio de la cabecera de respuesta al cliente TCP.
	 */	 
	if(!TCPSendMsg(sd, (char*)buffer, strlen(buffer)))
	{
		ARQLog(ERR_TRACE, "ExecuteAction::Error en envio xml(TCPSendMsg).");
		return EXEC_CANT_SEND;
	}	
	
	ARQLog(ALL_TRACE, "ExecuteAction::Cabecera enviada correctamente.");				 
	
	return 0;		
}


int WriteStatSnapshot(void)
{
	time_t now;
	struct tm *fechsist;
	char fechsistfmt[16], hora[20];
	char strfechsist[FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1];
	char fileOut[MAXFICH_LEN+FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1+1];
	FILE *fp = NULL;
	int i,j,ret,max_peticiones, num_peticiones;
	char aux[MAX_BUFFER]="";
	char szTotSesiones[LEN_MSG];
	long auxEjecutadas, auxSLimit, auxHLimit, auxTimeout, auxDown, auxTiempoEje, tamanio;
	float auxTMedio;
	dat_configuracion conf;
	key_t key;
  
	int shm_creada = 0;

	/* creacion fichero, Obtiene la hora-fecha actual en formato EPOCH. */
	now = time(NULL);
	fechsist = localtime(&now);
	sprintf(fechsistfmt, "%%0%dd%%0%dd%%0%dd", FECH_ANO_LEN, FECH_MES_LEN, FECH_DIA_LEN); /* 2-oct-2013 logs incorrectos */
	
	
	for( i=0; i < config.Instalations; i++)
	{
		key=config.Ipckey[i];
		tamanio=config.tamanio[i];
 		memset(buffer, '\0', sizeof(buffer));
		auxEjecutadas=0;
		auxSLimit=0;
		auxHLimit=0;
		auxTimeout=0;
		auxDown=0;
		auxTiempoEje=0;

		if (tabla[i] == NULL)
		{
			conf.max_procesos=config.max_procesos[i];
			conf.clave=key;			
			ret=crea_tabla(key,conf);

			if (ret != 0)
			{
				ARQLog(ERR_TRACE, "WriteStatSnapshot::ERROR en lectura de memoria compartida");
				return 1;
			}
		}
		
		tabla[i] = con_tabla_sinSem(0);
		
		if (tabla[i] == NULL)
		{
			ARQLog(ERR_TRACE, "WriteStatSnapshot::ERROR en acceso a la memoria compartida");
			return 1;
		}
		else		
		{
			/*  Crea una cadena con la fecha en formato: ddmmaaaa. (antes) ahora: aammdd */
			sprintf(strfechsist, fechsistfmt, fechsist->tm_year-100, fechsist->tm_mon+1, fechsist->tm_mday); /* 2-oct-2013 logs incorrectos */
			sprintf(fileOut, "%s/%s.%s.%s",config.rutaHistoricos, TAG_FICH_HIST, config.Name[i], strfechsist);

			if((fp = fopen(fileOut, "a+")) == NULL)
			{
				ARQLog(ERR_TRACE, "WriteStatSnapshot::ERROR en la escritura del fichero[%s]", fileOut);
				return 1;
			}
			
			/* acum. de la tabla de procesos */
 			for( j=0; j< tabla[i]->dglobal.num_max_procesos; j++)
 			{
     			if (tabla[i]->dproceso[j].pid > 0)
     			{
         			auxEjecutadas += tabla[i]->dproceso[j].pEjecutadasOK;
         			auxSLimit     += tabla[i]->dproceso[j].pSLimit;
         			auxHLimit     += tabla[i]->dproceso[j].pHLimit;
         			auxTimeout    += tabla[i]->dproceso[j].pTimeout;
         			auxDown       += tabla[i]->dproceso[j].pDown;
         			auxTiempoEje  += tabla[i]->dproceso[j].tiempoEje;
     			}
 			}

			auxEjecutadas = auxEjecutadas + tabla[i]->dglobal.totalEjecutadas - tabla[i]->dglobal.parcialEjecutadas;
			auxSLimit		  = auxSLimit		  + tabla[i]->dglobal.totalSLimit 		- tabla[i]->dglobal.parcialSLimit;
			auxHLimit			= auxHLimit			+ tabla[i]->dglobal.totalHLimit			- tabla[i]->dglobal.parcialHLimit;
			auxTimeout    = auxTimeout    + tabla[i]->dglobal.totalTimeout		- tabla[i]->dglobal.parcialTimeout;
			auxDown				= auxDown				+ tabla[i]->dglobal.totalDown				- tabla[i]->dglobal.parcialDown;
			auxTiempoEje  = auxTiempoEje  + tabla[i]->dglobal.totalTiempoEje	- tabla[i]->dglobal.parcialTiempoEje;
			
			/*consulta peticiones ocupadas*/
			consulta_peticiones(0, max_peticiones, &num_peticiones);		
			
			if (num_peticiones>maxpOcupados)
				maxpOcupados=num_peticiones;				
			
			/* INI montar xml con "la foto" de los contadores */
			fechsist = localtime(&now);
			strftime(hora,20,"%Y%m%d%H%M%S", fechsist);
			sprintf(aux,"<dato timestamp=\"%s\">\n", hora);
			
			strcat(buffer,aux);
			sprintf(aux," <fUltAcum>%s</fUltAcum>\n",tabla[i]->dglobal.fecha_ult_acum);
			strcat(buffer,aux);
			sprintf(aux," <cEjecutadas>%ld</cEjecutadas>\n",auxEjecutadas);
			strcat(buffer,aux);
			sprintf(aux," <csLimit>%ld</csLimit>\n",auxSLimit);
			strcat(buffer,aux);
			sprintf(aux," <chLimit>%ld</chLimit>\n",auxHLimit);
			strcat(buffer,aux);
			sprintf(aux," <cTimeout>%ld</cTimeout>\n",auxTimeout);
			strcat(buffer,aux);
			sprintf(aux," <cDown>%ld</cDown>\n",auxDown);
			strcat(buffer,aux);
			sprintf(aux," <TiempoEjec>%ld</TiempoEjec>\n",auxTiempoEje);
			strcat(buffer,aux);
			/* -- t.medio ejecuciones OK -- */	
			if (auxEjecutadas>0)
				auxTMedio = (float)auxTiempoEje / (float)auxEjecutadas;
			else
				auxTMedio = 0;
				
			sprintf(aux," <TiempoMedioOK>%.2lf</TiempoMedioOK>\n", auxTMedio);
			strcat(buffer,aux);
			sprintf(aux," <pOcupados>%ld</pOcupados>\n",num_peticiones);
			strcat(buffer,aux);			
			/* (2015-10-10), se annade el total de sesiones activas */
			sprintf(aux," <cSesiones>%d</cSesiones>\n", RecuperarTotSesiones());
			strcat(buffer,aux);			
			/* ------------------- */
			sprintf(aux,"</dato>\n");
			strcat(buffer,aux);
			/* FIN montar xml con "la foto" de los contadores */
			
			/* INI acceso a seccion critica (asignacion parciales) */
			Lock();
			tabla[i]->dglobal.parcialEjecutadas += auxEjecutadas;
			tabla[i]->dglobal.parcialSLimit  += auxSLimit;
			tabla[i]->dglobal.parcialHLimit  += auxHLimit;
			tabla[i]->dglobal.parcialTimeout += auxTimeout;
			tabla[i]->dglobal.parcialDown 	 += auxDown;
			tabla[i]->dglobal.parcialTiempoEje += auxTiempoEje;
			fechsist = localtime(&now);
			strftime(hora,20,"%Y%m%d%H%M%S", fechsist);
			strcpy( tabla[i]->dglobal.fecha_ult_acum, hora);	/* fecha-hora ult. volcado */
			Unlock();
			/* FIN acceso seccion critica */
			
			fprintf(fp,(char const *)buffer);
			fclose(fp);
		}	 /*fin if */
	}  /* fin for */

	return 0;
}


/*****************************************************************************/
error_t SocketsEventLoop(int highest_sd, long *interval)
/*****************************************************************************
Funcionalidad:
>> Bucle de eventos.
*****************************************************************************/
{
   int nfound, exec, i;
   fd_set readmask;
   struct timeval timeout;
   struct timespec spec;  /* tiempo con centesimas de seg */
   time_t tInicioSeg, tActualSeg;

	 /* -- 26nov -- */
   /* timeout.tv_sec = *interval; */
   /* timeout del select */
   timeout.tv_sec = DEFAULT_SELECT_TIMEOUT;
   timeout.tv_usec= 0;
   
   /* tomo tiempo */   
   clock_gettime(CLOCK_REALTIME, &spec);
	 tInicioSeg = spec.tv_sec;
   
   while(!stop_loop)
   {
			clock_gettime(CLOCK_REALTIME, &spec);
			tActualSeg = spec.tv_sec;
	  
			/*si no es supervisor de historicos */
			if (!supHistorico)   	
      	if(checkSHM())
        	 ARQLog(ERR_TRACE, "checkSHM::error en procesado memoria compartida");  /* se continua pese a todo */

			/* if (config.SupervisorInterval>0) TRAZAJM*/
			/*si no es supervisor de historicos */
			if (!supHistorico && (tActualSeg - tInicioSeg >= *interval) )
			{
				/* consolidacion contadores y escritura a ficheros */
				if(WriteStatSnapshot())
					ARQLog(ERR_TRACE, "WriteStatSnapshot::error en escritura fichero historico");  /* se continua pese a todo */
					
				tInicioSeg = tActualSeg;	/* reseteo el tiempo de inicio */
	  	}
	  	
      /* Actualizamos la mascara de lectura para el select() */
      FD_ZERO(&readmask);
      for(i = 0; i<MAX_OPENED_SOCKETS; i++)
         if(sockets[i].opened)
	    FD_SET(sockets[i].sd, &readmask);
	    
      /*
      ** El bucle permanece bloqueado hasta que llega algun
      ** mensaje a alguno de los sockets, bien de los sockets
      ** abiertos con clientes.
      */

      /* 2-oct-2013 se retira la traza 
         ARQLog(INF_TRACE, "SocketsEventLoop::Bloqueo en select, en espera de entrada de mensajes.");
      */

			
			/* 26-nov select con timeout pequeño (y fijo) */
      if((nfound = select(highest_sd+1, &readmask, 0, 0, &timeout)) == -1)
      {
				ARQLog(ERR_TRACE, "SocketsEventLoop::Error en operacion select(%d - %s).", errno, strerror(errno));
				if (errno != EINTR && flagrefresco==0)
	 			return EVENT_LOOP_ERROR;
      }
	  
      if (flagrefresco)
      {
				propagaRefresco();
	 			flagrefresco=0;
      }
      
      /* 2-oct-2013 se retira la traza 
         ARQLog(INF_TRACE, "SocketsEventLoop::Desbloqueo del select.");
      */ 
      /*
      ** Si se llega a este punto es p.q. se ha recibido algun
      ** mensaje en alguno de los sockets. Los sockets a los
      ** que han llegado mensajes seran aquellos que cumple lo
      ** siguiente:
      ** sockets[i].opened && FD_ISSET(sockets[i].sd, &readmask).
      ** (esta abierto y ha llegado algun mensaje).
      ** Para aquellos socket a los que ha llegado algun mensaje
      ** se lanzara la accion que tienen asignada.
      */
      /*
      ** Si se decide recorrer la tabla de sockets en orden inverso
      ** para asi asegurar que primero se detectan los mensajes 
      ** procedentes de la aplicacion, el bucle sera como sigue:
      **    for(i = MAX_OPENED_SOCKETS-1; i> = 0; i--)
      ** En caso contrario:
      **    for(i = 0; i<MAX_OPENED_SOCKETS; i++)
      */
      for(i = MAX_OPENED_SOCKETS-1; i >= 0; i--)
				if(sockets[i].opened && FD_ISSET(sockets[i].sd, &readmask))
				{
						ARQLog(ALL_TRACE, "SocketsEventLoop::Ejecutando accion %d sobre el socket %d.",
						sockets[i].action, sockets[i].sd);
						exec = ExecuteAction(sockets[i].sd, sockets[i].action, &highest_sd);
						if(exec != EXEC_OK)
								return exec;
				}
      
   } /* while(!stop_loop)*/
   
	 return EVENT_LOOP_STOPPED;
} /* SocketsEventLoop */


/*****************************************************************************/
error_t ExecuteAction(int sd, action_t action, int *highest_sd)
/*****************************************************************************
Funcionalidad:
>> Funcion que ejecuta una determinada accion sobre un socket.
*****************************************************************************/
{
   int new_sd;
   int ret;
   char *msg = NULL;
   char *xml_salida = NULL;
   char aux_xml[5]="";
   char fechaIni[MAX_FECHAX+2];
   char fechaFin[MAX_FECHAX+2];
   
   memset(fechaIni,'\0',MAX_FECHAX+2);
   memset(fechaFin,'\0',MAX_FECHAX+2);

   switch(action)
   {
      case ACCEPT_CALL:
	 			ARQLog(ALL_TRACE, "ExecuteAction::Aceptando llamada desde el socket %d.", sd);
				if(!TCPServerAcceptCall(sd, &new_sd))
					return EXEC_CANT_ACCEPT;

				ARQLog(ALL_TRACE, "ExecuteAction::Llamada aceptada en socket %d.", new_sd);

	 			ARQLog(ALL_TRACE, "ExecuteAction::Registrando socket %d con accion ATTEND_CLIENT.", new_sd);
	 			RegisterSocket(new_sd, highest_sd, ATTEND_CLIENT);
				break;

      case ATTEND_CLIENT:
				ARQLog(ALL_TRACE, "ExecuteAction::Atendiendo a cliente TCP desde el socket %d.", sd);
				ARQLog(ALL_TRACE, "ExecuteAction::Recibiendo cabecera.");
				msg = (char*)malloc(LEN_MSG); 

				switch(TCPReciveMsg(sd, (char*)msg, LEN_MSG))	
				{
            case TCP_OOB:
               UnRegisterSocket(sd, highest_sd);
               ARQLog(ALL_TRACE, "ExecuteAction::Detectado cierre de socket <%d> origen.", sd);
							 free(msg);
               return EXEC_OK;

            case TCP_ERROR:

            case TCP_INTR:
	       			 free(msg);
               return EXEC_CANT_RECIVE;

            case TCP_OK:
               break;
         }
				
				 if( strstr(msg, TAG_HISTORICO) )	/*peticion historico*/
				 {
				 	if( strstr(msg, TAG_FECINICIO) )
						strncpy(fechaIni, strstr(msg, TAG_FECINICIO)+strlen(TAG_FECINICIO)+2, MAX_FECHAX);
				 	else
				 		strcpy(fechaIni, TAG_SINFECHA);
				 		
				 	if( strstr(msg, TAG_FECFIN) )
						strncpy(fechaFin, strstr(msg, TAG_FECFIN)+strlen(TAG_FECFIN)+2, MAX_FECHAX);
				 	else
				 		strcpy(fechaFin, TAG_SINFECHA);
				 		
					ARQLog(INF_TRACE, "ExecuteAction::Peticion consulta HISTORICO [%s - %s] recibida correctamente.", fechaIni, fechaFin);
				 	ConsultaHistorico(sd, fechaIni,fechaFin);
				 }
				 else if ( strstr(msg, TAG_ONLINE) ) /*peticion online*/
				 {
						ARQLog(INF_TRACE, "ExecuteAction::Peticion consulta ONLINE recibida correctamente.");				 		
				 		ConsultaOnline(sd);
				 }
				 else if (strstr(msg, TAG_RELOAD) ) /*peticion recargar configuracion*/
				 {
				 	if (!supHistorico)
				 	{
				 		ARQLog(INF_TRACE, "ExecuteAction::Peticion RECARGAR CONFIGURACION recibida correctamente.");
				 		/* recargamos la configuracion del supervisor */
				 		readConfigTraza(configuracion);
						propagaRefresco();
	 					flagrefresco=0;	 					
	 				
						if(!TCPSendMsg(sd, "Configuracion recargada", 23))
						{
							ARQLog(ERR_TRACE, "ExecuteAction::Error en envio de mensaje.");
							return 1;
						}
					}
					else
						ARQLog(BAS_TRACE, "ExecuteAction::Peticion RECARGAR CONFIGURACION recibida en un supervisor para HISTORICOS.");
				 }
				 else if (strstr(msg, TAG_RESET) ) /*peticion reset fecha*/
				 {
				 	if (!supHistorico)
				 	{
				 		ARQLog(INF_TRACE, "ExecuteAction::Peticion RESET FECHA recibida correctamente.");
						
						resetFecha();
	 				
						if(!TCPSendMsg(sd, "Reset realizado", 15))
						{
							ARQLog(ERR_TRACE, "ExecuteAction::Error en envio de mensaje.");
							return 1;
						}
					}
					else
						ARQLog(BAS_TRACE, "ExecuteAction::Peticion RESET FECHA recibida en un supervisor para HISTORICOS.");
				 }				 
				 else  	/*peticion desconocida*/
				 {
						ARQLog(BAS_TRACE, "ExecuteAction::Peticion consulta desconocida.");			
						
						if(!TCPSendMsg(sd, "Supervisor OK", 13))
						{
							ARQLog(ERR_TRACE, "ExecuteAction::Error en envio de mensaje.");
							return 1;
						}						
					}
					
				 free(msg);
				 UnRegisterSocket(sd, highest_sd);
				 break;
	 
      default:
         ARQLog(INF_TRACE, "ExecuteAction::Saliendo por accion desconocida <%d>.", action);
         return EXEC_UNKNOW_ACTION;
	 
   } /* switch(action)*/
   
   return EXEC_OK;

} /* ExecuteAction */


/*****************************************************************************/
void RegisterSocket(int sd, int *highest_sd, action_t action)
/*****************************************************************************
Funcionalidad:
>> Funcion que registra un socket asignandole una accion. Es decir cuando
>> a dicho socket llegue un mensaje se ejecutara la accion que tiene
>> asignada.
*****************************************************************************/
{
   /*ARQLog(INF_TRACE, "RegisterSocket::Registrando socket <%d> con accion <%d>.", sd, action);*/
   sockets[sd].opened = 1;
   sockets[sd].sd = sd;
   sockets[sd].action = action;
   
   if(sd > *highest_sd)
      *highest_sd = sd;
   
} /* RegisterSocket */

/*****************************************************************************/
void UnRegisterSocket(int sd, int *highest_sd)
/*****************************************************************************
Funcionalidad:
>> Funcion que desactiva un socket no asignandole accion alguna. 
*****************************************************************************/
{
   int is;   /* index socket */
   
   /*ARQLog(INF_TRACE, "UnRegisterSocket::Desregistrando socket <%d>.", sd);*/
   sockets[sd].opened = 0;
   sockets[sd].sd = -1;
   sockets[sd].action = -1;
   
   /*
   ** Cerramos el socket.
   ** close(sd);
   */
   TCPDisconnectSocket(sd);
   
   /*
   ** Si el socket que cerramos es el highest_sd, tenemos que decrementar
   ** hasta que encontremos uno abierto y no hayamos llegado al final de
   ** la tabla 'sockets'.
   */
   
   if(sd == *highest_sd)
   {
      is = *highest_sd - 1;
      while( (!sockets[is].opened)&&(is) )
	 			is--;
			*highest_sd = is;
   }
} /* UnRegisterSocket */


/*****************************************************************************/
void CleanUp(int sig)
/*****************************************************************************/
{
   ARQLog(INF_TRACE, "CleanUp::Iniciando proceso de parada.");
   ARQLog(ALL_TRACE, "CleanUp::Iniciando proceso de parada del bucle de eventos.");
   stop_loop = 1;
   
   DeletePidFile(pidFile);
   /* 13-sep-2013 se retira  exit(EXIT_FAILURE); */
   ARQLog(INF_TRACE, "CleanUp::Supervisor parado");   
   kill(getpid(), SIGTERM);
} /* CleanUp */

/*****************************************************************************/
int EscribePidFile(char * pidFile)
/*****************************************************************************/
{
	FILE *f;
	pid_t pid;
	char txtpid[10]="";
	
	f=fopen(pidFile, "w");
	if (f == NULL)
	{
		ARQLog(ERR_TRACE, "EscribePidFile::Error en escritura de fichero");
		return 1;
	}
	pid=getpid();
	sprintf(txtpid,"%ld",pid);
	fwrite(txtpid, sizeof(txtpid), 1, f);
	fclose(f);
	
	ARQLog(INF_TRACE, "EscribePidFile::Escritura de pidFile %s",pidFile);

	return 0;
}

/*****************************************************************************/
int DeletePidFile(char * pidFile)
/*****************************************************************************/
{

	if( remove( pidFile ) == -1 )
	{
		ARQLog(INF_TRACE, "DeletePidFile::Error en borrado de fichero");
		return 1;
	}
	
	ARQLog(ALL_TRACE, "DeletePidFile::Borrado de pidFile %s",pidFile);
	return 0;
}

void flagPropagaRefresco(int sig)
{
	flagrefresco=1;
	ARQLog(INF_TRACE, "Llamada a recargar configuracion de los fast_cgis");
}

void resetFecha(void)
{		
	int i=0, j=0,numProcesos=0;
	char horaActual[20];
	time_t tiempo = time(0);
	
	struct tm *tlocal = localtime(&tiempo);
	strftime(horaActual,20,"%Y%m%d%H%M%S",tlocal);
	
	maxpOcupados=0;  /*reinicamos el contador de max procesos ocupados*/
	
	/* consolidacion contadores y escritura a ficheros */
	if(WriteStatSnapshot())
		ARQLog(ERR_TRACE, "WriteStatSnapshot::error en escritura fichero historico");  /* se continua pese a todo */	
		
	/*reiniciamos la tabla SHM*/
	for( i=0; i<config.Instalations; i++)
	{
		if (tabla[i] != NULL)
		{
 			numProcesos=tabla[i]->dglobal.num_max_procesos;

			Lock();
			tabla[i]->dglobal.totalEjecutadas = 0;
			tabla[i]->dglobal.totalSLimit  		= 0;
			tabla[i]->dglobal.totalHLimit  		= 0;
			tabla[i]->dglobal.totalTimeout 		= 0;
			tabla[i]->dglobal.totalDown 	 		= 0;
			tabla[i]->dglobal.totalTiempoEje 	= 0;

			tabla[i]->dglobal.parcialEjecutadas = 0;
			tabla[i]->dglobal.parcialSLimit  		= 0;
			tabla[i]->dglobal.parcialHLimit  		= 0;
			tabla[i]->dglobal.parcialTimeout 		= 0;
			tabla[i]->dglobal.parcialDown 	 		= 0;
			tabla[i]->dglobal.parcialTiempoEje 	= 0;
			strcpy( tabla[i]->dglobal.fecha_ult_reset, horaActual);

	 		for( j=0; j < numProcesos; j++)
     		if (tabla[i]->dproceso[j].pid > 0)
     		{
       		tabla[i]->dproceso[j].pEjecutadasOK	=0;
       		tabla[i]->dproceso[j].pSLimit       =0;
        	tabla[i]->dproceso[j].pHLimit       =0;
        	tabla[i]->dproceso[j].pTimeout      =0;
        	tabla[i]->dproceso[j].pDown         =0;
        	tabla[i]->dproceso[j].tiempoEje     =0;
     		}
     	
			Unlock();
		}
	}		
	
}

/*funcion que desencadena el refresco en caliente de lanzadores y supervisor*/
void propagaRefresco(void)
{
		int i=0, j=0, num_procesos=0, ret=0, pid=0;
		
		/*por cada instalacion, repasa los procesos existentes de la memoria*/
		for( i=0; i < config.Instalations; i++)
		{
			if (tabla[i]==NULL)
			{
				ARQLog(ERR_TRACE, "Error tratando la instalacion [%d] para recargar configuracion", i+1);
				continue;
			}
							
			num_procesos=tabla[i]->dglobal.num_max_procesos;
			/*enviamos la señal de refresco a cada proceso */
			for(j=0; j<num_procesos; j++)
			{
				if (tabla[i]->dproceso[j].pid > 0)
				{
					ARQLog(ALL_TRACE, "Enviando senial SIGUSR2 a proceso %ld",tabla[i]->dproceso[j].pid);
					ret = kill(tabla[i]->dproceso[j].pid , SIGUSR2);
					if (ret != 0)
						ARQLog(ERR_TRACE, "Error enviando refresco de configuracion al proceso %d",tabla[i]->dproceso[j].pid);
				}
			}
						
			/*recargamos la configuracion del supervisor*/
			if(readConfigPidfile(config.IpcFile[i]))
			{
				ARQLog(ERR_TRACE, "readConfigPidfile::no se pudo abrir el fichero de configuracion %s", config.IpcFile[i]);
				/* salir  */
				return ;
			}
			else
			{
				ARQLog(INF_TRACE,"Recargada configuracion en caliente. Configuracion: [%s]", config.IpcFile[i]);				
			}
		}
}

