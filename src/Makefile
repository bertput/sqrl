#
# Makefile for SQRL
#

CC = gcc

GTK_CFLAGS = `pkg-config gtk+-2.0 --cflags`
GTK_LIBS   = `pkg-config gtk+-2.0 --libs`

LIBSQRL_CFLAGS = -I $(HOME)/projects/libsqrl/src/
LIBSQRL_LIBS   = -L $(HOME)/projects/libsqrl/build/lib -l sqrl

HOMEDIR = $(HOME)/projects/sqrl
DOCDIR  = $(HOMEDIR)/doc/generated
SRCDIR  = $(HOMEDIR)/src
BINDIR  = $(HOMEDIR)/bin

INSTALLDIR = $(HOME)/bin

# DEBUG     = -ggdb
CFLAGS    = -c $(GTK_CFLAGS) $(LIBSQRL_CFLAGS) -pthread -Wall $(DEBUG)
LINKLIBS  = $(GTK_LIBS) $(LIBSQRL_LIBS) -pthread  -lssl -lcrypto -luriparser

# SQLITEDIR = $(HOME)/projects/sqlite
# SQLITELIB = $(SQLITEDIR)/libsqlite.a



HFILES   = \
	log.h \
	settings.h \
	uriparse.h \
	fifo.h \
	progresswindow.h \
	mainwindow.h \
	client.h
CPPFILES = \
	log.c \
	settings.c \
	uriparse.c \
	fifo.c \
	progresswindow.c \
	mainwindow.c \
	client.c \
	main.c
OBJFILES = \
	log.o \
	settings.o \
	uriparse.o \
	fifo.o \
	client.o \
	progresswindow.o \
	mainwindow.o \
	main.o
TARGETS  = sqrl

# Search for include files in each directory,
#  in addition to the "usual" places.
#
INCLUDES  =

LFLAGS = \
	$(OBJFILES) \
	$(LINKLIBS) \
	$(SQLITELIB)


# This must be the first target, so you can specify additional targets
# in the include files which follow.
#
all : $(TARGETS) tags

debug : $(TARGETS)

# Individual targets
sqrl : $(OBJFILES)
	$(CC) -o sqrl $(LFLAGS)

.cpp.o :
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

tags : $(HFILES) $(CPPFILES)
	ctags $(HFILES) $(CPPFILES)

doc :
	rm -rf $(DOCDIR)/*
	doxygen

install:
	cp sqrl $(INSTALLDIR)

clean:
	rm $(TARGETS) $(OBJFILES)
