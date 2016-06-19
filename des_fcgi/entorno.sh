#!/usr/bin/ksh
clear

export BASE_DIR=$PWD
export PATH=.:$BASE_DIR/tools:$BASE_DIR/extlibs/api_shm/bin:$PATH
export FCGI_INCLUDE=${BASE_DIR}/include
export FCGI_LIB=${BASE_DIR}/lib
export CFLAGS="-fPIC -DLINUX -fdiagnostics-show-option"
export ARFLAGS="rcs"
export LDFLAGS="-fPIC -lstdc++ -lrt"
export CC=gcc

alias astart='/opt/IBM/HTTPServer/bin/apachectl -k start'
alias astop='/opt/IBM/HTTPServer/bin/apachectl -k stop'
alias arestart='/opt/IBM/HTTPServer/bin/apachectl -k restart'
alias agraceful='/opt/IBM/HTTPServer/bin/apachectl -k graceful'

export IHS_HOME=/opt/IBM/HTTPServer
echo " "

# --- supervisor INI ------
export PATH=.:$BASE_DIR/supervisor:$PATH
# --- supervisor FIN ------

# --- lanzador INI ------
export FASTCGI_CONFIG=/home/kit/dev/des_fcgi/lanzador/conf/start.config
# --- lanzador FIN ------

# --- ejecutar crealinks
./tools/crealinks.sh

# --- muestro IP (interesante para enlazar con la consola)
echo "IP DE LA MAQUINA"
(/sbin/ifconfig 2>&1) | grep "inet addr" | grep -v "127.0.0.1"


