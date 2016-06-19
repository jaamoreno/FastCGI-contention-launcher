#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <sys/shm.h>
#include <stddef.h>
#include <procinfo.h>
#include <sys/types.h>

#include "cshmem.h"
#include "api_blk.h"

#include "api_shm.h"	
#include "control_shm.h"


static 	tabla_shm *tabla = NULL;


/* -------------------------------------------------------------------
* FUNCION               crea_tabla(long,dat_configuracion)
* DESCRIPCION   crea un area de memoria compartida (shm)
* PARAMETROS
*                       pid	  	numero fijo de pid para crear la tabla
*                       conf  	estructura con datos de configuración
*
* RETORNO               0 (exito) , >0 (error)
 ------------------------------------------------------------------- */

int crea_tabla (long pid, dat_configuracion  conf)	
{
	int ret=0;
	long tamanio=0;
	dat_proceso dprocesos;
	dat_global dglobal;
	int locksem;
	int shm_creada = 0;
	
	char hora[20];
	time_t tiempo = time(0);
	struct tm *tlocal = localtime(&tiempo);
	strftime(hora,20,"%Y%m%d%H%M%S",tlocal);
  
	// calcular tamanio de SHM 
	tamanio=conf.max_procesos * sizeof(dat_proceso); /*cambio la variable por el tamaño del tipo*/
	tamanio=tamanio+sizeof(dat_global);	/*cambio la variable por el tamaño del tipo*/

	// inicializar estructura de control shm 
	tabla = Map2SharedMem(conf.clave, tamanio, &shm_creada);
	//printf("conf.clave = %ld \n", conf.clave);

	if( ! tabla )
	{
	  	display_error("API_SHM: fallo accediendo a la memoria", errno);
		return SHM_OPEN;
	}

	/* Si somos el primer proceso, inicializamos el semaforo a 1*/
	locksem = SemCreate(conf.clave, 1);
	if (locksem == -1)
	{
		display_error("API_SHM: fallo creando semaforo", errno);
		return ERR_SEM_OPEN;
	}
	if( shm_creada )
	{
		Lock();

		/* iniciar valores */
		tabla->dglobal.num_proc_registrados = 0;
		tabla->dglobal.num_max_procesos = conf.max_procesos;
		tabla->dglobal.totalEjecutadas = 0;
		tabla->dglobal.totalSLimit = 0;
		tabla->dglobal.totalHLimit = 0;
		tabla->dglobal.totalTimeout = 0;
		tabla->dglobal.totalDown = 0;
		tabla->dglobal.totalTiempoEje = 0;
		strcpy(tabla->dglobal.fecha_ult_reset,hora);

		tabla->dglobal.parcialEjecutadas = 0;
		tabla->dglobal.parcialSLimit = 0;
		tabla->dglobal.parcialHLimit = 0;
		tabla->dglobal.parcialTimeout = 0;
		tabla->dglobal.parcialDown = 0;
		tabla->dglobal.parcialTiempoEje = 0;
		strcpy(tabla->dglobal.fecha_ult_acum,hora);		
		
		Unlock();
	}

	return EXITO;
}	

/* -------------------------------------------------------------------
* FUNCION               inserta_proceso(long, int*, dat_proceso)
* DESCRIPCION   		inseerta los datos de un proceso en la memoria compartida (shm)
* PARAMETROS
*                       pid	  	numero fijo de pid del area de memria compartida shm
*                       clave  	puntero de salida de la posicion en la tabla del registro
*			dproc	estructura con los datos del proceso
*
* RETORNO               0 (exito) , >0 (error)
 ------------------------------------------------------------------- */
 
