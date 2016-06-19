#include <stdio.h>
#include "control_shm.h"

int main(int argc, char *argv[])
{
	long clave;
	FILE *archivo;

	if (argc < 2 )
	{
		printf("ERROR: debe invocar el programa con el siguiente formato\n"
			"key <fichero_conf_lanzador>\n");
		return 1;
	}

	/* --- comprobar existe el fichero en argv[1] sino devuelve clave "ffffffff" --- */
	archivo = fopen(argv[1], "r");
	if (!archivo) 
	{
		printf("ERROR: el archivo [%s] no existe.\n", argv[1]);
		return 2;
	}

	clave = ftok(argv[1], SEED); 
	printf("%x\n",clave);

	return 0;
}
