/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: workwindow.h,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:04 $
 *	Purpose : display canvas, read directory
 *
 * $Log: workwindow.h,v $
 * Revision 1.1  2005/07/13 18:31:04  arkenoi
 * Initial revision
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#ifndef __WORKWINDOW__
#define __WORKWINDOW__

#include <sys/stat.h>

#include <X11/Xlib.h>

#include <xview/xview.h>
#include <xview/openmenu.h>
#include <xview/rect.h>
#include <xview/notify.h>
#include <sspkg/canshell.h>
#include <sspkg/drawobj.h>

#include "global.h"
#include "properties.h"

extern Notify_value
update_window_notify (Notify_client client, int which);

extern void
menu_directory (Menu menu, Menu_item menu_item);

extern void 
change_directory (Applic_T *app, char *path);

extern void
ww_sort_name (Menu menu, Menu_item menu_item);

extern void
ww_sort_type (Menu menu, Menu_item menu_item);

extern void
ww_list_name (Menu menu, Menu_item menu_item);

extern void
ww_list_type (Menu menu, Menu_item menu_item);

extern void
ww_list_size (Menu menu, Menu_item menu_item);

extern void
ww_list_date (Menu menu, Menu_item menu_item);

extern void
ww_small_icons (Menu menu, Menu_item menu_item);

extern void
ww_large_icons (Menu menu, Menu_item menu_item);

extern void
workwindow_init ();


#endif /* __WORKWINDOW__ */