int inserta_proceso(int* id, dat_proceso dproc, dat_configuracion  conf)
{
  int ret=0;
	int i=0,j=0;
	short encontrado=0;

	int num_procesos=0;
	int max_procesos=0;
	
	// crear tabla si no existe
	if( ! tabla )
	{
	ret = crea_tabla (getpid(), conf);	
	if ( ret )
		return ret;
	}

	// Acceso a seccion critica 
	Lock();

	// leer y actualizar area shm
	num_procesos=tabla->dglobal.num_proc_registrados;
	max_procesos=tabla->dglobal.num_max_procesos;

	// recorrer array de estructuras para ver que registro esta vacio
	while (i<=max_procesos)
	{
		if (tabla->dproceso[i].pid == 0) 
		{
			// se ha encontrado sitio vacio
			j=i+1;
			*id = j;
			tabla->dglobal.num_proc_registrados++;
			tabla->dproceso[i].pid=dproc.pid;
			tabla->dproceso[i].pid_hijo=dproc.pid_hijo;
			tabla->dproceso[i].ppid=dproc.ppid;
			strcpy(tabla->dproceso[i].id_sesion,dproc.id_sesion);
			tabla->dproceso[i].fecha_registro = dproc.fecha_registro;
			tabla->dproceso[i].fecha_inicio = dproc.fecha_inicio;
			tabla->dproceso[i].fecha_fin = dproc.fecha_fin;
			tabla->dproceso[i].estado=dproc.estado;
			tabla->dproceso[i].pEjecutadasOK=dproc.pEjecutadasOK;
			tabla->dproceso[i].pSLimit=dproc.pSLimit;
			tabla->dproceso[i].pHLimit=dproc.pHLimit;    
			tabla->dproceso[i].pTimeout=dproc.pTimeout;   
			tabla->dproceso[i].pDown=dproc.pDown; 
			tabla->dproceso[i].tiempoEje=dproc.tiempoEje;
			encontrado=1;
			break;
		}
		i++;
	}

	Unlock();

	// Hacer limpieza de procesos anteriores no bien terminados
	chequeo_peticiones(0);

	if (encontrado==0)
	{
		*id=0;
        	return ERR_TABLA_LLENA;
	}

    	return EXITO;
}


/* -------------------------------------------------------------------
* FUNCION               modifica_proceso(long, int*, dat_proceso)
* DESCRIPCION   		modifica los datos de un proceso en la memoria compartida (shm)
* PARAMETROS
*                       clave  	numero fijo de pid del area de memria compartida shm
*                       id  	puntero de salida de la posicion en la tabla del registro
*						pro		estructura con los datos del proceso
*
* RETORNO               0 (exito) , >0 (error)
 ------------------------------------------------------------------- */
 
int modifica_proceso(long clave, int id, dat_proceso pro)
{
	int ret=0;
	int registro=0;
	
	if (id <= 0)
		return ERR_POS_TABLA;

	if ( ! tabla )
		return ERR_SHM_NO_EXISTE;


	// ver si tiene datos el proceso
	registro=id-1;
	if (tabla->dproceso[registro].pid <= 0 )
	{
		return ERR_REG_VACIO;
	}

	// ver si el pid es diferente 
	if (tabla->dproceso[registro].pid != pro.pid)
	{
		return ERR_PID_DIFERENTE;		
	}
	
	// modificar datos
	//tabla->dproceso[registro].pid=pro.pid;
        //strcpy(tabla->dproceso[registro].fecha_registro,pro.fecha_registro);
	tabla->dproceso[registro].pid_hijo=pro.pid_hijo;
	strcpy(tabla->dproceso[registro].id_sesion,pro.id_sesion);
        tabla->dproceso[registro].fecha_inicio = pro.fecha_inicio;
        tabla->dproceso[registro].fecha_fin = pro.fecha_fin;
     	tabla->dproceso[registro].estado=pro.estado;

	return EXITO;
}


/* -------------------------------------------------------------------
* DESCRIPCION   		devuelve los contadores de un proceso
* PARAMETROS
*                       p estructura de tipo dat_procesos donde devolver los valores
*                       pos  	posicion de la tabla que queremos leer
												pid 	pid del proceso para verificacion
*
* RETORNO               0 (exito) , >0 (error)
 ------------------------------------------------------------------- */
int getContadoresProceso(dat_proceso* p, int pos,	long pid)
{
	if (pos <= 0)
		return ERR_POS_TABLA;

	if ( ! tabla )
		return ERR_SHM_NO_EXISTE;	
		
	if (tabla->dproceso[pos-1].pid <= 0 )
		return ERR_REG_VACIO;

	// ver si el pid es diferente 
	if (tabla->dproceso[pos-1].pid != pid)
		return ERR_PID_DIFERENTE;		
	
	p->pEjecutadasOK=tabla->dproceso[pos-1].pEjecutadasOK;
	p->pSLimit=tabla->dproceso[pos-1].pSLimit;
	p->pHLimit=tabla->dproceso[pos-1].pHLimit;    
	p->pTimeout=tabla->dproceso[pos-1].pTimeout;   
	p->pDown=tabla->dproceso[pos-1].pDown;
	p->tiempoEje=tabla->dproceso[pos-1].tiempoEje;
	
	return EXITO;
}

