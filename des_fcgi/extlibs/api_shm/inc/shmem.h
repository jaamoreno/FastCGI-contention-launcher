#ifndef SHMEM_H
#define SHMEM_H

#include "semaf.h"
#include "control_shm.h"

#define SHM_MODO_OPEN	0
#define SHM_MODO_CREATE	1

#define R_ONLY 0x01
#define FALLO		1
#define EXITO		0
#define ERR_ESPERA_SEM	9
#define ERR_SENAL_SEM	10

	/* Clase Area de memoria compartida */
typedef struct {
		int shmid;	/* id de la memoria */
		int modo;	 	/* lectura o escritura */
		int flags;
		int estado;	
		int acceso;	
		int num_lectores;
		void *addr;	/*puntero a la memoria */
		Semaforo *Acc; /*puntero a la estructura del semaforo*/
} ShMem;

ShMem *ShMemNew(long clave, long tam, int flags);
void ShMemDelete(ShMem *shm);

int ShMemCreate(ShMem *shm, long clave, long tam, int flags);
int ShMemOpen(ShMem *shm, long clave);
int ShMemDestroy(ShMem *shm);

	/* Obtener dirección para acceder según flags */
//void *ShMemGetPointerLectura(ShMem *shm, int flags);
void *xShMeGetPLectura(ShMem *shm, int flags);
void *ShMemGetPointer(ShMem *shm, int flags);

	/* Desasociarse */
int ShMemReleasePointer(ShMem *shm);

void libera_semaforo(int tiempo);
int set_alarm();
int unset_alarm();

	/* Errores */
#define SHM_ERR_CREATE	-201
#define SHM_ERR_OPEN	-202
#define SHM_ERR_OP	-203
#define SHM_ERR_EXISTE	-204
#define SHM_ERR_INVAL	-205
#define SHM_ERR_ESTADO	-206

#endif
