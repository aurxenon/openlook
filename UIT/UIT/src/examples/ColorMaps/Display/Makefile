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

OBJECTS = display.o

TARGET = display

all: $(TARGET)

display: display.o
	CC $(CCFLAGS) -o display $(OBJECTS) $(LDLIBS)

display.o: display.cc

delete:
	/bin/rm -f $(OBJECTS) $(TARGET)
