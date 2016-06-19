/*
** ****************************************************************************
** Funciones de arquitectura de trazas.
** ****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arqtraza.h"

static char fichtrz[MAXFILETRZ_LEN + 1] = "";
static char buffer[MAXMSG_LEN];

static int nivel = 0;

/*******************************************************************/
int TrzIni(char *fich)
/*******************************************************************
Funcionalidad:
>> Asigna nombre al fichero en el que se volcaran las trazas y
>> procede a su apertura.
*******************************************************************/
{
   char *ptr;

   FILE *fp = NULL;

   time_t now;

   struct tm *fechsist;

   char fechsistfmt[16];

   char strfechsist[FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1];

   char fileOut[MAXFICH_LEN+FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1+1];

   /*
   ** Inicializa la cadena que contendra la base
   ** del nombre del fichero sobre el que se
   ** volcaran las trazas.
   */
   memset(fichtrz, '\0', MAXFICH_LEN);

   /*
   ** Obtiene la hora/fecha actual en formato EPOCH.
   */
   now = time(NULL);

   fechsist = localtime(&now);

   sprintf(fechsistfmt, "%%0%dd%%0%dd%%0%dd", FECH_ANO_LEN, FECH_MES_LEN, FECH_DIA_LEN); /* 2-oct-2013 logs incorrectos */

   /*
   ** Crea una cadena con la fecha en formato: ddmmaaaa. (antes) ahora: aammdd
   */

   sprintf(strfechsist, fechsistfmt, fechsist->tm_year-100, fechsist->tm_mon+1, fechsist->tm_mday); /* 2-oct-2013 logs incorrectos */
  
   /*
   ** Determina el directorio en que se ubicara el fichero
   ** de logs a partir de la variable de entorno DIR_TRAZA
   ** Si no esta definida simplementa construye el nombre
   ** del fichero de trazas a partir del nombre de fichero
   ** pasado como parametro de la funcion. En caso contrario, 
   ** construye el nombre del fichero de trazas a partir del
   ** directorio especificado en la variable de entorno y el
   ** nombre del fichero.
   */
   if(!(ptr = getenv(DIR_TRAZA)))
      sprintf(fichtrz, "%.*s", MAXFICH_LEN, fich);
   else
      sprintf(fichtrz, "%s/%s", ptr, fich);

   /*
   ** Construye el nombre del fichero sobre
   ** el que se volcaran las trazas.
   */
   sprintf(fileOut, "%s.%s", fichtrz, strfechsist);

   /*
   ** Procede a la apertura del fichero en que se volcaran
   ** las trazas. Si existe el fichero lo prepara para empezar
   ** a volcar trazas a partir del final de este. Si no existe
   ** lo crea. Si no es posible la apertura retorna con codigo
   ** de error igual a -1.
   */
   if((fp = fopen(fileOut, "a+")) == NULL)
      return -1;

   fclose(fp);

   return 0;

} /* TrzIni */

/*******************************************************************/
int NivelTrazaActual()
/*******************************************************************
Funcionalidad:
>> Retorna el nivel de traza actual.
*******************************************************************/
{
   return TrzNivel();

   /* char *ptr;
   
      if(!(ptr = getenv(NIVEL_TRAZA)))
         return TRAZA_MAX;
      
      return atoi(ptr);
   */
} /* NivelTrazaActual */

/*******************************************************************/
int NivelVolcadoActual()
/*******************************************************************
Funcionalidad:
>> Retorna el nivel de volcado actual.
*******************************************************************/
{
   char *ptr;

   if(!(ptr = getenv(NIVEL_VOLCADO)))
      return VOLCADO_MAX;

   return atoi(ptr);

} /* NivelVolcadoActual */

