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
#include "arqtraza.h"


/* -------------------------------------------------------------------
* FUNCION               Limpia_procesos_tabla(long, int)
* DESCRIPCION   		chequea los pids de la tabla y borra los que no existan. Tambien mata procesos obdoletos:
*						- procesos con padre 1
*						- procesos con inactividad > parametro
* PARAMETROS
*                       clave	  	numero fijo de pid del area de memroia compartida shm
*						segKill		segundos de timeout para matar proceso
*
* RETORNO               0 = exito, >0 = error
 ------------------------------------------------------------------- */
 
int Limpia_procesos_tabla(long clave, int segKill, tabla_shm *tabla)
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

    /* control de pid */
    long pid;
    long ppid;
    long pidhijo;
    long pidborrado=0;
	

    if ( ! tabla )
	return ERR_SHM_NO_EXISTE;

    /* recorro tabla */
    reg=tabla->dglobal.num_proc_registrados;
    max_procesos=tabla->dglobal.num_max_procesos;

    for (i=0;i<max_procesos;i++)
    {
        if (tabla->dproceso[i].pid > 0)
        {
			pidborrado=0;
			
			/* chequeo si el pid existe */
			ret_sys = kill(tabla->dproceso[i].pid, 0);

			if (ret_sys != 0 && errno == ESRCH)
			{
				/* el pid no existe, borro el registro */
				ARQLog(INF_TRACE, "Limpia_procesos: borrando proceso %d de la tabla", tabla->dproceso[i].pid);
				ret=borra_proceso(clave,i+1);
			}
			else if (ret_sys == 0)
			{
				/* ver si ha pasado el tiempo de inactivdad*/
				ultima_fecha=tabla->dproceso[i].fecha_fin;
				
				nw=time(NULL);
				sprintf(kaux,"%ld",nw);
				ahora=atol(kaux);
				
				pid=tabla->dproceso[i].pid;
				ppid = tabla->dproceso[i].ppid;
				pidhijo = tabla->dproceso[i].pid_hijo;
				
				if ((ahora - ultima_fecha) > segKill && segKill > 0 && tabla->dproceso[i].estado == 0 && ultima_fecha > 0)
				{
					/* mato proceso y borro registro				*/
					ret_sys = kill(pid, SIGTERM);
					if (ret_sys != 0 && errno != ESRCH)
						ARQLog(ERR_TRACE, "Limpia_procesos: fallo al matar proceso [%ld] por inactividad %d",pid,errno);
					else
							ARQLog(INF_TRACE, "Limpia_procesos: eliminado proceso [%ld] por inactividad",pid);
							
					ret_sys = kill(pidhijo, SIGTERM);
					if (ret_sys != 0 && errno != ESRCH)
						ARQLog(ERR_TRACE, "Limpia_procesos: fallo al matar proceso hijo [%ld] por inactividad %d",pidhijo,errno);
					else
							ARQLog(INF_TRACE, "Limpia_procesos: eliminado proceso hijo [%ld] por inactividad",pidhijo);
					
					/* el pid no existe, borro el registro */
					ret=borra_proceso(clave,i+1);
					if (ret == EXITO)
					{
						pidborrado=pid;
					}

				}
				
			
				/* ver si el padre existe . Si no, hay que matarlo */
   			    	if (pidborrado == 0 )
				{
					ret_sys = kill(tabla->dproceso[i].ppid, 0);
					if (ret_sys != 0 && errno == ESRCH)
					{	
						/* mato proceso y borro registro				*/
						ret_sys = kill(pid, SIGTERM);
						if (ret_sys != 0)
							ARQLog(ERR_TRACE, "Limpia_procesos: fallo al matar proceso [%ld] por no existir padre %d",pid,errno);
						else
							ARQLog(INF_TRACE, "Limpia_procesos: eliminado proceso [%ld] por no existir padre",pid);
						if (pidhijo != 0)
						{
							ret_sys = kill(pidhijo, SIGTERM);
							if (ret_sys != 0)
								ARQLog(ERR_TRACE, "Limpia_procesos: fallo al matar proceso hijo [%ld] por no existir padre %d",pidhijo,errno);
							else
								ARQLog(INF_TRACE, "Limpia_procesos: eliminado proceso hijo [%ld] por no existir padre",pidhijo);
						}
						
						/* el pid no existe, borro el registro */
						ret=borra_proceso(clave,i+1);
						if (ret == EXITO)
							pidborrado=pid;

					}
				}
			}
               		suma++;
                }
        }
	
	return EXITO;
}
