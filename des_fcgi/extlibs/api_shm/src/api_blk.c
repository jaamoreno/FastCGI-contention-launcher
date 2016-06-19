#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "cshmem.h"
#include "debug.h"
#include "api_blk.h"

static int locksem = -1;


void Lock(void)
{
	/* Pedimos acceso exclusivo. El acceso se hace en modo transaccional
	   en modo transaccional por si algo falla dentro de la seccion critica */ 
	for (; sm_trwait(locksem, 1); )
	{
		switch (errno)
		{
		case EINTR:
			continue; /* reintentar */
		case EAGAIN: /*  Timeout */
		case EIDRM: /* Semaforo ha sido eliminado */
		default:
			{
				display_error("API_SHM: Lock", errno);
				exit(errno);
			}
		}
	}
}


void Unlock(void)
{
	if (sm_trsignal(locksem, 1) != 0)
		/* Poco podemos hacer => salir */
		display_error("API_SHM: signal(locksem)", errno);
}


int SemCreate(int key,int val)
{
	locksem = sm_create(key, val); /* antes 1 */
	return locksem;
}


