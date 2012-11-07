##
## Makefile per PP/Proxy (Proxy con Prefetching)
## (C) 2011 Fabian Priftaj <fmp@linuxmail.org>
##

## Directory
SRC	= src
HDR	= include
SUP	= support
DEPH	= $(SRC) $(HDR) $(SUP)

## Opzioni di compilazione/linking
CC	      = gcc
SDEFINE	= -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -D_REENTRANT -D_THREAD_SAFE
DEBUG		= -g -ggdb -O2
LIBRARY	= -lpthread -lm 
OPTIONS	= -Wall -pedantic -ansi
CFLAGS	= -I$(HDR) -I$(SUP) $(DEBUG) $(SDEFINE) $(UDEFINE) $(OPTIONS) -c
LFLAGS	= -o

all:	   client server proxy
	      @echo ' *** Compilazione terminata con successo *** '

debug:	CFLAGS := -DDEBUG $(CFLAGS)
debug:	all


## Compilazione
proxy.o:		$(SRC)/proxy.c $(DEPH)
				$(CC) $(CFLAGS) $(SRC)/proxy.c

server.o:	$(SRC)/server.c $(DEPH)
				$(CC) $(CFLAGS) $(SRC)/server.c

client.o:	$(SRC)/client.c $(DEPH)
				$(CC) $(CFLAGS) $(SRC)/client.c

util.o:		$(SRC)/util.c $(DEPH)
				$(CC) $(CFLAGS) $(SRC)/util.c

xlib.o:		$(SUP)/xlib.c $(DEPH)
				$(CC) $(CFLAGS) $(SUP)/xlib.c

cache.o:		$(SUP)/cache.c $(DEPH)
				$(CC) $(CFLAGS) $(SUP)/cache.c

log.o:		$(SUP)/log.c $(DEPH)
				$(CC) $(CFLAGS) $(SUP)/log.c

addrparser.o:	$(SUP)/addrparser.c $(DEPH)
				$(CC) $(CFLAGS) $(SUP)/addrparser.c

resparser.o:	$(SUP)/resparser.c $(DEPH)
				$(CC) $(CFLAGS) $(SUP)/resparser.c

reqparser.o:	$(SUP)/reqparser.c $(DEPH)
				$(CC) $(CFLAGS) $(SUP)/reqparser.c

## LINKING
proxy:		proxy.o util.o xlib.o addrparser.o resparser.o reqparser.o cache.o log.o
				$(CC) $(LFLAGS) proxy proxy.o util.o xlib.o addrparser.o resparser.o reqparser.o cache.o log.o $(LIBRARY)

server:		server.o util.o
				$(CC) $(LFLAGS) server server.o util.o $(LIBRARY)

client:		client.o util.o
				$(CC) $(LFLAGS) client client.o util.o $(LIBRARY)

## Pulizia di tutti i file generati dalla compilazione e dal linking
clean:
		rm -f *.log *~ *.o *.c~ core* *.stackdump client server proxy

## Creazione della documentazione tramite Doxygen
doc:
		doxygen doxydoc
