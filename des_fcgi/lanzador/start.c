#include "start.h"
#include "fcgios.h"     /* READABLE_UNIX_FD_DROP_DEAD_TIMEVAL */

extern char **environ;
FILE *fp=NULL;
FILE *fpE=NULL;
configuration config;
fcgiProcess fProcess;
dat_configuracion configSHM;
static char buffer[MAXBUF];
char msg[MAXBUF]="";

/* char entrada[MAXBUF]; */
char *entrada = NULL; /* ### Buffer de entrada por stdin */
int tam_entrada = 0;

int pid_comando = 0, readConf=0;
static char configFile[MAXPATH]="";


char * serverityLabel(int level)
{
	switch (level)
	{
	case ERRORL:
		return "[ERROR] ";
		break;
	case WARNL:
		return "[WARNING] ";
		break;
	case INFOL:
		return "[INFO] ";
		break;
	case DEBUGL:
		return "[DEBUG] ";
		break;
	default:
		return " ";
		break;
	};

}

/*traza la informacion de un proceso*/
void trazaProceso(fcgiProcess p)
{
	traza(DEBUGL, "******************************************");
	traza(DEBUGL, "*\tPosicion      [%d]",p.tPos);
	traza(DEBUGL, "*\tPid           [%ld]",p.dProcess.pid);
	traza(DEBUGL, "*\tPid Padre     [%ld]",p.dProcess.ppid);
	traza(DEBUGL, "*\tPid Hijo      [%ld]",p.dProcess.pid_hijo);
	traza(DEBUGL, "*\tEstado        [%d]",p.dProcess.estado);
	traza(DEBUGL, "*\tF.Registro    [%ld]",p.dProcess.fecha_registro);
	traza(DEBUGL, "*\tF.Inicio      [%ld]",p.dProcess.fecha_inicio);
	traza(DEBUGL, "*\tF.Fin         [%ld]",p.dProcess.fecha_fin);
	traza(DEBUGL, "*\tF.Inicio est. [%s]",p.fInicio);
	traza(DEBUGL, "*\tF.Fin est.    [%s]",p.fFin);
	traza(DEBUGL, "*\tH.Inicio est. [%s]",p.hInicio);
	traza(DEBUGL, "*\tH.Fin est.    [%s]",p.hFin);
	traza(DEBUGL, "*\tNum.Proc. Reg.[%d]",p.nProcesosReg);
	traza(DEBUGL, "*\tRetorno       [%d]",p.status);
	traza(DEBUGL,"*\tEje. Ok        [%d]",p.dProcess.pEjecutadasOK);
	traza(DEBUGL,"*\tErr. Soft Limit[%d]",p.dProcess.pSLimit);
	traza(DEBUGL,"*\tErr. Hard Limit[%d]",p.dProcess.pHLimit);
	traza(DEBUGL,"*\tErr. Timeout   [%d]",p.dProcess.pTimeout);
	traza(DEBUGL,"*\tErr. Gen       [%d]",p.dProcess.pDown);
	traza(DEBUGL,"*\tTiempo Eje Ok  [%d]",p.dProcess.tiempoEje);	
	traza(DEBUGL, "******************************************");
}

void traza(int level,char const *msg,...)
{
	va_list arglist;
	time_t now;
	char pid[10];
	int timelen;
	int maxlen;
	char name[MAXPATH];
	char fecha[MAXPATH];

	/*si la traza es mayor que el traceLevel  no trazamos*/
	if (level>config.traceLevel)
		return;

#ifndef DEBUGTRACE
	if (level==DEBUGL)
		return;
#endif

	now=time(NULL);

	/*comprobamos si ha cambiado el dia */
	strftime(fecha,MAXPATH,".%y%m%d",localtime(&now));
	sprintf(name,"%s%s.log",config.logName,fecha);

	if (strcmp(config.logFile,name)!=0) /*guardamos el nuevo fichero*/
	{
		strcpy(config.logFile,name);
		fp=fopen(config.logFile,"a");		
	}

	sprintf(pid, "[%d]",getpid());

	/*pendiente la rotacion de logs*/


	if (!fp) /*si el fichero no esta abierto lo abrimos*/
		fp=fopen(config.logFile,"a");

	va_start(arglist,msg);

	strftime(buffer,MAXBUF,"[%Y/%m/%d %H:%M:%S]",localtime(&now));

	strcat(buffer,pid);
	strcat(buffer,serverityLabel(level));

	timelen=strlen(buffer);
	maxlen=strlen(msg);

	if (maxlen<MAXBUF-1)
	{
		strncat(&buffer[timelen],msg,MAXBUF-timelen-2);
		strcat(&buffer[timelen+maxlen],"\n");
	}
	else
		strcat(&buffer[timelen],"Trace Too Long: Max 4000 characters");

	vfprintf(fp,(char const *)buffer,arglist);
	/* fclose(fp); */
	fflush(fp);
	va_end(arglist);
}