/*******************************************************************/
void Log(char *format, va_list arglist)
/*******************************************************************
Funcionalidad:
>> Escribe en el fichero de trazas los componentes de la lista
>> variable de argumentos, cada uno de ellos con el formato
>> indicado.
*******************************************************************/
{
   FILE *fp = NULL;

   time_t now;

   struct tm *fechsist;

   char fechsistfmt[16];

   char strfechsist[FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1];

   char fileOut[MAXFICH_LEN+FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1+1];

   /*
   ** char *svc;
   */

   /*
   ** Obtiene la hora/fecha actual en formato EPOCH.
   */
   now = time(NULL);

   fechsist = localtime(&now);

   sprintf(fechsistfmt, "%%0%dd%%0%dd%%0%dd", FECH_ANO_LEN, FECH_MES_LEN, FECH_DIA_LEN);  /* 2-oct-2013 logs incorrectos */

   sprintf(strfechsist, fechsistfmt, fechsist->tm_year-100, fechsist->tm_mon+1, fechsist->tm_mday); /* 2-oct-2013 logs incorrectos */
   sprintf(fileOut, "%s.%s", fichtrz, strfechsist);

   if((fp = fopen(fileOut, "a+")) == NULL)
      return;

   /*
   ** Transforma la hora/fecha obtenida en formato EPOCH
   ** en una hora/fecha en formato calendario almacenandola
   ** en la variable buffer en formato string.
   */
   strcpy(buffer, ctime(&now));
   strcpy(buffer + strlen(buffer) - 1, "  ");
	
   /*
   ** Procede al almacenamiento de cada uno de los componentes
   ** de la lista de argumentos variable pasado a la entrada en 
   ** la variable buffer con el formato indicado.
   ** Nota.- Equivala a sprintf pero con una lista variable de
   ** argumentos.
   */
   vsprintf(buffer + strlen(buffer) - 1, format, arglist);

   /*
   ** Procede a la escritura en el fichero de trazas del buffer
   ** construido.
   */
   if(fp) {
      fprintf(fp, "%s\n", buffer);
      fclose(fp);
   }

} /* Log */

/*******************************************************************/
int TrzDefNivel(int n)
/*******************************************************************
Funcionalidad:
>> Fija el nivel de traza al valor indicado y retorna
>> el valor previo a la asignacion.
*******************************************************************/
{
   int tmp = nivel;

   nivel = n;

   return tmp;

} /* TrzDefNivel */

/*******************************************************************/
int TrzNivel()
/*******************************************************************
Funcionalidad:
>> Devuelve el nivel de traza.
*******************************************************************/
{
   return nivel;

} /* TrzNivel */

/*******************************************************************/
void ARQLog(int Nivel, char *format, ...)
/*******************************************************************
Funcionalidad:
>> Determina si la traza, en base a su indicador de nivel, ha de ser
>> volcada y si es asi, escribe en el fichero de trazas los componentes
>> de la lista variable de argumentos, cada uno de ellos con el formato
>> indicado.
*******************************************************************/
{
   va_list arglist;

   int nivel_traza;

   FILE *fp = NULL;

   time_t now;

   struct tm *fechsist;

   /*
   ** char *svc;
   */

   char fechsistfmt[16];

   char strfechsist[FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1];

   char fileOut[MAXFICH_LEN+FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1+1];


   /*
   ** Obtiene el nivel de traza actual.
   */
   nivel_traza = NivelTrazaActual();

   /*
   ** Si el nivel de traza es 0 sale sin hacer nada pues 0
   ** es el nivel minimo. Por tanto, no se volcaran trazas.
   */
   if(!nivel_traza)
      return;

   /*
   ** Si el indicador de nivel de la traza a volcar es superior
   ** al nivel de traza sale sin hacer nada pues dicha traza no
   ** ha  de ser volcada por ser de nivel superior. Es decir, el
   ** nivel de traza determina el maximo nivel de las trazas que
   ** seran volcadas y por tanto, solo se volcaran aquellas trazas
   ** cuyo indicador de nivel de traza sea menor o igual que el 
   ** nivel de traza actual.
   */
   if(Nivel > nivel_traza)
      return;

   va_start(arglist, format);

   /*
   ** Obtiene la hora/fecha actual en formato EPOCH.
   */
   now = time(NULL);

   fechsist = localtime(&now);

   sprintf(fechsistfmt, "%%0%dd%%0%dd%%0%dd",FECH_ANO_LEN, FECH_MES_LEN, FECH_DIA_LEN);  /* 2-oct-2013 logs incorrectos */

   sprintf(strfechsist, fechsistfmt, fechsist->tm_year-100, fechsist->tm_mon+1, fechsist->tm_mday);  /* 2-oct-2013 logs incorrectos */
   sprintf(fileOut, "%s.%s", fichtrz, strfechsist);

   if((fp = fopen(fileOut, "a+")) == NULL)
      return;

   /*
   ** Transforma la hora/fecha obtenida en formato EPOCH
   ** en una hora/fecha en formato calendario almacenandola
   ** en la variable buffer en formato string.
   */
   strcpy(buffer, ctime(&now));
   strcpy(buffer + strlen(buffer) - 1, "  ");
	
   /*
   ** Procede al almacenamiento de cada uno de los componentes
   ** de la lista de argumentos variable pasado a la entrada en 
   ** la variable buffer con el formato indicado.
   ** Nota.- Equivala a sprintf pero con una lista variable de
   ** argumentos.
   */
   vsprintf(buffer + strlen(buffer) - 1, format, arglist);

   /*
   ** Procede a la escritura en el fichero de trazas del buffer
   ** construido.
   */
   if(fp) {
      fprintf(fp, "%s\n", buffer);
      fclose(fp);
   }

   va_end(arglist);

} /* ARQLog */