/* -------------------------------------------------------------------
* DESCRIPCION   		actualiza los contadores de un proceso en la tabla shm
* PARAMETROS
*                       p estructura de tipo dat_procesos donde van los valores
*                       pos  	posicion de la tabla que queremos modificar
												pid 	pid del proceso para verificacion
*
* RETORNO               0 (exito) , >0 (error)
 ------------------------------------------------------------------- */
int putContadoresProceso(dat_proceso* p, int pos,	long pid)
{
	if (pos <= 0)
		return ERR_POS_TABLA;

	if ( ! tabla )
		return ERR_SHM_NO_EXISTE;	
		
	if (tabla->dproceso[pos-1].pid <= 0 )
		return ERR_REG_VACIO;

	// ver si el pid es diferente 
	if (tabla->dproceso[pos-1].pid != pid)
		return ERR_PID_DIFERENTE;		
	
	tabla->dproceso[pos-1].pEjecutadasOK=p->pEjecutadasOK;
	tabla->dproceso[pos-1].pSLimit=p->pSLimit;
	tabla->dproceso[pos-1].pHLimit=p->pHLimit;    
	tabla->dproceso[pos-1].pTimeout=p->pTimeout;   
	tabla->dproceso[pos-1].pDown=p->pDown;  
	tabla->dproceso[pos-1].tiempoEje = p->tiempoEje;
	
	return EXITO;
}

/* -------------------------------------------------------------------
* FUNCION               borra_proceso(long, int)
* DESCRIPCION   		borra los datos de un proceso en la memoria compartida (shm)
* PARAMETROS
*                       clave  	numero fijo de pid del area de memria compartida shm
*                       id  	puntero de salida de la posicion en la tabla del registro
*
* RETORNO               0 (exito) , >0 (error)
 ------------------------------------------------------------------- */
 
int borra_proceso(long clave, int id)
{
	int ret=0;
	int registro=0;
	
	// controlar id
	if (id <= 0)
		return ERR_POS_TABLA;

	if ( ! tabla )
		return ERR_SHM_NO_EXISTE;

        // ver si tiene datos el proceso
	registro=id-1;
     	if (tabla->dproceso[registro].pid <= 0 )
	{
          return ERR_REG_VACIO;
	}

        // ver si el pid es diferente
        //if (tabla->dproceso[registro].pid != pro.pid)
        //{
        //        SemClear(shm->Acc, shm->num_lectores);
        //        free(shm->Acc);
        //        free(shm);
        //        return ERR_PID_DIFERENTE;

        //}

	// Acceso a seccion critica 
	Lock();

        // modificar datos
	if (tabla->dglobal.num_proc_registrados > 0)
     		tabla->dglobal.num_proc_registrados--;
     	tabla->dproceso[registro].pid=0;
	tabla->dproceso[registro].ppid=0;
	tabla->dproceso[registro].pid_hijo=0;
     	tabla->dproceso[registro].fecha_registro=0;
     	tabla->dproceso[registro].fecha_inicio=0;
     	tabla->dproceso[registro].fecha_fin=0;
     	tabla->dproceso[registro].estado=0;
     	
     	/*actualizamos los contadores*/
			tabla->dglobal.totalEjecutadas+=tabla->dproceso[registro].pEjecutadasOK;		
			tabla->dglobal.totalSLimit+=tabla->dproceso[registro].pSLimit;		
			tabla->dglobal.totalHLimit+=tabla->dproceso[registro].pHLimit;		
			tabla->dglobal.totalTimeout+=tabla->dproceso[registro].pTimeout;	
			tabla->dglobal.totalDown+=tabla->dproceso[registro].pDown;		
			tabla->dglobal.totalTiempoEje+=tabla->dproceso[registro].tiempoEje;

	Unlock();

     return EXITO;
}


