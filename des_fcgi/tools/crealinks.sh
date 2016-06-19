#!/usr/bin/ksh

rm -f ${BASE_DIR}/include/*
rm -f ${BASE_DIR}/lib/*

#librerias
ln -fs ${BASE_DIR}/extlibs/api_shm/lib/* ${BASE_DIR}/lib/.
ln -fs ${BASE_DIR}/extlibs/inih/*.a ${BASE_DIR}/lib/.
ln -fs ${BASE_DIR}/extlibs/fcgi-2.4.1-SNAP-0311112127/libfcgi/*.a ${BASE_DIR}/lib/.
ln -fs ${BASE_DIR}/extlibs/arqtraza/libarqtraza.a ${BASE_DIR}/lib/.
ln -fs ${BASE_DIR}/extlibs/tcputil/libtcputil.a ${BASE_DIR}/lib/.

#includes
ln -fs ${BASE_DIR}/extlibs/api_shm/inc/* ${BASE_DIR}/include/.
ln -fs ${BASE_DIR}/extlibs/inih/ini.h  ${BASE_DIR}/include/.
ln -fs ${BASE_DIR}/extlibs/arqtraza/arqtraza.h ${BASE_DIR}/include/.
ln -fs ${BASE_DIR}/extlibs/tcputil/tcputil.h ${BASE_DIR}/include/.
ln -fs ${BASE_DIR}/extlibs/fcgi-2.4.1-SNAP-0311112127/include/fcgi_config.h ${BASE_DIR}/include/.
ln -fs ${BASE_DIR}/extlibs/fcgi-2.4.1-SNAP-0311112127/include/fcgi_stdio.h ${BASE_DIR}/include/.
ln -fs ${BASE_DIR}/extlibs/fcgi-2.4.1-SNAP-0311112127/include/fcgiapp.h ${BASE_DIR}/include/.
# --- READABLE_UNIX_FD_DROP_DEAD_TIMEVAL 
ln -fs ${BASE_DIR}/extlibs/fcgi-2.4.1-SNAP-0311112127/include/fcgios.h ${BASE_DIR}/include/.

echo "Enlaces creados!"


