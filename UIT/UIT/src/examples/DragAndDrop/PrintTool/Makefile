#
#    SCCS ID: %Z%%M% %I% %E% SMI
#
#    Copyright (c) 1990,1991 Sun Microsystems, Inc.  All rights reserved.
#    See LEGAL_NOTICE file for terms of the license.
#

CPPFLAGS += -I$(UITHOME)/include/c++ -I$(OPENWINHOME)/include
LDFLAGS += -L$(UITHOME)/lib -L$(OPENWINHOME)/lib
LDLIBS += -lUIC -lxvps -lxview -lolgx -lX11 -lwire -lcps -lm

all: drag_source

drag_source: drag_source.o
	CC -o drag_source drag_source.o $(LDFLAGS) $(LDLIBS)

delete:
	/bin/rm -f *.o drag_source