void estadistica(fcgiProcess p)
{
	char est[MAXBUF]="";
	char line[MAXBUF];
	
	char *strSZ;
	char *strSZEnv;
	char szSD[MAX_SD+1]="";
	char szABT[MAX_ABT+1]="";
	
	memset(szSD,'\0',MAX_SD+1);
	memset(szABT,'\0',MAX_ABT+1);


	if (strcmp(config.stats,TRACEOFF)==0)
		return;

	/* poner cabecera al fichero de estadistica si es necesario
	if (!exists(config.statisticsFile))
		strcpy(est,"AQUI LA CABECERA DE LAS ESTADISTICAS\n");
	*/
	
	/*comprobamos si ha cambiado el dia */
/*	traza(INFOL,"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	traza(INFOL,"config.statisticsFile [%s] ", config.statisticsFile);
	traza(INFOL,"getStatisticsFileName [%s]", getStatisticsFileName());
	traza(INFOL,"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");	*/
	if (strcmp(config.statisticsFile,getStatisticsFileName())!=0)
	{
		strcpy(config.statisticsFile,getStatisticsFileName());
		if( fpE != NULL )	/* ### Cerrar el anterior */ 
			fclose(fpE);
		fpE=fopen(config.statisticsFile,"a");
	}
	
	/*buscamos _SD en QUERY_STRING */
	strSZEnv = getenv("QUERY_STRING");
	if (strSZEnv != NULL) /*hay query string*/
	{
		strSZ = strstr(strSZEnv, "_SD=");
		if (strSZ != NULL)
		{
				strncpy(szSD, strSZ+4, MAX_SD);
				strSZ = strstr(szSD,"&");
				if (strSZ != NULL)
					memset(strSZ,'\0',1);
		}
	}
	
	if (szSD[0]==0) /*no hay SD en QS, lo buscamos en la entrada*/
	{
		strSZ = strstr(entrada, "_SD=");
		if (strSZ != NULL)
			strncpy(szSD, strSZ+4, MAX_SD);
			strSZ = strstr(szSD,"&");				
			if (strSZ != NULL)
				memset(strSZ,'\0',1);
	}
		
	if (szSD[0]==0) /*no hay SD en QS ni en entrada*/
		strcpy(szSD,"NO_SD");
		

	/* buscamos ABT en QUERY_STRING */
	strSZEnv = getenv("QUERY_STRING");
	if (strSZEnv != NULL)
	{
		strSZ = strstr(strSZEnv, "_ABT_FROM_PART=");
		if (strSZ != NULL)
		{
     	strncpy(szABT, strSZ+15, MAX_ABT);
     	strSZ = strstr(szABT,"&");
     	if (strSZ != NULL)
     		memset(strSZ,'\0',1);
    }
  }
  
	if (szABT[0]==0) /* ABT no esta en QS lo buscamos en la entrada */
	{    		
		/* si no esta en QS lo buscamos ABT en la entrada */
		strSZ = strstr(entrada, "_ABT_FROM_PART=");
		if (strSZ != NULL)
		{
			strncpy(szABT, strSZ+15, MAX_ABT);
			strSZ = strstr(szABT,"&");
			if (strSZ != NULL)
				memset(strSZ,'\0',1);		        	
		}
	}
		
	if (szABT[0]==0) /* ABT no esta en QS ni en entrada */
		strcpy(szABT,"NO_ABT");        		
	
		
	if (!fpE) /*si el fichero no esta abierto lo abrimos*/
		fpE=fopen(config.statisticsFile,"a");
	
	if( fpE )	/* ### Escribir linea */
	{
	sprintf(line,"@start\t%ld\t%s\t%s\t%s\t%s\t%d\t%d\t%s\t%s\n",p.dProcess.pid,p.fInicio,p.hInicio,p.fFin,p.hFin,p.nProcesosReg,p.status,szSD,szABT);
	strcat(est,line);
	fprintf(fpE,(char const *)est);
	fflush(fpE);
	/* fclose(fpE); */
	}
	
} /*fin estadistica*/

static void
#if defined(__STDC__) || defined(__cplusplus)
	interceptor (int sig)
#else
	interceptor (sig)
	int sig;
#endif
{
	traza(INFOL,"Señal recibida: %d",sig);
	signal (sig,  SIG_DFL);

	/*liberar proceso en la lista de registros*/
	traza(DEBUGL,"Liberar proceso de SHM y actualiza contadores generales");
	if (borra_proceso(config.IPCKEY, fProcess.tPos)>0)
		traza(WARNL,"Error al liberar proceso [%d] de SHM",fProcess.tPos);
	kill(getpid(), sig);

	return;
}
/* ************************************************************************************** */


