#!/usr/bin/ksh

echo "------------------- generando libreria API SHM -------------------"
cd $BASE_DIR/extlibs/api_shm/mak 
make clean; make all

echo "------------------- generando libreria arqtraza -------------------"
cd $BASE_DIR/extlibs/arqtraza
make clean; make all

echo "------------------- generando libreria tcputil -------------------"
cd $BASE_DIR/extlibs/tcputil
make clean; make all

echo "------------------- generando libreria inih -------------------"
cd $BASE_DIR/extlibs/inih
make clean; make all

echo "------------------- generando libreria cliente fastCGI -------------------"
cd $BASE_DIR/extlibs/fcgi-2.4.1-SNAP-0311112127/libfcgi
./compila.sh

echo "------------------- generando supervisor -------------------"
cd $BASE_DIR/supervisor
make clean; make all

echo "------------------- generando lanzador FastCGI (start) -------------------"
cd $BASE_DIR/lanzador
make clean; make all

echo "------------------- generando utilidades de administracion -------------------"
echo "generando resetFecha"
cd $BASE_DIR/tools/fuentes/resetFecha
make clean; make all

echo "generando key"
cd $BASE_DIR/tools/fuentes/key
./compila_key.sh 

echo "generando checkFastCGI"
cd $BASE_DIR/tools/fuentes/checkFastCGI
make clean; make all

echo "generando tunerpt"
cd $BASE_DIR/tools/fuentes/tunerpt
make -f makefile_tunerptcero clean
make -f makefile_tunerptcero all






