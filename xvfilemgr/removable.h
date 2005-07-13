#ifndef __REMOVABLE__
#define __REMOVABLE__
/*
	System  : XVFilemgr
	File    : $RCSfile: removable.h,v $
	Author  : $Author: arkenoi $
	Date    : $Date: 2005/07/13 18:31:04 $
	Purpose : handling of cd-roms and floppy disks
*
* $Log: removable.h,v $
* Revision 1.1.1.1  2005/07/13 18:31:04  arkenoi
* Initial import of 0.2g
*
*
* Revision 1.2  1996/08/17 08:45:45  root
* rudimentary floppy and cdrom support.
*
* Revision 1.1  1996/07/31 19:16:08  root
* Initial revision
*
*
*/

#include <xview/panel.h>
#include <xview/openmenu.h>

#include "global.h"

extern void
remove_check_for_cdrom (Menu menu, Menu_item menu_item);

extern void
remove_cdrom_umount (Applic_T *app);

extern void
remove_eject_cdrom (Panel_item item);

extern void
remove_floppy_umount (Applic_T *app);

extern void
remove_check_for_floppy (Menu menu, Menu_item menu_item);

extern void
remove_format_floppy (Menu menu, Menu_item menu_item);

#endif /* __REMOVABLE__ */
