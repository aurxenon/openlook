/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: waste.c,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:03 $
 *	Purpose : handling the wastebasket
 *
 * $Log: waste.c,v $
 * Revision 1.1.1.1  2005/07/13 18:31:03  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.9  1996/11/23 11:07:54  root
 * update windows immediatly after emptying the waste basket.
 *
 * Revision 1.8  1996/10/08 16:28:11  root
 * wrong notify_set call.
 *
 * Revision 1.7  1996/10/08 16:26:29  root
 * adjusting includes.
 *
 * Revision 1.6  1996/10/08 16:25:41  root
 * eliminating all waitpid system calls.
 *
 * Revision 1.5  1996/07/26 19:48:11  root
 * change some constants
 *
 * Revision 1.4  1996/06/27 19:03:48  root
 * Wait with updating wastebasket till all files during emptying are removed.
 *
 * Revision 1.3  1996/06/23 17:08:53  root
 * New error handling.
 *
 * Revision 1.2  1996/05/26 17:42:32  root
 * support for global constant HOME.
 *
 * Revision 1.1  1995/12/01 15:32:53  root
 * Initial revision
 *
 *
 */

#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>

#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/icon.h>
#include <xview/notify.h>

#include "global.h"
#include "waste.h"
#include "file.h"

#include "icons/waste_fullicon"
#include "icons/waste_fullmask"
#include "icons/waste_emptyicon"
#include "icons/waste_emptymask"

static Server_image wastefull;
static Server_image wastefull_mask;
static Server_image wasteempty;
static Server_image wasteempty_mask;
static Notify_client client = (Notify_client) 11;

static Notify_value
waste_wait (Notify_client client, int pid, union wait *status, 
	   struct rusage *rusage)
{
  if (WIFEXITED(*status)) {
    return NOTIFY_DONE;
  } /* if */
  return NOTIFY_IGNORED;
} /* waste_wait */


static Notify_value
waste_wait_update (Notify_client client, int pid, union wait *status, 
		   struct rusage *rusage)
{
  if (WIFEXITED(*status)) {
    update_window ();
    return NOTIFY_DONE;
  } /* if */
  return NOTIFY_IGNORED;
} /* waste_wait */


extern void
waste_icon (Applic_T *app, int count) 
{
  /* set new icon for wastebasket */
  if (app->type == WASTE_WINDOW)
    if (count > 0) {
      xv_set (app->icon, 
	      ICON_IMAGE, wastefull,
	      ICON_MASK_IMAGE, wastefull_mask,
	      NULL);
    } /* if */
    else {
      xv_set (app->icon, 
	      ICON_IMAGE, wasteempty,
	      ICON_MASK_IMAGE, wasteempty_mask,
	      NULL);
    } /* else */
} /* waste_icon */


extern void
waste_update_closed (Applic_T *app)
{
  struct dirent *dirp;
  DIR *dp;
  char path [PATH_MAX];

  if ((dp = opendir (wastedir)) == NULL)
    error_message_name (app->frame, path);
  while ((dirp = readdir (dp)) != NULL) {
    if (strcmp (dirp->d_name, ".") == 0 ||
	strcmp (dirp->d_name, "..") == 0)
      continue;
    /* more than one file */
    waste_icon (app, 1);
    break;
  } /* while */
  closedir (dp);
} /* waste_update_closed */


extern void
waste_empty (Panel_item item)
{
  Applic_T *app;
  char     name [PATH_MAX];
  struct dirent *dirp;
  DIR      *dp;
  pid_t    pid;

  app = (Applic_T *) xv_get (item, XV_KEY_DATA, EMPTY_APP);
  /* remove all files */
  if ((dp = opendir (wastedir)) == NULL)
    error_message_name (app->frame, wastedir);
  while ((dirp = readdir (dp)) != NULL) {
    if (strcmp (dirp->d_name, ".") == 0 ||
	strcmp (dirp->d_name, "..") == 0)
      continue;
    sprintf (name, "%s/%s", wastedir, dirp->d_name);
    if ((pid = fork ()) < 0)
      error_message (app->frame);
    else 
      if (pid == 0) {
	if (execlp ("rm", "rm", "-f", "-r", name, (char *) 0) < 0)
	  error_message_name (app->frame, name);
      } /* if */
    notify_set_wait3_func (client, waste_wait_update, pid);
  } /* while */
  closedir (dp);
  strcpy (app->path, wastedir);
} /* waste_empty */


extern void
waste_init ()
{
  Rect label_rect;
  char path [PATH_MAX];
  struct stat statbuf;
  pid_t    pid;

  /* create images for wastebasket icon */
  rect_construct (&label_rect, 0, 54, 64, 10);
  wasteempty = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				    XV_WIDTH, waste_emptyicon_width,
				    XV_HEIGHT, waste_emptyicon_height,
				    SERVER_IMAGE_X_BITS, waste_emptyicon_bits,
				    NULL);
  wasteempty_mask = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				XV_WIDTH, waste_emptymask_width,
				XV_HEIGHT, waste_emptymask_height,
				SERVER_IMAGE_X_BITS, waste_emptymask_bits,
				NULL);
  wastefull = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				    XV_WIDTH, waste_fullicon_width,
				    XV_HEIGHT, waste_fullicon_height,
				    SERVER_IMAGE_X_BITS, waste_fullicon_bits,
				    NULL);
  wastefull_mask = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				XV_WIDTH, waste_fullmask_width,
				XV_HEIGHT, waste_fullmask_height,
				SERVER_IMAGE_X_BITS, waste_fullmask_bits,
				NULL);
  /* create wastebasket icon */
  wasteicon = (Icon) xv_create (XV_NULL, ICON,
				ICON_IMAGE, wasteempty,
				ICON_MASK_IMAGE, wasteempty_mask,
				ICON_LABEL_RECT, &label_rect,
				XV_LABEL, "Waste",
				NULL);
  /* create wastebasket if it not exists */
  wastedir = (char *) malloc (strlen (HOME) + 14);
  strcpy (wastedir, HOME);
  strcat (wastedir, "/.wastebasket");
  if (stat (wastedir, &statbuf) < 0) {
    /* waste doesn't exists */
    if ((pid = fork ()) < 0)
      fprintf (stderr, 
        "XVFilemgr: can't fork a new process for creation of wastebasket.\n");
    else 
      if (pid == 0)
	if (execlp ("mkdir", "mkdir", "-m", "700", wastedir, (char *) 0) < 0)
	  ;
    notify_set_input_func (client, waste_wait, pid);
  } /* if */
} /* waste_init */
