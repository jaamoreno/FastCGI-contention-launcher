#################
# 'make' build parameters
#################
MAKEFILE = makefile

SHELL = /bin/sh

APPDIR = $(BASE_DIR)/supervisor/prtcp/arqtraza
APPLIB = $(APPDIR)/../lib
APPINC = .

#################
# Compiler parameters
#################

COMPILER_FLAGS = -I$(APPINC) $(CFLAGS)


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

HDRS = arqtraza.h

#################
# Library parameters
#################

LIBNAME = libarqtraza

LIBRARY = $(LIBNAME).a

SRCS = arqtraza.c

OBJS = arqtraza.o

CP = cp

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
arqtraza.o: arqtraza.h
