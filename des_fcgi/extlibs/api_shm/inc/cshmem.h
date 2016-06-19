#ifndef CSHM_INCLUDED
#define CSHM_INCLUDED

void *Map2SharedMem(int key, int size, int *shm_creada); 

/* Se asocia a la zona d emeoria compartida definida por key.
   Si no existe se crea con un tamano de size bytes 
   Devuelve el puntero a la zona de meoria dentro del espacio
    del proceso llamante.  */

void *GetSharedMem(int key, int size);
/* Se asocia a la zona d emeoria compartida definida por key. */

int DetachSharedMem(void *ptr);
/* Desasocia la memoria. Si es el ultimo proceso, la libera */
#endif
