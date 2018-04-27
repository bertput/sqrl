#
# Makefile for SQRL
#

CC = gcc

GTK_CFLAGS = `pkg-config gtk+-2.0 --cflags`
GTK_LIBS   = `pkg-config gtk+-2.0 --libs`

# Find libsqrl...
ifneq (,$(LIBSQRL))
# Passed libsqrl directory as env variable LIBSQRL
LIBSQRL_CFLAGS = -I $(LIBSQRL)/src/
LIBSQRL_LIBS   = -lm
LIBSQRL_STATIC = $(LIBSQRL)/build/lib/libsqrl_static.a $(LIBSQRL)/build/lib/libsodium.a
$(info Using static libsqrl from: $(LIBSQRL))
else ifeq (1,$(shell test -f /usr/local/lib/libsqrl.so && echo "1"))
# Found libsqrl in /usr/local
LIBSQRL_CFLAGS = -I /usr/local/include
LIBSQRL_LIBS   = -L /usr/local/lib -l sqrl
LIBSQRL_STATIC =
$(info Using dynamic libsqrl from /usr/local)
else ifeq (1,$(shell test -d ~/projects/libsqrl/src && echo "1"))
# Try Bert's directory
LIBSQRL_CFLAGS = -I $(HOME)/projects/libsqrl/src/
LIBSQRL_LIBS   = -lm
LIBSQRL_STATIC = $(HOME)/projects/libsqrl/build/lib/libsqrl_static.a $(HOME)/projects/libsqrl/build/lib/libsodium.a
$(info Using static libsqrl from: ~/projects/libsqrl)
else
$(error Could not find libsqrl!)
endif

BUILDDIR = $(shell pwd)
DOCDIR   = $(BUILDDIR)/doc/generated
SRCDIR   = $(BUILDDIR)/src
BINDIR   = $(BUILDDIR)/bin

ifeq (,$(PREFIX))
PREFIX   = $(HOME)
endif

INSTALLDIR = $(PREFIX)/bin

DATADIR = $(HOME)/.sqrl
DATAINI = $(DATADIR)/sqrl.ini
DATAID  = $(DATADIR)/test.sqrl

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
	$(LIBSQRL_STATIC) \
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

installdata :
	@mkdir -p $(DATADIR)
ifneq (1,$(shell test -f $(DATAID) && echo "1"))
	$(info Installing $(DATAID))
	@cp $(BUILDDIR)/test.sqrl $(DATAID)
else
	$(info $(DATAID) already exists.  Skipping.)
endif
ifneq (1,$(shell test -f $(DATAINI) && echo "1"))
	$(info Installing $(DATAINI))
	@echo "[sqrl]" > $(DATAINI)
	@echo "id_filename=file://$(DATAID)" >> $(DATAINI)
	@echo "password=the password" >> $(DATAINI)
	@echo "rescue_code=948838472048277110863923" >> $(DATAINI)
else
	$(info $(DATAINI) already exists.  Skipping.)
endif


install : $(TARGETS) installdata
	@mkdir -p $(INSTALLDIR)
ifeq (1,$(shell test -f $(INSTALLDIR)/sqrl && echo "1"))
	$(info Overwriting $(INSTALLDIR)/sqrl)
else
	$(info Installing $(INSTALLDIR)/sqrl)
endif
	@cp sqrl $(INSTALLDIR)

clean:
	rm $(TARGETS) $(OBJFILES)
