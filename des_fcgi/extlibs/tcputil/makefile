#################
# 'make' build parameters
#################

APPLIB = .
APPINC1 = .
APPINC2 = $(BASE_DIR)/extlibs/arqtraza

#################
# Compiler parameters
#################

COMPILER_FLAGS = -I$(APPINC1) -I$(APPINC2) $(CFLAGS)

#################
# Archiver parameters
#################

ARCHIVER_FLAGS = $(ARFLAGS)

#################
# Header file parameters 
#################

#    Uncomment this if you want system header files to be expanded
#
# SYSHDRS =

EXTHDRS =

HDRS = tcputil.h

#################
# Library parameters
#################

LIBNAME = libtcputil

LIBRARY = $(LIBNAME).a

SRCS = tcputil.c

OBJS = tcputil.o

DEST = $(APPLIB)

#################
# Target Rules
#################

all:	$(LIBRARY)

$(LIBRARY):   $(OBJS) $(MAKEFILE)
	@echo "(*) Archiving $(LIBRARY) ..."
	ar $(ARCHIVER_FLAGS) $(LIBRARY) $(OBJS)
	@echo "done"

clean:
	@rm -f $(OBJS) $(LIBRARY) core

echo:
	@echo $(HDRS) $(SRCS)

.c.o:
	@echo "(*) Compilando $*.c..."
	$(CC) $(COMPILER_FLAGS) -c $*.c


###
tcputil.o: tcputil.h
