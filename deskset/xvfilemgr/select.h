/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: select.h,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:04 $
 *	Purpose : management of selected objects
 *
 * $Log: select.h,v $
 * Revision 1.1.1.1  2005/07/13 18:31:04  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#ifndef __SELECT__
#define __SELECT__

#include <sspkg/rectobj.h>

#include "global.h"

extern Selected_T *
select_add (Selected_T *select, Rectobj object);

extern Selected_T *
select_delete (Selected_T *select, Rectobj object);

extern Selected_T *
select_get (Selected_T *select, Rectobj object);

#endif /* __SELECT__ */
