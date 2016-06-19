#!/usr/bin/ksh
#
# ---------------------------------------------
# SUP_HOME es el path al binario del supervisor
# SUP_BIN nombre del binario del supervisor
# ---------------------------------------------

#SUP_HOME=/InternetD/fast_cgi/des_fcgi/supervisor
SUP_HOME=/home/kit/dev/des_fcgi/supervisor
SUP_BIN=supervisor

case $2 in
stop)
    PIDFILE=`grep PidFile $1 |cut -d = -f 2`
    if [ -f "$PIDFILE" ]; then
 	PID=`cat $PIDFILE`
 	kill ${PID} 	
	echo "Parando ${SUP_BIN} $1"
	sleep 1
	CONTADOR=`ps -ef | grep -w ${PID} | grep -v grep | wc -l`
	if [ ${CONTADOR} -eq 1 ]; then
		kill -9 ${PID}
	fi
    else
	echo "$PIDFILE not found" 
    fi
   ;;
start)
   cd $SUP_HOME
   nohup ${SUP_HOME}/${SUP_BIN} $1&
   cd - > /dev/null 2>&1
   ;;
restart)
   $0 stop
   sleep 10
   $0 start
   ;;
*)
   echo "USO: $0 <FicheroConfSupervisor> [ start | stop | restart ]"
   ;;
esac