/*******************************************************************/
void ARQError(char *format, ...)
/*******************************************************************
Funcionalidad:
>> Volcado de trazas de error, siempre y cuando el nivel de traza no 
>> sea 0. Determina si la traza, en base al nivel de traza, ha de ser
>> volcada y si es asi, escribe en el fichero de trazas los componentes
>> de la lista variable de argumentos, cada uno de ellos con el formato
>> indicado.
*******************************************************************/
{
   va_list arglist;

   int nivel_traza;

   FILE *fp = NULL;

   time_t now;

   struct tm *fechsist;

   /*
   ** char *svc;
   */

   char fechsistfmt[16];

   char strfechsist[FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1];

   char fileOut[MAXFICH_LEN+FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1+1];

   /*
   ** Obtiene el nivel de traza actual.
   */
   nivel_traza = NivelTrazaActual();

   /*
   ** Si el nivel de traza es 0 sale sin hacer nada pues 0
   ** es el nivel minimo. Por tanto, no se volcaran trazas.
   */
   if(!nivel_traza)
      return;

   va_start(arglist, format);

   /*
   ** Obtiene la hora/fecha actual en formato EPOCH.
   */
   now = time(NULL);

   fechsist = localtime(&now);

   sprintf(fechsistfmt, "%%0%dd%%0%dd%%0%dd", FECH_ANO_LEN, FECH_MES_LEN, FECH_DIA_LEN);  /* 2-oct-2013 logs incorrectos */

   sprintf(strfechsist, fechsistfmt, fechsist->tm_year-100, fechsist->tm_mon+1, fechsist->tm_mday);  /* 2-oct-2013 logs incorrectos */

   sprintf(fileOut, "%s.%s", fichtrz, strfechsist);

   /*
   ** Apertura del fichero.
   */
   if((fp = fopen(fileOut, "a+")) == NULL)
         return;

   /*
   ** Transforma la hora/fecha obtenida en formato EPOCH
   ** en una hora/fecha en formato calendario almacenandola
   ** en la variable buffer en formato string.
   */
   strcpy(buffer, ctime(&now));
   strcpy(buffer + strlen(buffer) - 1, "  ");

   /*
   ** Procede al almacenamiento de cada uno de los componentes
   ** de la lista de argumentos variable pasado a la entrada en 
   ** la variable buffer con el formato indicado.
   ** Nota.- Equivala a sprintf pero con una lista variable de
   ** argumentos.
   */
   vsprintf(buffer + strlen(buffer) - 1, format, arglist);

   /*
   ** Procede a la escritura en el fichero de trazas del buffer
   ** construido.
   */
   if(fp) {
      fprintf(fp, "%s\n", buffer);
      fclose(fp);
   }

   va_end(arglist);

} /* ARQError */

/*******************************************************************/
void ARQVolcadoDatos(FILE *fp, unsigned char *datos, long longitud)
/*******************************************************************
Funcionalidad:
>> Lleva a cabo el volcado de datos en el fichero de trazas.
*******************************************************************/
{
   long i, cont;

   /*
   ** Nos aseguramos de que el fichero este abierto.
   */
   if(!fp)
      return;

   fprintf(fp, "*** Datos  Long: %5d ***\n", longitud);

   /*
   ** Vuelca los datos al fichero de trazas tanto en formato
   ** hexadecimal como ascii aquellos que puedan ser visualizados
   ** correctamente.
   */
   for(i = 0; i < longitud; i += 16) {

      for(cont = 0; cont < 16 && i + cont < longitud; cont ++) {
         fprintf(fp, "%02x ", datos[i+cont]);
      }

      for(; cont < 16 ; cont++)
         fprintf(fp, "   ");

      fprintf(fp, "  |  ");

      for(cont = 0; cont < 16 && i + cont < longitud; cont ++) {
         if(isascii(datos[i+cont]))
            fprintf(fp, "%c", datos[i+cont]);
         else
            fprintf(fp, ".");
      }

      fprintf(fp, "\n");
   }

   fprintf(fp, "\n");

} /* ARQVolcadoDatos */