/* funcion que incrementa y actualiza el contador que recibe como parametro*/
void incrementaContPro(int pos,	long pid,int contador)
{
	dat_proceso procesoAux;

	if (getContadoresProceso(&procesoAux,pos,pid));
	{
		switch (contador)
		{
			case OK:
				procesoAux.pEjecutadasOK++;
				procesoAux.tiempoEje+=(atol(fProcess.hFin)- atol(fProcess.hInicio));
			break;
			case TIMEOUT:
				procesoAux.pTimeout++;
			break;
			case ERRORGEN:
				procesoAux.pDown++;
			break;
			case MAXHARDPROCESS:
				procesoAux.pHLimit++;
			break;
			case MAXSOFTPROCESS:
				procesoAux.pSLimit++;
			break;
		};
		putContadoresProceso(&procesoAux,pos,pid);

		/*actualizamos los valores en el proceso*/
		fProcess.dProcess.pEjecutadasOK=procesoAux.pEjecutadasOK;
		fProcess.dProcess.pSLimit=procesoAux.pSLimit;
		fProcess.dProcess.pHLimit=procesoAux.pHLimit;
		fProcess.dProcess.pTimeout=procesoAux.pTimeout;
		fProcess.dProcess.pDown=procesoAux.pDown;
		fProcess.dProcess.tiempoEje=procesoAux.tiempoEje;
	}
}


/* funcion que redirecciona a la pagina de error*/
void errorPage (int error)
{
	switch (error)
	{
		case TIMEOUT:
			traza(DEBUGL,"------ LANZAMOS PAGINA TIMEOUT ----- ");
			traza(DEBUGL,"[%s]",config.paginaErrorGenerico);
			printf("Location: %s\n",config.paginaErrorGenerico);
			printf("\n");
			break;

		case ERRORGEN:
			traza(DEBUGL,"------ LANZAMOS PAGINA ERROR GENERICO-----");
			traza(DEBUGL,"[%s]",config.paginaErrorGenerico);
			printf("Location: %s\n",config.paginaErrorGenerico);
			printf("\n");
			break;

		case MAXHARDPROCESS:
			traza(DEBUGL,"------ LANZAMOS PAGINA HARD-ERROR -----");
			traza(DEBUGL,"[%s]",config.paginaErrorHard);
			printf("Location: %s\n",config.paginaErrorHard);
			printf("\n");
			break;

		case MAXSOFTPROCESS:
			traza(DEBUGL,"------ LANZAMOS PAGINA SOFT-ERROR -----");
			traza(DEBUGL,"[%s]",config.paginaErrorSoft);
			printf("Location: %s\n",config.paginaErrorSoft);
			printf("\n");
			break;

		default:
			break;
	};

	return;
}

static void
#if defined(__STDC__) || defined(__cplusplus)
	timer_handler (int sig)
#else
	timer_handler (sig)
	int sig;
#endif
{
	if( pid_comando != 0 )
	{
		kill(pid_comando, SIGKILL);
		pid_comando = 0;
	}

	return;
}

int set_signals()
{
	if ((signal (SIGTERM,  interceptor) == SIG_ERR) )
	{
		traza(ERRORL,"ERROR: %d  errno: %d, %s\n", -1, errno, "Error estableciendo senal SIGTERM");
		return -1;
	}
	if ((signal (SIGUSR2,  readConfigSignal) == SIG_ERR) )
	{
		traza(ERRORL,"ERROR: %d  errno: %d, %s\n", -1, errno, "Error estableciendo senal SIGUSR2");
		return -1;
	}
	return 0;
}
int unset_signals()
{
	signal (SIGTERM,  SIG_DFL);
	signal (SIGUSR2,  SIG_DFL);
	return 0;
}

int set_sigalarm()
{
	if ((signal (SIGALRM,  timer_handler) == SIG_ERR) )
	{
		traza(ERRORL,"ERROR: %d  errno: %d, %s\n", -1, errno, "Error estableciendo senal");
		return -1;
	}
	return 0;
}

int unset_sigalarm()
{
	signal (SIGALRM,  SIG_DFL);
	return 0;
}