int borra_proceso_pid(long clave, int id, dat_proceso pro)
{
     int ret=0;
     int registro=0;
		
		// controlar id
	if (id <= 0)
		return ERR_POS_TABLA;

	if ( ! tabla )
		return ERR_SHM_NO_EXISTE;

        // ver si tiene datos el proceso
        registro=id-1;
        if (tabla->dproceso[registro].pid <= 0 )
        {
                return ERR_REG_VACIO;
        }

        // ver si el pid es diferente
        if (tabla->dproceso[registro].pid != pro.pid)
        {
                return ERR_PID_DIFERENTE;

        }

	// Acceso a seccion critica 
	Lock();

        // modificar datos
	if (tabla->dglobal.num_proc_registrados > 0)
        	tabla->dglobal.num_proc_registrados--;
        tabla->dproceso[registro].pid=0;
	tabla->dproceso[registro].ppid=0;
        tabla->dproceso[registro].pid_hijo=0;
        strcpy(tabla->dproceso[registro].id_sesion,"");
        tabla->dproceso[registro].fecha_registro=0;
        tabla->dproceso[registro].fecha_inicio=0;
        tabla->dproceso[registro].fecha_fin=0;
        tabla->dproceso[registro].estado=0;

	Unlock();

        return EXITO;
}

/* -------------------------------------------------------------------
* FUNCION               consulta_peticiones(long, int, int*)
* DESCRIPCION   	consulta el numero de procesos registrados en memoria compartida (shm)
* PARAMETROS
*                       clave	  	numero fijo de pid del area de memroia compartida shm
*			max_peticiones	número máximo de peticiones que puede haber en vuelo
*                       num_procesos  	puntero de salida del numero de procesos activos
*
* RETORNO               0 (exito) , >0 (error)
 ------------------------------------------------------------------- */
 
int consulta_peticiones(long clave, int max_peticiones, int* num_peticiones)
{
     	int ret=0;
	int reg=0;
     	int i=0;
     	int suma=0;
	int num_activos=0;
	int max_procesos = 0;

	if ( ! tabla )
		return ERR_SHM_NO_EXISTE;

	// hallar peticiones en vuelo.
	reg=tabla->dglobal.num_proc_registrados;
	max_procesos=tabla->dglobal.num_max_procesos;

	for (i=0; i<max_procesos; i++)
	{
			if (tabla->dproceso[i].pid > 0)
  		{
					if (tabla->dproceso[i].estado > 0)
					{
						suma++;
						num_activos++;
					}
		
					if (suma == tabla->dglobal.num_proc_registrados)
		      	break;
			}
	}

	// validar el numero de peticiones
	if (num_activos >= max_peticiones)
	{
		*num_peticiones=num_activos;
		return FALLO;
	}

	*num_peticiones=num_activos;
	
	return EXITO;
}

/* -------------------------------------------------------------------
* FUNCION               con_tabla_sinSem(long)
* DESCRIPCION   	consulta la tabla shm completa
* PARAMETROS
*                       clave	  	numero fijo de pid del area de memroia compartida shm
*
* RETORNO               tabla		estructura de datos de shm
 ------------------------------------------------------------------- */
 //tabla_shm* consulta_tabla_shm_sinSem(long clave)
 tabla_shm* con_tabla_sinSem(long clave) 			
{
	return tabla;
}

/* -------------------------------------------------------------------
* FUNCION               consulta_tabla_shm(long)
* DESCRIPCION   	consulta la tabla shm completa
* PARAMETROS
*                       clave	  	numero fijo de pid del area de memroia compartida shm
*
* RETORNO               tabla		estructura de datos de shm
 ------------------------------------------------------------------- */
 
tabla_shm* consulta_tabla_shm(long clave)
{
	return tabla;
}



