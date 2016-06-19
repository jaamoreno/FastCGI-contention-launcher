#include <unistd.h>  /* JAM PORTING LINUX */

#define SUPERVISOR_VERSION	"1.2" 	/* version 2014-10-22 */

#define LEN_MSG				149+3  /* 2-oct-2013, 3 caracteres mas para que quepa historico */
#define MAX_SHM				25
#define MAX_FCGI_NAME	100
#define MAX_SEED_NAME	250
#define MAX_PATH			256
#define MAX_PID_FILE	250
#define MAX_BUFFER		2000
#define MAX_FECHAX			8		 /* fecha en formato AAAAMMDD para XML*/
#define MAXBUF 				4096   /* buffer de respuesta a historicos*/
#define LINE_LEN       400   /* longitud máxima línea fichero de sesiones */

#define INTERVALOHISTORICO  0				/*intervalo definido para supervisor de historico*/
#define MAX_INTERVAL  32000				/*maximo valor para supervisorInterval*/
#define DEFAULT_SELECT_TIMEOUT  2		/* tiempo timeout selecting sobre sockets de lectura (26nov) */

#define CONFIG_FILE	"supervisor.cfg"

/*#define TAG_ONLINE	"peticion tipo=\"online\"" 
#define TAG_HISTORICO	"peticion tipo=\"historico\""
#define TAG_FECINICIO	"fechaInicio=\""
#define TAG_FECFIN	"fechaFin=\"" */


#define TAG_ONLINE	"online" 
#define TAG_HISTORICO	"historico"
#define TAG_FECINICIO	"fechaInicio"
#define TAG_FECFIN	"fechaFin"
#define TAG_RELOAD	"reload"
#define TAG_RESET	"resetFecha"



#define TAG_SINFECHA  "SIN FECHA"
#define TAG_FICH_HIST	"hst"

#define XML_HEADER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE servidores SYSTEM \"request.dtd\">\n<servidores>\n"
#define XML_TAIL   "</servidores>"

#define DEFAULT_SUPERVISORSRV_INTERVAL	5L

#define MAX_OPENED_SOCKETS 24

typedef enum 
{
   ACCEPT_CALL,
   ATTEND_CLIENT
} action_t;

typedef enum 
{
   EVENT_LOOP_ERROR,
   EVENT_LOOP_STOPPED,
   EXEC_CANT_RECIVE,
   EXEC_CANT_SEND,
   EXEC_CANT_ACCEPT,
   EXEC_ERROR,
   EXEC_UNKNOW_ACTION,
   EXEC_OK
} error_t;

typedef struct 
{
   int opened;
   int sd;
   int action;
} sockinfo_t;

typedef struct
{
	int ListenPort;
	int SupervisorInterval;
	int Instalations;
	char rutaHistoricos[MAX_PATH];
	key_t Ipckey[MAX_SHM];
	char IpcFile[MAX_SHM][MAX_SEED_NAME];
	char Name[MAX_SHM][MAX_FCGI_NAME];
	int tamanio[MAX_SHM];
	int max_procesos[MAX_SHM];
	int maxBusyProcess[MAX_SHM];
	int maxSoftBusyProcess[MAX_SHM];
} configuration;

#define exists(filename) (!access(filename, F_OK)) /* comprobar si existe un fichero */



/*
** Definicion de valores logicos.
*/
#define FALSE   0
#define TRUE    1

#define MAXPORT_LEN	5


/*
** ****************************************************************************
** ******************* PROTOTIPO DE FUNCIONES *********************************
** ****************************************************************************
*/

static int handler(void* user, const char* section, const char* name, const char* value);
static int handler_tamanio(void* user, const char* section, const char* name, const char* value);
static int handler_traza(void* user, const char* section, const char* name, const char* value);
int readConfig(char* configFile);
int readConfigPidfile(char* configFile);
int readConfigTraza(char* configFile);
int checkSHM(void);
/*int ConsultaOnline(char *fecha); */
int ConsultaOnline(int sd);
int ConsultaHistorico(int sd, char *fechaIni, char* fechaFin);
error_t SocketsEventLoop(int highest_sd, long *interval);
error_t ExecuteAction(int sd, action_t action, int *highest_sd);
void RegisterSocket(int sd, int *highest_sd, action_t action);
void UnRegisterSocket(int sd, int *highest_sd);
void CleanUp(int sig);
int EscribePidFile(char * pidFile);
int DeletePidFile(char * pidFile);
void flagPropagaRefresco(int sig);
void propagaRefresco(void);
int WriteStatSnapshot(void);
void resetFecha(void);
int consultaSesiones(void);

/*
** ****************************************************************************
** ******************* FIN PROTOTIPO DE FUNCIONES *****************************
** ****************************************************************************
*/
