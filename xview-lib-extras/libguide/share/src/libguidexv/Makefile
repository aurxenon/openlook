#
# makefile.linux
# to build libguide; please look at the original Makefile from
# SUN for copyright!
#
GUIDEHOME=/usr/openwin
OPENWINHOME=/usr/openwin

SOURCES.c = \
	gcm.c \
	gcc.c \
	gdd.c \
	gfm.c \
	gfm_load_dir.c \
	group.c \
	group_data.c \
	gutil.c \
        gcc_ui.c \
        gcc_stubs.c \
        gfm_ui.c \
        gfm_stubs.c

SOURCES.h = \
	gcm.h \
	gcc.h \
	gdd.h \
	gfm.h \
	group.h \
	group_impl.h \
	gutil.h \
        gcc_ui.h \
        gfm_ui.h \
        gfm_app.pr \
	gfm_data.pr \
	gfm_folder.pr \
	gfm_dotdot.pr \
	gfm_system.pr \
	gfm_unknown.pr \
	black16.pr \
	black32.pr

SOURCES = \
	$(SOURCES.h) \
	$(SOURCES.c)

OBJECTS = \
	gcm.o \
	gcc.o \
	gdd.o \
	gfm.o \
	gfm_load_dir.o \
	group.o \
	group_data.o \
	gutil.o \
        gcc_ui.o \
        gcc_stubs.o \
        gfm_ui.o \
        gfm_stubs.o

Targets.other = \
	gcc.info \
	gfm.info

LIBRARY = libguidexv.a

PROJECTDIR = $(GUIDEHOME)
DESTDIR = $(OPENWINHOME)

# Compiler flags.

CC = gcc
   
# CFLAGS += -g
CPPFLAGS += -I../libguide -I. -I$(OPENWINHOME)/include

# Standard targets.

all:	 $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar rv $@ $?
	ranlib $@
	rm -f $?

$(OBJECTS): $(SOURCES)
   
install: all
	install -d $(DESTDIR)/lib/locale/C/help
	install -m 444 $(Targets.other) $(DESTDIR)/lib/locale/C/help
	install -d $(DESTDIR)/lib
	install -m 664 $(LIBRARY) $(DESTDIR)/lib
	install -d $(DESTDIR)/include
	install -m 644 $(SOURCES.h) $(DESTDIR)/include
	(cd $(DESTDIR)/lib ; ranlib -t $(LIBRARY) ; chmod 444 $(LIBRARY))

clean:
	$(RM) $(OBJECTS) core


