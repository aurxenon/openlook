/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: about.h,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:04 $
 *	Purpose : displays the about-panel
 *
 * $Log: about.h,v $
 * Revision 1.1.1.1  2005/07/13 18:31:04  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.2  1996/07/28 08:24:00  root
 * move the prototypes to about.c to keep the interface small.
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#ifndef __ABOUT__
#define __ABOUT__

#include <X11/Xlib.h>

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <xview/defaults.h>

#include "global.h"

extern void
about_notify (Panel_item item, Event *event);

extern void
about_init (Applic_T *app);

extern void
feedback_init (Applic_T *app);

#endif /* __ABOUT__ */
