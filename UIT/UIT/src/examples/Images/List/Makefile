#
#    SCCS ID: %Z%%M% %I% %E% SMI
#
#    Copyright (c) 1990,1991 Sun Microsystems, Inc.  All rights reserved.
#    See LEGAL_NOTICE file for terms of the license.
#

CCFLAGS = -I$(OPENWINHOME)/include -I$(UITHOME)/include/c++ \
	  -L$(UITHOME)/lib  -L$(OPENWINHOME)/lib

LDLIBS = -lUIC -lxvps -lxview -lolgx -lX -lcps -lm
LDLIBS += -lwire

OBJECTS = list.o

TARGET = list

all: $(TARGET)

list: list.o
	CC $(CCFLAGS) -o list $(OBJECTS) $(LDLIBS)

list.o: list.cc

delete:
	/bin/rm -f $(OBJECTS) $(TARGET)
