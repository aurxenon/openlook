#ifndef __REMOTE__
#define __REMOTE__
/*
	System  : XVFilemanager
	File    : $RCSfile: remote.h,v $
	Author  : $Author: arkenoi $
	Date    : $Date: 2005/07/13 18:31:03 $
	Purpose : remote transfer of files
*
* $Log: remote.h,v $
* Revision 1.1.1.1  2005/07/13 18:31:03  arkenoi
* Initial import of 0.2g
*
*
* Revision 1.1  1996/06/21 18:27:09  root
* Initial revision
*
*
*/

#include <xview/xview.h>
#include <xview/openmenu.h>

#include "global.h"

extern void
remote_menu (Menu menu, Menu_item menu_item);

extern void
remote_init (Applic_T *app);

#endif /* __REMOTE__ */
