 
#       @(#) Makefile 1.1 93/04/06 

SSPKGHOME	= ../..

LIBDIR		= $(OPENWINHOME)/lib
LIBS		= -lxview -lolgx -lX11
SSPKGLIB	= $(SSPKGHOME)/lib/libsspkg.a

.KEEP_STATE:
.INIT: thermo.h

DBXFLAGS	=  -g 
CFLAGS		= $(DBXFLAGS) -I$(SSPKGHOME)/include -I$(OPENWINHOME)/include 
LDFLAGS		= -L$(LIBDIR) $(LIBS) $(SSPKGLIB) -lm

all: thermo

thermo: thermo.o main.o
	cc -o $@ thermo.o main.o $(LDFLAGS)

clean:
	-sccs clean
	rm -f *.o thermo

