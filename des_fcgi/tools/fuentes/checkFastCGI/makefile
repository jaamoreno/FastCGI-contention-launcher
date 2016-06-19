#CC=xlC
AR=ar
CPP=$(CC)

#------------------------------------------
# inicio MAKE checkFastCGI
#------------------------------------------

RAIZ=$(BASE_DIR)
ALL_OBJ=$(OBJ)
OBJ=$(RAIZ)/tools/fuentes/checkFastCGI
LIB=$(RAIZ)/lib
INC=$(RAIZ)/include
SRC=$(RAIZ)/tools/fuentes/checkFastCGI
BIN=$(RAIZ)/tools

FLAGS=-I$(INC) $(CFLAGS)
FLAGS_AR=$(ARFLAGS)
LD_FLAGS=$(LDFLAGS)

all: $(OBJ)/checkFastCGI

$(OBJ)/checkFastCGI: $(SRC)/checkFastCGI.c
	$(CC) $(FLAGS) -c $(SRC)/checkFastCGI.c -o $(OBJ)/checkFastCGI.o 
	$(CC) $(LD_FLAGS) -o $(BIN)/checkFastCGI checkFastCGI.o $(LIB)/libinih.a $(LIB)/shm.a
	@echo "done"

clean: ; rm -f ./*.o $(BIN)/checkFastCGI