#ifndef __MISC__
#define __MISC__
/*
	System  : XVFilemgr
	File    : $RCSfile: misc.h,v $
	Author  : $Author: arkenoi $
	Date    : $Date: 2005/07/13 18:31:00 $
	Purpose : miscellaneous things
*
* $Log: misc.h,v $
* Revision 1.1.1.1  2005/07/13 18:31:00  arkenoi
* Initial import of 0.2g
*
*
* Revision 1.2  1996/08/16 17:10:45  root
* expandation of filename expressions.
*
* Revision 1.1  1996/07/27 08:54:09  root
* Initial revision
*
*
*/

extern char *find_dotfile (char *dotfile);

extern char *expand_dirname (char *arg, Applic_T *app);

#endif /* __MISC__ */

