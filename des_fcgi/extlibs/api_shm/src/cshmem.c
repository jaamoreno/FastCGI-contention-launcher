#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <errno.h>

#include "cshmem.h"

#ifndef NULL
#define NULL ((void*)0)
#endif


static int mem_id;

void *Map2SharedMem(int key, int size, int *shm_creada)
{
	void *ptr;
	int iamthefirst = 0;

	mem_id = shmget(key, size, IPC_CREAT | IPC_EXCL | 0660);
	if (mem_id != -1) 
		{
			iamthefirst = 1;
			*shm_creada = 1;
		}	
	else
	{
		*shm_creada = 0;
		if (errno == EEXIST)
		{
			/* Somos otro proceso accediento. */
			mem_id = shmget(key, size, IPC_CREAT | 0660);
			if (mem_id == -1) return NULL;
		}
	}
	/* Aqui hemos creado o nos hemos mapeado
	   a la memoria compartida. Comnseguir un puntero */
	ptr = shmat(mem_id, NULL, 0);

	return ptr;
}

void *GetSharedMem(int key, int size)
{
        void *ptr;
        int iamthefirst = 0;

        mem_id = shmget(key, size, 0);
/*        if (mem_id != -1)      si shmget devuelve -1 es que no existe la memoria y retornamos null      */
        if (mem_id ==-1)
        {
                return NULL;
        }
        /* Aqui nos hemos mapeado
           a la memoria compartida. Comnseguir un puntero */
        ptr = shmat(mem_id, NULL, 0);

        return ptr;
}


int DetachSharedMem(void *ptr)
{
	struct shmid_ds buf;

	if (shmctl(mem_id, IPC_STAT, &buf) == -1) 
		return errno;
	else
		if ((buf.shm_nattch == 1) && (shmctl(mem_id, IPC_RMID, &buf) == -1))   /* si hay un proceso usandola no borra */
			return errno;

	if (shmdt(ptr) == -1) 
		return errno;
	else 
		return 0;
}
