/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: file.c,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:30:58 $
 *	Purpose : all tasks concerning a single file
 *
 * $Log: file.c,v $
 * Revision 1.1.1.1  2005/07/13 18:30:58  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.36  1998/10/18 01:54:44  root
 * Fixes.
 *
 * Revision 1.35  1998/10/18 01:51:01  root
 * Changed DEFAULT_CP_CMD, DEFAULT_MV_CMD, DEFAULT_RM_CMD, etc..
 *
 * Revision 1.34  1998/10/18 01:24:26  root
 * FreeBSD mods by Mark Ovens.
 *
 * Revision 1.33  1997/11/02 22:02:18  root
 * Small change to fix some problems that were reported.
 * Added "-dp" to flags in every execlp() using "cp".
 *
 * Revision 1.32  1997/05/12 21:26:08  root
 * Correct moving of files when dropped as multiple files.
 * Using incremental keys on XV_KEY_DATA attribute to avoid raise condition
 * in file_move_wait3 call.  file_move_wait3 call now gets the right key
 * data in every call.
 *
 * Revision 1.31  1997/03/23 11:36:26  root
 * removing unnecessary fprintfs
 *
 * Revision 1.30  1997/03/19 20:10:15  root
 * enlarge buffer for output of the file command
 *
 * Revision 1.29  1996/11/23 14:31:11  root
 * unused variable removed.
 *
 * Revision 1.28  1996/11/23 14:07:55  root
 * correct file_duplicate: now works in every directory.
 *
 * Revision 1.27  1996/11/23 12:36:52  root
 * file_move handles name conflicts in target directory.
 *
 * Revision 1.26  1996/10/26 12:41:47  root
 * count the number of files in the selection for drag and drop objects.
 *
 * Revision 1.25  1996/10/14 19:43:03  root
 * Different wait function for output window of custom command,
 * so the input function could be reset explizitly.  Otherwise
 * this leads to heavy cpu utilisation.
 *
 * Revision 1.24  1996/10/14 19:30:26  root
 * execution of custom commands with output and parameter window.
 *
 * Revision 1.23  1996/10/08 16:23:13  root
 * adjusting includes.
 *
 * Revision 1.22  1996/10/08 16:07:43  root
 * eliminating all wait system calls.
 *
 * Revision 1.21  1996/07/28 10:11:23  root
 * bugfix in file_move: Do not delete file when it is moved onto itself.
 *
 * Revision 1.20  1996/07/28 08:53:52  root
 * change some constants.
 * new file_file function returning the output from file(1).
 *
 * Revision 1.19  1996/07/24 20:37:44  root
 * Really delete a moved file when it is succesfully copied.
 *
 * Revision 1.18  1996/07/22 19:02:14  root
 * Use file_rename instead of file_move in file_change_name.
 *
 * Revision 1.17  1996/07/18 20:12:00  root
 * #include added.
 *
 * Revision 1.16  1996/07/18 19:46:12  root
 * When moving files do not delete the source until it is succesfully copied.
 *
 * Revision 1.15  1996/07/18 19:33:28  root
 * Changing of file name in responds to a diret editing of the icon text.
 *
 * Revision 1.14  1996/06/23 17:18:50  root
 * New error handling.
 *
 * Revision 1.13  1996/06/22 11:31:51  root
 * Display free bytes in file info popup.
 *
 * Revision 1.12  1996/06/01 14:03:25  root
 * display mount information in file info popup.
 *
 * Revision 1.11  1996/05/27 18:14:54  root
 * file selection in waste window.
 *
 * Revision 1.10  1996/05/27 17:07:37  root
 * file_move now when moving trough filesystems.
 *
 * Revision 1.9  1996/05/27 10:35:35  root
 * selection of elements in the canvas.
 *
 * Revision 1.8  1996/05/26 17:45:47  root
 * minor improvement.
 *
 * Revision 1.7  1996/05/26 17:39:29  root
 * file_delete now can delete a file in everey directory not just in the
 * current.
 *
 * Revision 1.6  1996/05/26 15:12:10  root
 * second delete for calling from outside file.c.
 *
 * Revision 1.5  1996/05/26 14:30:26  root
 * change signature of file_move.
 *
 * Revision 1.4  1996/05/26 14:26:44  root
 * converted file_move to extern for edit.c
 *
 * Revision 1.3  1996/05/26 13:23:55  root
 * bug fix.
 *
 * Revision 1.2  1996/05/26 13:21:39  root
 * added file_copy and file_link for cut, copy, paste support.
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/notice.h>
#include <xview/dragdrop.h>
#include <xview/sel_pkg.h>
#include <xview/notify.h>
#include <xview/textsw.h>
#include <sspkg/canshell.h>
#include <sspkg/drawobj.h>
#include <sspkg/rectobj.h>

#include "config.h"
#include "global.h"
#include "file.h"
#include "properties.h"
#include "select.h"
#include "error.h"
#include "mount.h"

#define OVERWRITE 1
#define RENAME    2
#define CANCEL    3

#define READ_PERM 1
#define WRIT_PERM 2
#define EXEC_PERM 4

/* some keys for XV_KEY_DATA */
#define PARA_APP       241
#define PARA_CMD       242
#define PARA_NAME      243
#define MOVE_FRAME     246
#define RENAME_FRAME   247
#define RENAME_TARGET  248
#define RENAME_SOURCE  249
#define RENAME_AS      250

static Frame      parameter_frame;
static Panel_item parameter_input;
static Frame      info_frame;
static Panel      info_panel;
static Panel_item info_name;
static Panel_item info_icon;
static Panel_item info_owner;
static Panel_item info_group;
static Panel_item info_size;
static Panel_item info_content;
static Panel_item info_type;
static Panel_item info_atime;
static Panel_item info_mtime;
static Panel_item info_pown;
static Panel_item info_pgrp;
static Panel_item info_poth;
static Panel_item info_open;
static Panel_item info_mpoint;
static Panel_item info_mfrom;
static Panel_item info_free;
static Panel_item info_apply;
static Panel_item info_reset;
static Textsw     output;
static Selection_requestor default_sel;
static Notify_client client = (Notify_client) 10;
static int        fd[2]; 
static int MOVE_SOURCE =  20000;
static int MOVE_TARGET =  20001;
static int MOVE_SOURCE_BAK =  20000;
static int MOVE_TARGET_BAK =  20001;


static Notify_value
file_wait (Notify_client client, int pid, union wait *status, 
	   struct rusage *rusage)
{
  if (WIFEXITED(*status)) {
    return NOTIFY_DONE;
  } /* if */
  return NOTIFY_IGNORED;
} /* file_wait */


static Notify_value
file_wait_update (Notify_client client, int pid, union wait *status, 
		  struct rusage *rusage)
{
  if (WIFEXITED(*status)) {
    update_window ();
    return NOTIFY_DONE;
  } /* if */
  return NOTIFY_IGNORED;
} /* file_wait */


extern void 
file_copy (Frame frame, char *from, char *to)
{
  pid_t    pid;

  if ((pid = fork ()) < 0)
    error_message (frame);
  else 
    if (pid == 0) {
#ifdef __FreeBSD__    /* FreeBSD doesn't support '-d' */
      if (execlp (DEFAULT_CP_CMD, DEFAULT_CP_CMD, "-pf", from, to, (char *) 0) < 0)
#else
      if (execlp (DEFAULT_CP_CMD, DEFAULT_CP_CMD, "-dpf", from, to, (char *) 0) < 0)
#endif
	error_message (frame);
    } /* if */
    else
      notify_set_wait3_func (client, file_wait, pid);
} /* file_copy */


