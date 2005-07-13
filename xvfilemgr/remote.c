/*
	System  : XVfilemgr
	File    : $RCSfile: remote.c,v $
	Author  : $Author: arkenoi $
	Date    : $Date: 2005/07/13 18:31:00 $
	Purpose : remote transfer of files
*
* $Log: remote.c,v $
* Revision 1.1.1.1  2005/07/13 18:31:00  arkenoi
* Initial import of 0.2g
*
*
* Revision 1.6  1996/10/08 16:22:58  root
* eliminating all waitpid system calls.
*
* Revision 1.5  1996/07/27 10:46:30  root
* respect setting of DIALOG_ALWAYS_RIGHT
*
* Revision 1.4  1996/07/26 19:46:12  root
* change some constants
*
* Revision 1.3  1996/06/23 17:05:13  root
* New error handling.
*
* Revision 1.2  1996/06/21 19:08:26  root
* actually copy files.
*
* Revision 1.1  1996/06/21 19:03:20  root
* Initial revision
*
*
*/


#include <limits.h>
#include <stdlib.h>

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/notify.h>

#include "config.h"
#include "global.h"
#include "remote.h"
#include "error.h"


static Frame      remote_frame;
static Panel      remote_panel;
static Panel_item remote_from_machine;
static Panel_item remote_from_path;
static Panel_item remote_to_machine;
static Panel_item remote_to_path;
static Notify_client client = (Notify_client) 10;


static Notify_value
remote_wait (Notify_client client, int pid, union wait *status, 
	     struct rusage *rusage)
{
  if (WIFEXITED(*status)) {
    return NOTIFY_DONE;
  } /* if */
  return NOTIFY_IGNORED;
} /* remote_wait */


static void
remote_copy (Panel_item item)
{
  pid_t    pid;
  char *from_path, *from_machine, *to_path, *to_machine;
  char *arg1, *arg2;

  from_machine = (char *) xv_get (remote_from_machine, PANEL_VALUE);
  from_path = (char *) xv_get (remote_from_path, PANEL_VALUE);
  to_machine = (char *) xv_get (remote_to_machine, PANEL_VALUE);
  to_path = (char *) xv_get (remote_to_path, PANEL_VALUE);
  arg1 = malloc (strlen (from_path) + strlen (from_machine) + 1);
  arg2 = malloc (strlen (to_path) + strlen (to_machine) + 1);
  sprintf (arg1, "%s:%s", from_machine, from_path);
  sprintf (arg2, "%s:%s", to_machine, to_path);
  /* actually copy the file. */
  if ((pid = fork ()) < 0)
    error_message (remote_frame);
  else 
    if (pid == 0)
      if (execlp ("rcp", "rcp", arg1, arg2, (char *) 0) < 0)
	error_message (remote_frame);
  notify_set_wait3_func (client, remote_wait, pid);
  xv_set (remote_frame, XV_SHOW, FALSE, NULL);
} /* remote_copy */


static void
remote_frame_init (Applic_T *app)
{
  remote_frame = (Frame) xv_create (app->frame, FRAME_CMD,
				  FRAME_LABEL, "Remote File Transfer",
				  FRAME_SHOW_RESIZE_CORNER, FALSE,
				  FRAME_SHOW_FOOTER, FALSE,
				  FRAME_MIN_SIZE, 530, 300,
				  XV_HEIGHT, 445,
				  XV_WIDTH, 550,
				  NULL);
  remote_panel = (Panel) xv_get (remote_frame, FRAME_CMD_PANEL);
  xv_set (remote_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  xv_create (remote_panel, PANEL_MESSAGE,
	     PANEL_LABEL_STRING, "SOURCE",
	     PANEL_LABEL_BOLD, TRUE,
	     XV_X, 195,
	     NULL);
  xv_create (remote_panel, PANEL_MESSAGE,
	     PANEL_LABEL_STRING, "Specify the machine and path name of the files to copy.",
	     XV_X, 5,
	     NULL);
  remote_from_machine = xv_create (remote_panel, PANEL_TEXT,
			   PANEL_LABEL_STRING, "Machine:",
			   PANEL_VALUE_DISPLAY_LENGTH, 40,
			   PANEL_VALUE_STORED_LENGTH, NAME_MAX,
			   PANEL_VALUE_X, 75,
			   NULL);
  remote_from_path = xv_create (remote_panel, PANEL_TEXT,
			   PANEL_LABEL_STRING, "Path:",
			   PANEL_VALUE_DISPLAY_LENGTH, 40,
			   PANEL_VALUE_STORED_LENGTH, PATH_MAX,
			   PANEL_VALUE_X, 75,
			   NULL);
  xv_create (remote_panel, PANEL_MESSAGE,
	     PANEL_LABEL_STRING, "DESTINATION",
	     PANEL_LABEL_BOLD, TRUE,
	     XV_X, 180,
	     XV_Y, 120,
	     NULL);
  xv_create (remote_panel, PANEL_MESSAGE,
	     PANEL_LABEL_STRING, "Specify the machine and path where you want the files to be copied.",
	     XV_X, 5,
	     NULL);
  remote_to_machine = xv_create (remote_panel, PANEL_TEXT,
			   PANEL_LABEL_STRING, "Machine:",
			   PANEL_VALUE_DISPLAY_LENGTH, 40,
			   PANEL_VALUE_STORED_LENGTH, NAME_MAX,
			   PANEL_VALUE_X, 75,
			   NULL);
  remote_to_path = xv_create (remote_panel, PANEL_TEXT,
			   PANEL_LABEL_STRING, "Path:",
			   PANEL_VALUE_DISPLAY_LENGTH, 40,
			   PANEL_VALUE_STORED_LENGTH, PATH_MAX,
			   PANEL_VALUE_X, 75,
			   NULL);
  xv_create (remote_panel, PANEL_BUTTON,
	     PANEL_LABEL_STRING, "Copy Files",
	     PANEL_NOTIFY_PROC, remote_copy,
	     XV_X, 185,
	     NULL);
  window_fit (remote_panel);
  window_fit (remote_frame);
} /* remote_init */


extern void
remote_menu (Menu menu, Menu_item menu_item)
{
  Applic_T *app;
  Rect     rect;
  Rect     rect2;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
#ifdef DIALOG_ALWAYS_RIGHT
  if (xv_get (remote_frame, XV_SHOW) == FALSE) {
    frame_get_rect (app->frame, &rect);
    frame_get_rect (remote_frame, &rect2);
    rect2.r_left = rect.r_left + rect.r_width;
    rect2.r_top = rect.r_top;
    frame_set_rect (remote_frame, &rect2);
  } /* if */
#endif
  xv_set (remote_frame, XV_SHOW, TRUE, NULL);
  xv_set (remote_frame, FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_IN, NULL);
} /* remote_menu */


extern void
remote_init (Applic_T *app)
{
  remote_frame_init (app);
} /* remote_init */
