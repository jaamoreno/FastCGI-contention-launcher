AR=ar
CPP=$(CC)

#------------------------------------------
# inicio MAKE 
#------------------------------------------

RAIZ=..
ALL_OBJ=$(OBJ)/api_shm.o  $(OBJ)/cshmem.o $(OBJ)/debug.o $(OBJ)/api_blk.o $(OBJ)/csem.o 
OBJ=$(RAIZ)/obj
LIB=$(RAIZ)/lib
INC=$(RAIZ)/inc
SRC=$(RAIZ)/src
BIN=$(RAIZ)/bin

COMPILER_FLAGS=-I$(INC) $(CFLAGS)
ARCHIVER_FLAGS=$(ARFLAGS)
LD_FLAGS=$(LDFLAGS)

all: $(LIB)/shm.a

$(OBJ)/api_shm.o: $(SRC)/api_shm.c
	$(CC) $(COMPILER_FLAGS) -c $(SRC)/api_shm.c -o $(OBJ)/api_shm.o 

$(OBJ)/cshmem.o: $(SRC)/cshmem.c
	$(CC) $(COMPILER_FLAGS) -c $(SRC)/cshmem.c -o $(OBJ)/cshmem.o 

$(OBJ)/debug.o: $(SRC)/debug.c
	$(CC) $(COMPILER_FLAGS) -c $(SRC)/debug.c -o $(OBJ)/debug.o 

$(OBJ)/api_blk.o: $(SRC)/api_blk.c
	$(CC) $(COMPILER_FLAGS) -c $(SRC)/api_blk.c -o $(OBJ)/api_blk.o 

$(OBJ)/csem.o: $(SRC)/csem.c
	$(CC) $(COMPILER_FLAGS) -c $(SRC)/csem.c -o $(OBJ)/csem.o 

$(LIB)/shm.a: $(ALL_OBJ)
	$(AR) $(ARCHIVER_FLAGS) $(LIB)/shm.a $(ALL_OBJ)
	@echo "done"

clean: ; rm -f $(OBJ)/*.o $(LIB)/*.a