extern void 
file_link (Frame frame, char *from, char *to)
{
  pid_t    pid;

  if ((pid = fork ()) < 0)
    error_message (frame);
  else 
    if (pid == 0) {
      if (execlp ("ln", "ln", "-s", from, to, (char *) 0) < 0)
	error_message (frame);
    } /* if */
    else
      notify_set_wait3_func (client, file_wait, pid);
} /* file_link */


extern void 
file_delete2 (Frame frame, char *name)
{
  pid_t    pid;

  if ((pid = fork ()) < 0)
    error_message (frame);
  else 
    if (pid == 0) {
      if (execlp (DEFAULT_RM_CMD, DEFAULT_RM_CMD, "-f", "-R", name, (char *) 0) < 0)
	error_message (frame);
    } /* if */
    else
      notify_set_wait3_func (client, file_wait_update, pid);
} /* file_delete2 */


static void
file_rename (Frame frame, char *from, char *to)
{
  pid_t    pid;

  if ((pid = fork ()) < 0)
    error_message (frame);
  else 
    if (pid == 0) {
      if (execlp (DEFAULT_MV_CMD, DEFAULT_MV_CMD, "-f", from, to, (char *) 0) < 0)
	error_message (frame);
    } /* if */
    else
      notify_set_wait3_func (client, file_wait, pid);
} /* file_rename */


static void
file_chown (Frame frame, char *name, char *to)
{
  pid_t    pid;

  if ((pid = fork ()) < 0)
    error_message (frame);
  else 
    if (pid == 0) {
      if (execlp ("chown", "chown", "-f", to, name, (char *) 0) < 0)
	error_message (frame);
    } /* if */
    else
      notify_set_wait3_func (client, file_wait, pid);
} /* file_chown */


static void
file_chgrp (Frame frame, char *name, char *to)
{
  pid_t    pid;

  if ((pid = fork ()) < 0)
    error_message (frame);
  else 
    if (pid == 0) {
      if (execlp ("chgrp", "chgrp", "-f", to, name, (char *) 0) < 0)
	error_message (frame);
    } /* if */
    else
      notify_set_wait3_func (client, file_wait, pid);
} /* file_chgrp */


static void
file_chmod (Frame frame, char *name, char *perm)
{
  pid_t  pid;

  if ((pid = fork ()) < 0)
    error_message (frame);
  else 
    if (pid == 0) {
      if (execlp ("chmod", "chmod", "-f", perm, name, (char *) 0) < 0)
	error_message (frame);
    } /* if */
    else
      notify_set_wait3_func (client, file_wait, pid);
} /* file_chmod */


extern void
file_selected (Applic_T *app)
{
  if (app->type != WASTE_WINDOW) {
    /* the waste windwo has no file menu */
    xv_set (app->file_open, MENU_INACTIVE, FALSE, NULL);
    xv_set (app->file_duplicate, MENU_INACTIVE, FALSE, NULL);
    xv_set (app->file_information, MENU_INACTIVE, FALSE, NULL);
  } /* if */
} /* file_selected */


extern void
nofile_selected (Applic_T *app)
{
  if (app->type != WASTE_WINDOW) {
    /* the waste windwo has no file menu */
    xv_set (app->file_open, MENU_INACTIVE, TRUE, NULL);
    xv_set (app->file_duplicate, MENU_INACTIVE, TRUE, NULL);
    xv_set (app->file_information, MENU_INACTIVE, TRUE, NULL);
  } /* if */
} /* nofile_selected */


extern void
file_select (Rectobj object, int selected, Event *event)
{
  Applic_T *app;
  
  app = (Applic_T *) xv_get (object, XV_KEY_DATA, FILE_APP);
  if (selected == TRUE)
    /* add the selected object to the  selected list of the Applic_T struct */
    app->select = select_add (app->select, object);
  else
    /* delete the selected item */
    app->select = select_delete (app->select, object);
  if (app->select != NULL)
    file_selected (app);
  else
    nofile_selected (app);
} /* file_select */


extern void
file_select_all (Menu menu, Menu_item menu_item)
{
  Applic_T *app;
  File_T   *file;
  int      selected;
  
  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  /* iterate through all files in the canvas an select them */
  file = app->files;
  while (file != NULL) {
    selected = (int) xv_get (file->icon, RECTOBJ_SELECTED);
    if (selected == FALSE) {
      /* this causes a call to file_select automaticly */
      xv_set (file->icon, RECTOBJ_SELECTED, TRUE, NULL);
    } /* if */
    file = file->next;
  } /* while */
} /* file_select_all */


static void
open_textedit (Applic_T *app, char *name)
{
  char  *full_name;
  pid_t pid;

  if ((pid = fork ()) < 0)
    error_message (app->frame);
  else 
    if (pid == 0) {
      if ((pid = fork ()) < 0)
	error_message (app->frame);
      else if (pid > 0)
	exit (0);
      full_name = malloc (strlen (app->path) + strlen (name) + 2);
      sprintf (full_name, "%s/%s", app->path, name);
      if (execlp (DEFAULT_EDITOR, DEFAULT_EDITOR, full_name, (char *) 0) < 0)
	error_message (app->frame);
      free (full_name);
    } /* if */
  else
    notify_set_wait3_func (client, file_wait, pid);
} /* open_textedit */


static void
execute_cmdstr (Applic_T *app, char *cmdstr, char *file)
{
  pid_t pid;
  char  *method;
  int   c = 0;
  char  *curr_arg; 
  char  *next_arg;
  char  *name;
  char  *args [25];

  method = malloc (strlen (cmdstr) + 1);
  strcpy (method, cmdstr);
  next_arg = method;
  do {
    curr_arg = next_arg;
    next_arg = strchr (next_arg, ' ');
    if (next_arg != NULL) {
      *next_arg = '\0';
      next_arg++;
    } /* if */
    if (strcmp (curr_arg, "$FILE") == 0) {
      /* full path name as argument */
      if (file != NULL) {
	name = malloc (strlen (app->path) + strlen (file) + 2);
	sprintf (name, "%s/%s", app->path, file);
	args [c++] = name;
      } /* if */
      else {
	/* no file selected */
	error_message (app->frame);
      } /* else */
    } /* if */
    else
      /* valid argument? */
      if (strlen (curr_arg) > 0)
	args [c++] = curr_arg;
  } while (next_arg != NULL);
  args [c] = NULL;
  if ((pid = fork ()) < 0)
    error_message (app->frame);
  else 
    if (pid == 0) {
      if ((pid = fork ()) < 0)
	error_message (app->frame);
      else if (pid > 0)
	exit (0);
      if (execvp (args [0], &args[0]) < 0)
	error_message_name (app->frame, args[0]);
      free (name);
    } /* if */
    else
      notify_set_wait3_func (client, file_wait, pid);
} /* execute_cmdstr */


static Notify_value
execute_command_read_input (Notify_client client, int fd)
{
  char line[64];
  int  n, i;

  if (ioctl (fd, FIONREAD, &n) == 0)
    while (n > 0) {
      if ((i = read (fd, line, sizeof line)) > 0) {
	textsw_insert(output, line, i);
	n -= i;
      } /* if */
    } /* while */
  return NOTIFY_DONE;
} /* execute_command_read_input */


static Notify_value
execute_command_wait (Notify_client client, int pid, union wait *status, 
		     struct rusage *rusage)
{
  if (WIFEXITED(*status)) {
    notify_set_input_func(client, NOTIFY_FUNC_NULL,
			  (client == client)? fd[0] : 0);
    return NOTIFY_DONE;
  } /* if */
  return NOTIFY_IGNORED;
} /* execute_command_wait */


