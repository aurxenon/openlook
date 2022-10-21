/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: find.c,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:02 $
 *	Purpose : finding a file
 *
 * $Log: find.c,v $
 * Revision 1.1.1.1  2005/07/13 18:31:02  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.10  1997/11/02 22:55:11  root
 * Minor size adjustments.
 *
 * Revision 1.9  1997/11/02 21:44:59  root
 * Added icons for adjusting types of searchable files.
 *
 * Revision 1.8  1996/10/13 16:57:32  root
 * bug fix.
 *
 * Revision 1.7  1996/10/08 16:33:58  root
 * eliminating all waitpid system calls.
 *
 * Revision 1.6  1996/08/22 18:51:30  root
 * correct display of inactive buttons.
 *
 * Revision 1.5  1996/07/27 10:45:32  root
 * respect setting of DIALOG_ALWAYS_RIGHT
 *
 * Revision 1.4  1996/07/26 19:53:37  root
 * change some constants
 *
 * Revision 1.3  1996/06/23 17:11:08  root
 * New error handling.
 *
 * Revision 1.2  1996/05/13 22:56:27  root
 * bugfix.
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/notify.h>
#include <sspkg/canshell.h>
#include <sspkg/drawobj.h>
#include <sspkg/rectobj.h>

#include "config.h"
#include "global.h"
#include "find.h"
#include "properties.h"
#include "file.h"
#include "error.h"

#include "icons/folder16_glyph.icon"
#include "icons/folder16_mask.icon"
#include "icons/regular16_glyph.icon"
#include "icons/regular16_mask.icon"
#include "icons/linkdir16_glyph.icon"
#include "icons/linkdir16_mask.icon"

static Frame      find_frame;
static Panel      find_panel;
static Panel_item find_type;
static Panel_item find_dir;
static Panel_item find_name;
static Panel_item find_name_ie;
static Panel_item find_owner;
static Panel_item find_owner_ie;
static Panel_item find_after;
static Panel_item find_after_mh;
static Panel_item find_before;
static Panel_item find_before_mh;
static Panel_item find_list;
static Panel_item find_finding;
static Panel_item find_opening;
static Panel_item find_stoping;
static Panel_item find_reseting;
static Server_image dir_glyph, file_glyph, link_glyph;
static Notify_client client = (Notify_client) 14;


static Notify_value
find_wait (Notify_client client, int pid, union wait *status, 
	   struct rusage *rusage)
{
  if (WIFEXITED(*status)) {
    return NOTIFY_DONE;
  } /* if */
  return NOTIFY_IGNORED;
} /* find_wait */


static void
find_list_notify (Panel_item item)
{
  int n;

  n = (int) xv_get (item, PANEL_LIST_NROWS);
  if (n > 0)
    xv_set (find_opening, PANEL_INACTIVE, FALSE, NULL);
  else
    xv_set (find_opening, PANEL_INACTIVE, TRUE, NULL);
} /* find_list_notify */


