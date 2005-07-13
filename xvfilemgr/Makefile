# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.2 from Makefile.am

# Copyright (C) 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy, distribute and modify it.

#


SHELL = /bin/sh

srcdir = .
top_srcdir = .
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

pkgdatadir = $(datadir)/xvfilemgr
pkglibdir = $(libdir)/xvfilemgr
pkgincludedir = $(includedir)/xvfilemgr

top_builddir = .
DEFAULT_CP_CMD = /bin/cp
DEFAULT_EDITOR = /usr/openwin/bin/textedit
DEFAULT_MAILER = /bin/mail
DEFAULT_MV_CMD = /bin/mv
DEFAULT_RM_CMD = /bin/rm
HAVE_SLINGSHOT = yes
X11_CFLAGS = -I/usr/X11R6/include
X11_LIBS = -L/usr/X11R6/lib
HAVE_XVIEW = yes
HAVE_XVIEW_PRIVATE_HEADERS = yes
PACKAGE = xvfilemgr
SLINGSHOT_CFLAGS =   -I/usr/openwin/include 
SLINGSHOT_LIBS =   -L/usr/openwin/lib 
VERSION = 0.2e ALPHA
XVIEW_CFLAGS =   -I/usr/openwin/include
XVIEW_LIBS =   -L/usr/openwin/lib
xvdep = xvdep
xvfilemgr_bindir = /usr/openwin/bin
xvfilemgr_libdir = /usr/openwin/lib/pixmaps
xvfilemgr_mandir = /usr/openwin/man

COMPILE = $(CC) $(INCLUDES) $(CPPFLAGS)  $(DEFS) $(CFLAGS) $(X11_CFLAGS) $(SLINGSHOT_CFLAGS) $(XVIEW_CFLAGS)
LINK    = $(CC) $(CFLAGS) $(LDFLAGS) $(SLINGSHOT_LIBS) $(XVIEW_LIBS) $(X11_LIBS) -o $@
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA    = ${INSTALL} -m 644
XV_LIBS	= -lsspkg -lxview -lolgx -lXpm  -lX11  -lm
XV_LIBS_STATIC = /usr/openwin/lib/libsspkg.a /usr/openwin/lib/libxview.a /usr/openwin/lib/libolgx.a -lXpm  -lX11  -lm

NORMAL_INSTALL = true
PRE_INSTALL = true
POST_INSTALL = true
NORMAL_UNINSTALL = true
PRE_UNINSTALL = true
POST_UNINSTALL = true
CC = gcc
CFLAGS = -pipe -g -Wall
CPPFLAGS = 
MKDEP = $(CC) -M $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)

SRC = $(sort $(wildcard *.c) )
HDR = $(sort $(wildcard *.h) )
MANS1 = $(sort $(wildcard *.1) )
OBJ = $(SRC:.c=.o)
PROGRAM = $(PACKAGE)

# subdirectories in the distribution
SUBDIRS = images16 images32

all: $(PROGRAM) $(PROGRAM)-static

$(PROGRAM): $(OBJ)
	$(LINK) $(OBJ) $(XV_LIBS)

$(PROGRAM)-static: $(OBJ)
	$(LINK) $(OBJ) $(XV_LIBS_STATIC)

install: install-icons install-man install-bin

install-icons:
	mkdirhier $(xvfilemgr_libdir)
	list='$(SUBDIRS)'; for subdir in $$list; do \
	(cd $$subdir && $(MAKE) install); \
	done

install-man:
	mkdirhier $(xvfilemgr_mandir)/man1
	man='$(MANS1)'; for manpage in $$man; do \
	( $(INSTALL_DATA) $$manpage $(xvfilemgr_mandir)/man1/$$manpage ); \
	echo $(xvfilemgr_mandir)/man1/$$manpage ; \
	done

install-bin:
	mkdirhier $(xvfilemgr_bindir)
	$(INSTALL_PROGRAM) $(PROGRAM) $(xvfilemgr_bindir)
	$(INSTALL_DATA) filetype /etc/filetype

clean:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	(cd $$subdir && $(MAKE) clean); \
	done
	$(RM) -f $(OBJ) $(PROGRAM) $(PROGRAM)-static

distclean: clean
	list='$(SUBDIRS)'; for subdir in $$list; do \
	(cd $$subdir && $(MAKE) distclean); \
	done
	$(RM) -f config.cache config.h config.status config.log Makefile

.SUFFIXES: .c .o
.c.o:
	$(COMPILE) -c $<

.deps/%.P: %.c
	@echo "Computing dependencies for $<..."
	@o='o'; \
	test -n "$o" && o='$$o'; \
	$(MKDEP) $< >$@.tmp \
	  && sed "s,^\(.*\)\.o:,\1.$$o \1.l$$o $@:," < $@.tmp > $@ \
	  && rm -f $@.tmp

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
