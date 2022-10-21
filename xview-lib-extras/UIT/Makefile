#
#      SCCS ID: @(#)Makefile 1.5 93/02/08 SMI
#
#      Copyright (c) 1990-1992 Sun Microsystems, Inc.  All rights reserved.
#      See LEGAL_NOTICE file for terms of the license.
#
GENERICHOME=./Generic
UITHOME=./UIT
UITDST=/usr/openwin
SLINGSHOT=SLINGSHOT
SSPKGHOME=/usr/openwin

world:
	cd Generic/src; make
	cd UIT/src/UIC; make

install:
	cd Generic/src; make install
	cd UIT/src/UIC; make install

examples:
	cd UIT/src/examples; make

clean:
	cd $(GENERICHOME)/src; make clean
	cd $(UITHOME)/src/UIC; make clean
