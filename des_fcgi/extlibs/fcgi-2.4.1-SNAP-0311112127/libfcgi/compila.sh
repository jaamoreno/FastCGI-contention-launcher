set -x
echo "COMPILACION LIBRERIA CLIENTE FASTCGI LINUX OMITIDO AUTOCONF"
exit 0

rm -f *.o *.a *.so

${CC} -DHAVE_CONFIG_H -I../include -D_THREAD_SAFE ${CFLAGS} -c fcgi_stdio.c -o fcgi_stdio.o 
${CC} -DHAVE_CONFIG_H -I../include -D_THREAD_SAFE ${CFLAGS} -c fcgiapp.c    -o fcgiapp.o 
${CC} -DHAVE_CONFIG_H -I../include -D_THREAD_SAFE ${CFLAGS} -c fcgio.cpp    -o fcgio.o 
${CC} -DHAVE_CONFIG_H -I../include -D_THREAD_SAFE ${CFLAGS} -c os_unix.c    -o os_unix.o 

ar ${ARFLAGS} ./fcgi.a ./*.o
${CC} ${LDFLAGS} -G -o ./libfcgi.so ./*.o