/*******************************************************************/
void ARQVolcadoFichero(FILE *fp, void *msg, long longitud)
/*******************************************************************
Funcionalidad:
>> Procede al volcado del buffer de entrada al fichero distinguiendo
>> la parte del buffer correspondiente a cabecera y la parte del 
>> buffer correspondiente a datos si es que la tiene.
*******************************************************************/ 
{
   unsigned char *datos;

   time_t now;

   char buffer[80];

   long longdat = 0;

   /*
   ** Obtiene la hora/fecha actual en formato EPOCH.
   */
   now = time(NULL);

   /*
   ** Transforma la hora/fecha obtenida en formato EPOCH
   ** en una hora/fecha en formato calendario almacenandola
   ** en la variable buffer en formato string.
   */
   strcpy(buffer, ctime(&now));
   strcpy(buffer + strlen(buffer) - 1, " ");

   fprintf(fp, "### %s Long: %5d\n", buffer, longitud);

   /*
   ** Chequeo de la longitud del buffer. Si excede el tamano
   ** maximo se da un aviso y se trunca el tamano del buffer
   ** al tamano maximo permitido(MAX_LONG_DUMP).
   */
   if(longitud > MAX_LONG_DUMP) {
      fprintf(fp, "ARQVolcado: WARN - Longitud excesiva.\n");
      longitud = MAX_LONG_DUMP;
   }

   datos = (unsigned char *)((char *) msg);

   /*
   ** La longitud de los datos sera la diferencia entre la total
   ** y la parte del buffer que no corresponde a datos.
   */
   longdat = longitud;

   /*
   ** Si el buffer posee datos estos son volcados al fichero de
   ** trazas.
   */
   if(datos)
      ARQVolcadoDatos(fp, datos, longdat);

   fprintf(fp, "###\n");

} /* ARQVolcadoFichero */

/*******************************************************************/
void ARQVolcado(void *msg, long longitud)
/*******************************************************************
Funcionalidad:
>> Procede al volcado del buffer de entrada al fichero de trazas actual
>> distinguiendo la parte del buffer correspondiente a cabecera y la
>> parte del buffer correspondiente a datos si es que la tiene.
*******************************************************************/
{
   FILE *fp = NULL;

   int nivel_volcado;

   time_t now;

   struct tm *fechsist;

   char fechsistfmt[16];

   char strfechsist[FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1];

   char fileOut[MAXFICH_LEN+FECH_DIA_LEN+FECH_MES_LEN+FECH_ANO_LEN+1+1];

   /*
   ** Obtiene el nivel de volcado de trazas actual.
   */
   nivel_volcado = NivelVolcadoActual();

   /*
   ** Si el nivel de volcado de trazas actual es 0
   ** abandona la funcion sin hacer nada, es decir,
   ** sin volcar las trazas.
   */
   if(!nivel_volcado)
      return;

   /*
   ** Obtiene la hora/fecha actual en formato EPOCH.
   */
   now = time(NULL);

   fechsist = localtime(&now);

   sprintf(fechsistfmt, "%%0%dd%%0%dd%%0%dd", FECH_ANO_LEN, FECH_MES_LEN, FECH_DIA_LEN);  /* 2-oct-2013 logs incorrectos */

   sprintf(strfechsist, fechsistfmt, fechsist->tm_year-100, fechsist->tm_mon+1, fechsist->tm_mday);  /* 2-oct-2013 logs incorrectos */

   sprintf(fileOut, "%s.%s", fichtrz, strfechsist);

   /*
   ** Apertura del fichero.
   */
   if((fp = fopen(fileOut, "a+")) == NULL)
      return;

   /*
   ** Se llama a la funcion ARQVolcadoFichero que
   ** procedera al volcado del buffer de entrada al fichero.
   */
   ARQVolcadoFichero(fp, msg, longitud);

   /*
   ** Cierre del fichero.
   */
   fclose(fp);

} /* ARQVolcado */


/*******************************************************************/
int ARQTrazaIni(char *fich)
/*******************************************************************
Funcionalidad:
>> Asigna nombre al fichero en el que se volcaran las trazas y
>> procede a su apertura.
*******************************************************************/
{
   return TrzIni(fich);

} /* ARQTrazaIni */
