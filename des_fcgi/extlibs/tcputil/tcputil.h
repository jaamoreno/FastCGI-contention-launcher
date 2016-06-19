#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


#define INVALID_SOCKET	0

#define TCP_NUM_SOCKETS 24

#define PROTOCOL "tcp"

#define MAX_CONCURRENT_CALLS 10

extern int errno;

typedef enum {
   TCP_ERROR,
   TCP_OOB,
   TCP_INTR,
   TCP_OK
} retcodes;

/*
** **********************************************
** Funciones exportadas por la libreria.
** **********************************************
*/
int TCPServerListen(int *,char *,int);

int TCPSetupListenSocket(int *,char *);

int TCPServerAcceptCall(int ,int *);

int TCPConnectSocket(int *,char *, char *);

int TCPDisconnectSocket(int);

int TCPReciveMsg(int,  char *, int);

int TCPSendMsg(int, char *, int);