int piped_child (const char **command, const char **env, FCGX_Stream * in, FCGX_Stream * out )
{
	int pid, stat_loc, nbytes, childPid, retorno,rt;
	int to_child_pipe[2], from_child_pipe[2], iPrimeraVez;
	char salida[MAXBUF];


	if (pipe (to_child_pipe) < 0)
	    traza(ERRORL,"ERROR: %d  errno: %d, %s\n", 1, errno, "cannot create pipe: to_child_pipe");
	if (pipe (from_child_pipe) < 0)
	    traza(ERRORL,"ERROR: %d  errno: %d, %s\n", 1, errno, "cannot create pipe: from_child_pipe");

	/* actualizar estado del proceso (OCUPADO y F.INI) antes del fork, para que el padre tenga los valores */
	fProcess.dProcess.fecha_inicio =getDateNum(TEPOCH);
	fProcess.dProcess.estado=PROCESO_OCUPADO;
	strcpy(fProcess.fInicio,getDate(TEPOCH));
	strcpy(fProcess.hInicio,getDate(TEPOCHMS));

	pid = fork ();

	if (pid < 0)
	{
		/* dejamos el estado anterior del proceso (OCUPADO y F.INI) antes del fork, para que el padre tenga los valores */
		fProcess.dProcess.fecha_inicio=0;
		fProcess.dProcess.estado=PROCESO_LIBRE;
		strcpy(fProcess.fInicio,"");
		strcpy(fProcess.hInicio,"");
		traza(ERRORL,"ERROR: %d  errno: %d, %s\n", 1, errno, "cannot fork process");
	}

	/* ***** Proceso hijo ***** */
	if (pid == 0)
	{
		if (dup2 (to_child_pipe[0], STDIN_FILENO) < 0)
		    traza(ERRORL,"ERROR: %d  errno: %d, %s\n", 1, errno, "cannot dup2 pipe: to_child_pipe[0]");

		if (close (to_child_pipe[1]) < 0)
		    traza(ERRORL,"ERROR: %d  errno: %d, %s\n", 1, errno, "cannot close: to_child_pipe[1]");

		if (close (from_child_pipe[0]) < 0)
		    traza(ERRORL,"ERROR: %d  errno: %d, %s\n", 1, errno, "cannot close: from_child_pipe[0]");

		if (dup2 (from_child_pipe[1], STDOUT_FILENO) < 0)
		    traza(ERRORL,"ERROR: %d  errno: %d, %s\n", 1, errno, "cannot dup2 pipe: to_child_pipe[1]");

		/*ponemos el pid del hijo*/
		fProcess.dProcess.pid_hijo=getpid();

		/* actualizar el proceso en la tabla SHM(OCUPADO)*/
		retorno=0;
		retorno=modifica_proceso(config.IPCKEY,fProcess.tPos , fProcess.dProcess);
		if (retorno > 0)
			traza(WARNL,"Error al modificar proceso [%d] en SHM con pid [%d]: retorno [%d]",fProcess.tPos,fProcess.dProcess.pid,retorno);
		else
		{
			traza(DEBUGL,"Proceso modificado [%d]",fProcess.tPos);
			trazaProceso(fProcess);
		}


		/* Ejecutar comando */
		traza (DEBUGL,"\t Ejecuta CGI");
		rt=execve ((char *)command[0], (char **)command, (char **) env);
		traza(ERRORL,"Cannot exec CGI. Return [%d]",rt);

		unset_signals();
	 	traza(ERRORL,"Process [%d] die",getpid());
		/* 		kill(getpid(), SIGTERM);       	*/
		/* 		kill(getpid(), SIGKILL);       */
		exit(EXIT_FAILURE);		/*resvisar salida del programa*/

	        /*ha habido error al ejecutar cgi*/
	        /* parar la alarma del time out
	        	 actualizar el estado del proceso
	        	 guardar estadistica
	        	 suicidate
        	 */
	} /*fin del proceso hijo hijo*/

  /* ***** Proceso padre ***** */
  set_sigalarm();
  pid_comando = pid;
  alarm(config.childTimeOut);

  if (close (to_child_pipe[0]) < 0)
      traza(ERRORL,"ERROR: %d  errno: %d, %s\n", 1, errno, "cannot close pipe: to_child_pipe[0]");
  if (close (from_child_pipe[1]) < 0)
      traza(ERRORL,"ERROR: %d  errno: %d, %s\n", 1, errno, "cannot close pipe: from_child_pipe[1]");

  traza(DEBUGL, "\tEntrada al comando:");   /* --- Leer entrada de stdin y pasarla al proceso hijo --- */
/* JM pasamos el FCGI_fgets a la funcion de lectura */
  traza(DEBUGL,"%s", entrada);

	retorno = write(to_child_pipe[1], entrada, strlen(entrada));

  if (close (to_child_pipe[1]) < 0)
	traza(ERRORL,"ERROR: %d  errno: %d, %s\n", 1, errno, "cannot close pipe: to_child_pipe[1]");

  traza(DEBUGL, "Salida del comando:");

  /** Leer salida de proceso hijo y devolverla como stdout
  quitaremos un Content-type para no confundir al mod_fastcgi  */
  iPrimeraVez=1;
  while( (nbytes = read(from_child_pipe[0], salida, sizeof(salida)-1)) > 0)
  {
		char *str;
		int i;

		salida[nbytes] = 0;
		if (iPrimeraVez )
		{
			str = strstr(salida, CONTENT_TYPE_TAG);
			if (str != NULL )
			{
				str = strstr(salida+ strlen(CONTENT_TYPE_TAG), CONTENT_TYPE_TAG);
				if (str != NULL )
				{
					memset(str, ' ', strlen(CONTENT_TYPE_TAG));
					str+= strlen(CONTENT_TYPE_TAG);
					for(i=0; str[i] != '\n'; i++)
						str[i] = ' ';
				}
			}
		}
		iPrimeraVez = 0;

		FCGI_fputs(salida, FCGI_stdout);

		/* Traza */
		traza(DEBUGL,"%s", salida);
  }

  /* cerramos la tuberia from_child_pipe[0] */
  close(from_child_pipe[0]);

  /** Esperar a que termine el hijo. */
  /*    childPid = waitpid(pid, &stat_loc, 0);		*/
  childPid = wait(&stat_loc);

	fProcess.dProcess.pid_hijo=childPid;

  alarm(0);
  unset_sigalarm();

  /* Traza */
  traza(DEBUGL,"retorno del hijo [%d]: stat_loc=[%x]",childPid,(short)stat_loc);
  
  /* actualizar estado del proceso en la tabla (LIBRE)*/
  fProcess.dProcess.fecha_fin=getDateNum(TEPOCH);
  fProcess.dProcess.estado=PROCESO_LIBRE;
  strcpy(fProcess.fFin,getDate(TEPOCH));
  strcpy(fProcess.hFin,getDate(TEPOCHMS));

  /*comprobamos el retorno del hijo*/
  if ((short)stat_loc==9)
		if (pid_comando==0)
		{
			traza(INFOL,"Excedido time out del hijo");		/* excedido CHILDTIMEOUT*/
			fProcess.status=TIMEOUT;

			/*actualizamos los contadores del proceso*/
			incrementaContPro(fProcess.tPos,fProcess.dProcess.pid,TIMEOUT);

			/*lanzamos la pagina de error*/
			errorPage(TIMEOUT);
		}
		else
		{
			traza(INFOL,"El hijo esta tardando mucho en responder");	 /*time out de apache*/
			fProcess.status=ERRORGEN;

			/*actualizamos los contadores del proceso*/
			incrementaContPro(fProcess.tPos,fProcess.dProcess.pid,ERRORGEN);

			/*lanzamos la pagina de error*/
			errorPage(ERRORGEN);
		}
  else if ((short)stat_loc==0)		/*cgi termina ok*/
  {
		traza(DEBUGL,"Termina CGI");
		fProcess.status=OK;

		/*leemos los contadores del proceso y los actualizamos*/
		incrementaContPro(fProcess.tPos,fProcess.dProcess.pid,OK);

	}

  /* actualizar estado del proceso en la tabla (LIBRE)*/
/*  fProcess.dProcess.fecha_fin=getDateNum(TEPOCH);
  fProcess.dProcess.estado=PROCESO_LIBRE;
  strcpy(fProcess.fFin,getDate(TEPOCH));
  strcpy(fProcess.hFin,getDate(TEPOCHMS));
*/
  

  /*escribimos fichero de estadisticas*/
  trazaProceso(fProcess);
  estadistica(fProcess);


  /* si el proceso no ha podido ser insertado en la tabla su posicion es 0*/
  /* comprobamos si es 0 y lo volvemos a insertar*/

  retorno=0;
	if (fProcess.tPos==0) /*el proceso no fue insertado --> lo insertamos*/
  {
		retorno=inserta_proceso(&fProcess.tPos, fProcess.dProcess, configSHM);		/* pasar dat_configuracion en lugar del ipc*/
		if (retorno>0)
			traza(WARNL,"DE NUEVO proceso NO insertado en shm. retorno [%d]",retorno);
		else
		{
			traza(INFOL,"POR FIN proceso insertado correctamente en pos [%d]",fProcess.tPos);
			trazaProceso(fProcess);
		}
	}
  else /*se inserto correctamente --> lo modificamos*/
	{
		retorno=modifica_proceso(config.IPCKEY,fProcess.tPos , fProcess.dProcess);
		if (retorno>0)
			traza(WARNL,"Error al modificar proceso en SHM: retorno [%d]",retorno);
		else
	  {
			traza(DEBUGL,"Proceso modificado [%d]",fProcess.tPos);
			trazaProceso(fProcess);
		}
	}

	traza(DEBUGL,"Termina hijo con status: %d - Pid = [%d]", (short)stat_loc,childPid);

	return pid;
}
/* ************************************************************************************** */



