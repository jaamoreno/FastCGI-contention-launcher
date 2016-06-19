#!/usr/bin/ksh

while :
do
	clear
	$BASE_DIR/tools/mataipc
	echo ------------------------------------------
	ps -ef|grep echoCGI |grep -v grep|wc -l|xargs echo Procesos CGI $1
	ps -ef|grep lanzador|grep -v grep |wc -l |xargs echo Procesos start $1
        echo ------------------------------------------
	date +"%H:%M:%S"
	sleep 1
done
