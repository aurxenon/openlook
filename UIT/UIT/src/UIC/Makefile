#
#    SCCS ID: @(#)Makefile 1.8 93/06/01 SMI
#
#    Copyright (c) 1990-1992 Sun Microsystems, Inc.  All rights reserved.
#    See LEGAL_NOTICE file for terms of the license.
#
LIB     = $(UITHOME)/lib
INCLUDE = $(UITHOME)/include/c++
GENINC  = $(GENERICHOME)/include

# Use the CCFLAGS with -pic to create a .so file
CCFLAGS = -c -pic -I$(GENINC) -I$(INCLUDE) -I$(OPENWINHOME)/include
#CCFLAGS = -c -g -v -I$(GENINC) -I$(INCLUDE) -I$(OPENWINHOME)/include

#CCFLAGS += -DNO_XVPS

OBJECTS = UIObject.o UIWindow.o BaseWindow.o PopUpWindow.o \
	   ComponentDisplay.o UIDisplay.o UIComponent.o TextItem.o \
	   UIInputTextItem.o AlphanumericInput.o NumericInput.o Button.o \
	   Gauge.o Slider.o Scroller.o TextDisplay.o TermDisplay.o UIChoice.o \
	   ToggleChoice.o StackChoice.o CheckBoxChoice.o ListChoice.o \
	   ColorMap.o Notifier.o Image.o CharacterFont.o \
  	   UIMenu.o CommandMenu.o ChoiceMenu.o NoticePrompt.o \
	   HotRegion.o ComponentGroup.o MultilineInput.o \
	   DropTarget.o InputEvent.o TTYDisplay.o FileChooser.o

include Make-ss.OBJECTS
include Make.TARGET

delete:
	/bin/rm -f $(OBJECTS)
	/bin/rm -f $(SSOBJECTS)
	/bin/rm -f $(TARGET)
	/bin/rm -f $(INCLUDE)/Generic.h
	/bin/rm -f $(INCLUDE)/GenericList.h
	/bin/rm -f $(INCLUDE)/GenericHash.h
	/bin/rm -f $(INCLUDE)/Boolean.h

depend:
	/bin/rm -f $(INCLUDE)/Generic.h
	/bin/rm -f $(INCLUDE)/GenericList.h
	/bin/rm -f $(INCLUDE)/GenericHash.h
	/bin/rm -f $(INCLUDE)/Boolean.h
	makedepend -- $(CCFLAGS) -I$(CCINC) -- \
		   $(OBJECTS:.o=.cc) $(SSOBJECTS:.o=.cc)
	/bin/cp $(GENINC)/Generic.h $(INCLUDE)
	/bin/cp $(GENINC)/GenericList.h $(INCLUDE)
	/bin/cp $(GENINC)/GenericHash.h $(INCLUDE)
	/bin/cp $(GENINC)/Boolean.h $(INCLUDE)

##############################################################################
# makedepend will create the dependencies
