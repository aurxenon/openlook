/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: find.h,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:02 $
 *	Purpose : finding a file
 *
 * $Log: find.h,v $
 * Revision 1.1.1.1  2005/07/13 18:31:02  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.2  1996/06/21 18:26:13  root
 * clean up #includes.
 *
 * Revision 1.1  1995/12/01 15:32:53  root
 * Initial revision
 *
 *
 */

#ifndef __FIND__
#define __FIND__

#include <xview/xview.h>
#include <xview/openmenu.h>

#include "global.h"

extern void
find_menu (Menu menu, Menu_item menu_item);

extern void
file_init (Applic_T *app);

#endif /* __FIND__ */
