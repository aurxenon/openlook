/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: waste.h,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:02 $
 *	Purpose : handling the wastebasket
 *
 * $Log: waste.h,v $
 * Revision 1.1.1.1  2005/07/13 18:31:02  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#ifndef __WASTE__
#define __WASTE__

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/icon.h>

#include "global.h"

Icon wasteicon;
char *wastedir;

extern void
waste_icon (Applic_T *app, int count);

extern void
waste_update_closed (Applic_T *app);

extern void
waste_empty (Panel_item item);

extern void
waste_init ();

#endif /* __WASTE__ */