static void
find_find (Panel_item item)
{
  pid_t pid;
  int   fd [2];
  int   n;
  int	findtype;
  int   c = 0;  /* counter vor argument array */
  char  line [PATH_MAX];
  char  *args [20];
  char  *option [] = {"find", "-not", "-name", "-user", "-mmin", "-mtime", "-type"};
  char	*fileoptions [] = {"d", "f", "l"};
  char  after [5];
  char  before [5];
  char	filetype [5];

  /* delete list */
  n = (int) xv_get (find_list, PANEL_LIST_NROWS);
  xv_set (find_list, PANEL_LIST_DELETE_ROWS, 0, n, NULL);
  xv_set (find_stoping, PANEL_INACTIVE, FALSE, NULL);
  xv_set (XVfilemgr_Server, SERVER_SYNC_AND_PROCESS_EVENTS, NULL);
  if (pipe (fd) < 0)
    error_message (find_frame);
  if ((pid = fork ()) < 0)
    error_message (find_frame);
  else
    if (pid > 0) {  /* parent */

      /* attach process id to stop button */
      xv_set (find_stoping, PANEL_CLIENT_DATA, pid, NULL);
      close (fd [1]);
      if (fd [0] != STDIN_FILENO) {
	if (dup2 (fd [0], STDIN_FILENO) != STDIN_FILENO)
	  error_message (find_frame);
	close (fd [0]);
      } /* if */
      n = 0;
      while (gets (line) != NULL) {
	xv_set (find_list, 
		PANEL_LIST_INSERT, n,
		PANEL_LIST_STRING, n, line, 
		NULL);
	n++;
      } /* while */
      notify_set_wait3_func (client, find_wait, pid);
      close (fd[1]);
    } /* if */
    else {          /* child */
      close (fd [0]);
      if (fd [1] != STDOUT_FILENO) {
	if (dup2 (fd [1], STDOUT_FILENO) != STDOUT_FILENO)
	  error_message (find_frame);
	close (fd [1]);
      } /* if */
      /* set options */
      /* programm name find and path*/
      args [c++] = option [0];
      args [c++] = (char *) xv_get (find_dir, PANEL_VALUE);
      /* name to include or exclude */
      if (strlen ((char *) xv_get (find_name, PANEL_VALUE)) > 0) {
	if (xv_get (find_name_ie, PANEL_VALUE) == 1)
	  args [c++] = option [1];
	args [c++] = option [2];
	args [c++] = (char *) xv_get (find_name, PANEL_VALUE);
      } /* if */
      /* owner to  or exclude */
      if (strlen ((char *) xv_get (find_owner, PANEL_VALUE)) > 0) {
	if (xv_get (find_owner_ie, PANEL_VALUE) == 1)
	  args [c++] = option [1];
	args [c++] = option [3];
	args [c++] = (char *) xv_get (find_owner, PANEL_VALUE);
      } /* if */
      /* modified after */
      if ((int) xv_get (find_after, PANEL_VALUE) > 0) {
	if ((int) xv_get (find_after_mh, PANEL_VALUE) == 0)
	  args [c++] = option [4];
	else
	  args [c++] = option [5];
	sprintf (after, "-%d", (int) xv_get (find_after, PANEL_VALUE));
	args [c++] = after;
      } /* if */
      /* modified before */
      if ((int) xv_get (find_before, PANEL_VALUE) > 0) {
	if ((int) xv_get (find_before_mh, PANEL_VALUE) == 0)
	  args [c++] = option [4];
	else
	  args [c++] = option [5];
	sprintf (before, "+%d", (int) xv_get (find_before, PANEL_VALUE));
	args [c++] = before;
      } /* if */
      /* Decide wether to include files, directories */
      if ((int) xv_get (find_type, PANEL_VALUE) >= 0) {
        findtype = (int) xv_get (find_type, PANEL_VALUE);
        if ( (findtype & 1) == 0 ) {
	  args [c++] = option [1];
	  args [c++] = option [6];
	  args [c++] = fileoptions[0];
	}
        if ( ((findtype & 2) / 2) == 0 ) {
	  args [c++] = option [1];
	  args [c++] = option [6];
	  args [c++] = fileoptions[1];
	}
        if ( ((findtype & 4) / 4) == 0 ) {
	  args [c++] = option [1];
	  args [c++] = option [6];
	  args [c++] = fileoptions[2];
	}
      }

      args [c] = NULL;
      if (execvp (args [0], &args[0]) < 0)
	error_message (find_frame);
    } /* else */
  xv_set (find_stoping, PANEL_INACTIVE, TRUE, NULL);
  find_list_notify (find_list);
} /* find_find */


static void
find_open (Panel_item item)
{
  Properties_T *props;
  int          n;
  char         *name;
  pid_t        pid;
  char         *method;
  int          c = 0;
  char         *curr_arg; 
  char         *next_arg;
  char         *args [25];

  props = get_properties ();
  n = (int) xv_get (find_list, PANEL_LIST_FIRST_SELECTED);
  name = (char *) xv_get (find_list, PANEL_LIST_STRING, n);
    if (props->other == 1) {
      /* use other editor */
      method = malloc (strlen (props->edit) + 1);
      strcpy (method, props->edit);
      next_arg = method;
      do {
	curr_arg = next_arg;
	next_arg = strchr (next_arg, ' ');
	if (next_arg != NULL) {
	  *next_arg = '\0';
	  next_arg++;
	} /* if */
	if (strcmp (curr_arg, "$FILE") == 0)
	  args [c++] = name;
	else
	  args [c++] = curr_arg;
      } while (next_arg != NULL);
      args [c] = NULL;
      if ((pid = fork ()) < 0)
	error_message (find_frame);
      else 
	if (pid == 0) {
	  if ((pid = fork ()) < 0)
	    error_message (find_frame);
	  else if (pid > 0)
	    exit (0);
	  if (execvp (args [0], &args[0]) < 0)
	    error_message_name (find_frame, args [0]);
	} /* if */
    } /* if */
    else {
      /* use Text Editor */
      if ((pid = fork ()) < 0)
	error_message (find_frame);
      else 
	if (pid == 0) {
	  if ((pid = fork ()) < 0)
	    error_message (find_frame);
	  else if (pid > 0)
	    exit (0);
	  if (execlp ("textedit", "textedit", name, (char *) 0) < 0)
	    error_message_name (find_frame, name);
	} /* if */
    } /* else */
} /* find_open */


