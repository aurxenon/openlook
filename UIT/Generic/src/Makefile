#
#      SCCS ID: @(#)Makefile 1.3 93/02/08 SMI
#
#      Copyright (c) 1990-1992 Sun Microsystems, Inc.  All rights reserved.
#      See LEGAL_NOTICE file for terms of the license.
#
CCFLAGS  = -c -pic
#CCFLAGS  = -c -g

INCLUDE = ../include/
LIB     = ../lib/
TEST    = ../test/

OBJECTS = Generic.o GenericHash.o GenericList.o

all: $(LIB)libGeneric.a

$(LIB)libGeneric.a : $(OBJECTS)
	ranlib $(LIB)libGeneric.a

xs:
	#load $(CCFLAGS) -I$(INCLUDE) $(OBJECTS:.o=.cc)

Generic.o : Generic.cc $(INCLUDE)Generic.h
	CC $(CCFLAGS) Generic.cc -I$(INCLUDE)
	ar rv $(LIB)libGeneric.a Generic.o

GenericHash.o : GenericHash.cc $(INCLUDE)GenericHash.h
	CC $(CCFLAGS) GenericHash.cc -I$(INCLUDE)
	ar rv $(LIB)libGeneric.a GenericHash.o

GenericList.o : GenericList.cc $(INCLUDE)GenericList.h
	CC $(CCFLAGS) GenericList.cc -I$(INCLUDE)
	ar rv $(LIB)libGeneric.a GenericList.o

delete:
	/bin/rm -f *.o $(LIB)*.a $(TEST)*test

test: hashtest listtest

hashtest:
	CC -o hashtest GenericHash.cc -DTEST -L$(LIB) -lGeneric -lm -I$(INCLUDE)
	mv hashtest $(TEST)

listtest:
	CC -o listtest GenericList.cc -DTEST -L$(LIB) -lGeneric -I$(INCLUDE)
	mv listtest $(TEST)
