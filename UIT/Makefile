#
#      SCCS ID: @(#)Makefile 1.5 93/02/08 SMI
#
#      Copyright (c) 1990-1992 Sun Microsystems, Inc.  All rights reserved.
#      See LEGAL_NOTICE file for terms of the license.
#
GENERICHOME=/usr/installed2/UITV3/Generic
UITHOME=/usr/installed2/UITV3/UIT
SLINGSHOT=SLINGSHOT
SSPKGHOME=/usr/openwin

world:
	cd $(GENERICHOME)/src; make
	cd $(UITHOME)/src/UIC; make depend
	cd $(UITHOME)/src/UIC; make

world_svr4:
	cd $(GENERICHOME)/src; make -f Makefile.svr4
	cd $(UITHOME)/src/UIC; make -f Makefile.svr4

world_no_makedepend:
	cd $(GENERICHOME)/src; make
	cd $(UITHOME)/src/UIC; make -f Makefile.no_makedepend

world_NO_XVPS:
	cd $(GENERICHOME)/src; make
	cd $(UITHOME)/src/UIC; make -f Makefile.NO_XVPS

world_svr4_NO_XVPS:
	cd $(GENERICHOME)/src; make -f Makefile.svr4
	cd $(UITHOME)/src/UIC; make -f Makefile.svr4.NO_XVPS

world_g++:
	cd $(GENERICHOME)/src; make -f Makefile.g++
	cd $(UITHOME)/src/UIC; make -f Makefile.g++

world_g++_svr4:
	cd $(GENERICHOME)/src; make -f Makefile.g++.svr4
	cd $(UITHOME)/src/UIC; make -f Makefile.g++.svr4

world_g++_NO_XVPS:
	cd $(GENERICHOME)/src; make -f Makefile.g++
	cd $(UITHOME)/src/UIC; make -f Makefile.g++.NO_XVPS

world_g++_svr4_NO_XVPS:
	cd $(GENERICHOME)/src; make -f Makefile.g++.svr4
	cd $(UITHOME)/src/UIC; make -f Makefile.g++.svr4.NO_XVPS

delete:
	cd $(GENERICHOME)/src; make -i delete
	cd $(UITHOME)/src/UIC; make -i delete