static void
find_stop (Panel_item item)
{
  pid_t pid;

  pid = (pid_t) xv_get (item, PANEL_CLIENT_DATA);
  kill (pid, SIGTERM);
} /* find_stop */


static void
find_reset (Panel_item item)
{
  Applic_T *app;

  app = (Applic_T *) xv_get (item, PANEL_CLIENT_DATA);
  xv_set (find_type, PANEL_VALUE, 7, NULL);
  xv_set (find_dir, PANEL_VALUE, app->path, NULL);
  xv_set (find_name, PANEL_VALUE, "", NULL);
  xv_set (find_name_ie, PANEL_VALUE, 0, NULL);
  xv_set (find_owner, PANEL_VALUE, "", NULL);
  xv_set (find_owner_ie, PANEL_VALUE, 0, NULL);
  xv_set (find_after, PANEL_VALUE, 0, NULL);
  xv_set (find_after_mh, PANEL_VALUE, 1, NULL);
  xv_set (find_before, PANEL_VALUE, 0, NULL);
  xv_set (find_before_mh, PANEL_VALUE, 1, NULL);
} /* find_reset */

static void
find_newsize (Xv_Window window, Event event, Notify_arg arg)
{
  int    width, height;
  int    rows;
  int    font_height;
  XEvent *xevent;

  xevent = event_xevent (&event);
/* this causes a segmentation fault. Don't know why. */
/*  if (! ((xevent->type == ConfigureNotify) && 
	 (xevent->xconfigure.send_event == TRUE))) {*/
    width = (int) xv_get (find_frame, XV_WIDTH);
    height = (int) xv_get (find_frame, XV_HEIGHT);
    xv_set (find_list, PANEL_LIST_WIDTH, -1, NULL);
    font_height = (int) xv_get (find_list, PANEL_LIST_ROW_HEIGHT);
    rows = (height - 245) / font_height - 1;
    xv_set (find_list, PANEL_LIST_DISPLAY_ROWS, rows, NULL);
/*  } */ /* if */
} /* find_newsize */


