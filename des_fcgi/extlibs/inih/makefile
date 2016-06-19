SRC = ./ini.c
OBJ = $(SRC:.c=.o)
OUT = ./libinih.a
INCLUDES = -I.
COMPILER_FLAGS = $(CFLAGS)
ARCHIVER_FLAGS = $(ARFLAGS)

all: $(OUT)

.c.o:
	$(CC) $(INCLUDES) $(COMPILER_FLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar $(ARCHIVER_FLAGS) $(OUT) $(OBJ) $(EXTRAARFLAGS) 
	@echo "done"

clean:
	rm -f $(OBJ) $(OUT)
