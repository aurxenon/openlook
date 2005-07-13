#ifndef __EDIT__
#define __EDIT__
/*
	System  : XVfilemgr
	File    : $RCSfile: edit.h,v $
	Author  : $Author: arkenoi $
	Date    : $Date: 2005/07/13 18:31:03 $
	Purpose : cut, copy, paste and link
*
* $Log: edit.h,v $
* Revision 1.1  2005/07/13 18:31:03  arkenoi
* Initial revision
*
* Revision 1.2  1996/05/26 15:11:47  root
* prototype for edit_exit.
*
* Revision 1.1  1996/05/19 19:27:15  root
* Initial revision
*
*
*/

#include <xview/openmenu.h>

#include "global.h"

extern void 
edit_cut (Menu menu, Menu_item item);

extern void 
edit_copy (Menu menu, Menu_item item);

extern void 
edit_paste (Menu menu, Menu_item item);

extern void 
edit_link (Menu menu, Menu_item item);

extern void
edit_init (Applic_T *app);

extern void
edit_exit ();

#endif /* __EDIT__ */
