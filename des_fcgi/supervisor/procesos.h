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

int Limpia_procesos_tabla(long clave, int segKill, tabla_shm *tabla);
