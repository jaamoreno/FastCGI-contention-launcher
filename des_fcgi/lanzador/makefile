#trazas detalladas annadir a CFLAGS "-DDEBUGTRACE"

all: start

start: start.c
	$(CC) -I$(BASE_DIR)/include $(CFLAGS) -c start.c -o start.o 
	$(CC) $(LDFLAGS) start.o -o start $(BASE_DIR)/lib/fcgi.a $(BASE_DIR)/lib/libinih.a $(BASE_DIR)/lib/shm.a
	@echo "done"

clean: ; rm -f start; rm -f start.o
