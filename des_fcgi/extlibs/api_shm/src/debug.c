#include "debug.h"
#include <stdio.h>

#include <string.h>
void display_error(char *where, int err)
{
	char *msg = strerror(err);

	fprintf(stderr, "%s-> %s\n", where ? where : "", err ? "OK" : msg);
}
