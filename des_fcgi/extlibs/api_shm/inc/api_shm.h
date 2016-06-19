#ifndef API_SHM_H
#define API_SHM_H
#include <signal.h>

#include "control_shm.h"


#define EXITO		0
#define FALLO		1
#define SHM_EXISTE	2
#define SHM_OPEN	3
#define ERR_PETICIONES	100
#define ERR_TABLA_LLENA       2

#define ERR_MALLOC	3
#define ERR_LECTURA	4
#define ERR_ESCRITURA	5
#define ERR_REG_VACIO	6
#define ERR_PID_DIFERENTE	7
#define ERR_POS_TABLA	8

#define ERR_SHM_NO_EXISTE	9
#define ERR_SEM_OPEN	10


/* prototipo de funciones de interfaz con FCGI */

int crea_tabla (long clave, dat_configuracion conf);			

int inserta_proceso(int* id, dat_proceso dproc, dat_configuracion conf); 	

int modifica_proceso(long clave, int id, dat_proceso pro);	

int getContadoresProceso(dat_proceso* p, int pos,	long pid); /*devuelve los contadores de un proceso*/
int putContadoresProceso(dat_proceso* p, int pos,	long pid); /*actualiza los contadores de un proceso en la tabla shm*/

int borra_proceso(long clave, int id);

int consulta_peticiones(long clave, int max_peticiones, int* num_peticiones);

tabla_shm* consulta_tabla_shm(long clave);

tabla_shm* con_tabla_sinSem(long clave);


int inic_estruc_proc(dat_global* dglobal,dat_proceso* dproceso);


int imprime_tabla(tabla_shm tabla);

int chequeo_peticiones(long clave);

int consulta_registrados();

tabla_shm* getSHM(long clave, int size);

#endif

