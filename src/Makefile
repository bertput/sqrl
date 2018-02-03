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

# DEBUG     = -ggdb
CFLAGS    = -c $(GTK_CFLAGS) $(LIBSQRL_CFLAGS) -pthread -Wall $(DEBUG)
LINKLIBS  = $(GTK_LIBS) $(LIBSQRL_LIBS) -pthread  -lssl -lcrypto

# SQLITEDIR = $(HOME)/projects/sqlite
# SQLITELIB = $(SQLITEDIR)/libsqlite.a



HFILES   = \
	log.h \
	settings.h \
	client.h
CPPFILES = \
	log.c \
	settings.c \
	client.c \
	main.c
OBJFILES = \
	log.o \
	settings.o \
	client.o \
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

clean:
	rm $(TARGETS) $(OBJFILES)