static void
find_frame_init (Applic_T *app)
{
  find_frame = (Frame) xv_create (app->frame, FRAME_CMD,
				  FRAME_LABEL, "Find File",
				  FRAME_SHOW_RESIZE_CORNER, TRUE,
				  FRAME_SHOW_FOOTER, TRUE,
				  WIN_EVENT_PROC, find_newsize,
				  WIN_CONSUME_EVENTS, WIN_RESIZE, NULL,
				  FRAME_MIN_SIZE, 563, 400,
				  XV_WIDTH, 550,
/*				  XV_HEIGHT, 355, */
				  NULL);
  dir_glyph = (Server_image) xv_create (
			XV_NULL, SERVER_IMAGE,
			XV_WIDTH, folder16_glyph_width,
			XV_HEIGHT, folder16_glyph_height,
			SERVER_IMAGE_X_BITS, folder16_glyph_bits,
			NULL);
  file_glyph = (Server_image) xv_create (
			XV_NULL, SERVER_IMAGE,
			XV_WIDTH, regular16_glyph_width,
			XV_HEIGHT, regular16_glyph_height,
			SERVER_IMAGE_X_BITS, regular16_glyph_bits,
			NULL);
  link_glyph = (Server_image) xv_create (
			XV_NULL, SERVER_IMAGE,
			XV_WIDTH, linkdir16_glyph_width,
			XV_HEIGHT, linkdir16_glyph_height,
			SERVER_IMAGE_X_BITS, linkdir16_glyph_bits,
			NULL);

  find_panel = (Panel) xv_get (find_frame, FRAME_CMD_PANEL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  find_type = xv_create (find_panel, PANEL_TOGGLE,
			PANEL_CHOICE_NROWS, 1,
			PANEL_LAYOUT, PANEL_HORIZONTAL,
			PANEL_VALUE_X, 220,
			PANEL_LABEL_STRING,	"File Type:",
			PANEL_CHOICE_IMAGES,
				dir_glyph,
				file_glyph,
				link_glyph,
				NULL,
			PANEL_VALUE, 7,
			NULL);
  find_dir = xv_create (find_panel, PANEL_TEXT,
			PANEL_LABEL_STRING, "Find in and below folder:",
			PANEL_VALUE_DISPLAY_LENGTH, 30,
			PANEL_VALUE_STORED_LENGTH, PATH_MAX,
			PANEL_VALUE_X, 220,
			NULL);
  find_name = xv_create (find_panel, PANEL_TEXT,
			 PANEL_LABEL_STRING, "Name:",
			 PANEL_VALUE_DISPLAY_LENGTH, 20,
			 PANEL_VALUE_STORED_LENGTH, NAME_MAX,
			 PANEL_VALUE_X, 220,
			 NULL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  find_name_ie = xv_create (find_panel, PANEL_CHOICE,
			    PANEL_CHOICE_STRINGS, "Include", "Exclude", NULL,
			    NULL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);

  find_owner = xv_create (find_panel, PANEL_TEXT,
			  PANEL_LABEL_STRING, "Owner:",
			  PANEL_VALUE_DISPLAY_LENGTH, 20,
			  PANEL_VALUE_STORED_LENGTH, 40,
			  PANEL_VALUE_X, 220,
			  NULL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  find_owner_ie = xv_create (find_panel, PANEL_CHOICE,
			    PANEL_CHOICE_STRINGS, "Include", "Exclude", NULL,
			    NULL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);

  find_after = xv_create (find_panel, PANEL_NUMERIC_TEXT,
			  PANEL_LABEL_STRING, "Modified After:",
			  PANEL_VALUE_DISPLAY_LENGTH, 5,
			  PANEL_VALUE_STORED_LENGTH, 10,
			  PANEL_VALUE_X, 220,
			  NULL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  find_after_mh = xv_create (find_panel, PANEL_CHOICE,
			    PANEL_CHOICE_STRINGS, "Minutes", "Hours", NULL,
			    NULL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);

  find_before = xv_create (find_panel, PANEL_NUMERIC_TEXT,
			  PANEL_LABEL_STRING, "Modified Before:",
			  PANEL_VALUE_DISPLAY_LENGTH, 5,
			  PANEL_VALUE_STORED_LENGTH, 10,
			  PANEL_VALUE_X, 220,
			  NULL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  find_before_mh = xv_create (find_panel, PANEL_CHOICE,
			    PANEL_CHOICE_STRINGS, "Minutes", "Hours", NULL,
			    NULL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);

  find_opening = xv_create (find_panel, PANEL_BUTTON,
			    PANEL_LABEL_STRING, "Open",
			    PANEL_NOTIFY_PROC, find_open,
			    PANEL_INACTIVE, TRUE,
			    NULL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  find_finding = xv_create (find_panel, PANEL_BUTTON,
			    PANEL_LABEL_STRING, "Find",
			    PANEL_NOTIFY_PROC, find_find,
			    XV_X, 94,
			    NULL);
  find_stoping = xv_create (find_panel, PANEL_BUTTON,
			    PANEL_LABEL_STRING, "Stop",
			    PANEL_NOTIFY_PROC, find_stop,
			    PANEL_INACTIVE, TRUE,
			    NULL);
  find_reseting = xv_create (find_panel, PANEL_BUTTON,
			    PANEL_LABEL_STRING, "Reset",
			    PANEL_NOTIFY_PROC, find_reset,
			    PANEL_CLIENT_DATA, app,
			    NULL);
  xv_set (find_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  
  find_list = xv_create (find_panel, PANEL_LIST,
			 PANEL_LIST_DISPLAY_ROWS, 6,
			 PANEL_LIST_WIDTH, -1,
			 PANEL_LIST_MODE, PANEL_LIST_READ,
			 PANEL_LIST_SORT, PANEL_FORWARD,
			 PANEL_NOTIFY_PROC, find_list_notify,
			 PANEL_NEXT_ROW, 15,
			 NULL);
  find_reset (find_reseting);
  window_fit (find_panel);
  window_fit (find_frame);
} /* find_frame_init */


extern void
find_menu (Menu menu, Menu_item menu_item)
{
  Applic_T *app;
  Rect     rect;
  Rect     rect2;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
#ifdef DIALOG_ALWAYS_RIGHT
  if (xv_get (find_frame, XV_SHOW) == FALSE) {
    frame_get_rect (app->frame, &rect);
    frame_get_rect (find_frame, &rect2);
    rect2.r_left = rect.r_left + rect.r_width;
    rect2.r_top = rect.r_top;
    frame_set_rect (find_frame, &rect2);
  } /* if */
#endif
  xv_set (find_frame, XV_SHOW, TRUE, NULL);
  xv_set (find_frame, FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_IN, NULL);
} /* find_menu */


extern void
find_init (Applic_T *app)
{
  find_frame_init (app);
} /* file_init */