int imprime_tabla(tabla_shm tabla)
{
        int reg= tabla.dglobal.num_proc_registrados;
        int i=0;
        int suma=0;

        // como hay huecos, imprime en numero de registros totales
        // buscando los que tienen datos
        for (i=0; i<tabla.dglobal.num_max_procesos; i++)
        {
                if (tabla.dproceso[i].pid > 0)
                {
                        printf("\t -------------------------------- \n");
                        printf("\t REGISTRO %d \n", i+1);
                        printf("\t pid = %ld\n",tabla.dproceso[i].pid);
                        printf("\t pid_hijo = %ld\n",tabla.dproceso[i].pid_hijo);
                        printf("\t estado = %ld\n",tabla.dproceso[i].estado);
                        printf("\t id_sesion = %s\n",tabla.dproceso[i].id_sesion);
                        printf("\t fecha_registro = %s\n",tabla.dproceso[i].fecha_registro);
                        printf("\t fecha_inicio = %s\n",tabla.dproceso[i].fecha_inicio);
                        printf("\t fecha_fin = %s\n",tabla.dproceso[i].fecha_fin);
			printf("\t cont.ejecutadas = %ld\n", tabla.dproceso[i].pEjecutadasOK);
			printf("\t cont.SLimit = %ld\n", tabla.dproceso[i].pSLimit);
			printf("\t cont.HLimit = %ld\n", tabla.dproceso[i].pHLimit);
			printf("\t cont.Timeout = %ld\n", tabla.dproceso[i].pTimeout);
			printf("\t cont.Down = %ld\n", tabla.dproceso[i].pDown);
			printf("\t cont.TiempoOK = %ld\n", tabla.dproceso[i].tiempoEje);			
                        printf(" \n");
                        suma++;

                        if (suma == tabla.dglobal.num_proc_registrados)
                                break;
                }
        }

        printf(" \n");
	printf(" DATOS GLOBALES\n");
        printf(" - num. procesos registrados = %d \n", tabla.dglobal.num_proc_registrados);
        printf(" - num. maximo = %d \n", tabla.dglobal.num_max_procesos);
        printf(" - total ejecutadas = %ld \n", tabla.dglobal.totalEjecutadas);
        printf(" - total SLimit = %ld \n", tabla.dglobal.totalSLimit);
        printf(" - total HLimit = %ld \n", tabla.dglobal.totalHLimit);
        printf(" - total Timeout = %ld \n", tabla.dglobal.totalTimeout);
        printf(" - total Down = %ld \n", tabla.dglobal.totalDown);
        printf(" - total TiempoOK = %ld \n", tabla.dglobal.totalTiempoEje);        
        printf(" - fec. ultimo reset = %s \n", tabla.dglobal.fecha_ult_reset);

        return 0;
}

/* -------------------------------------------------------------------
* FUNCION               chequeo_peticiones(long)
* DESCRIPCION   	chequea los pids de la tabla y borra los que no existan
* PARAMETROS
*                       clave	  	numero fijo de pid del area de memroia compartida shm
*
* RETORNO               0 = exito, >0 = error
 ------------------------------------------------------------------- */
 
int chequeo_peticiones(long clave)
{
    	int ret=0;
	int reg=0;
     	int i=0;
     	int suma=0;
	int ret_sys=0;
	char cpid[50]="";
	int max_procesos = 0;
	char kaux[20]="";

	// control de pid
	pid_t pid;
	pid_t ppid;
	pid_t pidhijo;

	if ( ! tabla )
		return ERR_SHM_NO_EXISTE;

	// recorro tabla
	reg=tabla->dglobal.num_proc_registrados;
	max_procesos=tabla->dglobal.num_max_procesos;

	for (i=0;i<max_procesos;i++)
    {
        if (tabla->dproceso[i].pid > 0)
        {
			// chequeo si el pid existe
			ret_sys = kill(tabla->dproceso[i].pid, 0);

			if (ret_sys != 0)
			{
				// el pid no existe, borro el registro
				ret=borra_proceso(clave,i+1);
				/*if (ret != EXITO)
					display_error("API_SHM: fconsulta_tabla_shm: ERROR borra_proceso", ret);*/
			}			
               		suma++;
                        //if (suma == tabla->dglobal.num_proc_registrados)
                        //        break;
        }
    }
	
	return EXITO;
}

/* -------------------------------------------------------------------
* FUNCION               Limpia_procesos(long, int)
* DESCRIPCION   		chequea los pids de la tabla y borra los que no existan. Tambien mata procesos obdoletos:
*						- procesos con padre 1
*						- procesos con inactividad > parametro
* PARAMETROS
*                       clave	  	numero fijo de pid del area de memroia compartida shm
*						segKill		segundos de timeout para matar proceso
*
* RETORNO               0 = exito, >0 = error
 ------------------------------------------------------------------- */
 
