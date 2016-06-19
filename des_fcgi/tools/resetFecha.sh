#!/bin/ksh

CONF=$BASE_DIR/lanzador/conf/start.cf
BIN=$BASE_DIR/tools/resetFecha

TAM=$((0+0))
TAM=$(grep maxProcess $CONF| cut -d'=' -f2 | awk '{print $1}')
echo "tamanio de registros en tabla de memoria = $TAM"

if [ $TAM -le 0 ]; then
        echo "Error en la obtencion del tamanio de tabla"
        exit 1
fi

$BIN $CONF $TAM

if [ $? -ne 0 ]; then
	echo "\nERROR en el reseteo de la fecha de reset\n"
fi


