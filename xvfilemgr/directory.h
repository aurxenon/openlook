/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: directory.h,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:06 $
 *	Purpose : all tasks concerning directories, folder view
 *
 * $Log: directory.h,v $
 * Revision 1.1  2005/07/13 18:31:06  arkenoi
 * Initial revision
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#ifndef __DIRECTORY__
#define __DIRECTORY__

#include <xview/xview.h>
#include <xview/openmenu.h>
#include <xview/svrimage.h>

#include "properties.h"

Server_image folder_image;

extern void
dir_new (char *name, FolderProp_T *fprops);

extern void
dir_delete (char *name);

extern FolderProp_T *
dir_get (char *name);

extern void
dir_set (char *name, FolderProp_T *fprops);

extern void
dir_create (Menu menu, Menu_item menu_item);

extern void
dir_folder_view (Menu menu, Menu_item menu_item);

extern void
dir_init ();

#endif /* __DIRECTORY__ */
