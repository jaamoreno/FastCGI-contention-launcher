#include "checkFastCGI.h"


char fConfig[MAXPATH];
int verbose =0;
startConfiguration startConfig;
supConfiguration supConfig;

static 	tabla_shm *tabla = NULL;


void initStartConfig()
{
	memset(&startConfig,0,sizeof(startConfiguration));
}


void initSupConfig()
{
	memset(&supConfig,0,sizeof(supConfiguration));	
}


void imprime(char const *msg,...)
{	
	if (verbose==vOn)
	{
		va_list arglist;
		
		va_start(arglist,msg);		
		vprintf((char const *)msg,arglist);
		va_end(arglist);	
		//sleep(1);
	}
}


int exec(char* cmd,char* ret) 
{
	FILE* pipe = popen(cmd, "r");
	char buffer[128];
	char result[1024] = "";


	if (!pipe) 
		return 1;
	
	while(!feof(pipe)) 
	{
		if(fgets(buffer, 128, pipe) != NULL)
			strcat(result,buffer);
	}

	pclose(pipe);
	strcpy(ret,result);
	return 0;
}


int checkPid (pid_t pid)
{
	if (kill(pid, 0) == 0) {
	    /* process is running or a zombie */
	    return 0;
	} else if (errno == ESRCH) {
	    /* no such process with the given pid is running */
	    return errno;
	} else {
	    /* some other error... use perror("...") or strerror(errno) to report */
	    return errno;
	}
}


int checkDir(char * fileName)
{
	struct stat st;
	
	if(stat(fileName,&st) == 0)
	    if(st.st_mode & S_IFDIR != 0)	
		return 0;	/* -- la ruta existe -- */
  
	return 1;
}


/*funcion que obtiene la ruta de un fichero*/
char * getPath(char * file)
{
	static char path[MAXPATH];
	int i;
	i=strlen(file)-1;
	
	memset(path,'\0',MAXPATH);	
	
	while((i>=0)&&(file[i]!='/'))
		i--;

	strncpy(path,file,i);
	path[i+1]='\0';
	
	return path;
}


static int parseStartConfig(void* user, const char* section, const char* name, const char* value)
{
    startConfiguration* pconfig = (startConfiguration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("config", "logName")) {
        strcpy(pconfig->logName,strdup(value));
    } else if (MATCH("config", "cgiProgram")) {
        strcpy(pconfig->cgiProgram,strdup(value));
    } else if (MATCH("config", "statisticsName")) {
        strcpy(pconfig->statisticsName,strdup(value));
    } else if (MATCH("config", "traceLevel")) {
    	pconfig->traceLevel = atoi(value);
    } else if (MATCH("config", "childTimeOut")) {
    	pconfig->childTimeOut = atoi(value);
    } else if (MATCH("config", "maxProcess")) {
    	pconfig->maxProcess = atoi(value);
    } else if (MATCH("config", "maxBusyProcess")) {
    	pconfig->maxBusyProcess = atoi(value);
    } else if (MATCH("config", "maxSoftBusyProcess")) {
    	pconfig->maxSoftBusyProcess = atoi(value);
    } else if (MATCH("config", "paginaErrorGenerico")) {
        strcpy(pconfig->paginaErrorGenerico,strdup(value));
    } else if (MATCH("config", "paginaErrorSoft")) {
        strcpy(pconfig->paginaErrorSoft,strdup(value));
    } else if (MATCH("config", "paginaErrorHard")) {
        strcpy(pconfig->paginaErrorHard,strdup(value));
    } else if (MATCH("config", "stats")) {	/* -- stats -- */
        strcpy(pconfig->stats,strdup(value));
    } else {
        return 0;  /* unknown section/name, error */
    }
   
    return 1;
}


