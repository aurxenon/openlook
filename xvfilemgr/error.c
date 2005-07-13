/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: error.c,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:02 $
 *	Purpose : handle errors
 *
 * $Log: error.c,v $
 * Revision 1.1.1.1  2005/07/13 18:31:02  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.4  1996/08/22 14:03:38  root
 * removed unnecessary #include dependency.
 *
 * Revision 1.3  1996/06/23 17:51:09  root
 * Switch to simpler error handling using strerror and errno.
 *
 * Revision 1.2  1996/05/27 20:20:56  root
 * additional errors for special device files and fifos.
 *
 * Revision 1.1  1995/12/01 15:32:53  root
 * Initial revision
 *
 *
 */

#include <errno.h>
#include <string.h>

#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/notice.h>

#include "error.h"


extern void
error_message (Frame frame)
{
  Xv_notice notice;
  char      *message;

  message = strerror (errno);
  notice = (Xv_notice) xv_create (frame, NOTICE,
				  NOTICE_MESSAGE_STRINGS, 
				  message,
				  NULL,
				  NOTICE_BUTTON_YES, "Confirm",
				  XV_SHOW, TRUE,
				  NULL);
} /* error_message */


extern void
error_message_name (Frame frame, char *name)
{
  Xv_notice notice;
  char      *message, *n;

  n = malloc (strlen (name) + 2);
  message = strerror (errno);
  sprintf (n, "%s:", name);
  notice = (Xv_notice) xv_create (frame, NOTICE,
				  NOTICE_MESSAGE_STRINGS, 
				  n, message, NULL,
				  NOTICE_BUTTON_YES, "Confirm",
				  XV_SHOW, TRUE,
				  NULL);
} /* error_message_name */


