#ifndef CONTROL_SHM_H
#define CONTROL_SHM_H
#ifdef  __cplusplus
extern "C" {
#endif

#define MAX_FECHA	20
#define MAX_NOMBRE	50
#define MAX_IDSESION	250
#define MAX_XML		2500
#define MAX_PROCESO	1

#define PROCESO_LIBRE	0
#define PROCESO_OCUPADO	1

#define SEED 66

/* estructura de datos globales de SHM */
typedef struct 
{
	int num_proc_registrados;		/* numero de procesos registrados en la tabla*/
	int num_max_procesos;		/* tamaño de la tabla */
	long totalEjecutadas;		/* total peticiones ejecutadas (correctamente) */
	long totalSLimit;		/* total peticiones rechazadas por Soft Limit */
	long totalHLimit;		/* total peticiones rechazadas por Hard Limit */
	long totalTimeout;		/* total peticiones rechazadas por Timeout */
	long totalDown;		/* total peticiones rechazadas por caida del CGI (error generico) */
	long totalTiempoEje; /*total tiempo de ejecucion ok*/
	char fecha_ult_reset[MAX_FECHA];						/*fecha ultima puesta a 0 de los contadores */		
	long parcialEjecutadas;		/* parcial peticiones ejecutadas (correctamente) */
	long parcialSLimit;		/* parcial peticiones rechazadas por Soft Limit */
	long parcialHLimit;		/* parcial peticiones rechazadas por Hard Limit */
	long parcialTimeout;		/* parcial peticiones rechazadas por Timeout */
	long parcialDown;		/* parcial peticiones rechazadas por caida del CGI (error generico) */
	long parcialTiempoEje; /*parcial tiempo de ejecucion ok*/
	char fecha_ult_acum[MAX_FECHA];						/*fecha del ultimo acumlado de historicos */			
}dat_global;

/* estructura de datos de cada proceso en SHM */
typedef struct 
{
	long pid;
	long ppid;			
	long pid_hijo;
	char id_sesion[MAX_IDSESION];
	long fecha_registro;
	long fecha_inicio;
	long fecha_fin;
	int estado;			/*estado del proceso 0 ->libre 1 -> ocupado*/
	long pEjecutadasOK; 		/*total de ejecuciones OK del proceso*/
	long pSLimit;					/*peticiones rechazadas por Soft Limit*/
	long pHLimit;					/*peticiones rechazadas por Hard Limit*/
	long pTimeout;				/*peticiones rechazadas por Timeout*/
	long pDown;						/*peticiones rechazadas por Caida del CGI (error generico) */	
	long tiempoEje;				/*tiempo total de las ejecuciones ok*/
}dat_proceso;

/* estructura de configuracion de SHM */
typedef struct 
{
	int max_procesos;		/*tamaño de la tabla*/
	long clave;					/*clave IPC*/
}dat_configuracion;

typedef struct 
{
    dat_global dglobal;
    dat_proceso dproceso[MAX_PROCESO];
}tabla_shm;



#ifdef  __cplusplus
}
#endif
#endif