static int parseSupConfig(void* user, const char* section, const char* name, const char* value)
{
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	char str[MAXPATH];
	char * pch;
	int indice=0;
	int nivel=0;


	supConfiguration *pconfig = (supConfiguration*)user;

	if (MATCH("General", "ListenPort")) 
		pconfig->ListenPort = atoi(value);
	else if (MATCH("General", "SupervisorInterval")) 
		pconfig->SupervisorInterval = atoi(value);
	else if (MATCH("General", "Instalations")) 
		pconfig->Instalations = atoi(value);
	else if (MATCH("General", "PidFile")) 
		strcpy(pconfig->pidFile, value);
	else if (MATCH("General", "TimeOutKill")) 
		pconfig->timeKill=atoi(value);
	else if (MATCH("General", "TraceLevel")) 
		nivel = atoi(value);
	else if (MATCH("General", "FicheroTraza"))
		strcpy(pconfig->FicheroTraza, value);
	else if (MATCH("Instalations", "StartConfigFiles")) 
	{
		strcpy(str, value);
		pch = strtok (str,",");
		
		while (pch != NULL)
		{
			strcpy(pconfig->IpcFile[indice], pch);
			pconfig->Ipckey[indice]=ftok(pconfig->IpcFile[indice],SEED);
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
		return 1;

	return 0; 
}


int main(int argc, char *argv[])
{
	int i;
	FILE* pidFile;
	char pid [MAX_PID]="";
	long int li_pid;
	
	
	if (argc <= 1)
	{
		printf ("Uso: checkInstall <supervisor_configFile>\n");
		return 0;
	}	
	
	initSupConfig();
	initStartConfig();
	
	if ((argv[2]!=NULL) && (strcmp(argv[2],"-V")==0))
		verbose=vOn;
	
	strcpy(fConfig,argv[1]);
	// JAM 5-jun-2015    system("clear");
	/*comprobamos que existe el fichero de configuracion*/
	imprime("\nComprobando fichero de configuracion %s:\t",fConfig);
	if (!exists(fConfig))	
	{
		imprime(" --> ERROR\n\t - El fichero de configuracion del supervisor %s no existe\n",fConfig);
		if (verbose==vOff)
			return CODCONFIGURACION;
	}
	else /*parseamos el fichero de configuracion del supervisor*/
	{
		if (ini_parse(fConfig, parseSupConfig, &supConfig) < 0) 
		{
			imprime(" --> ERROR\n\t - No puedo leer el fichero \n");
			if (verbose==vOff)
				return CODCONFIGURACION;
		}
		else 	
		{
			imprime(" --> OK \n");
			/* comprobamos si esta levantado el supervisor */
			imprime("\nComprobando proceso supervisor \t");
			if (strcmp(supConfig.pidFile,"")==0)
			{
				imprime(" \n--> PidFile no configurado \n");
			}
			else
			{
				if (!exists(supConfig.pidFile))	
				{				
					imprime("\n--> Fichero %s no existe.",supConfig.pidFile);
				}
				else
				{				
					pidFile=fopen(supConfig.pidFile,"r");
					if (pidFile!= NULL)
					{					
   						fseek(pidFile, SEEK_SET, 0);	/* seek beginning of file */
						fgets(pid,sizeof(pid),pidFile);
						fclose(pidFile);						

						li_pid = atol(pid);

						if ((pid_t) li_pid != li_pid) /* check that value was not out of range of pid_t */
						{
						   imprime(" --> ERROR\n\t - El tamaño del PID de proceso no se puede manejar");
						   if (verbose==vOff)
						   {	
							return CODNOSUPERVISOR;
						   }
						}
						else
						{
						   if (checkPid((pid_t)li_pid) == 0)  
						   {
							imprime(" --> OK\n");
						   }
						   else
						   {
							imprime(" --> ERROR\n\t - El proceso supervisor no esta corriendo\n");
							if (verbose==vOff)
								return CODNOSUPERVISOR;
						   }
						}
					}
				}
			}
			
			/*comprobamos el directorio de logs*/	
			imprime("\nComprobando log del supervisor \t");			
			if (strcmp(supConfig.FicheroTraza,"")==0)
			{
				imprime(" \n--> Fichero de log no configurado \n");				
			}
			else if (checkDir(getPath(supConfig.FicheroTraza))!=0)
			{
				imprime(" --> ERROR\n\t - La ruta %s NO EXISTE\n",getPath(supConfig.FicheroTraza));
			}else
				imprime(" --> OK\n");
									
									
			/*comprobamos las configuraciones de los lanzadores*/
			imprime("\nComprobando configuracion de los procesos start\n");
			for (i=0;i<supConfig.Instalations;i++)
			{
				imprime(" -Instalacion: %s \n",supConfig.Name[i]);
				imprime("   Configuracion: %s \t",supConfig.IpcFile[i]);
								
				if (!exists(supConfig.IpcFile[i]))
					imprime(" --> ERROR\n\t - El fichero %s NO EXISTE\n",supConfig.IpcFile[i]);

				/*si existe el fichero de confgiruacion del start*/			
				else if (ini_parse(supConfig.IpcFile[i], parseStartConfig, &startConfig) < 0) 
				{
					imprime(" --> ERROR\n\t - No puedo leer el fichero %s\n",supConfig.IpcFile[i]);
					return 1;
				}
				else 							
				{
					char command[100]="";
					char ret[1024]="";

					imprime(" --> OK\n");
					imprime("   CGI: %s \t",startConfig.cgiProgram);				
					if (!exists(startConfig.cgiProgram))
					{
						imprime(" --> ERROR\n\t - El CGI %s NO EXISTE\n",startConfig.cgiProgram);
						if (verbose==vOff)
							return CODNOCGI;						
					}
					else
						imprime(" --> OK\n");					
					imprime("   IPC: [%ld]  hex: [%x] \n",supConfig.Ipckey[i],supConfig.Ipckey[i]);					
					imprime("   maxProcess:          %03d\n",startConfig.maxProcess);
					imprime("   Limite de sesion:    %03d\n",startConfig.maxSoftBusyProcess);
					imprime("   Limite de servicio:  %03d\n",startConfig.maxBusyProcess);
					
					/*comprobamos si existe la memoria*/
					sprintf(command,"ipcs -m -a |grep %x",supConfig.Ipckey[i]);
					exec(command, ret);
					if (strcmp(ret,"")==0)
					{
						imprime("   SHM no creada");
						if (verbose==vOff)
							return CODNOSHM;
					}
					else
					{			
						sprintf(command," ipcs -m -a |grep %x| awk '{print $5}'",supConfig.Ipckey[i]);	
						exec(command,ret); 
						imprime("   SHM creada por el usuario %s",ret);

						sprintf(command," ipcs -m -a |grep %x| awk '{print $9}'",supConfig.Ipckey[i]); 
						exec(command,ret); 
						imprime("   Procesos adjuntados en SHM: %s",ret);	
					}
					
					/*comprobamos si existe el semaforo*/
					sprintf(command,"ipcs -s -a |grep %x",supConfig.Ipckey[i]);
					exec(command, ret);
					if (strcmp(ret,"")==0)
					{
						imprime("   Semaforo no creado");
						if (verbose==vOff)
							return CODNOSEM;
					}
					else
					{			
						sprintf(command," ipcs -s -a |grep %x| awk '{print $5}'",supConfig.Ipckey[i]);	
						exec(command,ret); 
						imprime("   Semaforo creado por el usuario %s",ret);
					}					
					
					/*comprobando directorios de log y estadisticas de los lanzadores*/
					imprime("   Ruta de logs: %s \t",getPath(startConfig.logName));
					if (checkDir(getPath(startConfig.logName))!=0)
					{
						imprime(" --> ERROR\n\t - La ruta %s NO EXISTE\n",getPath(startConfig.logName));
						if (verbose==vOff)
							return CODNOSTARTLOG;
					}else
						imprime(" --> OK\n");
					
					/* ------------------------------------------------------------------ */
					for (i=0; i<strlen(startConfig.stats); i++)
						startConfig.stats[i] = toupper(startConfig.stats[i]);

					if (strcmp(startConfig.stats, "ON") == 0)
					{
						imprime("   Ruta de estadisticas: %s \t",getPath(startConfig.statisticsName));
						if (checkDir(getPath(startConfig.statisticsName))!=0)
						{
							imprime(" --> ERROR\n\t - La ruta %s NO EXISTE\n",getPath(startConfig.statisticsName));
							if (verbose==vOff)
								return CODNOSTARTSTA;
						}else
							imprime(" --> OK\n");						
					}	
					else
						imprime("   Estadisticas: %s", startConfig.stats);
					/* ------------------------------------------------------------------ */
				}
				imprime ("\n");
			}  /* -- fin comprobacion conf. lanzadores -- */
		}
	}
	
	return 0; 
}