int Limpia_procesos(long clave, int segKill)
{
    int ret=0;
	int reg=0;
    int i=0;
    int suma=0;
	int ret_sys=0;
	char cpid[50]="";
	int max_procesos = 0;
	long ultima_fecha=0;
	char kaux[20]="";
	
    time_t nw;
	long ahora=0;

	// control de pid
	long pid;
	long ppid;
	long pidhijo;
	long pidborrado=0;

	if ( ! tabla )
		return ERR_SHM_NO_EXISTE;

	// recorro tabla
	reg=tabla->dglobal.num_proc_registrados;
	max_procesos=tabla->dglobal.num_max_procesos;

	for (i=0;i<max_procesos;i++)
    {
        if (tabla->dproceso[i].pid > 0)
        {
			pidborrado=0;
			
			// chequeo si el pid existe
			ret_sys = kill(tabla->dproceso[i].pid, 0);

			if (ret_sys != 0)
			{
				// el pid no existe, borro el registro
				ret=borra_proceso(clave,i+1);
				/*if (ret != EXITO)
				{
        				display_error("API_SHM: consulta_tabla_shm: ERROR borra_proceso por no existir", errno);
				}*/
			}
			else
			{
				// ver si ha pasado el tiempo de inactivdad
				ultima_fecha=tabla->dproceso[i].fecha_fin;
				
				nw=time(NULL);
				sprintf(kaux,"%ld",nw);
				ahora=atol(kaux);
				
				pid=tabla->dproceso[i].pid;
				ppid = tabla->dproceso[i].ppid;
				pidhijo = tabla->dproceso[i].pid_hijo;
				
				if ((ahora - ultima_fecha) > segKill && segKill > 0 && tabla->dproceso[i].estado == 0 && ultima_fecha > 0)
				{
					// mato proceso y borro registro				
					ret_sys = kill(pid, SIGTERM);
					/*if (ret_sys != 0 && errno != ESRCH)
					{
						display_error("API_SHM: fallo al matar proceso por inactividad", errno);
					}*/
					ret_sys = kill(pidhijo, SIGTERM);
					/*if (ret_sys != 0 && errno != ESRCH)
					{
						display_error("API_SHM: fallo al matar proceso hijo por inactividad", errno);
					}*/
					
					// el pid no existe, borro el registro
					ret=borra_proceso(clave,i+1);
					if (ret == EXITO)
					{
						pidborrado=pid;
					}
					/*else
					{
						display_error("consulta_tabla_shm: ERROR borra_proceso por inactividad", errno);
					}*/
				}
				
			
				// ver si el padre existe . Si no, hay que matarlo
   			    if (pidborrado == 0 )
				{
										
					ret_sys = kill(tabla->dproceso[i].ppid, 0);
					if (ret_sys != 0)
					{	
						// mato proceso y borro registro				
						ret_sys = kill(pid, SIGTERM);
						/*if (ret_sys != 0 && errno != ESRCH)
						{
							display_error("API_SHM: fallo al matar proceso por no existir padre", errno);
						}*/
						ret_sys = kill(pidhijo, SIGTERM);
						/*if (ret_sys != 0 && errno != ESRCH)
						{
							display_error("API_SHM: fallo al matar proceso hijo por no existir padre", errno);
						}*/
						
						// el pid no existe, borro el registro
						ret=borra_proceso(clave,i+1);
						if (ret == EXITO)
							pidborrado=pid;
						/*else
							display_error("API_SHM: consulta_tabla_shm: ERROR borra_proceso por no existir el padre", errno);*/

					}
					
				}
				
			}
			
               		suma++;
                        //if (suma == tabla->dglobal.num_proc_registrados)
                        //        break;
                }
        }
	
	return EXITO;
}


/* -------------------------------------------------------------------
* FUNCION               consulta_registrados(long)
* DESCRIPCION   				devuelve el numero de procesos registrados en shm
* PARAMETROS
*                       clave	  	numero fijo de pid del area de memroia compartida shm
*
* RETORNO               numero de procesos registrados
 ------------------------------------------------------------------- */

int consulta_registrados()
{
	return tabla->dglobal.num_proc_registrados;
	}


/* -------------------------------------------------------------------
* FUNCION               getSHM(long,int)
* DESCRIPCION   				devuelve el numero de procesos registrados en shm
* PARAMETROS
*                       clave	  	numero fijo de pid del area de memroia compartida shm
*												size			numero de registros
* RETORNO               tabla con la shm
 ------------------------------------------------------------------- */
tabla_shm* getSHM(long clave, int numProcesos)
{
	long tamanio=0;

	tamanio=numProcesos * sizeof(dat_proceso);
	tamanio=tamanio+sizeof(dat_global);	
		
	tabla=GetSharedMem(clave, tamanio);
	
	return tabla;
	}


/* -------------------------------------------------------------------
* FUNCION               destruye_tabla(long)
* DESCRIPCION   crea un area de memoria compartida (shm)
* PARAMETROS
*                       pid	  	numero fijo de pid para crear la tabla
*
* RETORNO               0 (exito) , >0 (error)
 ------------------------------------------------------------------- */

int destruye_tabla (long clave)	
{
	int ret=0;

	if ( ! tabla )
		return ERR_SHM_NO_EXISTE;

	// destruir SHM 
	DetachSharedMem(tabla);

	return EXITO;
}





