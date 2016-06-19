#include <string.h> /* JAM PORTING LINUX */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#include "api_shm.h"
#include "control_shm.h"

// ------------------------------
// VARIABLES
// ------------------------------

// configuracion
int clave = 0;
dat_configuracion conf;
// JAM char semilla[10]="SEED";
char fConfig[256]; /* JM*/

int ret=0;
int id;
int primeraVez = 0;
int tamanio;
int nmax_procesos;
dat_proceso dproceso;
dat_global dglobal;

tabla_shm ltabla;
tabla_shm *tabla = NULL;

// ------------------------------
// FUNCIONES
// ------------------------------

int hora()
{
        char output[128];
        time_t tiempo = time(0);
        struct tm *tlocal = localtime(&tiempo);
        strftime(output,128,"%d/%m/%y %H:%M:%S",tlocal);

        return 0;
}

modifica()
{
	int j=0;
        long totalEjecutadas=0;
        long totalSLimit=0;
        long totalHLimit=0;
        long totalTimeout=0;
        long totalDown=0;
        long max_procesos=0;


	char hora[20];
        time_t tiempo = time(0);
        struct tm *tlocal = localtime(&tiempo);
        strftime(hora,20,"%Y%m%d%H%M%S",tlocal);

	clave=ftok(fConfig, SEED);

	conf.max_procesos= nmax_procesos;
        conf.clave = clave;

        ret = crea_tabla(clave,conf);
        if (ret != 0)
        printf(" ERROR: retorno crea_tabla = %d \n", ret);

        tabla = con_tabla_sinSem(clave);
	if (tabla == NULL)
		return 1;

	/*resetear valores*/
	strcpy(tabla->dglobal.fecha_ult_reset,hora);

	Lock();
	max_procesos=tabla->dglobal.num_max_procesos;
        for( j=0; j < max_procesos; j++)
        {
        	if (tabla->dproceso[j].pid > 0)
                {
                	totalEjecutadas+=tabla->dproceso[j].pEjecutadasOK;
                        totalSLimit+=tabla->dproceso[j].pSLimit;
                        totalHLimit+=tabla->dproceso[j].pHLimit;
                        totalTimeout+=tabla->dproceso[j].pTimeout;
                        totalDown+=tabla->dproceso[j].pDown;
                }
        }

        totalEjecutadas+=tabla->dglobal.totalEjecutadas;
        totalSLimit+=tabla->dglobal.totalSLimit;
        totalHLimit+=tabla->dglobal.totalHLimit;
        totalTimeout+=tabla->dglobal.totalTimeout;
        totalDown+=tabla->dglobal.totalDown;

	Unlock();

	return 0;
}

// ------------------------------
// PRINCIPAL
// ------------------------------


int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printf ("Error: uso: resetFecha <configFile> <num_procesos>\n");
		return 0;
	}
	strcpy(fConfig,argv[1]);
	nmax_procesos = atoi(argv[2]);

	if(modifica())
		return 1;

	return 0;
}