static int parseIni(void* user, const char* section, const char* name, const char* value)
{
	char aviso[MAXBUF]="";

	configuration* pconfig = (configuration*)user;

	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("config", "logName")) {
        strcpy(pconfig->logName,strdup(value));
    } else if (MATCH("config", "cgiProgram")) {
        strcpy(pconfig->cgiProgram,strdup(value));
/*    } else if (MATCH("config", "IPCKEY")) {
        pconfig->IPCKEY= atol(value);        */
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
    } else if (MATCH("config", "stats")) {
        strcpy(pconfig->stats,strdup(value));
    } else {
				sprintf(aviso,"Clave no usada. [%s] %s = %s\n", section, name, value);
				strcat(msg,aviso);
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

/*funcion que comprueba si hemos llegado al limite de procesos*/
int checkBusyProcess()
{
	int nProcesos=0;
	int retConsulta;
	retConsulta=consulta_peticiones(config.IPCKEY,config.maxBusyProcess,&nProcesos);

	if (retConsulta==0 || retConsulta==ERR_LECTURA)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

/*funcion que devuelve el numero de procesos ocupados*/
int getBusyProcess()
{
	int nProcesos=0;
	int retConsulta;
	retConsulta=consulta_peticiones(config.IPCKEY,config.maxBusyProcess,&nProcesos);

	return nProcesos;
}

	/* ### Lectura de stdin */
int getIn()
{
	int tam=0;
  int posicion=0;

	/* la entrada es de tama¤o fijo (16 KB), posible asignacion dinamica */

	entrada[0] = 0;
	posicion = 0;
	while (( FCGI_fgets(entrada + posicion, MAXBUF, FCGI_stdin)) != NULL )
	{
		tam = strlen(entrada);
		posicion = tam;
		if (tam >= tam_entrada - 1)
		{						/* ### Redimensionar buffer de entrada */
			char * nueva_entrada;
			nueva_entrada = realloc(entrada, tam_entrada + MAXBUF);
			if( nueva_entrada )
			{
				entrada = nueva_entrada;
				tam_entrada += MAXBUF;
			}
			else
			{		/* ### Error : se conserva el buffer entrada original */
				traza(ERRORL,"ERROR redimensionando bufffer de entrada: %d", tam_entrada+ MAXBUF);
				return -1;
			}
		}
	}

	return 0;
}


int tieneSesion()
{
	char *str;

	str = strstr(entrada, NOSESIONTEXT);
	return (str == NULL );
}


char* getDate(int type)
{
	static char sdate[MAXPATH];
	struct timespec spec;
	long ms;
	/*long roundMs;*/ /* Milliseconds */

	/* generamos la mascara de fecha*/
	time_t nw,s;	 /* s son segundos */


	nw=time(NULL);  /*igual a time(&nw) */
	if (type==TFICHERO)	 /*tipo fichero*/
		strftime(sdate,MAXPATH,".%y%m%d",localtime(&nw));
	else if (type==TTRAZA) /*tipo traza*/
		strftime(sdate,MAXPATH,"%Y/%m/%d %H:%M:%S",localtime(&nw));
	else if (type == TEPOCH) /*tipo estadistica*/
	{
		struct tm * timeInfo;
		timeInfo = localtime(&nw);
		sprintf(sdate,"%d",nw);
	}
	else if (type == TEPOCHMS) /*tipo estadistica con milisegundos*/
	{
		clock_gettime(CLOCK_REALTIME, &spec);
		/* roundMs = round(spec.tv_nsec / 1.0e7); */ /*centesimas de segundo*/
		ms = spec.tv_nsec / 1.0e7;
		s  = spec.tv_sec;

		sprintf(sdate,"%ld%02ld",(intmax_t)s,ms);
	}

	return sdate;
}


long getDateNum(int type)
{
/*	char dateAux[MAXPATH]; */
	long sdate=0;
	time_t nw;

	nw=time(NULL);  /*igual a time(&nw) */
	if (type == TEPOCH)
	{
		struct tm * timeInfo;
		timeInfo = localtime(&nw);
		sdate=nw;
	}
/*	sdate=atol(getDate(type)); */
	return sdate;
}


/*funcion que genera el nombre del archivo de log*/
void getLogFileName(char* name)
{
	char sdate[MAXPATH];

	strcpy(sdate,getDate(TFICHERO));
	sprintf(name,"%s%s.log",config.logName,sdate);
}

/*funcion que genera el nombre del archivo de estadisticas*/
char* getStatisticsFileName()
{
	static char name[MAXPATH];
	char sdate[MAXPATH];

	strcpy(sdate,getDate(TFICHERO));
	sprintf(name,"%s%s.trc",config.statisticsName,sdate);

	return name;
}

void readConfigSignal(int sig)
{
	readConf = 1;
	if ((signal (SIGUSR2,  readConfigSignal) == SIG_ERR) )
		traza(ERRORL,"ERROR: %d  errno: %d, %s\n", -1, errno, "Error estableciendo senal SIGUSR2");

}


void readConfig()
{
	char sdate[MAXPATH];

	/*reiniciamos la lectura de la configuracion*/
	readConf =0;

	strcpy(msg,"");

	strcpy(sdate,getDate(TFICHERO));

	/*		Cargamos la configuracion del fichero	si es necesario*/
	if (configFile[0]==0)
		if (getenv("FASTCGI_CONFIG")!=NULL)
				strcpy(configFile,getenv("FASTCGI_CONFIG"));
		else
		{
			strcpy(configFile,DEFAULTCONFIGFILE);
		}

	/*por defecto siempre off (por si se olvida de poner en el config)*/
	strcpy(config.stats,DEFAULTTRACE);

	if (ini_parse(configFile, parseIni, &config) < 0)
	{
		strcpy(config.logName,DEFAULTLOGNAME);
		strcpy(config.cgiProgram,DEFAULTCGIPROGRAM);
		strcpy(config.statisticsName,DEFAULTSTATISTICSNAME);
		config.traceLevel = DEFAULTTRACELEVEL;
		config.childTimeOut = DEFAULTCHILDTIMEOUT;
		config.maxProcess = DEFAULTMAXPROCESS;
		config.maxBusyProcess = DEFAULTMAXBUSYPROCESS;
		config.maxSoftBusyProcess = DEFAULTMAXSOFTBUSYPROCESS;

		config.IPCKEY = ftok(DEFAULTIPCDIR, SEED);

		/*generamos el nombre del archivo de log*/
		getLogFileName(config.logFile);

		/*generamos el nombre del archivo de estadisticas */
		strcpy(config.statisticsFile,getStatisticsFileName());

		traza(ERRORL,"Error al leer el fichero de configuracion %s. Se establece configuracion por defecto.",configFile);
	}
	else
	{
		/*generamos el nombre del archivo de log*/
		getLogFileName(config.logFile);

		/*generamos el nombre del archivo de estadisticas */
		strcpy(config.statisticsFile,getStatisticsFileName());

		config.IPCKEY = ftok(configFile, SEED);

		traza(DEBUGL,"Cargando configuracion desde %s",configFile);

		if (!exists(config.statisticsFile))
			traza(INFOL,"Fichero de estadisticas no existe");

	}

	/*inicializamos la configuracion de la shm*/
	configSHM.max_procesos = config.maxProcess;
	configSHM.clave = config.IPCKEY;

	/* configuracion leida */
	traza(DEBUGL," CONFIGURACION:");
	traza(DEBUGL," logName = [%s]",config.logName);
	traza(DEBUGL," logFile = [%s]",config.logFile);
	traza(DEBUGL," program = [%s]",config.cgiProgram);
	traza(DEBUGL," statisticsName = [%s]",config.statisticsName);
	traza(DEBUGL," statisticsFile = [%s]",config.statisticsFile);
	traza(DEBUGL," traceLevel = [%d]",config.traceLevel);
	traza(DEBUGL," childTimeOut = [%d]",config.childTimeOut);
	traza(DEBUGL," IPCKEY = [%ld]",config.IPCKEY);
	traza(DEBUGL," maxProcess = [%d]",config.maxProcess);
	traza(DEBUGL," maxBusyProcess = [%d]",config.maxBusyProcess);
	traza(DEBUGL," maxSoftBusyProcess = [%d]",config.maxSoftBusyProcess);

	traza(DEBUGL,"paginaErrorGenerico = [%s]",config.paginaErrorGenerico);
	traza(DEBUGL,"paginaErrorSoft = [%s]",config.paginaErrorSoft);
	traza(DEBUGL,"paginaErrorHard = [%s]",config.paginaErrorHard);
	traza(DEBUGL,"Activacion estadistica = [%s]",config.stats);
}


/*int main(void) */
int main(int argc, char *argv[])
{
	char *argv_exec[MAX_ARG];
	char *envp_exec[MAX_ENV];
	int i, ret, retorno, maxBP=0;

	FCGX_Init();

	readConfig();

	traza(DEBUGL,">>>>>>> Inicio Proceso Fast CGI <<<<<<<");

	/* inicializamos los datos del proceso */
	/*fecha de registro del proceso en la shm*/
	fProcess.dProcess.fecha_registro=getDateNum(TEPOCH);
	traza(DEBUGL, "fecha: %ld", fProcess.dProcess.fecha_registro);
	fProcess.dProcess.pid=getpid();
	fProcess.dProcess.ppid=getppid();
	fProcess.dProcess.pid_hijo=0;
	fProcess.dProcess.estado=PROCESO_LIBRE;
	fProcess.nProcesosReg=0;
	fProcess.status=OK;
	fProcess.dProcess.fecha_inicio=0;
	fProcess.dProcess.fecha_fin=0;
	strcpy(fProcess.fInicio,"0000000000");
	strcpy(fProcess.hInicio,"000000000000");
	strcpy(fProcess.fFin,"0000000000");
	strcpy(fProcess.hFin,"000000000000");
	fProcess.dProcess.pEjecutadasOK=0;
	fProcess.dProcess.pSLimit=0;
	fProcess.dProcess.pHLimit=0;
	fProcess.dProcess.pTimeout=0;
	fProcess.dProcess.pDown=0;
	fProcess.dProcess.tiempoEje=0;


	/* registramos el proceso en la tabla*/
	traza(DEBUGL, "Registrando proceso SHM...");
	retorno=0;
	retorno=inserta_proceso(&fProcess.tPos, fProcess.dProcess, configSHM);		/* pasar dat_configuracion en lugar del ipc*/

	if (retorno>0)
		traza(INFOL,"Proceso NO insertado en shm. retorno [%d] [%d]",retorno,fProcess.dProcess.pid);
	else
		traza(DEBUGL,"Proceso insertado correctamente en pos [%d]",fProcess.tPos);

	trazaProceso(fProcess);

	set_signals();


	/* Rellenar lista de argumentos : la misma que llega */

	argv_exec[0] = malloc(strlen(config.cgiProgram) + 1);	  /* FASE2 */
	strcpy(argv_exec[0], config.cgiProgram);

	for(i = 1; i < argc; i++)    /* al llamar desde el IHS  argv[1] contiene "" por lo que cuenta 1 param. */
	{
		argv_exec[i] = malloc(strlen(argv[i]) + 1);
		strcpy(argv_exec[i], argv[i]);
		traza(DEBUGL,"%s",argv_exec[i]);
	}
	argv_exec[i] = NULL;

		/* ### Adquirir buffer de entrada */
	tam_entrada = MAXBUF;
     if( ! (entrada = malloc(tam_entrada)) )
	{
		traza(ERRORL, "ERROR adquiriendo memoria para entrada");
		return -1;
	}

	while (FCGI_Accept() >= 0)
	{
		/* --- INI tto. version --- */
		if( argc == 2 )
		{
			if( !strcmp(argv_exec[1], "-T") )
				printf("READABLE_UNIX_FD_DROP_DEAD_TIMEVAL: %d\n", READABLE_UNIX_FD_DROP_DEAD_TIMEVAL);
			
			if( !strcmp(argv[1], "-V") )
				printf("%s version: %s\n", argv[0], START_VERSION);

			return;
		}
		/* --- FIN tto. version --- */

		if (readConf)
		{
			traza(INFOL,"Recarga la configuracion en caliente");
			readConfig();
		}
		/*regeneramos el nombre del fichero de log (rotacion de logs) */


		/* ### leemos el stdin*/
		if( getIn() != 0 )
		{	/* error de lectura */
			FCGI_Finish();
			continue;
		}


		/* Rellenar lista de variables de entorno */
		for(i = 0; (i<MAX_ENV && environ[i]!= NULL); i++)
		{
			envp_exec[i] = malloc(strlen(environ[i]) + 1);
			strcpy(envp_exec[i], environ[i]);
			traza(DEBUGL,"%s",environ[i]);
		}
		envp_exec[i] = NULL;

		/*obtenemos el numero de procesos registrados*/
		fProcess.nProcesosReg=consulta_registrados();

		if (fProcess.nProcesosReg > config.maxSoftBusyProcess)
		{
			/*obtenemos el numero de procesos ocupados*/
			maxBP=getBusyProcess();
			if (maxBP>config.maxBusyProcess) /* numero de procesos ocupados > hard limit*/
			{
				/*numero de procesos ocupados ha llegado al limite*/
				/* actualizamos el proceso y registramos la estadistica */
				fProcess.status=MAXHARDPROCESS;
				fProcess.dProcess.pid_hijo=0;
				strcpy(fProcess.fFin,"0000000000");
				strcpy(fProcess.hFin,"000000000000");
				/* actualizar estado del proceso */
				strcpy(fProcess.fInicio,getDate(TEPOCH));
				strcpy(fProcess.hInicio,getDate(TEPOCHMS));

				/*actualizamos los contadores del proceso*/
				incrementaContPro(fProcess.tPos,fProcess.dProcess.pid,MAXHARDPROCESS);

				/*escribimos la estadistica*/
				estadistica(fProcess);
				traza(WARNL,"El numero de procesos ocupados [%d] ha llegado al limite\n",maxBP);  /* 2014-10-20  nivel de traza pasa de INFOL a WARNL */

				errorPage(MAXHARDPROCESS); 	/*Rechazo 99*/
			}
			else if ((maxBP> config.maxSoftBusyProcess) && (!tieneSesion())) /*estamos entre el soft limit y el hard limit*/
			{
				/*el numero de procesos ocupado ha llegado al limite para usuarios sin sesion creada*/
				/* actualizamos el proceso y registramos la estadistica */
				fProcess.status=MAXSOFTPROCESS;
				fProcess.dProcess.pid_hijo=0;
				strcpy(fProcess.fFin,"0000000000");
				strcpy(fProcess.hFin,"000000000000");
				/* actualizar estado del proceso */
				strcpy(fProcess.fInicio,getDate(TEPOCH));
				strcpy(fProcess.hInicio,getDate(TEPOCHMS));

				/*actualizamos los contadores del proceso*/
				incrementaContPro(fProcess.tPos,fProcess.dProcess.pid,MAXSOFTPROCESS);

				/*escribimos la estadistica*/
				estadistica(fProcess);
				traza(WARNL,"El numero de procesos ocupados [%d] ha llegado al limite para usuarios sin sesion\n",maxBP);  /* 2014-10-20  nivel de traza pasa de INFOL a WARNL */

				errorPage(MAXSOFTPROCESS);			/*Rechazo 66*/
			}
			else /*procesos OK*/
			{
				traza(DEBUGL,"Llamando CGI ");
				ret = piped_child((const char **) argv_exec, (const char **) envp_exec, NULL,NULL);
				traza(DEBUGL,"Fin llamada CGI - Retorno [%d]",ret);
			}
		}
		else /*procesos OK*/
		{
			traza(DEBUGL,"Llamando CGI ");
			ret = piped_child((const char **) argv_exec, (const char **) envp_exec, NULL,NULL);
			traza(DEBUGL,"Fin llamada CGI - Retorno [%d]",ret);
		}

	  /* finalizamos y liberamos variables*/
		for(i = 0; (i<MAX_ENV && environ[i]!= NULL); i++)
			free(envp_exec[i]);

	  FCGI_Finish(); /*pte version 2*/
	}

		/* ### Liberar buffer de entrada */
	if( entrada != NULL )
		free(entrada);

	return 0;
}
