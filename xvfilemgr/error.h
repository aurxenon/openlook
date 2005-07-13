/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: error.h,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:03 $
 *	Purpose : handle errors
 *
 * $Log: error.h,v $
 * Revision 1.1  2005/07/13 18:31:03  arkenoi
 * Initial revision
 *
 * Revision 1.4  1996/08/22 14:03:12  root
 * removed unnecessary #include dependency.
 *
 * Revision 1.3  1996/06/23 17:19:49  root
 * Switch to a small interface handling all kinds of errors intern.
 *
 * Revision 1.2  1996/05/27 20:21:53  root
 * prototypes for error messages concerning special device files and fifos.
 *
 * Revision 1.1  1995/12/01 15:32:53  root
 * Initial revision
 *
 *
 */

#ifndef __ERROR__
#define __ERROR__

#include <xview/xview.h>
#include <xview/frame.h>

extern void
error_message (Frame frame);

extern void 
error_message_name (Frame frame, char *name);

#endif /* __ERROR__ */
