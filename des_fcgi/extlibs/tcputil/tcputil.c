/*
** ***************************************************************************
** tcputil.c
** ***************************************************************************
** - Rutinas para establecimiento de sockets TCP y establecimiento de
**   comunicaciones a traves de ellos.
** ***************************************************************************
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arqtraza.h>

#include "tcputil.h"

/*****************************************************************************/   
int TCPServerListen(int *tcp_sd, char *serv, int numsocks)
/*****************************************************************************/   
{
   int    ls;
   unsigned int    addrlen;   /* de int a unsigned int (xlC v6) */ 
   int    i;
  
   struct sockaddr_in l_addr;
   struct sockaddr_in addr;
   struct servent  *service;   
   
   ARQLog(INF_TRACE, "TCPServerListen::Entrada rutina.");

   /*
   ** *************************************************************
   ** int socket(int domain, int type, int protocol);
   ** *************************************************************
   ** La funcion socket crea un socket en un dominio de
   ** comunicaciones y devuelve un descriptor de socket
   ** que puede ser utilizado posteriormente en funciones
   ** que operan con sockets. La funcion toma los sgtes
   ** argumentos:
   ** - domain: Especifica un dominio de comunicacion en
   **   el socket sera creado.
   ** - type: Especifica el tipo de socket a crear.
   ** - protocol: Especifica un protocolo particular a ser
   **   usado con el socket. Especificando un valor 0 provoca
   **   que sea usado un protocolo apropiado para el tipo
   **   especificado.
   **
   ** Si la funcion finaliza con exito retorna un entero no negativo
   ** correspndiente al descriptor del socket. En otro caso
   ** retorna un valor de -1 y la variable errno es fijada para 
   ** indicar el tipo de error.
   ** *************************************************************
   */
   ARQLog(INF_TRACE, "TCPServerListen::Estableciendo socket de tipo SOCK_STREAM en dominio AF_INET.");

   if((ls = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      ARQLog(ERR_TRACE, "TCPServerListen::Error en estableciendo socket(%s).", strerror(errno));
      return TCP_ERROR;
   }

   ARQLog(INF_TRACE, "TCPServerListen::Socket establecido con exito: %d.", ls);

   /*
   ** *************************************************************
   ** Establece la familia de direcciones para el socket.
   ** *************************************************************
   ** Socket address, internet style.
   ** struct sockaddr_in {
   **      sa_family_t     sin_family;
   **      in_port_t       sin_port;
   **      struct  in_addr sin_addr;
   ** #if !defined(_XPG4_2) || defined(__EXTENSIONS__)
   **      char            sin_zero[8];
   ** #else
   **      unsigned char   sin_zero[8];
   ** #endif /-* !defined(_XPG4_2) || defined(__EXTENSIONS__) *-/
   ** };
   ** *************************************************************
   */
   l_addr.sin_family = (sa_family_t)AF_INET;

  
   /*
   ** *************************************************************
   ** struct servent *getservbyname(const char *name, const char *proto);
   ** *************************************************************
   ** La funcion getservbyname busca en la base de datos
   ** del sistema la primera entrada para la cual el servicio
   ** cuyo nombre y protocolo coincida con los especificados.
   ** Si la funcion tuvo exito se devuelve un puntero a una
   ** estructura de tipo servent y un puntero nulo en caso
   ** contrario.
   ** struct  servent {
   **    char    *s_name;        /-* official service name *-/
   **    char    **s_aliases;    /-* alias list *-/
   **    int     s_port;         /-* port # *-/
   **    char    *s_proto;       /-* protocol to use *-/
   ** };
   ** *************************************************************
   */

   /*
   ** Si la cadena pasada como servicio es una cadena
   ** alfanumerica representando un nombre de servicio
   ** se obtiene la informacion a partir de la entrada
   ** en el /etc/services para tal servicio. Si es un
   ** numero se asume que tal numero es el puerto.
   */
   ARQLog(INF_TRACE, "TCPServerListen::Llevando a cabo obtencion del puerto.");

   if(!atoi(serv)) {
      if((service = getservbyname(serv, PROTOCOL)) == (struct servent *)NULL) {
         ARQLog(ERR_TRACE, "TCPServerListen::Error en obtencion puerto del servicio en base a nombre %s.", serv);
         return TCP_ERROR;
      }

      l_addr.sin_port = (in_port_t)service->s_port;
   }
   else
      l_addr.sin_port = (in_port_t)htons(atoi(serv));

   
   ARQLog(INF_TRACE, "TCPServerListen::Puerto de escucha: #%d.", l_addr.sin_port);
   
   l_addr.sin_addr.s_addr = (in_addr_t)INADDR_ANY;

   /*
   ** *********************************************************************
   ** int bind(int socket, const struct sockaddr *address, size_t address_len);
   ** *********************************************************************
   ** La funcion bind asigna una direccion a un socket no nombrado.
   ** Los sockets creados con esta funcion estan inicialmente no 
   ** nombrados y son solo identificados por su familia de direcciones.
   ** La funcion toma los siguientes argumentos:
   ** - socket: Especifica el file descriptor asociado al
   **   socket que va a ser enlazado.
   ** - address: Puntero a una estrutura de tipo sockaddr que contiene
   **   la direccion a ser enlazada por el socket. La longitud y formato
   **   de la direccion depende de la familia de direcciones asociadas
   **   al socket.
   ** - address_len: Especifica la longitud de la estructura sockaddr apuntada por el
   **   segundo argumento.
   ** 
   ** Si la funcion tuvo exito retorna 0 y -1 en cualq. otro caso.
   ** *********************************************************************
   */
   ARQLog(INF_TRACE, "TCPServerListen::Llevando a cabo enlazado del socket %d.", ls);

   if(bind(ls, (struct sockaddr*)&l_addr, sizeof(l_addr))) {
      ARQLog(ERR_TRACE, "TCPServerListen::Error en enlazado del socket %d(%s).", ls, strerror(errno));
      return TCP_ERROR;
   }
  
   /*
   ** ********************************************************************
   ** int listen(int s, int backlog);
   ** ********************************************************************
   ** Para aceptar conexiones, un socket es primero creado con socket y
   ** posteriormente las conexiones son aceptadas con accept. Las llamadas
   ** a la funcion listen() solo son aplicables a sockets de tipo 
   ** SOCK_STREAM or SOCK_SEQPACKET. El segundo de los parametros especifica
   ** la maxima longitud hasta donde la cola de conexiones puede crecer. 
   ** La funcion devuelve 0 si tuvo exito y -1 en caso contrario, fijando
   ** errno para indicar el error que ha tenido lugar.
   ** ********************************************************************
   */
   ARQLog(INF_TRACE, "TCPServerListen::Atendiendo conexiones para el socket %d.", ls);

   if(listen(ls, TCP_NUM_SOCKETS)) {
      ARQLog(ERR_TRACE, "TCPServerListen::Error en la atencion de conexiones para el socket %d(%s).",
             ls,
             strerror(errno));
      return TCP_ERROR;
   }
   
   for(i = 0; i < numsocks; i++) {
      ARQLog(INF_TRACE, "TCPServerListen::Esperando %d llamadas sobre el socket %d.", numsocks-i, ls);

      addrlen = sizeof(addr);

      /*
      ** ********************************************************************
      ** int accept(int s, struct sockaddr *addr, int *addrlen);
      ** ********************************************************************
      ** La funcion accept acepta una llamada sobre un socket.
      ** El primer argumento es un socket que ha sido creado
      ** con socket() y enlazada a una direccion con bind(), y
      ** que esta escuchando conexiones tras una llamada a listen().
      ** La funcion accept() extrae la primera conexion sobre la 
      ** cola de conexiones pendientes, crea un nuevo socket con las 
      ** propiedades de s, y reserva un nuevo descriptor de fichero
      ** para el socket. Si no hay conexiones pendientes en la cola y el
      ** socket no esta marcado como non-blocking, accept() bloquea el 
      ** proceso que usa la funcion hasta que una conexion este presente.
      ** Si el socket esta marcado como non-blocking y no existen  
      ** conexiones pendientes en la cola, la funcion accept() retorna
      ** un error. La funcion accept() usa el fichero /etc/netconfig
      ** para determinar el nombre del fichero de dispositivo STREAMS
      ** asociado con s. Es decir, el dispositivo sobre el cual la
      ** conexion sera aceptada. El socket acceptado, ns, es usado para
      ** lectura y escritura de datos a desde el socket que conecto con
      ** ns; no es usado para aceptar mas conexiones. El socket original, 
      ** s, permanece abierto para aceptar conexiones adicionales.
      **
      ** El argumento addr es una parametro de salida que es completado
      ** con la direccion de la entidad que esta tratando de conectarse
      ** tal y como es conocida por el nivel de comunicaciones. El formato
      ** exacto de este parametro es determinado por el dominio en el cual
      ** la comunicacion ocurre.
      **
      ** El argumento addrlen es de entrada-salida. Inicialmente contiene
      ** la cantidad de espacio apuntado por addr y a la salida contiene
      ** la longitud en bytes de la direccion retornada.
      **
      ** La funcion devuelve un -1 en caso de error. Si tiene exito, 
      ** retorna un entero no negativo correspondiente al descriptor 
      ** para el socket asociado.
      ** ********************************************************************
      */
      if((tcp_sd[i] = accept(ls, (struct sockaddr*)&addr, &addrlen)) == -1) {
         ARQLog(ERR_TRACE, "TCPServerListen::Error aceptando llamadas sobre el socket %d(%s).",
                (int)ls,
                strerror(errno));
	 return TCP_ERROR;
      }
      
      ARQLog(INF_TRACE, "TCPServerListen::Llamada aceptada(tcp_sd[%d] = <%d>).", i, tcp_sd[i]);
   }
   
   close(ls);
   
   return TCP_OK;
	 
} /* TCPServerListen */

/*****************************************************************************/   
int TCPSetupListenSocket(int *tcp_ls, char *serv)
/*****************************************************************************/   
{ 
   struct sockaddr_in l_addr;
   struct servent  *service;   
   
   ARQLog(INF_TRACE, "TCPSetupListenSocket::Entrada rutina.");

   /*
   ** *************************************************************
   ** int socket(int domain, int type, int protocol);
   ** *************************************************************
   ** La funcion socket crea un socket en un dominio de
   ** comunicaciones y devuelve un descriptor de socket
   ** que puede ser utilizado posteriormente en funciones
   ** que operan con sockets. La funcion toma los sgtes
   ** argumentos:
   ** - domain: Especifica un dominio de comunicacion en
   **   el socket sera creado.
   ** - type: Especifica el tipo de socket a crear.
   ** - protocol: Especifica un protocolo particular a ser
   **   usado con el socket. Especificando un valor 0 provoca
   **   que sea usado un protocolo apropiado para el tipo
   **   especificado.
   ** *************************************************************
   */
   ARQLog(INF_TRACE, "TCPSetupListenSocket::Estableciendo socket de tipo SOCK_STREAM en dominio AF_INET.");

   if((*tcp_ls = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      ARQLog(ERR_TRACE, "TCPServerListen::Error en estableciendo socket(%s).", strerror(errno));
      return TCP_ERROR;
   }
   
   ARQLog(INF_TRACE, "TCPSetupListenSocket::Socket establecido con exito: %d.", *tcp_ls);

   /*
   ** *************************************************************
   ** Establece la familia de direcciones para el socket.
   ** *************************************************************
   ** Socket address, internet style.
   ** struct sockaddr_in {
   **      sa_family_t     sin_family;
   **      in_port_t       sin_port;
   **      struct  in_addr sin_addr;
   ** #if !defined(_XPG4_2) || defined(__EXTENSIONS__)
   **      char            sin_zero[8];
   ** #else
   **      unsigned char   sin_zero[8];
   ** #endif /-* !defined(_XPG4_2) || defined(__EXTENSIONS__) *-/
   ** };
   ** *************************************************************
   */
   l_addr.sin_family = (sa_family_t)AF_INET;
   
   /*
   ** *************************************************************
   ** struct servent *getservbyname(const char *name, const char *proto);
   ** *************************************************************
   ** La funcion getservbyname busca en la base de datos
   ** del sistema la primera entrada para la cual el servicio
   ** cuyo nombre y protocolo coincida con los especificados.
   ** Si la funcion tuvo exito se devuelve un puntero a una
   ** estructura de tipo servent y un puntero nulo en caso
   ** contrario.
   ** struct  servent {
   **    char    *s_name;        /-* official service name *-/
   **    char    **s_aliases;    /-* alias list *-/
   **    int     s_port;         /-* port # *-/
   **    char    *s_proto;       /-* protocol to use *-/
   ** };
   ** *************************************************************
   */
   /*
   ** Si la cadena pasada como servicio es una cadena
   ** alfanumerica representando un nombre de servicio
   ** se obtiene la informacion a partir de la entrada
   ** en el /etc/services para tal servicio. Si es un
   ** numero se asume que tal numero es el puerto.
   */
   ARQLog(INF_TRACE, "TCPSetupListenSocket::Llevando a cabo obtencion del puerto.");

   if(!atoi(serv)) {
      if((service = (struct servent *)getservbyname(serv, PROTOCOL)) == (struct servent *)NULL) {
         ARQLog(ERR_TRACE, "TCPSetupListensocket::Error en obtencion puerto servicio del servicio en base a nombre.");
         return TCP_ERROR;
      }

      l_addr.sin_port = (in_port_t)service->s_port;
   }
   else
      l_addr.sin_port = (in_port_t)htons(atoi(serv));

   ARQLog(INF_TRACE, "TCPSetupListenSocket::Puerto de escucha: #%d.", l_addr.sin_port);
   
   l_addr.sin_addr.s_addr = (in_addr_t)INADDR_ANY;
      
   /*
   ** *********************************************************************
   ** int bind(int socket, const struct sockaddr *address, size_t address_len);
   ** *********************************************************************
   ** La funcion bind asigna una direccion a un socket no nombrado.
   ** Los sockets creados con esta funcion estan inicialmente no 
   ** nombrados y son solo identificados por su familia de direcciones.
   ** La funcion toma los siguientes argumentos:
   ** - socket: Especifica el file descriptor asociado al
   **   socket que va a ser enlazado.
   ** - address: Puntero a una estrutura de tipo sockaddr que contiene
   **   la direccion a ser enlazada por el socket. La longitud y formato
   **   de la direccion depende de la familia de direcciones asociadas
   **   al socket.
   ** - address_len: Especifica la longitud de la estructura sockaddr apuntada por el
   **   segundo argumento.
   ** 
   ** Si la funcion tuvo exito retorna 0 y -1 en cualq. otro caso.
   ** *********************************************************************
   */
   ARQLog(INF_TRACE, "TCPSetupListenSocket::Llevando a cabo enlazado del socket %d.", *tcp_ls);

   if(bind(*tcp_ls, (struct sockaddr*)&l_addr, sizeof(l_addr))) {
      ARQLog(ERR_TRACE, "TCPServerListen::Error en enlazado del socket %d(%s).", *tcp_ls, strerror(errno));
      return TCP_ERROR;
   }
   
   /*
   ** ********************************************************************
   ** int listen(int s, int backlog);
   ** ********************************************************************
   ** Para aceptar conexiones, un socket es primero creado con socket y
   ** posteriormente las conexiones son aceptadas con accept. Las llamadas
   ** a la funcion listen() solo son aplicables a sockets de tipo 
   ** SOCK_STREAM or SOCK_SEQPACKET. El segundo de los parametros especifica
   ** la maxima longitud hasta donde la cola de conexiones puede crecer. 
   ** ********************************************************************
   */
   ARQLog(INF_TRACE, "TCPSetupListenSocket::Atendiendo conexiones para el socket %d.", *tcp_ls);

   if(listen(*tcp_ls, MAX_CONCURRENT_CALLS)) {
      ARQLog(ERR_TRACE, "TCPServerListen::Error en la atencion de conexiones para el socket %d(%s).",
             *tcp_ls,
             strerror(errno));
      return TCP_ERROR;
   }
   
   ARQLog(INF_TRACE, "TCPSetupListenSocket::Socket %d escuchando.", *tcp_ls);

   return TCP_OK;

} /* TCPSetupListenSocket */

/*****************************************************************************/   
int TCPServerAcceptCall(int tcp_ls, int *tcp_sd)
/*****************************************************************************/   
{
   struct sockaddr_in addr;
   unsigned int    addrlen;   /* de int a unsigned int (xlC v6) */ 
   
   addrlen = sizeof(addr);
   
   ARQLog(INF_TRACE, "TCPServerAcceptCall::Entrada rutina.");

   /*
   ** ********************************************************************
   ** int accept(int s, struct sockaddr *addr, int *addrlen);
   ** ********************************************************************
   ** La funcion accept acepta una llamada sobre un socket.
   ** El primer argumento es un socket que ha sido creado
   ** con socket() y enlazada a una direccion con bind(), y
   ** que esta escuchando conexiones tras una llamada a listen().
   ** La funcion accept() extrae la primera conexion sobre la 
   ** cola de conexiones pendientes, crea un nuevo socket con las 
   ** propiedades de s, y reserva un nuevo descriptor de fichero
   ** para el socket. Si no hay conexiones pendientes en la cola y el
   ** socket no esta marcado como non-blocking, accept() bloquea el 
   ** proceso que usa la funcion hasta que una conexion este presente.
   ** Si el socket esta marcado como non-blocking y no existen  
   ** conexiones pendientes en la cola, la funcion accept() retorna
   ** un error. La funcion accept() usa el fichero /etc/netconfig
   ** para determinar el nombre del fichero de dispositivo STREAMS
   ** asociado con s. Es decir, el dispositivo sobre el cual la
   ** conexion sera aceptada. El socket acceptado, ns, es usado para
   ** lectura y escritura de datos a desde el socket que conecto con
   ** ns; no es usado para aceptar mas conexiones. El socket original, 
   ** s, permanece abierto para aceptar conexiones adicionales.
   **
   ** El argumento addr es una parametro de salida que es completado
   ** con la direccion de la entidad que esta tratando de conectarse
   ** tal y como es conocida por el nivel de comunicaciones. El formato
   ** exacto de este parametro es determinado por el dominio en el cual
   ** la comunicacion ocurre.
   **
   ** El argumento addrlen es de entrada-salida. Inicialmente contiene
   ** la cantidad de espacio apuntado por addr y a la salida contiene
   ** la longitud en bytes de la direccion retornada.
   **
   ** La funcion devuelve un -1 en caso de error. Si tiene exito, 
   ** retorna un entero no negativo correspondiente al descriptor 
   ** para el socket asociado.
   ** ********************************************************************
   */
   ARQLog(INF_TRACE, "TCPServerAcceptCall::Esperando llamadas sobre el socket %d.", tcp_ls);

   if((*tcp_sd = accept(tcp_ls, (struct sockaddr*)&addr, &addrlen)) == -1) {
      ARQLog(ERR_TRACE, "TCPServerAcceptCall::Error aceptando llamadas sobre el socket %d(%s).",
             tcp_ls,
             strerror(errno));
      return TCP_ERROR;
   }
      
   ARQLog(INF_TRACE, "TCPServerAcceptCall::Llamada aceptada sobre el socket %d(tcp_sd = <%d>).", tcp_ls, *tcp_sd);
   
   return TCP_OK;

} /* TCPServerAcceptCall */
	 
/*****************************************************************************/   
int TCPConnectSocket(int *tcp_sd, char *serv, char *host)
/*****************************************************************************/   
{
   int    addrlen;
  
   struct sockaddr_in serv_addr;
   struct hostent  *hostp;
   struct servent  *service;   
   
   ARQLog(INF_TRACE, "TCPConnectSocket::Entrada rutina.");

   /*
   ** *************************************************************
   ** int socket(int domain, int type, int protocol);
   ** *************************************************************
   ** La funcion socket crea un socket en un dominio de
   ** comunicaciones y devuelve un descriptor de socket
   ** que puede ser utilizado posteriormente en funciones
   ** que operan con sockets. La funcion toma los sgtes
   ** argumentos:
   ** - domain: Especifica un dominio de comunicacion en
   **   el socket sera creado.
   ** - type: Especifica el tipo de socket a crear.
   ** - protocol: Especifica un protocolo particular a ser
   **   usado con el socket. Especificando un valor 0 provoca
   **   que sea usado un protocolo apropiado para el tipo
   **   especificado.
   ** *************************************************************
   */
   ARQLog(INF_TRACE, "TCPConnectSocket::Estableciendo socket de tipo SOCK_STREAM en dominio AF_INET.");

   if((*tcp_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      ARQLog(ERR_TRACE, "TCPServerListen::Error en estableciendo socket(%s).", strerror(errno));
      return TCP_ERROR;
   }
      
   ARQLog(INF_TRACE, "TCPConnetSocket::Socket establecido con exito: %d.", *tcp_sd);

   /*
   ** *************************************************************
   ** Establece la familia de direcciones para el socket.
   ** *************************************************************
   ** Socket address, internet style.
   ** struct sockaddr_in {
   **      sa_family_t     sin_family;
   **      in_port_t       sin_port;
   **      struct  in_addr sin_addr;
   ** #if !defined(_XPG4_2) || defined(__EXTENSIONS__)
   **      char            sin_zero[8];
   ** #else
   **      unsigned char   sin_zero[8];
   ** #endif /-* !defined(_XPG4_2) || defined(__EXTENSIONS__) *-/
   ** };
   ** *************************************************************
   */
   serv_addr.sin_family = (sa_family_t)AF_INET;

   /*
   ** *************************************************************
   ** struct servent *getservbyname(const char *name, const char *proto);
   ** *************************************************************
   ** La funcion getservbyname busca en la base de datos
   ** del sistema la primera entrada para la cual el servicio
   ** cuyo nombre y protocolo coincida con los especificados.
   ** Si la funcion tuvo exito se devuelve un puntero a una
   ** estructura de tipo servent y un puntero nulo en caso
   ** contrario.
   ** struct  servent {
   **    char    *s_name;        /-* official service name *-/
   **    char    **s_aliases;    /-* alias list *-/
   **    int     s_port;         /-* port # *-/
   **    char    *s_proto;       /-* protocol to use *-/
   ** };
   ** *************************************************************
   */
   /*
   ** Si la cadena pasada como servicio es una cadena
   ** alfanumerica representando un nombre de servicio
   ** se obtiene la informacion a partir de la entrada
   ** en el /etc/services para tal servicio. Si es un
   ** numero se asume que tal numero es el puerto.
   */
   ARQLog(INF_TRACE, "TCPConnectSocket::Llevando a cabo obtencion del puerto.");

   if(!atoi(serv)) {
      if((service = (struct servent *)getservbyname(serv, PROTOCOL)) == (struct servent *)NULL) {
         ARQLog(ERR_TRACE, "TCPConnectSocket::Error en obtencion puerto del servicio en base a nombre %s.", serv);
         return TCP_ERROR;
      }

      serv_addr.sin_port = (in_port_t)service->s_port;
   }
   else {
      /*
      ** La funcion htons devuelve el valor del argumento de entrada
      ** convertido desde formato de red a formato host byte order.
      */
      serv_addr.sin_port = (in_port_t)htons(atoi(serv));
   }

   ARQLog(INF_TRACE, "TCPConnectSocket::Puerto de escucha: #%d.", serv_addr.sin_port);
   
   /*
   ** *************************************************************
   ** struct hostent *gethostbyname(const char *name);
   ** *************************************************************
   ** La funcion gethostbyname busca en la base de datos del sistema
   ** la primera entrada para la cual el nombre del host coincide
   ** con el especificado.
   ** *************************************************************
   ** struct  hostent {
   **     char    *h_name;        /-* official name of host *-/
   **     char    **h_aliases;    /-* alias list *-/
   **     int     h_addrtype;     /-* host address type *-/
   **     int     h_length;       /-* length of address *-/
   **     char    **h_addr_list;  /-* list of addresses from name server *-/
   ** #define h_addr  h_addr_list[0]  /-* address, for backward compatiblity *-/
   ** };
   ** *************************************************************
   */
   /*
   ** *************************************************************
   ** in_addr_t inet_addr(const char *cp);
   ** *************************************************************
   ** La funcion inet_addr convierte la cadena especificada como 
   ** parametro, en notacion standard internet formada por ternas
   ** separadas por puntos a un valor entero adecuado para uso
   ** como una direccion de Internet.
   ** Si la conversion tuvo exito la funcion retorna la direccion
   ** Internet. En cualquier otro caso devuelve (in_addr_t)-1.
   ** *************************************************************
   */
   ARQLog(INF_TRACE, "TCPConnectSocket::Llevando a cabo obtencion del host.");

   if((serv_addr.sin_addr.s_addr = (in_addr_t)inet_addr(host)) == (in_addr_t)-1) {
      ARQLog(INF_TRACE, "TCPConnectSocket::El host especificado %s no viene en format AAA.BBB.CCC.DDD.", host);

      ARQLog(INF_TRACE, "TCPConnectSocket::Obtencion del host en base al nombre %s.", host);

      if((hostp = (struct hostent *)gethostbyname((char*)host)) == (struct hostent *)NULL) {
         ARQLog(ERR_TRACE, "TCPConnectSocket::Error en obtencion host en base a nombre %s.", host);
         return TCP_ERROR;
      }
      else {
         memcpy((char *)&serv_addr.sin_addr.s_addr, hostp->h_addr, hostp->h_length);
         /*
         ** memcpy((char *)&serv_addr.sin_addr.s_addr, hostp->h_addr, hostp->h_length);
         ** memcpy((char *)&serv_addr.sin_addr, hostp->h_addr, hostp->h_length);
         */
      }
   }
   
   ARQLog(INF_TRACE, "TCPConnectSocket::Identificacion host: %lu.", serv_addr.sin_addr.s_addr);
   
  
   ARQLog(INF_TRACE, "TCPConnectSocket::Llevando a cabo conexion al socket %d.", *tcp_sd);

   /*
   ** *************************************************************
   ** int connect(int socket, const struct sockaddr *address, size_t address_len);
   ** *************************************************************
   ** La funcion connect solicita una conexion sobre un socket.
   ** Toma tres argumentos:
   ** - socket: Especifica el file descritor asociado al socket.
   ** - address: Apunta a la estructura de tipo sockaddr que 
   **   contiene la direccion. La longitud y formato de la 
   **   direccion depende de la familia de direcciones del socket.
   ** - address_len: Especifica la longitud de la estructura de tipo
   **   sockaddr apuntada por el argumento address.
   **
   ** La funcion connect devuelve 0 en caso de exito y -1 en caso
   ** contrario, fijando errno para indicar el error.
   ** *************************************************************
   */
   if(connect(*tcp_sd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
      ARQLog(ERR_TRACE, "TCPConnectSocket::Error conexion al servicio %s del host %s a traves del socket %d(%s).",
             serv,
             host,
             *tcp_sd,
             strerror(errno));
      return TCP_ERROR;
   }

   ARQLog(INF_TRACE, "TCPConnectSocket::Conexion al socket %d realizada con exito.", *tcp_sd);

   return TCP_OK;

} /* TCPConnectSocket */

/*****************************************************************************/   
int TCPDisconnectSocket(int tcp_sd)
/*****************************************************************************/   
{
   ARQLog(INF_TRACE, "TCPDisconnectSocket::Entrada rutina.");

   /*
   ** *************************************************************
   ** int close(int fildes);
   ** *************************************************************
   ** Cierre del file descriptor asociado al socket.
   ** *************************************************************
   */
   ARQLog(INF_TRACE, "TCPConnectSocket::Llevando a cabo desconexion al socket %d.", tcp_sd);

   close(tcp_sd);
   
   return TCP_OK;

} /* TCPDisconnectSocket */

/*****************************************************************************/   
int TCPReciveMsg(int sd, char *msg, int msglen)
/*****************************************************************************/   
{
   int   nbytes;

   ARQLog(INF_TRACE, "TCPReciveMsg::Entrada rutina.");
   
   ARQLog(INF_TRACE, "TCPReciveMsg::Recepcion de mensaje a traves de socket %d.", sd);
 
   /*
   ** *************************************************************
   ** ssize_t recv(int socket, void *buffer, size_t length, int flags);
   ** *************************************************************
   ** La funcion recv() recive un mensaje desde un socket. Toma los
   ** siguientes argumentos:
   ** - socket: Especifica el file descriptor asociado al socket.
   ** - buffer: Apunta a un buffer donde el mensaje deberia ser
   **   almacenado.
   ** - length: Especifica la longitud en bytes del buffer.
   ** - flags: Especifica el tipo de recepcion de mensajes. 
   **   Los valores de este argumento son formados mediante la 
   **   operacion logica OR de 0 o mas de los siguientes valores:
   **      MSG_PEEK       Peeks at an incoming  message.
   **                     The  data is treated as unread
   **                     and the next recv() or similar
   **                     function   will  still  return
   **                     this data.
   **
   **      MSG_OOB        Requests   out-of-band   data.
   **                     The significance and semantics
   **                     of   out-of-band   data    are
   **                     protocol-specific.
   **
   **      MSG_WAITALL    Requests  that  the   function
   **                     block until the full amount of
   **                     data    requested    can    be
   **                     returned.   The  function  may
   **                     return  a  smaller  amount  of
   **                     data  if  a  signal is caught,
   **                     the connection is  terminated,
   **                     or an error is pending for the
   **                     socket.
   **
   ** La funcion retorna la longitud del mensaje escrito en el 
   ** buffer apuntado por el argumento buffer.
   ** 
   ** Tras un finalizacion exitosa la funcion devuelve la longitud
   ** del mensaje en bytes. Si no hay mensajes disponibles para ser
   ** recibidos y su par ha realizado una bajada ordenada devuelve
   ** 0. En otro caso, devuelve -1 y errno es fijado para indicar
   ** el error.
   ** *************************************************************
   */
   
   /*
   ** *************************************************************
   ** Asumimos que el espacio para el almacenamiento del mensaje
   ** ha sido previamente reservado y por tanto, no es necesario
   ** reservarlo ya que si lo hicieramos se reasignaria el puntero
   ** y el mensaje recibido no seria retornado a la salida de la
   ** funcion.
   ** *************************************************************
   ** msg = (char *)malloc(msglen + 1);
   ** *************************************************************
   */
  
   /*
   ** Leemos el mensaje.
   */
   if((nbytes = recv(sd, msg, msglen, 0)) == -1) {
      ARQLog(ERR_TRACE, "TCPReciveMsg::Error en recepcion de mensaje a traves de socket %d(%s).",
             sd, strerror(errno));
      return TCP_ERROR;
   }
   
   msg[msglen] = 0;

   /*   
   ** ARQLog(INF_TRACE, "TCPReciveMsg::Mensaje recibido(%d bytes):", msglen);
   ** ARQVolcado(msg, msglen);
   */
      
   return TCP_OK;

} /* TCPReciveMsg */

/*****************************************************************************/   
int TCPSendMsg(int sd, char *msg, int msglen)
/*****************************************************************************/   
{
   ssize_t    sentbytes=0;
   
   ARQLog(INF_TRACE, "TCPSendMsg::Entrada rutina.");
   
   ARQLog(INF_TRACE, "TCPSendMsg::Envio de mensaje a traves de socket %d(%d bytes).", sd, msglen);

   /*
   ** *************************************************************
   ** ssize_t send(int socket, const void *buffer, size_t  length, int flags);
   ** *************************************************************
   ** La funcion send() inicia la transmision de un mensaje desde
   ** el socket especificado a su par. La funcion envia el mensaje
   ** solo cuando el socket esta conectado. Toma los sgtes argumentos:
   ** - socket: File descriptor asociado al socket.
   ** - buffer: Puntero al buffer que contiene el mensaje a enviar.
   ** - length: Longitud del mensaje en bytes.
   ** - flags: Especifica el tipo de transmision del mensaje.
   **   Los valores de este argumento son formados mediante operacion
   **   logica OR de 0 o mas de los siguientes flags:
   **                 MSG_EOR        Terminates a record  (if  sup-
   **                                ported by the protocol)
   ** 
   **                 MSG_OOB        Sends  out-of-band   data   on
   **                                sockets  that  support out-of-
   **                                band communications.  The sig-
   **                                nificance   and  semantics  of
   **                                out-of-band      data      are
   **                                protocol-specific.
   ** La longitud del mensaje a ser enviado es especificada por el
   ** segundo argumento. Si el mensaje es demasiado largo send falla
   ** y ningun dato es transmitido.
   ** Un retorno exitoso de la funcion send() no garantiza la 
   ** entrega del mensaje. Un valor de -1 indica solo errores locales.
   ** Si no hay espacio disponible en el socket para mantener el
   ** mensaje a ser transmitido y el descriptor de fichero asociado
   ** al socket no ha sido fijado a O_NONBLOCK, send() queda bloqueado
   ** hasta que haya espacio disponible. 
   ** Si no hay espacio disponible en el socket para mantener el
   ** mensaje a ser transmitido y el descriptor de fichero asociado
   ** al socket ha sido fijado a O_NONBLOCK, send() fallara.
   ** La funcion retorna el numero de bytes enviados tras una
   ** finalizacion con exito. En otro caso, retorna -1 y errno es 
   ** fijado para indicar el error.
   ** *************************************************************
   */
   if((sentbytes = send(sd, msg, msglen, 0)) == -1) {
   		ARQLog(ERR_TRACE, "sentbytes=[%ld]",sentbytes);
      ARQLog(ERR_TRACE, "TCPSendMsg::Error en envio de mensaje a traves de socket %d(%s).",
             sd, strerror(errno)); 
      return TCP_ERROR;
   }
  
   ARQLog(INF_TRACE, "TCPSendMsg::Mensaje enviado(%ld bytes).", sentbytes);
      
   return TCP_OK;

} /* TCPSendMsg */
