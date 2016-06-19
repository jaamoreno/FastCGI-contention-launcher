#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>


union semun
{
	int val;                        /* value for SETVAL */
	struct semid_ds *buf;           /* buffer for IPC_STAT & IPC_SET */
	unsigned short int *array;      /* array for GETALL & SETALL */
	struct seminfo *__buf;          /* buffer for IPC_INFO */
};

int sm_create(int key, int val)
{
	int semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0660);

	if (semid != -1)
	{
		/* Somos el proceso creador. Inicializar el semaforo*/
		union semun init;
		if (val < 0) val = 0;
		init.val = val;
		if (semctl(semid, 0, SETVAL, init) < 0) return -1;
		return semid;
	}
/* La llamada ha fallado. Puede que el semaforo ya exista*/
	if (errno != EEXIST) return -1; /* No. ha sido otro error*/
	semid = semget(key, 1, IPC_CREAT | 0660);
	return semid;
}

int sm_destroy(int semid)  /* no se llama (solo por destroy y es por limpiar la prueba */
{
	return semctl(semid, 0, IPC_RMID);
}

/* Devuelve 0 si todo ha ido bien o el error que se
   ha producido (errno) */
int sm_trwait(int semid, int flag)
{
	struct sembuf op;

	op.sem_num = 0; /* Semaforo por el que esperamos*/
	op.sem_op = -1; /* Decrementar y esperar si es cero */
	op.sem_flg = flag ? SEM_UNDO : 0;
	return semop(semid, &op, 1) ? errno : 0;
}

/* Si flag <> 0, el valor se ajusta al morir el proceso*/

int sm_trsignal(int semid, int flag)
{
	struct sembuf op;

	op.sem_num = 0; /* Semaforo por el que esperamos*/
	op.sem_op = 1; /* Incrementar */
	op.sem_flg = flag ? SEM_UNDO : 0;
	return semop(semid, &op, 1) ? errno : 0;
}


/* Devuelve EAGAIN si no puede bloquear */

int sm_trywait(int semid, int flag)
{
	struct sembuf op;

	op.sem_num = 0; /* Semaforo por el que esperamos*/
	op.sem_op = -1; /* Decrementar y esperar si es cero */
	op.sem_flg = flag ? SEM_UNDO : 0;
	return semop(semid, &op, 1) ? errno : 0;
}

