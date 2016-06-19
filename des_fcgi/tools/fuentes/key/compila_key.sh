#!/usr/bin/ksh

${CC} ${CFLAGS} -I ${BASE_DIR}/include key.c -o key
mv key ${BASE_DIR}/tools
echo "compilado key"
