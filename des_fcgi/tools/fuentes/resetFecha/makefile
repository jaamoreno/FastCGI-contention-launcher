#------------------------------------------
# inicio MAKE 
#------------------------------------------

RAIZ=$(BASE_DIR)
ALL_OBJ=$(OBJ)
OBJ=$(RAIZ)/tools/fuentes/resetFecha
LIB=$(RAIZ)/lib
INC=$(RAIZ)/include
SRC=$(RAIZ)/tools/fuentes/resetFecha
BIN=$(RAIZ)/tools

FLAGS=-I$(INC) $(CFLAGS)

all: $(OBJ)/reset_fecha

$(OBJ)/reset_fecha: $(SRC)/reset_fecha.c
	$(CC) $(FLAGS) $(SRC)/reset_fecha.c -o $(BIN)/resetFecha -L$(LIB) $(LIB)/shm.a
	@echo "done"


clean: ; rm -f $(OBJ)/reset_fecha.o $(BIN)/resetFecha