static void
execute_cmdstr_output (Applic_T *app, char *cmdstr, char *file)
{
  pid_t pid;
  char  *method;
  int   c = 0;
  char  *curr_arg; 
  char  *next_arg;
  char  *name;
  char  *args [25];
  char  *title;
  Frame frame;

  method = malloc (strlen (cmdstr) + 1);
  strcpy (method, cmdstr);
  next_arg = method;
  do {  /* process command string */
    curr_arg = next_arg;
    next_arg = strchr (next_arg, ' ');
    if (next_arg != NULL) {
      *next_arg = '\0';
      next_arg++;
    } /* if */
    if (strcmp (curr_arg, "$FILE") == 0) {
      /* full path name as argument */
      if (file != NULL) {
	name = malloc (strlen (app->path) + strlen (file) + 2);
	sprintf (name, "%s/%s", app->path, file);
	args [c++] = name;
      } /* if */
      else {
	/* no file selected */
	error_message (app->frame);
      } /* else */
    } /* if */
    else
      /* valid argument? */
      if (strlen (curr_arg) > 0)
	args [c++] = curr_arg;
  } while (next_arg != NULL);
  args [c] = NULL;
  /* create output window */
  title = malloc (strlen (cmdstr) + 12);
  sprintf (title, "XVFilemgr: %s", cmdstr);
  frame = (Frame) xv_create (app->frame, FRAME_CMD,
			     FRAME_LABEL, title,
			     FRAME_SHOW_RESIZE_CORNER, TRUE,
			     FRAME_SHOW_FOOTER, FALSE,
			     FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_IN,
			     NULL);
  output = xv_create (frame, TEXTSW,
		      TEXTSW_BROWSING, TRUE,
		      TEXTSW_DISABLE_LOAD, TRUE,
		      TEXTSW_IGNORE_LIMIT, TEXTSW_INFINITY,
		      TEXTSW_MEMORY_MAXIMUM, MAX_LOG_LENGTH,
		      XV_X, 0,
		      XV_Y, 0,
		      NULL);
  window_fit (frame);
  free (title);
  xv_set (frame, XV_SHOW, TRUE, NULL);
  if (pipe (fd) < 0)
    error_message (app->frame);
  if ((pid = fork ()) < 0)
    error_message (app->frame);
  else 
    if (pid > 0) {  /* parent */
      close (fd [1]);
      notify_set_input_func (client, execute_command_read_input, fd[0]);
      notify_set_wait3_func (client, execute_command_wait, pid);
    } /* if */
    else {          /* child */
      close (fd [0]);
      /* write all output from stdin to the pipe */
      if ((fd [1] != STDOUT_FILENO) && (fd[1] != STDERR_FILENO)) {
	if (dup2 (fd [1], STDOUT_FILENO) != STDOUT_FILENO)
	  error_message (app->frame);
	if (dup2 (fd [1], STDERR_FILENO) != STDERR_FILENO)
	  error_message (app->frame);
	close (fd [1]);
      } /* if */
      if (execvp (args [0], &args[0]) < 0)
	error_message_name (app->frame, args[0]);
      free (name);
    } /* else */
} /* execute_cmdstr_output */


static void
execute_command (Applic_T *app, Command_T *command, char *filename, 
		 char *parameter)
{
  char *cmd;

  cmd = malloc (strlen (command->command) + strlen (parameter) + 2);
  sprintf (cmd, "%s %s", command->command, parameter);
  if (command->outwin == 0)
    /* output window */
    execute_cmdstr_output (app, cmd, filename);
  else 
    /* no output window */
    execute_cmdstr (app, cmd, filename);
  free (cmd);
} /* execute_command */


static void
open (Applic_T *app, Properties_T *props, File_T *file)
{
  pid_t pid;
  char  *method;
  int   c = 0;
  char  *curr_arg; 
  char  *next_arg;
  char  *name;
  char  *args [25];

  if (file->type->open != NULL) {
    /* use filetype specific open method */
    execute_cmdstr (app, file->type->open, file->name);
  } /* if */
  else {
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
	if (strcmp (curr_arg, "$FILE") == 0) {
	  /* full path name as argument */
	  name = malloc (strlen (app->path) + strlen (file->name) + 2);
	  sprintf (name, "%s/%s", app->path, file->name);
	  args [c++] = name;
	} /* if */
	else
	  args [c++] = curr_arg;
      } while (next_arg != NULL);
      args [c] = NULL;
      if ((pid = fork ()) < 0)
	error_message (app->frame);
      else 
	if (pid == 0) {
	  if ((pid = fork ()) < 0)
	    error_message (app->frame);
	  else if (pid > 0)
	    exit (0);
	  if (execvp (args [0], &args[0]) < 0)
	    error_message_name (app->frame, args[0]);
	  free (name);
	} /* if */
	else
	  notify_set_wait3_func (client, file_wait, pid);
    } /* if */
    else
      /* use Text Editor */
      open_textedit (app, file->name);
  } /* else */
} /* open */


extern void
file_menu_open (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Selected_T   *sel;
  Properties_T *props;
  File_T       *file;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  sel = (Selected_T *) app->select;
  props = get_properties ();
  while (sel != NULL) {
    file = (File_T *) xv_get (sel->object, XV_KEY_DATA, FILE_FILE);
    open (app, props, file);
    sel = sel->next;
  } /* while */
} /* file_menu_open */


extern void
file_menu_textedit (Menu menu, Menu_item menu_item)
{
  Applic_T   *app;
  Selected_T *sel;
  File_T     *file;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  sel = (Selected_T *) app->select;
  while (sel != NULL) {
    file = (File_T *) xv_get (sel->object, XV_KEY_DATA, FILE_FILE);
    open_textedit (app, file->name);
    sel = sel->next;
  } /* while */
} /* file_menu_open */


extern void
file_menu_gotoarg (Menu menu, Menu_item menu_item)
{
  Applic_T *app;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
} /* file_menu_open */


static void
file_parameter_ok (Panel_item item)
{
  Applic_T  *app;
  Command_T *command;
  char      *filename;
  char      *parameter;

  command = (Command_T *) xv_get (item, PANEL_CLIENT_DATA);
  xv_set (parameter_frame, 
	  FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_OUT,
	  XV_SHOW, FALSE, 
	  NULL);
  app = (Applic_T *) xv_get (item, XV_KEY_DATA, PARA_APP);
  command = (Command_T *) xv_get (item, XV_KEY_DATA, PARA_CMD);
  filename = (char *) xv_get (item, XV_KEY_DATA, PARA_NAME);
  parameter = (char *) xv_get (parameter_input, PANEL_VALUE);
  /* now invoke the command with the parameters */
  execute_command (app, command, filename, parameter);
} /* file_parameter_ok */


static void
file_parameter_cancel (Panel_item item)
{
  xv_set (parameter_frame, 
	  FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_OUT,
	  XV_SHOW, FALSE, 
	  NULL);
} /* file_parameter_cancel */


