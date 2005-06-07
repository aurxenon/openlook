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

OBJECTS = driver.o PropertiesWindow.o

TARGET = driver

all: $(TARGET)

driver: driver.o PropertiesWindow.o
	CC $(CCFLAGS) -o driver $(OBJECTS) $(LDLIBS)

driver.o: driver.cc PropertiesWindow.h
PropertiesWindow.o: PropertiesWindow.cc PropertiesWindow.h

delete:
	/bin/rm -f $(OBJECTS) $(TARGET)
