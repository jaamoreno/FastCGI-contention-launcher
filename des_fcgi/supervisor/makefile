#################
# 'make' build parameters
#################
MAKEFILE = makefile

SHELL = /bin/sh

APPDIR = .
APPINC = $(APPDIR)/../include
APPLIB = $(APPDIR)/../lib

#################
# Compiler parameters
#################

#CC = xlC

COMPILER_FLAGS = -I. -I$(APPINC) $(CFLAGS)

#################
# Header file parameters 
#################

#    Uncomment this if you want system header files to be expanded
#
# SYSHDRS       =

EXTHDRS	      = arqtraza.h \
                tcputil.h  \
                inih.h  

HDRS	      = supervisor.h

#################
# Loader and options to be used for the final program link step.
#################

LINKER_FLAGS = $(LDFLAGS) 

#################
# Library parameters
#################

LIBS          =  -lpthread $(APPLIB)/libarqtraza.a $(APPLIB)/libtcputil.a $(APPLIB)/libinih.a $(APPLIB)/shm.a

#################
# Program parameters
#################
PROGRAM     = supervisor

SRCS	    = supervisor.c
SRCPROCESOS = procesos.c

OBJS	    = supervisor.o
OBJPROCESOS = procesos.o

#################
# Target Rules
#################

all:		$(PROGRAM)


$(PROGRAM):     $(OBJS) $(MAKEFILE)
		$(CC) $(COMPILER_FLAGS) -o $(OBJPROCESOS) -c $(SRCPROCESOS) 
		$(CC) $(COMPILER_FLAGS) -o $(OBJS) -c $(SRCS) 
		$(CC) $(LINKER_FLAGS) -o $(PROGRAM) $(OBJS) $(OBJPROCESOS) $(LIBS)
		@echo "done"


clean:
	@rm -f $(OBJS)  $(OBJPROCESOS) core

clobber:
	@rm -f $(OBJS) $(OBJPROCESOS) $(PROGRAM) core

echo:
	@echo $(HDRS) $(SRCS)


.c.o:
	@echo "(*)Compilando $*.c..."
	$(CC) $(COMPILER_FLAGS) -c $*.c