static void
file_get_parameter (Applic_T *app, Command_T *command, char *filename)
{
  Panel      panel;
  Rect       rect;

  parameter_frame = (Frame) xv_create (app->frame, FRAME_CMD,
				       FRAME_LABEL, "Parameter",
				       FRAME_SHOW_RESIZE_CORNER, FALSE,
				       FRAME_SHOW_FOOTER, FALSE,
				       FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_IN,
				       NULL);
#ifdef DIALOG_ALWAYS_RIGHT
  frame_get_rect (app->frame, &rect);
  rect.r_left += rect.r_width;
  frame_set_rect (parameter_frame, &rect);
#endif
  panel = (Panel) xv_get (parameter_frame, FRAME_CMD_PANEL);
  parameter_input = xv_create (panel, PANEL_TEXT,
			       PANEL_LABEL_STRING, command->prompt,
			       PANEL_VALUE_DISPLAY_LENGTH, 50,
			       PANEL_VALUE_STORED_LENGTH, 150,
			       NULL);
  xv_set (panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  (void) xv_create (panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "OK",
		    PANEL_NOTIFY_PROC, file_parameter_ok,
		    XV_KEY_DATA, PARA_APP, app,
		    XV_KEY_DATA, PARA_CMD, command,
		    XV_KEY_DATA, PARA_NAME, filename,
		    NULL);
  xv_set (panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  (void) xv_create (panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Cancel",
		    PANEL_NOTIFY_PROC, file_parameter_cancel,
		    NULL);
  window_fit (panel);
  window_fit (parameter_frame);
  xv_set (parameter_frame, XV_KEY_DATA, PARA_NAME, filename, NULL);
  xv_set (parameter_frame, XV_SHOW, TRUE, NULL);
} /* file_get_parameter */


extern void
file_command (Menu menu, Menu_item menu_item)
{
  Applic_T   *app;
  Selected_T *sel;
  File_T     *file;
  Command_T  *command;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  sel = (Selected_T *) app->select;
  while (sel != NULL) {
    file = (File_T *) xv_get (sel->object, XV_KEY_DATA, FILE_FILE);
    if ((command = props_command_get ((char *) 
      xv_get (menu_item, MENU_STRING))) != NULL)
      if (command->prmwin == 0) {
	/* prompting for parameter */
	file_get_parameter (app, command, file->name);
      } /* if */
      else
	execute_command (app, command, file->name, "");
    sel = sel->next;
  } /* while */
} /* file_command */


extern void
file_open (Xv_Window paint_window, Event *event, 
	   Canvas_shell canvas, Rectobj object)
{
  Applic_T     *app;
  Properties_T *props;
  File_T       *file;

  file = (File_T *) xv_get (object, XV_KEY_DATA, FILE_FILE);
  app = (Applic_T *) xv_get (object, XV_KEY_DATA, FILE_APP);
  props = get_properties ();
  open (app, props, file);
} /* open_file */


extern void
file_duplicate (Menu menu, Menu_item menu_item)
{
  Applic_T *app;
  File_T   *file;
  Rectobj  object;
  pid_t    pid;
  char     *from;
  char     *name;
  int      copy = 1;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  if (app->select != NULL) {
    file = (File_T *) xv_get (app->select->object, XV_KEY_DATA, FILE_FILE);
    name = malloc (strlen (app->path) + strlen (file->name) + 5);
    do {
      sprintf (name, "%s/%s.%d", app->path, file->name, copy);
      copy++;
    } while (access (name, F_OK) == 0);
    from = malloc (strlen (app->path) + strlen (file->name) + 2);
    sprintf (from, "%s/%s", app->path, file->name);
    if ((pid = fork ()) < 0)
      error_message (app->frame);
    else 
      if (pid == 0) {
#ifdef __FreeBSD__    /* FreeBSD doesn't support '-d' */
      if (execlp (DEFAULT_CP_CMD, DEFAULT_CP_CMD, "-p", from, name, (char *) 0) < 0)
#else
      if (execlp (DEFAULT_CP_CMD, DEFAULT_CP_CMD, "-dp", from, name, (char *) 0) < 0)
#endif
	  error_message_name (app->frame, file->name);
      } /* if */
      else
	notify_set_wait3_func (client, file_wait_update, pid);
    free (from);
    free (name);
  } /* if */
} /* file_duplicate */


extern void
file_create (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;
  struct stat  buf;
  pid_t        pid;
  char         name [PATH_MAX];
  int          count = 0;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  props = get_properties ();
  sprintf (name, "%s/%s", app->path, props->newdoc);
  while (stat (name, &buf) == 0) {
    count++;
    sprintf (name, "%s/%s.%d", app->path, props->newdoc, count);
  } /* while */
  if ((pid = fork ()) < 0)
    error_message (app->frame);
  else
    if (pid == 0) {
      if (execlp ("touch", "touch", name, (char *) 0) < 0)
	error_message_name (app->frame, name);
    } /* if */
    else
      notify_set_wait3_func (client, file_wait_update, pid);
} /* file_create */


extern void
file_delete (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;
  Selected_T   *select;
  File_T       *file;
  char         *path, *full_name;

  int f;

  props = get_properties ();
  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  select = app->select;
  while (select != NULL) {
    file = (File_T *) xv_get (select->object, XV_KEY_DATA, FILE_FILE);
    full_name = malloc (strlen (app->path) + strlen(file->name) + 2);
    sprintf (full_name, "%s/%s", app->path, file->name);
    if (props->delete == TO_WASTE) {
      path = malloc (strlen (HOME) + 14);
      sprintf (path, "%s/.wastebasket", HOME);
      file_move (app->frame, full_name, path, NULL);
    } /* if */
    else
      file_delete2 (app->frame, full_name);
    select = select->next;
  } /* while */
} /* file_delete */


extern void
file_change_name (Drawtext text)
{
  Applic_T *app;
  File_T   *file;
  char     *new_name;
  char     *from, *to;

  /* callback for editing directly the text of an icon */
  app = (Applic_T *) xv_get (text, XV_KEY_DATA, TEXT_APP);
  file = (File_T *) xv_get (text, XV_KEY_DATA, TEXT_FILE);
  new_name = (char *) xv_get (text, DRAWTEXT_STRING);
  if (strcmp (file->name, new_name) != 0) {
    from = malloc (strlen (app->path) + strlen (file->name) + 2);
    sprintf (from, "%s/%s", app->path, file->name);
    to = malloc (strlen (app->path) + strlen (new_name) + 2);
    sprintf (to, "%s/%s", app->path, new_name);
    file_rename (app->frame, from, to);
    free (from);
    free (to);
  } /* if */
  update_window ();
} /* file_change_name */


static int
file_exists (Frame frame, char *full_to)
{
  Xv_notice notice;
  char      *message;
  int       status;

  message = strerror (errno);
  notice = (Xv_notice) xv_create (frame, NOTICE,
				  NOTICE_MESSAGE_STRINGS, 
				  full_to, "exists!",
				  NULL,
				  NOTICE_BUTTON, "Overwrite", 1,
				  NOTICE_BUTTON, "Rename", 2,
				  NOTICE_BUTTON, "Cancel", 3,
				  NOTICE_STATUS, &status,
				  XV_SHOW, TRUE,
				  NULL);
  return (status);
} /* file_exists */


static void
file_move_rename_ok (Panel_item item)
{
  Frame frame;
  Panel_item text;
  char  *source;
  char  *target;
  char  *as;

  frame = (Frame) xv_get (item, XV_KEY_DATA, RENAME_FRAME);
  source = (char *) xv_get (item, XV_KEY_DATA, RENAME_SOURCE);
  target = (char *) xv_get (item, XV_KEY_DATA, RENAME_TARGET);
  text = (Panel_item) xv_get (item, XV_KEY_DATA, RENAME_AS);
  as = (char *) xv_get (text, PANEL_VALUE);
  xv_set (frame, FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_OUT, NULL);
  xv_set (frame, XV_SHOW, FALSE, NULL);
  file_move (frame, source, target, as);
} /* file_move_rename_ok */


static void
file_move_rename (Frame f, char *source, char *target, char *to)
{
  Frame      frame;
  Panel      panel;
  Panel_item as;
  Panel_item button;
  Rect       rect;
  Rect       rect2;

  frame = (Frame) xv_create (f, FRAME_CMD,
			     FRAME_LABEL, "Rename File",
				  FRAME_SHOW_RESIZE_CORNER, FALSE,
				  FRAME_SHOW_FOOTER, FALSE,
				  FRAME_MIN_SIZE, 530, 300,
				  XV_HEIGHT, 445,
				  XV_WIDTH, 550,
				  NULL);
  panel = (Panel) xv_get (frame, FRAME_CMD_PANEL);
  xv_set (panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  xv_create (panel, PANEL_MESSAGE,
	     PANEL_LABEL_STRING, "Copy",
	     PANEL_LABEL_BOLD, TRUE,
	     XV_X, 5,
	     NULL);
  xv_create (panel, PANEL_MESSAGE,
	     PANEL_LABEL_STRING, source,
	     XV_X, 25,
	     NULL);
  xv_create (panel, PANEL_MESSAGE,
	     PANEL_LABEL_STRING, "To",
	     PANEL_LABEL_BOLD, TRUE,
	     XV_X, 5,
	     NULL);
  xv_create (panel, PANEL_MESSAGE,
	     PANEL_LABEL_STRING, target,
	     XV_X, 25,
	     NULL);
  as = xv_create (panel, PANEL_TEXT,
		  PANEL_LABEL_STRING, "As",
		  PANEL_VALUE_DISPLAY_LENGTH, 40,
		  PANEL_VALUE_STORED_LENGTH, NAME_MAX,
		  PANEL_VALUE_X, 25,
		  NULL);
  button = xv_create (panel, PANEL_BUTTON,
	     PANEL_LABEL_STRING, "MoveAs",
	     PANEL_NOTIFY_PROC, file_move_rename_ok,
	     XV_X, 140,
	     XV_Y, 128,
	     NULL);
  window_fit (panel);
  window_fit (frame);
  xv_set (button, 
	  XV_KEY_DATA, RENAME_AS, as,
	  XV_KEY_DATA, RENAME_FRAME, frame,
	  XV_KEY_DATA, RENAME_TARGET, target,
	  XV_KEY_DATA, RENAME_SOURCE, source,
	  NULL);
#ifdef DIALOG_ALWAYS_RIGHT
  if (xv_get (frame, XV_SHOW) == FALSE) {
    frame_get_rect (f, &rect);
    frame_get_rect (frame, &rect2);
    rect2.r_left = rect.r_left + rect.r_width;
    rect2.r_top = rect.r_top;
    frame_set_rect (frame, &rect2);
  } /* if */
#endif
  xv_set (frame, XV_SHOW, TRUE, NULL);
  xv_set (frame, FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_IN, NULL);
} /* file_move_rename */


static Notify_value
file_move_wait (Notify_client client, int pid, union wait *status, 
		struct rusage *rusage)
{
  Frame frame;
  char  *target;
  char  *source;

  if (WIFEXITED(*status)) {
    frame = (Frame) client;
    target = (char *) xv_get (frame, XV_KEY_DATA, MOVE_TARGET_BAK);
    source = (char *) xv_get (frame, XV_KEY_DATA, MOVE_SOURCE_BAK);
    /* delete source only if it is succesfully copied */
    if (access (target, F_OK) == 0) {
      /* ... then remove them from the source */
      file_delete2 (frame, source);
      free (target);
    } /* if */
    xv_set (frame, XV_KEY_DATA_REMOVE, MOVE_TARGET_BAK, NULL);
    xv_set (frame, XV_KEY_DATA_REMOVE, MOVE_SOURCE_BAK, NULL);
    MOVE_TARGET_BAK = MOVE_TARGET_BAK + 2;
    MOVE_SOURCE_BAK = MOVE_SOURCE_BAK + 2;
    return NOTIFY_DONE;
  } /* if */
  else {
    update_window ();
    return NOTIFY_IGNORED;
  } /* else */
} /* file_move_wait */


extern void
file_move (Frame frame, char *source, char *target, char *as)
{
  pid_t pid;
  int   allowed = TRUE;
  char  *to;
  char  *full_to;

  /* copy file onto itself ? */
  if (strncmp (target, source, strlen (target)) == 0)
    /* target is a prefix of source */
    if (strchr (&source [strlen (target) +1], '/') == NULL)
      allowed = FALSE;
  if (allowed == TRUE) {
    if (as != NULL)
      to = as;
    else {
      /* file name of source (without path) */
      to = source + strlen (source);
      while (*to != '/')
	to--;
      to++;
    } /* else */
    full_to = malloc (strlen (target) + strlen (to) + 2);
    sprintf (full_to, "%s/%s", target, to);
    /* file exists in target directory ? */
    if (access (full_to, F_OK) == 0)
      /* file with same name exists in target directory */
      switch (file_exists (frame, full_to)) {
      case OVERWRITE:
	break;
      case RENAME:
	file_move_rename (frame, source, target, to);
	allowed = FALSE;
	break;
      case CANCEL:
	allowed = FALSE;
	break;
      } /* switch */
    if (allowed == TRUE) {
      xv_set (frame, 
	      XV_KEY_DATA, MOVE_SOURCE, source,
	      XV_KEY_DATA, MOVE_TARGET, full_to,
	      NULL);
      MOVE_TARGET = MOVE_TARGET + 2;
      MOVE_SOURCE = MOVE_SOURCE + 2;
      if ((pid = fork ()) < 0)
	error_message (frame);
      else {
	if (pid == 0) {
	  /*  first copy the files to the destination ...*/
#ifdef __FreeBSD__    /* FreeBSD doesn't support '-d' */
        if (execlp (DEFAULT_CP_CMD, DEFAULT_CP_CMD, "-pf", "-R", source, full_to, (char *) 0) < 0)
#else
        if (execlp (DEFAULT_CP_CMD, DEFAULT_CP_CMD, "-dpf", "-R", source, full_to, (char *) 0) < 0)
#endif
	    error_message (frame);
	} /* if */
	else
	  notify_set_wait3_func (frame, file_move_wait, pid);
      } /* else */
    } /* if */
  } /* if */
} /* file_move */


extern void
file_file (Frame frame, char *name, char *output)
{
  char   line [200];
  char   *text;
  int    fd [2];
  pid_t  pid;

  if (pipe (fd) < 0)
    error_message (frame);
  if ((pid = fork ()) < 0)
    error_message (frame);
  else
    if (pid > 0) {  /* parent */
      close (fd [1]);
      if (fd [0] != STDIN_FILENO) {
	if (dup2 (fd [0], STDIN_FILENO) != STDIN_FILENO)
	  error_message (frame);
	close (fd [0]);
      } /* if */
      while (gets (line) != NULL) {
	text = strchr (line, ' ');
	/* throw away leading spaces */
	while (*text == ' ')
	  text++;
      } /* while */
      close (fd [0]);
      notify_set_wait3_func (client, file_wait, pid);
    } /* if */
    else {          /* child */
      close (fd [0]);
      if (fd [1] != STDOUT_FILENO) {
	if (dup2 (fd [1], STDOUT_FILENO) != STDOUT_FILENO)
	  error_message (frame);
	close (fd [1]);
      } /* if */
      if (execlp ("file", "file", name, (char *) 0) < 0)
	error_message_name (frame, name);
    } /* else */
  strcpy (output, text);
} /* file_file */


extern int
file_frame_drop (Panel_item item, int value, Event *event)
{
  Applic_T     *app;
  Selection_requestor sel;
  Xv_drop_site ds;
  int          length;
  int          format;
  char         *source;
  char         *f1;
  char         *f2;

  app = (Applic_T *) xv_get (item, XV_KEY_DATA, PANEL_APP);
  sel = xv_get (item, PANEL_DROP_SEL_REQ);
  switch (event_action (event)) {
  case ACTION_DRAG_COPY:
    ds = dnd_decode_drop (sel, event);
    if (ds != XV_ERROR) {
      xv_set (sel, SEL_TYPE, XA_STRING, NULL);
      source = (char *) xv_get (sel, SEL_DATA, &length, &format);
      f1 = source;
      f2 = strchr (source, ' ');
      while (f2 != NULL) {
	*f2 = '\0';
	file_move (app->frame, f1, app->path, NULL);
	f2++;
	f1 = f2;
	f2 = strchr (f2, ' ');
      } /* while */
      if (!strcmp (f1, " "))
	file_move (app->frame, f1, app->path, NULL);
    } /* if */
    dnd_done (sel);
    break;
  } /* switch */
  return XV_OK;
} /* file_frame_drop */


extern void
file_default_drop (Xv_window window, Event *event, Canvas_shell canvas,
		   Rectobj object)
{
  Applic_T     *app;
  Selection_requestor sel;
  Xv_drop_site ds;
  int          length;
  int          format;
  char         *source;
  char         *f1;
  char         *f2;

  app = (Applic_T *) xv_get (canvas, XV_KEY_DATA, CANVAS_APP);
  sel = xv_create (canvas, SELECTION_REQUESTOR, NULL);
  switch (event_action (event)) {
  case ACTION_DRAG_COPY:
    ds = dnd_decode_drop (sel, event);
    if (ds != XV_ERROR) {
      xv_set (sel, SEL_TYPE, XA_STRING, NULL);
      source = (char *) xv_get (sel, SEL_DATA, &length, &format);
      f1 = source;
      f2 = strchr (source, ' ');
      while (f2 != NULL) {
	*f2 = '\0';
	file_move (app->frame, f1, app->path, NULL);
	f2++;
	f1 = f2;
	f2 = strchr (f2, ' ');
      } /* while */
      if (!strcmp (f1, " "))
	file_move (app->frame, f1, app->path, NULL);
    } /* if */
    dnd_done (sel);
    break;
  } /* switch */
} /* file_default_drop */


extern void
file_drop (Xv_window window, Event *event, Canvas_shell canvas, 
	   Rectobj object)
{
  Applic_T     *app;
  Selection_requestor sel;
  Xv_drop_site ds;
  int          length;
  int          format;
  char         *target;
  char         *source;
  char         *f1;
  char         *f2;
  char         message [PATH_MAX + 8] = "Move to ";

  app = (Applic_T *) xv_get (canvas, XV_KEY_DATA, CANVAS_APP);
  sel = xv_create (canvas, SELECTION_REQUESTOR, NULL);
  target = (char *) xv_get (object, XV_KEY_DATA, PATH);
  switch (event_action (event)) {
  case ACTION_DRAG_PREVIEW:
    if (event_id (event) == LOC_WINENTER) {
      strcat (message, target);
      xv_set (app->frame, FRAME_LEFT_FOOTER, target, NULL);
    } /* if */
    if (event_id (event) == LOC_WINEXIT) {
      strcat (message, target);
      xv_set (app->frame, FRAME_LEFT_FOOTER, "", NULL);
    } /* if */
    break;
  case ACTION_DRAG_COPY:
    ds = dnd_decode_drop (sel, event);
    if (ds != XV_ERROR) {
      xv_set (sel, SEL_TYPE, XA_STRING, NULL);
      source = (char *) xv_get (sel, SEL_DATA, &length, &format);
      f1 = source;
      f2 = strchr (source, ' ');
      while (f2 != NULL) {
	*f2 = '\0';
	file_move (app->frame, f1, target, NULL);
	f2++;
	f1 = f2;
	f2 = strchr (f2, ' ');
      } /* while */
      if (!strcmp (f1, " "))
	file_move (app->frame, f1, target, NULL);
      free (source);
    } /* if */
    dnd_done (sel);
    break;
  case ACTION_DRAG_MOVE:
    ds = dnd_decode_drop (sel, event);
    break;
  case ACTION_DRAG_LOAD:
    break;
  } /* switch */
} /* file_drop */


static int
file_convert_selection (Selection_owner sel, Atom *type, Xv_opaque *data,
			long *length, int *format)
{
  return (sel_convert_proc (sel, type, data, length, format));
} /* file_convert_selection */


static void
file_selection_done (Selection_owner owner, Xv_opaque data, Atom target)
{
  free ((char *) data);
} /* file_selection_done */


extern Selection_item
file_selection (Applic_T *app, Dnd dnd_object, int *count)
{
  int        length = 0;
  Selected_T *s;
  char       *item;
  Selection_item sel;
  
  /* determine the length of the whole selection item */
  s = app->select;
  while (s != NULL) {
    length += strlen ((char *) xv_get (xv_get (
              s->object, DRAWICON_TEXT), DRAWTEXT_STRING)) + 
	      strlen (app->path) + 2;
    s = s->next;
  } /* while */
  length++;
  item = malloc (length);
  item [0] = '\0';
  /* fill selection item */
  s = app->select;
  while (s != NULL) {
    strcat (item, app->path);
    strcat (item, "/");
    strcat (item, (char *) xv_get (xv_get (
            s->object, DRAWICON_TEXT), DRAWTEXT_STRING));
    strcat (item, " ");
    s = s->next;
    (*count)++;
  } /* while */
  item [length - 1] = '\0';
  sel = xv_create (dnd_object, SELECTION_ITEM,
		   SEL_CONVERT_PROC, file_convert_selection,
		   SEL_DONE_PROC, file_selection_done,
		   SEL_TYPE, (Atom) XA_STRING,
		   SEL_DATA, (Xv_opaque) item,
		   NULL);
  return sel;
} /* file_selection */


/*                        *
 * file_information panel *
 *                        */

static void
file_info_set (File_T *file, char *path)
{
  char   buf [200];
  char   *full_name;
  long   free_bytes;
  int    value;
  struct tm *time;
  struct passwd *pw;
  struct group  *gp;
  Server_image icon;

  /* full name of file */
  full_name = malloc (strlen (path) + strlen (file->name) + 2);
  strcpy (full_name, path);
  strcat (full_name, "/");
  strcat (full_name, file->name);
  /* set panel_items according to file properties */
  xv_set (info_name, PANEL_VALUE, file->name, NULL);
  pw = getpwuid (file->stat.st_uid);
  xv_set (info_owner, PANEL_VALUE, pw->pw_name, NULL);
  gp = getgrgid (file->stat.st_gid);
  xv_set (info_group, PANEL_VALUE, gp->gr_name, NULL);
  icon = xv_get (file->icon, DRAWIMAGE_IMAGE1);
  xv_set (info_icon, PANEL_LABEL_IMAGE, icon, NULL);
  sprintf (buf, "%d Bytes", file->stat.st_size);
  xv_set (info_size, PANEL_LABEL_STRING, buf, NULL);
  if (S_ISDIR (file->stat.st_mode))
    xv_set (info_content, XV_SHOW, TRUE, NULL);
  else
    xv_set (info_content, XV_SHOW, FALSE, NULL);
  /* time values */
  time = localtime (&file->stat.st_mtime);
  strftime (buf, 24, "%x %X", time);
  xv_set (info_mtime, PANEL_LABEL_STRING, buf, NULL);
  time = localtime (&file->stat.st_atime);
  strftime (buf, 24, "%x %X", time);
  xv_set (info_atime, PANEL_LABEL_STRING, buf, NULL);
  /* permissions */
  value = 0;
  if (file->stat.st_mode & S_IRUSR) value = 1;
  if (file->stat.st_mode & S_IWUSR) value += 2;
  if (file->stat.st_mode & S_IXUSR) value += 4;
  xv_set (info_pown, PANEL_VALUE, value, NULL);
  value = 0;
  if (file->stat.st_mode & S_IRGRP) value = 1;
  if (file->stat.st_mode & S_IWGRP) value += 2;
  if (file->stat.st_mode & S_IXGRP) value += 4;
  xv_set (info_pgrp, PANEL_VALUE, value, NULL);
  value = 0;
  if (file->stat.st_mode & S_IROTH) value = 1;
  if (file->stat.st_mode & S_IWOTH) value += 2;
  if (file->stat.st_mode & S_IXOTH) value += 4;
  xv_set (info_poth, PANEL_VALUE, value, NULL);
  /* open method, not changeable */
  if (file->type->open != NULL)
    xv_set (info_open, PANEL_LABEL_STRING, file->type->open, NULL);
  else
    xv_set (info_open, PANEL_LABEL_STRING, "", NULL);
  /* mount information */
  xv_set (info_mpoint, PANEL_LABEL_STRING, mount_get_point (full_name), NULL);
  xv_set (info_mfrom, PANEL_LABEL_STRING, mount_get_from (full_name), NULL);
  free_bytes = mount_get_free_bytes (full_name) / 1024;
  if (free_bytes > 1024) {
    free_bytes = free_bytes / 1024;
    sprintf (buf, "%u MBytes", free_bytes);
  } /* if */
  else
    sprintf (buf, "%u KBytes", free);
  xv_set (info_free, PANEL_LABEL_STRING, buf, NULL);
  file_file (info_frame, full_name, buf);
  xv_set (info_type, PANEL_LABEL_STRING, buf, NULL);
  free (full_name);
} /* file_info_set */


static void
file_info_get (File_T *file, char *path)
{
  /* retrieve value from panel_items and set file accordigly *
   * exec chmod, chown, mv, to change the actual file        */
  File_T *newfile;
  struct passwd *pw;
  struct group  *gp;
  int    value;
  char   perm [18] = "u=";
  char   *full_name;

  full_name = malloc (strlen (file->name) + strlen (path) + 2);
  strcpy (full_name, path);
  strcat (full_name, "/");
  strcat (full_name, file->name);
  newfile = malloc (sizeof (File_T));
  newfile->name = malloc (strlen ((char *) 
				  xv_get (info_name, PANEL_VALUE)) + 
			  strlen (path) + 2);
  strcpy (newfile->name, path);
  strcat (newfile->name, "/");
  strcat (newfile->name, (char *) xv_get (info_name, PANEL_VALUE));
  if (strcmp (full_name, newfile->name) != 0)
    file_rename (info_frame, full_name, newfile->name);
  pw = getpwuid (file->stat.st_uid);
  if (strcmp (pw->pw_name, 
	      (char *) xv_get (info_owner, PANEL_VALUE)) != 0)
      file_chown (info_frame, full_name, 
		  (char *) xv_get (info_owner, PANEL_VALUE));
  gp = getgrgid (file->stat.st_gid);
  if (strcmp (gp->gr_name, 
	      (char *) xv_get (info_group, PANEL_VALUE)) != 0)
      file_chgrp (info_frame, full_name, 
		  (char *) xv_get (info_group, PANEL_VALUE));
  value = (int) xv_get (info_pown, PANEL_VALUE);
  if ((value & READ_PERM) != 0) strcat (perm, "r");
  if ((value & WRIT_PERM) != 0) strcat (perm, "w");
  if ((value & EXEC_PERM) != 0) strcat (perm, "x");
  strcat (perm, ",g=");
  value = (int) xv_get (info_pgrp, PANEL_VALUE);
  if ((value & READ_PERM) != 0) strcat (perm, "r");
  if ((value & WRIT_PERM) != 0) strcat (perm, "w");
  if ((value & EXEC_PERM) != 0) strcat (perm, "x");
  strcat (perm, ",o=");
  value = (int) xv_get (info_poth, PANEL_VALUE);
  if ((value & READ_PERM) != 0) strcat (perm, "r");
  if ((value & WRIT_PERM) != 0) strcat (perm, "w");
  if ((value & EXEC_PERM) != 0) strcat (perm, "x");
  file_chmod (info_frame, full_name, perm);
  free (newfile->name);
  free (newfile);
  free (full_name);
  update_window ();
} /* file_info_get */


static void
file_dir_contents (Panel_item item)
{
  File_T *file;
  pid_t  pid;
  char   buf [25];
  char   line [NAME_MAX];
  char   *full_name;
  char   *path;
  int    fd [2];
  int    size;

  path = (char *) xv_get (item, XV_KEY_DATA, PATH);
  file = (File_T *) xv_get (item, XV_KEY_DATA, FILE_FILE);
  full_name = malloc (strlen (path) + strlen (file->name) + 2);
  strcpy (full_name, path);
  strcat (full_name, "/");
  strcat (full_name, file->name);
  if ((int) xv_get (item, PANEL_VALUE) != 0) {
    if (pipe (fd) < 0)
      error_message (info_frame);
    if ((pid = fork ()) < 0)
      error_message (info_frame);
    else
      if (pid > 0) { /* parent */
	close (fd [1]);
	if (fd [0] != STDIN_FILENO) {
	  if (dup2 (fd [0], STDIN_FILENO) != STDIN_FILENO)
	    error_message (info_frame);
	  close (fd [0]);
	} /* if */
	gets (line);
	sscanf (line, "%d", &size);
	sprintf (buf, "%d KBytes", size);
	xv_set (info_size, PANEL_LABEL_STRING, buf, NULL);
	close (fd [0]);
	notify_set_wait3_func (client, file_wait, pid);
      } /* if */
      else {
	close (fd [0]);
	if (fd [1] != STDOUT_FILENO) {
	  if (dup2 (fd [1], STDOUT_FILENO) != STDOUT_FILENO)
	    error_message (info_frame);
	  close (fd [1]);
	} /* if */
	if (execlp ("du", "du", "-s", "-k", full_name, (char *) 0) < 0)
	  error_message_name (info_frame, file->name);
      } /* else */
  } /* if */
  else {
    sprintf (buf, "%d Bytes", file->stat.st_size);
    xv_set (info_size, PANEL_LABEL_STRING, buf, NULL);
  } /* else */
  free (full_name);
} /* file_dir_contents */


static void
file_info_apply (Panel_item item)
{
  File_T *file;
  char   *path;

  file = (File_T *) xv_get (item, XV_KEY_DATA, FILE_FILE);
  path = (char *) xv_get (item, XV_KEY_DATA, PATH);
  file_info_get (file, path);
} /* file_info_apply */


static void
file_info_reset (Panel_item item)
{
  char   *path;
  File_T *file;

  file = (File_T *) xv_get (item, XV_KEY_DATA, FILE_FILE);
  path = (char *) xv_get (item, XV_KEY_DATA, PATH);
  file_info_set (file, path);
} /* file_info_reset */


static void
file_information_init (Applic_T *app)
{
  info_frame = (Frame) xv_create (app->frame, FRAME_CMD,
				  FRAME_LABEL, "File Properties",
				  FRAME_SHOW_RESIZE_CORNER, FALSE,
				  FRAME_SHOW_FOOTER, FALSE,
				  NULL);
  info_panel = (Panel) xv_get (info_frame, FRAME_CMD_PANEL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  info_name = xv_create (info_panel, PANEL_TEXT,
			 PANEL_LABEL_STRING, "Name:",
			 PANEL_VALUE_DISPLAY_LENGTH, 25,
			 PANEL_VALUE_STORED_LENGTH, NAME_MAX,
			 PANEL_VALUE_X, 113,
			 NULL);
  info_owner = xv_create (info_panel, PANEL_TEXT,
			  PANEL_LABEL_STRING, "Owner:",
			  PANEL_VALUE_DISPLAY_LENGTH, 25,
			  PANEL_VALUE_STORED_LENGTH, 50,
			  PANEL_VALUE_X, 113,
			  NULL);
  info_group = xv_create (info_panel, PANEL_TEXT,
			  PANEL_LABEL_STRING, "Group:",
			  PANEL_VALUE_DISPLAY_LENGTH, 25,
			  PANEL_VALUE_STORED_LENGTH, 50,
			  PANEL_VALUE_X, 113,
			  NULL);
  (void) xv_create (info_panel, PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "Size:",
		    PANEL_LABEL_BOLD, TRUE,
		    PANEL_VALUE_X, 113,
		    NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  info_size = xv_create (info_panel, PANEL_MESSAGE,
			 NULL);
  info_content = xv_create (info_panel, PANEL_TOGGLE,
			    PANEL_CHOICE_STRINGS, "Contents", NULL,
			    PANEL_NOTIFY_PROC, file_dir_contents,
			    XV_Y, 77,
			    XV_X, 240,
			    NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  (void) xv_create (info_panel, PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "Last Modified:",
		    PANEL_LABEL_BOLD, TRUE,
		    PANEL_VALUE_X, 113,
		    NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  info_mtime = xv_create (info_panel, PANEL_MESSAGE,
			  NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  (void) xv_create (info_panel, PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "Last Accessed:",
		    PANEL_LABEL_BOLD, TRUE,
		    PANEL_VALUE_X, 113,
		    NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  info_atime = xv_create (info_panel, PANEL_MESSAGE,
			  NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  (void) xv_create (info_panel, PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "Type:",
		    PANEL_LABEL_BOLD, TRUE,
		    PANEL_VALUE_X, 113,
		    XV_Y, 162,
		    NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  info_type = xv_create (info_panel, PANEL_MESSAGE,
			 NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  (void) xv_create (info_panel, PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "Permissions  Read   Write   Execute",
		    PANEL_LABEL_BOLD, TRUE,
		    XV_X, 15,
		    NULL);
  info_pown = xv_create (info_panel, PANEL_CHECK_BOX,
			 PANEL_LABEL_STRING, "Owner:",
			 PANEL_VALUE_X, 113,
			 PANEL_LAYOUT, PANEL_HORIZONTAL,
			 PANEL_CHOICE_STRINGS, "  ", "  ", "  ", NULL,
			 NULL);
  info_pgrp = xv_create (info_panel, PANEL_CHECK_BOX,
			 PANEL_LABEL_STRING, "Group:",
			 PANEL_VALUE_X, 113,
			 PANEL_LAYOUT, PANEL_HORIZONTAL,
			 PANEL_CHOICE_STRINGS, "  ", "  ", "  ", NULL,
			 NULL);
  info_poth = xv_create (info_panel, PANEL_CHECK_BOX,
			 PANEL_LABEL_STRING, "World:",
			 PANEL_VALUE_X, 113,
			 PANEL_LAYOUT, PANEL_HORIZONTAL,
			 PANEL_CHOICE_STRINGS, "  ", "  ", "  ", NULL,
			 NULL);
  (void) xv_create (info_panel, PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "Open Method:",
		    PANEL_LABEL_BOLD, TRUE,
		    PANEL_VALUE_X, 113,
		    NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  info_open = xv_create (info_panel, PANEL_MESSAGE,
			 XV_X, 116,
			 NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  (void) xv_create (info_panel, PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "Mount Point:",
		    PANEL_LABEL_BOLD, TRUE,
		    PANEL_VALUE_X, 113,
		    NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  info_mpoint = xv_create (info_panel, PANEL_MESSAGE,
			   XV_X, 116,
			   NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  (void) xv_create (info_panel, PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "Mounted From:",
		    PANEL_LABEL_BOLD, TRUE,
		    PANEL_VALUE_X, 113,
		    NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  info_mfrom = xv_create (info_panel, PANEL_MESSAGE,
			 XV_X, 116,
			 NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  (void) xv_create (info_panel, PANEL_MESSAGE,
		    PANEL_LABEL_STRING, "Free Space:",
		    PANEL_LABEL_BOLD, TRUE,
		    PANEL_VALUE_X, 113,
		    NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  info_free = xv_create (info_panel, PANEL_MESSAGE,
			 XV_X, 116,
			 NULL);
  info_icon = xv_create (info_panel, PANEL_MESSAGE,
			 XV_X, 12,
			 XV_Y, 30,
			 NULL);

  xv_set (info_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
  info_apply = xv_create (info_panel, PANEL_BUTTON,
			  PANEL_LABEL_STRING, "Apply",
			  PANEL_NOTIFY_PROC, file_info_apply,
			  XV_X, 94,
			  NULL);
  xv_set (info_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
  info_reset = xv_create (info_panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Reset",
		    PANEL_NOTIFY_PROC, file_info_reset,
		    NULL);
  
  window_fit (info_panel);
  window_fit (info_frame);
} /* file_information_init */


extern void
file_information (Frame frame, File_T *file, char *path)
{
  Rect       rect;
  Rect       rect2;

#ifdef DIALOG_ALWAYS_RIGHT
  if (xv_get (info_frame, XV_SHOW) == FALSE) {
    frame_get_rect (frame, &rect);
    frame_get_rect (info_frame, &rect2);
    rect2.r_left = rect.r_left + rect.r_width;
    rect2.r_top = rect.r_top;
    frame_set_rect (info_frame, &rect2);
  } /* if */
#endif
  xv_set (info_apply, XV_KEY_DATA, FILE_FILE, file, NULL);
  xv_set (info_apply, 
	  XV_KEY_DATA, PATH, path,
	  XV_KEY_DATA_REMOVE_PROC, PATH, free_data,
	  NULL);
  xv_set (info_reset, XV_KEY_DATA, FILE_FILE, file, NULL);
  xv_set (info_reset, 
	  XV_KEY_DATA, PATH, path,
	  XV_KEY_DATA_REMOVE_PROC, PATH, free_data,
	  NULL);
  xv_set (info_content, XV_KEY_DATA, FILE_FILE, file, NULL);
  xv_set (info_content, 
	  XV_KEY_DATA, PATH, path,
	  XV_KEY_DATA_REMOVE_PROC, PATH, free_data,
	  NULL);
  file_info_set (file, path);
  xv_set (info_frame, XV_SHOW, TRUE, NULL);
} /* file_information */


extern void
file_info_menu (Menu menu, Menu_item menu_item)
{
  Applic_T *app;
  File_T   *file;
  Rectobj  object;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  if (app->select != NULL) {
    object = app->select->object;
    file = (File_T *) xv_get (object, XV_KEY_DATA, FILE_FILE);
    file_information (app->frame, file, app->path);
  } /* if */
} /* file_info_menu */


extern void
file_init (Applic_T *app)
{
  default_sel = xv_create (app->file_canvas, SELECTION_REQUESTOR, NULL);
  file_information_init (app);
} /* file_init */
