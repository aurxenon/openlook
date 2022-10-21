/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: workwindow.c,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:30:56 $
 *	Purpose : display canvas, read directory
 *
 * $Log: workwindow.c,v $
 * Revision 1.1.1.1  2005/07/13 18:30:56  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.33  2000/02/18 07:13:34  root
 * More fixes for xvfilemgr. New code for removeable devices.
 *
 * Revision 1.32  1999/01/02 13:00:08  root
 * This makes the frame go busy at an earlier stage.
 *
 * Revision 1.31  1998/10/18 01:30:06  root
 * FreeBSD mods by Mark Ovens.
 *
 * Revision 1.30  1998/10/18 01:20:40  root
 * new_directory mods by Mark Ovens.
 *
 * Revision 1.29  1997/11/23 16:46:23  root
 * Ugly hack for the workwindow's label because it now assumes that the
 * length of "File Manager V" + XVFILEMGR_VERSION + " : " is inferior to
 * 32 chars.
 * That's probably true but then we have extra chars in the 'label'.
 *
 * Revision 1.28  1997/05/27 18:37:49  root
 * Open file/directory with default dbl click method when dragging it
 * on the root window.
 *
 * Revision 1.27  1997/03/19 20:09:25  root
 * set open method for standard file type.
 *
 * Revision 1.26  1997/03/14 20:28:49  root
 * Fix of the path panel bug.  Thanks to James B. Hiller.
 *
 * Revision 1.25  1997/01/18 16:44:24  root
 * avoid highliting of not selected folders in path panel.
 *
 * Revision 1.24  1996/11/23 13:55:45  root
 * generic code for default icons (make_filetype, make_icon).
 *
 * Revision 1.23  1996/11/22 19:33:56  root
 * try to avoid tricky raise condition.
 *
 * Revision 1.22  1996/10/26 12:42:43  root
 * use different cursors for drag and drop operations with a single and
 * with multiple files.
 *
 * Revision 1.21  1996/10/13 17:53:35  root
 * default filetype for unspecified file.
 *
 * Revision 1.20  1996/10/08 16:29:46  root
 * eliminating all waitpid syste calls.
 *
 * Revision 1.19  1996/08/16 19:18:01  root
 * correct goto menu handling:
 *   number of paths in properties corresponds to number of visited paths
 *   insert new visited path at top of visited paths
 *   delete last path if number of paths exceeds the maximum
 *
 * Revision 1.18  1996/08/16 17:25:58  root
 * improvement of goto menu handling
 *
 * Revision 1.17  1996/07/30 17:39:16  root
 * speed up of file type testing with file(1) using only one call to file(1).
 *
 * Revision 1.15  1996/07/28 08:48:36  root
 * new filetype format.
 * extended icon handling.
 *
 * Revision 1.14  1996/07/22 19:03:50  root
 * bugfix.
 *
 * Revision 1.13  1996/07/18 19:47:25  root
 * Direct changing of icon text.
 *
 * Revision 1.12  1996/06/27 18:57:59  root
 * Print error message when pixmap does not exist.
 *
 * Revision 1.11  1996/06/23 17:07:12  root
 * New error handling.
 *
 * Revision 1.10  1996/06/22 11:35:07  root
 * Display free MB or KB.
 *
 * Revision 1.9  1996/06/22 11:18:01  root
 * Display available bytes.
 *
 * Revision 1.8  1996/06/22 10:24:54  root
 * No extra folder icon in path canvas when changing directory from
 * root folder to any other folder.
 *
 * Revision 1.7  1996/06/17 19:38:32  root
 * add a comment.
 *
 * Revision 1.6  1996/06/02 10:36:42  root
 * change directory to / possible.
 *
 * Revision 1.5  1996/05/27 20:20:16  root
 * support for link, special files and pipes.
 *
 * Revision 1.4  1996/05/27 18:36:16  root
 * display size of all files in directory.
 *
 * Revision 1.3  1996/05/26 17:43:46  root
 * support for global constant HOME.
 *
 * Revision 1.2  1996/05/26 11:53:03  root
 * bug fix concerning opening of folders from goto menu.
 *
 * Revision 1.1  1995/12/01 15:32:53  root
 * Initial revision
 *
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <fnmatch.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include <stdlib.h>
  
#ifdef __FreeBSD__
#include <sys/socket.h>
#endif

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/svrimage.h>
#include <xview/cursor.h>
#include <xview/scrollbar.h>
#include <xview/openmenu.h>
#include <xview/rect.h>
#include <xview/notify.h>
#include <xview/dragdrop.h>
#include <xview/sel_pkg.h>
#include <sspkg/canshell.h>
#include <sspkg/drawobj.h>
#include <sspkg/rectobj.h>

#include "config.h"
#include "global.h"
#include "workwindow.h"
#include "waste.h"
#include "file.h"
#include "directory.h"
#include "properties.h"
#include "error.h"

#define MENU_DIR  17
#define MAX_LINE  300   /* maximum characters on one output line, unsafe */
#define MAX_FILES 1024  /* maximum number of files in a directory, unsafe */

/* These are the icons to be used if we can allocate the needed colors. */
#include "images16/execute16.xpm"
#include "images16/folder16.xpm"
#include "images16/regular16.xpm"
#include "images16/link16.xpm"
#include "images16/linkdir16.xpm"
#include "images16/special16.xpm"
#include "images16/fifo16.xpm"
#include "images16/unknown16.xpm"
#include "images32/execute.xpm"
#include "images32/regular.xpm"
#include "images32/link.xpm"
#include "images32/linkdir.xpm"
#include "images32/special.xpm"
#include "images32/fifo.xpm"
#include "images32/unknown.xpm"
#include "images32/folder.xpm"
#include "images32/folderopen.xpm"
#include "images32/waste.xpm"
#include "images32/cdrom.xpm"
#include "images32/floppy.xpm"

/* These are the icons to be used if we cannot allocate the needed colors. */
#include "icons/folder16_glyph.icon"
#include "icons/folder16_mask.icon"
#include "icons/regular16_glyph.icon"
#include "icons/regular16_mask.icon"
#include "icons/linkdir16_glyph.icon"
#include "icons/linkdir16_mask.icon"
#include "icons/folder32_glyph.icon"
#include "icons/folder32_mask.icon"
#include "icons/folderopen32_glyph.icon"
#include "icons/folderopen32_mask.icon"
#include "icons/regular32_glyph.icon"
#include "icons/regular32_mask.icon"
#include "icons/linkdir32_glyph.icon"
#include "icons/linkdir32_mask.icon"
#include "icons/waste_glyph.icon"
#include "icons/waste_mask.icon"
#include "icons/cdrom_glyph.icon"
#include "icons/cdrom_mask.icon"
#include "icons/floppy_glyph.icon"
#include "icons/floppy_mask.icon"

/* Icon used as drag and drop cursors */
#include "icons/dragsingleicon"
#include "icons/dragmultiicon"

static Xv_Cursor      drag_single_cursor;
static Xv_Cursor      drag_multi_cursor;
static Server_image   folder_open32_glyph;
static Server_image   waste32_glyph;
static Server_image   cdrom32_glyph;
static Server_image   floppy32_glyph;
static char           current_path [PATH_MAX];
static char           *files[MAX_FILES];    
                         /* array of files which are tested through file(1) */
static File_T         *filetypes[MAX_FILES]; 
                                      /* same as above containing filetypes */
static Notify_client client = (Notify_client) 12;
static int            separate_window = FALSE;

extern void
menu_directory (Menu menu, Menu_item menu_item);

extern void
down_directory (Xv_Window paint_window, Event *event, 
		Canvas_shell canvas, Rectobj object);

static void
open_special_file (Xv_Window paint_window, Event *event, 
		   Canvas_shell canvas, Rectobj object);

static void
open_fifo (Xv_Window paint_window, Event *event, 
	   Canvas_shell canvas, Rectobj object);

extern void
free_data (Xv_object object, int key, caddr_t data)
{
  free (data);
} /* free_data */


static Notify_value
workwindow_wait (Notify_client client, int pid, union wait *status, 
		 struct rusage *rusage)
{
  if (WIFEXITED(*status)) {
    return NOTIFY_DONE;
  } /* if */
  return NOTIFY_IGNORED;
} /* workwindow_wait */


static void
paint_path (Applic_T *app, Properties_T *props)
{
  char     *path;
  char     *full_path = NULL;
  char     *name;
  char     *next1;
  char     *next2;
  int      x, y, i;
  Drawicon ic;

/*  rectobj_destroy_children ((Rectobj) app->path_canvas); */
  i = 0;
  while (app->pathicon[i] != (Rectobj) NULL) {
    xv_destroy_safe(app->pathicon[i]);
    app->pathicon[i++] = (Rectobj) NULL;
  } /* while */
  i = 0;
  path = malloc (strlen (app->path) + 1);
  path [0] = '\0';
  x = props->gap_x;
  y = props->gap_y;
  if (strlen (app->path) > 1) {
    switch (app->type) {
    case FOLDER_WINDOW:
      strcpy (path, "/");
      name = malloc (strlen (path) + 1);
      strcpy (name, path);
      ic = (Drawicon) xv_create (app->path_canvas, DRAWICON,
				 DRAWTEXT_STRING, path,
				 DRAWIMAGE_IMAGE1, directory->large_icon,
				 NULL);
      full_path = malloc (strlen (app->path) + 1);
      strcpy (full_path, app->path);
      /* path without leading / */
      next1 = &full_path[1];
      break;
    case WASTE_WINDOW:
      strcpy (path, "/");
      name = malloc (strlen (wastedir) + 1);  
      strcpy (name, wastedir);
      ic = (Drawicon) xv_create (app->path_canvas, DRAWICON,
				 DRAWTEXT_STRING, "Waste",
				 DRAWIMAGE_IMAGE1, waste32_glyph,
				 NULL);
      full_path = malloc (strlen (app->path) + 1);
      strcpy (full_path, app->path);
      /* path beginning with /.wastebasket */
      next1 = strstr (&full_path [1], "/.wastebasket");
      /* to end of wasket path */
      next1 = next1 + 13;
      if (strlen (next1) > 0)
	next1++;
      break;
    case CDROM_WINDOW:
      strcpy (path, "/");
      name = malloc (strlen (CDROM_MOUNTPOINT) + 1);
      strcpy (name, CDROM_MOUNTPOINT);
      ic = (Drawicon) xv_create (app->path_canvas, DRAWICON,
				 DRAWTEXT_STRING, "CD-Rom",
				 DRAWIMAGE_IMAGE1, cdrom32_glyph,
				 NULL);
      full_path = malloc (strlen (app->path) + 1);
      strcpy (full_path, app->path);
      /* path beginning with CDROM_MOUNTPOINT */
      next1 = strstr (&full_path [0], CDROM_MOUNTPOINT);
      /* to end of CDROM_MOUNTPOINT path */
      next1 = next1 + strlen (CDROM_MOUNTPOINT);
      if (strlen (next1) > 0)
	next1++;
      break;
    case FLOPPY_WINDOW:
      strcpy (path, "/");
      name = malloc (strlen (FLOPPY_MOUNTPOINT) + 1);
      strcpy (name, FLOPPY_MOUNTPOINT);
      ic = (Drawicon) xv_create (app->path_canvas, DRAWICON,
				 DRAWTEXT_STRING, "Floppy",
				 DRAWIMAGE_IMAGE1, floppy32_glyph,
				 NULL);
      full_path = malloc (strlen (app->path) + 1);
      strcpy (full_path, app->path);
      /* path beginning with FLOPPY_MOUNTPOINT */
      next1 = strstr (&full_path [0], FLOPPY_MOUNTPOINT);
      /* to end of FLOPPY_MOUNTPOINT path */
      next1 = next1 + strlen (FLOPPY_MOUNTPOINT);
      if (strlen (next1) > 0)
	next1++;
      break;
    } /* switch */
    app->pathicon[i++] = ic;
    xv_set (ic,DRAWICON_LAYOUT_VERTICAL, TRUE,
	    RECTOBJ_DBL_CLICK_PROC, new_directory,
	    RECTOBJ_DRAGGABLE, TRUE,
	    RECTOBJ_START_DRAG_PROC, start_drag,
	    RECTOBJ_ACCEPTS_DROP, TRUE,
	    RECTOBJ_DROP_PROC, file_drop,
	    RECTOBJ_SELECTED, FALSE,
	    XV_KEY_DATA, PATH, name,
	    XV_KEY_DATA_REMOVE_PROC, PATH, free_data,
	    XV_X, x,
	    XV_Y, y,
	    NULL);
    rectobj_set_paint_style(ic, NULL, RECTOBJ_NORMAL);
    x += (int) xv_get (ic, XV_WIDTH) + props->gap_x;
    next2 = next1;
    while ((next1 = strpbrk (next2, "/")) != 0) {
      *next1 = 0;   /* to get the right portion of the path name */
      strcat (path, next2);
      name = malloc (strlen (path) + 1);
      strcpy (name, path);
      ic = (Drawicon) xv_create (app->path_canvas, DRAWICON,
				 DRAWTEXT_STRING, next2,
				 DRAWIMAGE_IMAGE1, directory->large_icon,
				 DRAWICON_LAYOUT_VERTICAL, TRUE,
				 RECTOBJ_DBL_CLICK_PROC, new_directory,
				 RECTOBJ_DRAGGABLE, TRUE,
				 RECTOBJ_START_DRAG_PROC, start_drag,
				 RECTOBJ_ACCEPTS_DROP, TRUE,
				 RECTOBJ_DROP_PROC, file_drop,
				 XV_KEY_DATA, PATH, name,
				 XV_KEY_DATA_REMOVE_PROC, PATH, free_data,
				 XV_X, x,
				 XV_Y, y,
				 NULL);
      app->pathicon[i++] = ic;
      strcat (path, "/");
      xv_set (ic, RECTOBJ_SELECTED, FALSE, NULL);
      rectobj_set_paint_style(ic, NULL, RECTOBJ_NORMAL);
      x += (int) xv_get (ic, XV_WIDTH) + props->gap_x;
      *next1 = '/'; /* restore the original path name */
      next2 = ++next1;
    } /* while */
  } /* if */
  else 
    next2 = app->path;
  if (strlen (next2) > 0) {
    strcat (path, next2);
    name = malloc (strlen (path) + 1);
    strcpy (name, path);
    ic = (Drawicon) xv_create (app->path_canvas, DRAWICON,
			       DRAWTEXT_STRING, next2,
			       DRAWIMAGE_IMAGE1, folder_open32_glyph,
			       DRAWICON_LAYOUT_VERTICAL, TRUE,
			       RECTOBJ_DBL_CLICK_PROC, new_directory,
			       RECTOBJ_DRAGGABLE, TRUE,
			       RECTOBJ_START_DRAG_PROC, start_drag,
			       RECTOBJ_ACCEPTS_DROP, TRUE,
			       RECTOBJ_DROP_PROC, file_drop,
			       XV_KEY_DATA, PATH, name,
			       XV_KEY_DATA_REMOVE_PROC, PATH, free_data,
			       XV_X, x,
			       XV_Y, y,
			       NULL);
    app->pathicon[i++] = ic;
    xv_set (ic, RECTOBJ_SELECTED, FALSE, NULL);
    rectobj_set_paint_style(ic, NULL, RECTOBJ_NORMAL);
  } /* if */
  /* update the label of the application icon */
  if (app->type == FOLDER_WINDOW) {
    xv_set (app->icon, XV_LABEL, next2, NULL);
  }
  free (full_path);
} /* paint_path */


static int
compare_file_name (File_PTR fst, File_PTR snd)
{
  return strcmp (fst->name, snd->name);
} /* compare_file_name */


static int
compare_file_type (File_PTR fst, File_PTR snd)
{
  int c;

   if ((c = fst->type->rank - snd->type->rank) == 0)
    return compare_file_name (fst, snd);
  else
    return c;
} /* compare_file_type */


static int
compare_file_size (File_PTR fst, File_PTR snd)
{
  return (fst->stat.st_size - snd->stat.st_size);
} /* compare_file_size */


static int
compare_file_date (File_PTR fst, File_PTR snd)
{
  return (fst->stat.st_mtime - snd->stat.st_mtime);
} /* compare_file_date */


static File_PTR
insert_file_repr (File_PTR first, File_PTR insert, 
		  int (* method) (File_PTR, File_PTR))
{
  File_PTR file1, file2;

  if (first != NULL) {
    file1 = first; file2 = NULL;
    while ((file1 != NULL) && (method (insert, file1) > 0)) {
      file2 = file1;
      file1 = file1->next;
    } /* while */
    if (file2 == NULL) {
      /* insert on 1. position in the list */
      insert->next = first;
      first = insert;
    } /* if */
    else {
      /* insert in the middle or at the end of the list */
      insert->next = file2->next;
      file2->next = insert;
    } /* else */
  } /* if */
  else {
    first = insert;
    first->next = NULL;
  } /* else */
  return first;
} /* insert_file_repr */


extern void
start_drag (Xv_window window, Event *event, Canvas_shell canvas,
	    Rectobj object, int x, int y, int adjust)
{
  Dnd            dnd_object;
  Selection_item sel;
  Applic_T       *app;
  int            count = 0; /* number of selected files */
  void (* open) (Xv_Window paint_window, Event *event, 
		 Canvas_shell canvas, Rectobj object);
  Rectobj        obj;
  Selected_T     *select;

  /* create the dnd object */
  dnd_object = xv_create (canvas_paint_window(canvas), DRAGDROP,
			  DND_TYPE, DND_COPY,
			  NULL);
  app = (Applic_T *) xv_get (canvas, XV_KEY_DATA, CANVAS_APP);
  sel = file_selection (app, dnd_object, &count);
  if (count > 1) 
    xv_set (dnd_object,
	    DND_CURSOR, drag_multi_cursor,
	    DND_ACCEPT_CURSOR, drag_multi_cursor,
	    NULL);
  else
    xv_set (dnd_object,
	    DND_CURSOR, drag_single_cursor,
	    DND_ACCEPT_CURSOR, drag_single_cursor,
	    NULL);
  switch (dnd_send_drop (dnd_object)) {
  case XV_OK:
    break;
  case DND_ABORTED:
    break;
  case DND_TIMEOUT:
    break;
  case DND_ROOT:
    /* drop an root window, call open method of object */
    select = app->select;
    while (select != NULL) {
      obj = select->object;
      separate_window = TRUE; /* open every directory in separate window */
      open = (void (*) (Xv_Window, Event *, Canvas_shell, Rectobj)) 
	xv_get(obj, RECTOBJ_DBL_CLICK_PROC);
      open (window, event, canvas, obj);
      select = select->next;
    } /* while */
    break;
  case DND_ILLEGAL_TARGET:
    break;
  case DND_SELECTION:
    break;
  case XV_ERROR:
    break;
  } /* switch */
} /* start_drag */


static Drawicon
create_icon (Canvas_shell canvas, File_PTR file,
	     void (* open) (Xv_Window paint_window, Event *event, 
		       Canvas_shell canvas, Rectobj object),
	     Applic_T *app, Properties_T *props) 
{
  Drawicon drawicon;
  Drawtext drawtext;
  char     *path;

  if ((props->current_folder.icon == ICON_SMALL) ||
      (props->current_folder.icon == LIST)) {
    drawicon = (Drawicon) xv_create (canvas, DRAWICON,
			     DRAWIMAGE_IMAGE1, file->type->small_icon,
			     DRAWICON_LAYOUT_VERTICAL, FALSE,
			     RECTOBJ_DRAGGABLE, TRUE,
			     RECTOBJ_START_DRAG_PROC, start_drag,
			     RECTOBJ_DBL_CLICK_PROC, open,
			     RECTOBJ_SELECTION_PROC, file_select,
			     XV_KEY_DATA, FILE_APP, app,
			     XV_KEY_DATA, FILE_FILE, file,
			     NULL);
  } /* if */
  if (props->current_folder.icon == ICON_LARGE) {
    drawicon = (Drawicon) xv_create (canvas, DRAWICON,
			     DRAWIMAGE_IMAGE1, file->type->large_icon,
			     DRAWICON_LAYOUT_VERTICAL, TRUE,
			     RECTOBJ_DRAGGABLE, TRUE,
			     RECTOBJ_START_DRAG_PROC, start_drag,
			     RECTOBJ_DBL_CLICK_PROC, open,
			     RECTOBJ_SELECTION_PROC, file_select,
			     XV_KEY_DATA, FILE_APP, app,
			     XV_KEY_DATA, FILE_FILE, file,
			     NULL);
  } /* if */
  /* retrieve the drawtext of the icon to set XV_KEY_DATA explizit.
     The callback for changeing the name is now able to get the
     necessary data. */
  drawtext = (Drawtext) xv_get (drawicon, DRAWICON_TEXT);
  xv_set (drawtext, 
	  DRAWTEXT_STRING, file->name,
	  DRAWTEXT_EDITABLE, TRUE,
	  DRAWTEXT_NOTIFY_PROC, file_change_name,
	  XV_KEY_DATA, TEXT_APP, app,
	  XV_KEY_DATA, TEXT_FILE, file, 
	  NULL);
  if (file->type == directory) {
    path = malloc (strlen (app->path) + strlen (file->name) + 2);
    sprintf (path, "%s/%s", app->path, file->name);
    /* allow drops on folder icons */
    xv_set (drawicon,
	    RECTOBJ_ACCEPTS_DROP, TRUE,
	    RECTOBJ_DROP_PROC, file_drop,
	    XV_KEY_DATA, PATH, path,
	    XV_KEY_DATA_REMOVE_PROC, PATH, free_data,
	    NULL);
    /* insert directory into folder view */
    dir_new (path, &props->new_folder);
  } /* if */
  return drawicon;
} /* create_icon */


static Drawtext
paint_dirtext (Canvas_shell canvas, File_PTR file, Properties_T *props)
{
  char entry [65] = "";
  char buf [25];
  struct tm *ltime;
  struct passwd *pw;
  struct group  *gp;
  
  if (props->current_folder.show & SHOW_PERM) {
    strcpy (entry, "---------");
    if (file->stat.st_mode & S_IRUSR) entry [0] = 'r';
    if (file->stat.st_mode & S_IWUSR) entry [1] = 'w';
    if (file->stat.st_mode & S_IXUSR) entry [2] = 'x';
    if (file->stat.st_mode & S_ISUID) entry [2] = 's';

    if (file->stat.st_mode & S_IRGRP) entry [3] = 'r';
    if (file->stat.st_mode & S_IWGRP) entry [4] = 'w';
    if (file->stat.st_mode & S_IXGRP) entry [5] = 'x';
    if (file->stat.st_mode & S_ISGID) entry [5] = 's';

    if (file->stat.st_mode & S_IROTH) entry [6] = 'r';
    if (file->stat.st_mode & S_IWOTH) entry [7] = 'w';
    if (file->stat.st_mode & S_IXOTH) entry [8] = 'x';
  } /* if */
  if (props->current_folder.show & SHOW_LINK) {
    sprintf (buf, "%3d", file->stat.st_nlink);
    strcat (entry, buf);
  } /* if */
  if (props->current_folder.show & SHOW_OWNER) {
    if ((pw = getpwuid (file->stat.st_uid)) != NULL)
      sprintf (buf, " %9s", pw->pw_name);
    else
      sprintf (buf, " %9d", file->stat.st_uid);
    strcat (entry, buf);
  } /* if */
  if (props->current_folder.show & SHOW_GROUP) {
    if ((gp = getgrgid (file->stat.st_gid)) != NULL)
      sprintf (buf, " %9s", gp->gr_name);
    else
      sprintf (buf, " %9d", file->stat.st_gid);
    strcat (entry, buf);
  } /* if */
  if (props->current_folder.show & SHOW_SIZE) {
    sprintf (buf, "%9d", file->stat.st_size);
    strcat (entry, buf);
  } /* if */
  if (props->current_folder.show & SHOW_DATE) {
    ltime = localtime (&file->stat.st_mtime);
    strftime (buf, 24, " %x %X", ltime);
    strcat (entry, buf);
  } /* if */
  return (Drawtext) xv_create (canvas, DRAWTEXT,
			       DRAWTEXT_STRING, entry,
			       DRAWTEXT_EDITABLE, FALSE,
			       DRAWTEXT_FONT, list_font,
			       RECTOBJ_SELECTABLE, FALSE,
			       NULL);
} /* paint_dirtext */


static void
files_command (Applic_T *app, Properties_T *props, Canvas_shell canvas,
	       int (* method) (File_PTR, File_PTR))
{
  Filetype_T *first;
#ifdef __FreeBSD__
  FILE   *in;
#endif
  char   line [MAX_LINE];
  char   *text;
  int    fd [2];
  int    index = 1;
  int    match;
  pid_t  pid;

  if (pipe (fd) < 0)
    error_message (app->frame);
  if ((pid = fork ()) < 0)
    error_message (app->frame);
  else
    if (pid > 0) {  /* parent */
      close (fd [1]);

#ifdef __FreeBSD__
            /* There appears to be a difference in semantics  */
            /* between FreeBSD & Linux which prevents the     */
            /* read end of the pipe being set to STDIN_FILENO */
            /* using dup2(), so we use fdopen() instead       */
      if ((in = fdopen(fd[0], "r")) == NULL) {
        close (fd [0]);
        error_message (app->frame);
      } /* if */

      /* read file names from pipe */
                      
      line[sizeof line - 1] = '\0';
      while (fgets(line, sizeof line - 1, in)) {
#else
      if (fd [0] != STDIN_FILENO) {
	if (dup2 (fd [0], STDIN_FILENO) != STDIN_FILENO)
	  error_message (app->frame);
	close (fd [0]);
      } /* if */
      /* read file names from pipe */
      while (gets (line) != NULL) {
#endif
	text = strchr (line, ' ');
	/* throw away all leading spaces */
	while (*text == ' ')
	  text++;
	/* compare with magic text from filetype */
	first = regular;
	match = 0;
	while (first != NULL) {
	  if (first->pattern[0] == '-')
	    /* only use the types with magic patterns */
	    if (strcmp (first->file, text) == 0) {
	      /* pattern matches */
	      filetypes[index]->type = first;
	      filetypes[index]->icon = create_icon (canvas, filetypes[index], 
						    file_open, app, props);
	      match = 1;
	      break;
	    } /* if */
	  first = first->next;
	} /* while */
	if (match == 0) {
	  /* no pattern matches, use default type */
	  filetypes[index]->type = allfile;
	  filetypes[index]->icon = create_icon (canvas, filetypes[index], 
						file_open, app, props);
	} /* if */
	/* insert new file in the list of all files */
	app->files = insert_file_repr (app->files, filetypes[index], method);
	index++;
      } /* while */
#ifdef __FreeBSD__
      fclose(in);
#endif
      close (fd [0]);
      notify_set_wait3_func (client, workwindow_wait, pid);
    } /* if */
    else {          /* child */
      close (fd [0]);
      if (fd [1] != STDOUT_FILENO) {
	if (dup2 (fd [1], STDOUT_FILENO) != STDOUT_FILENO)
	  error_message (app->frame);
	close (fd [1]);
      } /* if */
      if (execvp ("file", files) < 0)
	error_message (app->frame);
    } /* else */
} /* files_command */


static int
set_type (Applic_T *app, Properties_T *props, Canvas_shell canvas, 
	  File_T *file, int index)
{
  struct stat statbuf;
  int         match = 0;
  char        name [PATH_MAX];      /* full name of file */
  Filetype_T  *first;

  /* Determine the type of a file.  Maybe some day this could be done
     with the help of the magic file. */
  switch (file->stat.st_mode & S_IFMT) {
  case S_IFLNK: {
    sprintf (name, "%s/%s", app->path, file->name);
    if (stat (name, &statbuf) < 0)
      error_message_name (app->frame, name);
    if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {
      /* link to a file */
      file->type = linkfile;
      file->icon = create_icon (canvas, file, file_open, 
				app, props);
    } /* if */
    else {
      /* link to a directory */
      file->type = linkdir;
      file->icon = create_icon (canvas, file, down_directory, 
				app, props);
    } /* else */
    break;
  } /* case */
  case S_IFDIR:
    file->type = directory;
    file->icon = create_icon (canvas, file, down_directory, 
			      app, props);
    break;
  case S_IFBLK:
  case S_IFCHR:
    file->type = special;
    file->icon = create_icon (canvas, file, open_special_file, 
			      app, props);
    break;
  case S_IFIFO:
    file->type = fifo;
    file->icon = create_icon (canvas, file, open_fifo, 
			      app, props);
    break;
  case S_IFREG:
    first = regular;
    while (first != NULL) {
      /* Type determined through file matching pattern */
      if (fnmatch (first->pattern, file->name, FNM_PERIOD) == 0) {
	file->type = first;
	file->icon = create_icon (canvas, file, file_open, app, props);
	match = 1;
	break;
      } /* if */
      first = first->next;
    } /* while */
    if (match == 0)
      if ((file->stat.st_mode & S_IXUSR) ||
	  (file->stat.st_mode & S_IXGRP) ||
	  (file->stat.st_mode & S_IXOTH)) {
	file->type = execute;
	file->icon = create_icon (canvas, file, file_open, app, props);
      } /* if */
      else {
        /* Type determined through output from file(1) ? 
	   collect all files and examine them later */
	files[index] = malloc (strlen (app->path) + strlen (file->name) + 2);
	filetypes[index] = file;
	sprintf (files[index], "%s/%s", app->path, file->name);
	index++;
      } /* else */
    break;
  default:
    file->type = unknown;
    file->icon = create_icon (canvas, file, down_directory, app, props);
    break;
  } /* switch */
  return index;
} /* set_type */


static void
paint_icons (File_PTR file, Applic_T *app, Properties_T *props, int *x, int *y,
	     Canvas_shell canvas, int canvas_width, int canvas_height)
{
  int           width;

  if (props->current_folder.icon == ICON_LARGE) {
    width = (int) xv_get (file->icon, XV_WIDTH);
    xv_set (file->icon,
	    XV_X, *x + (props->pixel_per_file_x - width) / 2,
	    XV_Y, *y,
	    NULL);
  } /* if */
  else {
    xv_set (file->icon,
	    XV_X, *x,
	    XV_Y, *y,
	    NULL);
  } /* */
  if (props->current_folder.icon == LIST) {
    /* only one file per line in the list mode */
    file->text = paint_dirtext (canvas, file, props);
    xv_set (file->text,
	    XV_X, *x + props->pixel_per_file_x,
	    XV_Y, *y,
	    NULL);
    *x = props->gap_x;
    *y += props->pixel_per_file_y;
  } /* if */
  else {
    /* horizontal or vertical layout with small or large icons */
    if (props->current_folder.lout == LAYOUT_EW) {
      *x += props->pixel_per_file_x;
      if ((*x + props->pixel_per_file_x) > canvas_width) {
	*x = props->gap_x;
	*y += props->pixel_per_file_y;
      } /* if */
    } /* if */
    else {
      *y += props->pixel_per_file_y;
      if ((*y + props->pixel_per_file_y) > canvas_height) {
	*y = props->gap_y;
	*x += props->pixel_per_file_x;
      } /* if */
    } /* else */
  } /* else */
} /* paint_icons */


static void
paint_files (Applic_T *app, Properties_T *props)
{
  struct stat   statbuf;
  struct dirent *dirp;
  DIR           *dp;
  int           x = props->gap_x;
  int           y = props->gap_y;
  int           paint_width, paint_height;
  int           canvas_width, canvas_height;
  int           file_c = 0;          /* number of files in current directory */
  int           files_index = 1;     /* number of files testet with file(1) */
  int           old_index = 1;
  long          free_bytes;
  long          total_bytes;
  long int      size = 0;            /* size of all files in directory */
  char          path [PATH_MAX];
  char          msg [31];
  Panel_item    file_msg;
  File_PTR      file, file2;
  int           (* method) (File_PTR, File_PTR);

  xv_set (app->file_canvas, CANVAS_SHELL_DELAY_REPAINT, TRUE, NULL);
  switch (props->current_folder.sort) {
  case SORT_NAME :
    method = compare_file_name;
    break;
  case SORT_TYPE :
    method = compare_file_type;
    break;
  case SORT_SIZE :
    method = compare_file_size;
    break;
  case SORT_DATE :
    method = compare_file_date;
    break;
  } /* switch */
  /* delete file representation list and destroy icons safe */
  file2 = app->files;
  while (file2 != NULL) {
    file = file2;
    file2 = file->next;
    free (file->name);
    xv_set (file->icon, XV_SHOW, FALSE, NULL);
    xv_destroy_safe (file->icon);
    if (file->text != XV_NULL) {
      xv_set (file->text, XV_SHOW, FALSE, NULL);
      xv_destroy_safe (file->text);
    } /* if */
    free (file);
  } /* while */
  app->files = NULL;
  /* flush display buffer */
  xv_set (app->file_canvas, CANVAS_SHELL_DELAY_REPAINT, FALSE, NULL);
  xv_set (app->file_canvas, CANVAS_SHELL_DELAY_REPAINT, TRUE, NULL);
  /* open current directory and ... */
  if ((dp = opendir (app->path)) == NULL)
    error_message_name (app->frame, app->path);
  /* ... collect file information */
  while ((dirp = readdir (dp)) != NULL) {
    if (strcmp (dirp->d_name, ".") == 0 ||
	strcmp (dirp->d_name, "..") == 0)
      continue;
    sprintf (path, "%s/%s", app->path, dirp->d_name);
    /* follow symbolic link? */
    if (props->link == FOLLOW_LNK)
      if (stat (path, &statbuf) < 0)
	error_message_name (app->frame, path);
      else ;
    else
      if (lstat (path, &statbuf) < 0)
	error_message_name (app->frame, path);
    /* hide file ? */
    if ((props->current_folder.hide == 0) && (dirp->d_name[0] == '.'))
      continue;
    /* match view filter ? */
    if ((props->filter != NULL) && (strlen (props->filter) > 0))
      if (fnmatch (props->filter, dirp->d_name, FNM_PERIOD) != 0)
	continue;
    file_c++;
    size = size + statbuf.st_size;
    file = malloc (sizeof (File_T));
    file->name = (char *) malloc (strlen (dirp->d_name) + 1);
    strcpy (file->name, dirp->d_name);
    file->stat = statbuf;
    files_index = set_type (app, props, app->file_canvas, file, files_index);
    file->text = XV_NULL;
    file->next = NULL;
    if (old_index == files_index)
      app->files = insert_file_repr (app->files, file, method);
    else
      old_index = files_index;
  } /* while */
  if (files_index > 1) {
    files[files_index] = NULL;
    files_command (app, props, app->file_canvas, method);
  } /* if */
  closedir (dp);
  /* display files in canvas */
  paint_width = (int) xv_get (app->file_canvas, XV_WIDTH);
  if (props->current_folder.icon == LIST)
    paint_height = (file_c * props->pixel_per_file_y) + props->gap_y;
  else
    paint_height = ((file_c / ((paint_width - props->gap_x) / 
			       props->pixel_per_file_x)) + 1) *
				 props->pixel_per_file_y + props->gap_y;
  xv_set (app->file_canvas, CANVAS_HEIGHT, paint_height, NULL);
  file = app->files;
  while (file != NULL) {
    paint_icons (file, app, props, &x, &y, app->file_canvas, 
		 paint_width, paint_height);
    file = file->next;
  } /* while */
  xv_set (app->file_canvas, CANVAS_SHELL_DELAY_REPAINT, FALSE, NULL);
  /* move scrollbar to top */
  xv_set (app->file_scroll_v, SCROLLBAR_VIEW_START, 0, NULL);
  xv_set (app->file_scroll_h, SCROLLBAR_VIEW_START, 0, NULL);
  /* get dimension of file window */
  canvas_width = (int) xv_get (app->file_canvas, XV_WIDTH);
  canvas_height = (int) xv_get (app->file_canvas, XV_HEIGHT);
  /* set scrollbar dimensions according to file window dimension */
  xv_set (app->file_scroll_v, 
	  SCROLLBAR_PIXELS_PER_UNIT, props->pixel_per_file_y,
	  SCROLLBAR_OBJECT_LENGTH, (paint_height / props->pixel_per_file_y+ 1),
	  SCROLLBAR_VIEW_LENGTH, (canvas_height / props->pixel_per_file_y), 
	  SCROLLBAR_PAGE_LENGTH, (canvas_height / props->pixel_per_file_y), 
	  NULL);
  if (props->current_folder.icon == LIST)
    xv_set (app->file_scroll_h, 
	    SCROLLBAR_PIXELS_PER_UNIT, props->pixel_per_file_x,
	    SCROLLBAR_OBJECT_LENGTH, (paint_width),
	    SCROLLBAR_VIEW_LENGTH, (canvas_width), 
	    SCROLLBAR_PAGE_LENGTH, (canvas_width), 
	    NULL);
  else
    xv_set (app->file_scroll_h, 
	    SCROLLBAR_PIXELS_PER_UNIT, props->pixel_per_file_x,
	    SCROLLBAR_OBJECT_LENGTH, (paint_width / props->pixel_per_file_x+1),
	    SCROLLBAR_VIEW_LENGTH, (canvas_width / props->pixel_per_file_x), 
	    SCROLLBAR_PAGE_LENGTH, (canvas_width / props->pixel_per_file_x), 
	    NULL);
  free_bytes = mount_get_free_bytes (app->path) / 1024;
  total_bytes = mount_get_total_bytes (app->path) / 1024;
  if (free_bytes > 1024) {
    free_bytes = free_bytes / 1024;
    sprintf (msg, 
    "Directory contains %d files,   %u KBytes,   %u MBytes available (%u %%)", 
	     file_c, size / 1024, free_bytes,
	     (free_bytes * 102400) / total_bytes);
  } /* if */
  else
    sprintf (msg, 
    "Directory contains %d files,   %u KBytes,   %u KBytes available (%u %%)", 
	     file_c, size / 1024, free_bytes,
	     (free_bytes * 100) / total_bytes);
  waste_icon (app, file_c);
  file_msg = (Panel_item) xv_get (app->file_canvas, XV_KEY_DATA, FILE_MSG);
  xv_set (file_msg, PANEL_LABEL_STRING, msg, NULL);
} /* paint_files */


static int
directory_open (Applic_T *app, char *new_path)
{
  Applic_T     *app2;

  /* is this directory already open? */
  app2 = first_app;
  while (app2 != NULL) {
    if (strcmp (app2->path, new_path) == 0) {
      /* open frame if it is closed */
      if (xv_get (app2->frame, FRAME_CLOSED) == TRUE)
	xv_set (app2->frame, FRAME_CLOSED, FALSE, NULL);
      else
	xv_set (app->frame, FRAME_LEFT_FOOTER, "Folder already open!", NULL);
      return TRUE;
    } /* if */
    app2 = (Applic_T *) app2->next;
  } /* while */
  xv_set (app->frame, FRAME_LEFT_FOOTER, "", NULL);
  return FALSE;
} /* directory_open */


static Applic_T *
new_folder (Applic_T *app_old)
{
  Applic_T     *app;
  Properties_T *props;

  props = get_properties ();
  if ((props->open == NEW_WIN) || (separate_window == TRUE)) {
    /* open new window for this folder */
    app = xvfmgr_new_frame (FOLDER_WINDOW);
    app->next = first_app->next;
    (Applic_T *) first_app->next = app;
    return app;
  } /* if */
  return app_old;
} /* new_folder */


extern void 
change_directory (Applic_T *app, char *path)
{
  Menu_item    new_item, menu_item;
  Panel_item   path_item;
  int          equal = 0;
  int          c;    /* number of reserved menu items */
  int          n_menu;
  char         *menu_string;
  char         label [PATH_MAX + 32] = "File Manager V";
  char         error [PATH_MAX + 26] = "Can't change directory to ";
  Properties_T *props;
  FolderProp_T *fprops;
  struct stat  statbuf;

  strcat(label,XVFILEMGR_VERSION);
  strcat(label," : ");
  props = get_properties ();
  xv_set (app->frame, FRAME_BUSY, TRUE, NULL);
  /* save current folders props */
  dir_set (app->path, &props->current_folder);
  /* change directory to new path if it exists */
  if (stat (path, &statbuf) < 0) {
    strcat (error, path);
    xv_set (app->frame, FRAME_LEFT_FOOTER, error, NULL);
  } /* if */
  else {
    strcpy (app->path, path);
    /* update the label of the application frame */
    strcat (label, app->path);
    xv_set (app->frame, FRAME_LABEL, label, NULL);
    /* fill goto menu */
    n_menu = (int) xv_get (goto_menu, MENU_NITEMS);
    c = props_menu_no_label() + 1;  /* empty entry after labels */
    /* is the file in the goto menu? */
    while (c <= n_menu) {
      menu_item = (Menu_item) xv_get (goto_menu, MENU_NTH_ITEM, c);
      if (strcmp (app->path, (char *) xv_get (menu_item, MENU_STRING)) == 0)
	break;
      c++;
    } /* while */
    if (c > n_menu) {
      /* insert new item in the goto menu at the top of visited directories */
      c = props_menu_no_label() + 1;  /* empty entry after labels */
      dir_new (app->path, &props->new_folder);
      menu_string = malloc (strlen (app->path) + 1);
      strcpy (menu_string, app->path);
      new_item = (Menu_item) xv_create (XV_NULL, MENUITEM,
					MENU_STRING, menu_string,
					MENU_NOTIFY_PROC, menu_directory,
					MENU_RELEASE,
					NULL);
      xv_set (goto_menu, MENU_INSERT, c, new_item, NULL);
      set_folder_props (&props->new_folder);
      /* remove one item if there are too many */
      if (n_menu == (props->nfolder + c)) {
	/*      dir_delete ((char *) xv_get ((Menu_item) 
		xv_get (goto_menu, MENU_NTH_ITEM), MENU_STRING));*/
	xv_set (goto_menu, MENU_REMOVE, (props->nfolder + c + 1), NULL);
      } /* if */
    } /* if */
    else {
      /* we already have visited this directory */
      set_folder_props (dir_get (app->path));
      dir_set (app->path, &props->current_folder);
    } /* else */
    /* update file and path window */
    paint_path (app, props);
    paint_files (app, props);
    app->last_update = statbuf.st_mtime;
    /* if (app->type != WASTE_WINDOW) {
      path_item = (Panel_item) xv_get (app->frame, XV_KEY_DATA, GOTO_FILE);
      xv_set(path_item,
	      PANEL_VALUE, app->path,
	      NULL);
    } */ /* if */
  } /* else */
  xv_set (app->frame, FRAME_BUSY, FALSE, NULL);
} /* change_directory */


extern void
menu_directory (Menu menu, Menu_item menu_item)
{
  Applic_T *app;
  char     *label;
  char     *path;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  if (app->changed_dir != 0)
    app->changed_dir = 0;
  else {
    label = (char *) xv_get (menu_item, MENU_STRING);
    path = props_menu_get (label);
    if (directory_open (app, path) == FALSE) {
      app = new_folder (app);
      change_directory (app, path);
    } /* if */
  } /* else */
} /* menu_directory */


extern void
down_directory (Xv_Window paint_window, Event *event, 
	       Canvas_shell canvas, Rectobj object)
{
  Applic_T *app;
  char     *path;

  /* change directory to some in the current directory */
  app = (Applic_T *) xv_get (canvas, XV_KEY_DATA, CANVAS_APP);
  path = (char *) xv_get (object, DRAWTEXT_STRING);
  path = (char *) malloc (strlen (app->path) + strlen (path) + 2);
  strcpy (path, app->path);
  if (strlen (path) > 1)
    /* Current directory is not the root so there must be a slash between
       these to path components. */
    strcat (path, "/");
  strcat (path, (char *) xv_get (object, DRAWTEXT_STRING));
  if (directory_open (app, path) == FALSE) {
    app = new_folder (app);
    change_directory (app, path);
  } /* if */
  free (path);
} /* down_directory */


static void
open_special_file (Xv_Window paint_window, Event *event, 
		   Canvas_shell canvas, Rectobj object)
{
  Applic_T *app;

  app = (Applic_T *) xv_get (canvas, XV_KEY_DATA, CANVAS_APP);
  error_message (app->frame);
} /* open_special_file */


static void
open_fifo (Xv_Window paint_window, Event *event, 
	      Canvas_shell canvas, Rectobj object)
{
  Applic_T *app;

  app = (Applic_T *) xv_get (canvas, XV_KEY_DATA, CANVAS_APP);
  error_message (app->frame);
} /* open_fifo */


extern void
new_directory (Xv_window paint_window, Event *event, 
	       Canvas_shell canvas, Rectobj object)
{
  Applic_T *app;
  char *path, *temp;

  /* goto some directory in the path to the current */
  app = (Applic_T *) xv_get (canvas, XV_KEY_DATA, CANVAS_APP);
  temp = (char *) xv_get (object, XV_KEY_DATA, PATH);
  if (app->type == CDROM_WINDOW && strcmp(temp, CDROM_MOUNTPOINT) != 0)
{
    path = (char *) malloc(strlen(path) + strlen(CDROM_MOUNTPOINT) + 1);
    strcpy(path, CDROM_MOUNTPOINT);
    strcat(path, temp);
  }
  else
    path = temp;
  if (directory_open (app, path) == FALSE) {
    app = new_folder (app);
    change_directory (app, path);
  } /* if */
} /* new_directory */


extern void
update_window ()
{
  Applic_T     *app;
  struct stat  statbuf;
  
  app = first_app;
  while (app != NULL) {
    /* avoid unnecessary call to update_window_notify */
    if (stat (app->path, &statbuf) < 0) ;
    app->last_update = statbuf.st_mtime;
    /* reread and repaint the canvases */
    change_directory (app, app->path);
    app = (Applic_T *) app->next;
  } /* while */
} /* update_window */


extern Notify_value
update_window_notify (Notify_client client, int which)
{
  Applic_T     *app;
  Properties_T *props;
  struct stat  statbuf;
  
  app = first_app;
  props = get_properties ();
  while (app != NULL) {
    if (xv_get (app->frame, FRAME_CLOSED) == FALSE) {
      /* update only if the frame is not iconified */
      if (stat (app->path, &statbuf) < 0) ;
      if (app->last_update != statbuf.st_mtime) { 
	/* some changes in curr. dir */
	change_directory (app, app->path);
      } /* if */
    } /* if */
    else 
      if (app->type == WASTE_WINDOW)
	/* always update the wastebasket icon */
	waste_update_closed (app);
    app = (Applic_T *) app->next;
  } /* while */
  return NOTIFY_DONE;
} /* update_window_notify */


static Notify_value
first_notify (Notify_client client, int which)
{
  Applic_T      *app;
  Properties_T  *props;
  static struct itimerval timer;

  props = get_properties ();
  app = xvfmgr_new_frame (WASTE_WINDOW);
  strcpy (app->path, wastedir);
  dir_new (app->path, &props->current_folder);
  (Applic_T *) first_app->next = app;
  change_directory (app, app->path);
  /* install the update routine */
  timer.it_value.tv_sec = 1;
  timer.it_interval.tv_sec = props->update;
  notify_set_itimer_func((Notify_client) first_app->frame, 
			 update_window_notify, 
			 ITIMER_REAL, &timer, NULL);
} /* first_notify */


extern void
ww_sort_name (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  props = get_properties ();
  props_sort_name ();
  props_large_icons ();
  paint_path (app, props);
  paint_files (app, props);
} /* ww_sort_name */


extern void
ww_sort_type (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  props = get_properties ();
  props_sort_type ();
  props_large_icons ();
  paint_path (app, props);
  paint_files (app, props);
} /* ww_sort_type */


extern void
ww_list_name (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  props = get_properties ();
  props_sort_name ();
  props_list ();
  paint_path (app, props);
  paint_files (app, props);
} /* ww_list_name */


extern void
ww_list_type (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  props = get_properties ();
  props_sort_type ();
  props_list ();
  paint_path (app, props);
  paint_files (app, props);
} /* ww_list_type */


extern void
ww_list_size (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  props = get_properties ();
  props_sort_size ();
  props_list ();
  paint_path (app, props);
  paint_files (app, props);
} /* ww_list_size */


extern void
ww_list_date (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  props = get_properties ();
  props_sort_date ();
  props_list ();
  paint_path (app, props);
  paint_files (app, props);
} /* ww_list_date */


extern void
ww_small_icons (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  props = get_properties ();
  props_small_icons ();
  paint_path (app, props);
  paint_files (app, props);
} /* ww_small_icons */


extern void
ww_large_icons (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  props = get_properties ();
  props_large_icons ();
  paint_path (app, props);
  paint_files (app, props);
} /* ww_large_icons */


static Server_image
make_icon (Display *display, int dim,
	   char *image[], unsigned char glyph[])
{
  Pixmap       pixmap;
  Pixmap       pixmask;
  Server_image icon;

  if (XpmCreatePixmapFromData (display, 
		     RootWindow (display, DefaultScreen (display)),
		     image, &pixmap, &pixmask, NULL) == 0)
    icon = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				     XV_WIDTH, dim,
				     XV_HEIGHT, dim,
				     SERVER_IMAGE_DEPTH, 4,
				     SERVER_IMAGE_PIXMAP, pixmap,
				     NULL);
  else
    /* We were not able to allocate colors correctly.
       Try to replace the color pixmap we were trying to load
       by an icon made of an X bitmap. */
    icon = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				     XV_WIDTH, dim,
				     XV_HEIGHT, dim,
				     SERVER_IMAGE_X_BITS, glyph,
				     NULL);
  return icon;
} /* make_icon */


static int
read_filetype (Display *display, char *name, int rank)
{
  FILE         *fp;
  Pixmap       pixmap;
  Pixmap       pixmask;
  Filetype_T   *new_type;
  char buf [MAX_LINE];
  char *b;
  char *pattern;
  char *magic;
  char *large_icon_name;
  char *small_icon_name;
  char *open;

  char *temp_string;
  char *large_icon_name_long;
  char *small_icon_name_long;
  int  opened_pixmaps_ok;
  
  large_icon_name_long = malloc (PATH_MAX + NAME_MAX + 1);
  small_icon_name_long = malloc (PATH_MAX + NAME_MAX + 1);
  temp_string = malloc (PATH_MAX + NAME_MAX + 1);

  if ((fp = fopen (name, "r")) != NULL){

    buf[0] = '\0';
    fgets (buf, sizeof (buf), fp);

    while ( buf[0] != '\0' ) {

      pattern = strtok (buf, ",");
      if ((pattern == NULL) || (pattern [0]== '\n') || 
	  (pattern [0] == '\0') || (pattern [0] == '#')){
 	/* empty line or comment */
        buf[0] = '\0';
        fgets (buf, sizeof (buf), fp);
	continue;
      }

      /* Clean up the pattern to make sure only alpha-numerical are */
      /* within the pattern's string. */
      sscanf(pattern,"%s",temp_string);
      strcpy(pattern,temp_string);

      magic = strtok (NULL, ",");

      large_icon_name = strtok (NULL, ",");
      /* Clean up the icon name to make sure only alpha-numerical are */
      /* within the icon's name. */
      sscanf(large_icon_name,"%s",temp_string);
      strcpy(large_icon_name,temp_string);

      small_icon_name = strtok (NULL, ",");
      /* Clean up the icon name to make sure only alpha-numerical are */
      /* within the icon's name. */
      sscanf(small_icon_name,"%s",temp_string);
      strcpy(small_icon_name,temp_string);

      open = strtok (NULL, "\0");
      /* get the open method */
      new_type = malloc (sizeof (Filetype_T));
      new_type->next = regular;
      regular = new_type;
      new_type->rank = rank++;
      new_type->pattern = malloc (strlen (pattern) + 1);
      strcpy (new_type->pattern, pattern);
      new_type->file = malloc (strlen (magic) + 1);
      strcpy (new_type->file, magic);
      new_type->open = malloc (strlen (open) + 1);
      /* erase spaces, tabs or new lines at the end of buf */
      b = &open [strlen(open) - 1]; /* last character */
      while ((*b == ' ') || (*b == '\n') || (*b == '\t')) {
	*b = '\0'; 
	b--;
      } /* while */
      /* erase spaces, tabs or new lines at the beginning of buf */
      b = open;
      while ((*b == ' ') || (*b == '\n') || (*b == '\t'))
	b++; 
      if ((strlen (b) > 0) && (strcmp (b, "-") != 0))
	strcpy (new_type->open, b);
      else
	new_type->open = NULL;
      opened_pixmaps_ok = TRUE;

      /* We are now ready to try to get the files for the icons. 
	 Let's try the Large Icons first...*/
      if (access (large_icon_name, R_OK) < 0) {
        sprintf (large_icon_name_long, "%s/%s", IMAGESDIR, large_icon_name);
          if (access (large_icon_name_long, R_OK) < 0) {
            fprintf (stderr, "XVFilemgr: can't load pixmap for %s\n"
				, pattern);
	    fprintf (stderr, "XVFilemgr: requested LARGE pixmap was: %s\n"
				,large_icon_name_long);
            opened_pixmaps_ok = FALSE;
          } /* if */
      } else {
	/* We opened the file right away, use the correct name. */
	strcpy(large_icon_name_long,large_icon_name);
      } /* else */

      /* We can now try the Small Icons...*/
      if (access (small_icon_name, R_OK) < 0) {
        sprintf (small_icon_name_long, "%s/%s", IMAGESDIR, small_icon_name);
          if (access (small_icon_name_long, R_OK) < 0) {
            fprintf (stderr, "XVFilemgr: can't load pixmap for %s\n"
				, pattern);
	    fprintf (stderr, "XVFilemgr: requested SMALL pixmap was: %s\n"
				,small_icon_name_long);
            opened_pixmaps_ok = FALSE;
          } /* if */
      } else {
	/* We opened the file right away, use the correct name. */
	strcpy(small_icon_name_long,small_icon_name);
      } /* else */
      if (opened_pixmaps_ok == FALSE) {
	new_type->large_icon = allfile->large_icon;
	new_type->small_icon = allfile->small_icon;
	continue;
      } /* if */
      /* We can now Proceed to open the correct icons for all files */
      if (XpmReadFileToPixmap (display, 
			     RootWindow (display, DefaultScreen (display)),
			     large_icon_name_long,
			     &pixmap, &pixmask, NULL) == 0) {
	new_type->large_icon = (Server_image) xv_create ((int) NULL, 
						 SERVER_IMAGE,
						 XV_WIDTH, 32,
						 XV_HEIGHT, 32,
						 SERVER_IMAGE_DEPTH, 4,
						 SERVER_IMAGE_PIXMAP, pixmap,
						 NULL);
      } /* if */
      else {
	/* We were not able to allocate colors correctly.
	   Try to replace the colors pixmap we were trying to load
	   by an icon made of an X bitmap. */
	new_type->large_icon = (Server_image) xv_create (
				 XV_NULL, SERVER_IMAGE,
				 XV_WIDTH, regular32_glyph_width,
				 XV_HEIGHT, regular32_glyph_height,
				 SERVER_IMAGE_X_BITS, regular32_glyph_bits,
				 NULL);

      } /* else */
      if (XpmReadFileToPixmap (display, 
			     RootWindow (display, DefaultScreen (display)),
			     small_icon_name_long,
			     &pixmap, &pixmask, NULL) == 0) {
	new_type->small_icon = (Server_image) xv_create ((int) NULL, 
						 SERVER_IMAGE,
						 XV_WIDTH, 16,
						 XV_HEIGHT, 16,
						 SERVER_IMAGE_DEPTH, 4,
						 SERVER_IMAGE_PIXMAP, pixmap,
						 NULL);
      } /* if */
      else {
	/* We were not able to allocate colors correctly. 
	   Try to replace the colors pixmap we were trying to load 
	   by an icon made of an X bitmap. */
	new_type->small_icon = (Server_image) xv_create (
				XV_NULL, SERVER_IMAGE,
				XV_WIDTH, regular16_glyph_width,
				XV_HEIGHT, regular16_glyph_height,
				SERVER_IMAGE_X_BITS, regular16_glyph_bits,
				NULL);

      } /* else */

      buf[0] = '\0';
      fgets (buf, sizeof (buf), fp);

    } /* while */

    fclose (fp); 
  } /* if */

  free (large_icon_name_long);
  free (small_icon_name_long);
  free (temp_string);
  return rank;
} /* read_filetype */


static Filetype_T *
make_filetype (Display *display, int *rank, 
	       char *image16[], unsigned char glyph16[],
	       char *image32[], unsigned char glyph32[])
{
  Filetype_T *filetype;

  filetype = malloc (sizeof (Filetype_T));
  filetype->rank = (*rank)++;
  filetype->pattern = NULL;
  filetype->open = NULL;
  filetype->next = NULL;
  filetype->small_icon = make_icon (display, 16, image16, glyph16);
  filetype->large_icon = make_icon (display, 32, image32, glyph32);
  return filetype;
} /* make file_type */


extern void
workwindow_init (Applic_T *app)
{
  Server_image image;
  Pixmap       pixmap;
  Pixmap       pixmask;
  Properties_T *props;
  char         *path = FILETYPE;
  int          rank = 0;
  static struct itimerval timer;

  /* We must go FRAME_BUSY early but this doesn't work earlier..??? */
  xv_set (app->frame, FRAME_BUSY, TRUE, NULL);

  /* initialize drag and drop cursors */
  image = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				    XV_WIDTH, dragsingleicon_width,
				    XV_HEIGHT, dragsingleicon_height,
				    SERVER_IMAGE_X_BITS, dragsingleicon_bits,
				    NULL);
  drag_single_cursor = (Xv_Cursor) xv_create (XV_NULL, CURSOR,
				      CURSOR_IMAGE, image,
				      CURSOR_XHOT, dragsingleicon_width / 2,
				      CURSOR_YHOT, dragsingleicon_height / 2,
				      NULL);
  image = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				    XV_WIDTH, dragmultiicon_width,
				    XV_HEIGHT, dragmultiicon_height,
				    SERVER_IMAGE_X_BITS, dragmultiicon_bits,
				    NULL);
  drag_multi_cursor = (Xv_Cursor) xv_create (XV_NULL, CURSOR,
				      CURSOR_IMAGE, image,
				      CURSOR_XHOT, dragmultiicon_width / 2,
				      CURSOR_YHOT, dragmultiicon_height / 2,
				      NULL);
  props = get_properties ();
  directory = make_filetype (app->display, &rank, 
			     folder16_xpm, folder16_glyph_bits,
			     folder_xpm, folder32_glyph_bits);
  folder_image = directory->large_icon;
  linkdir = make_filetype (app->display, &rank, 
			   linkdir16_xpm, linkdir16_glyph_bits,
			   linkdir_xpm, linkdir32_glyph_bits);
  execute = make_filetype (app->display, &rank, 
			   execute16_xpm, regular16_glyph_bits,
			   execute_xpm, regular32_glyph_bits);
  linkfile = make_filetype (app->display, &rank, 
			   link16_xpm, regular16_glyph_bits,
			   link_xpm, regular32_glyph_bits);
  special = make_filetype (app->display, &rank, 
			   special16_xpm, regular16_glyph_bits,
			   special_xpm, regular32_glyph_bits);
  fifo = make_filetype (app->display, &rank, 
			fifo16_xpm, regular16_glyph_bits,
			fifo_xpm, regular32_glyph_bits);
  allfile = make_filetype (app->display, &rank, 
			   regular16_xpm, regular16_glyph_bits,
			   regular_xpm, regular32_glyph_bits);
  allfile->rank = 9999;
  allfile->pattern = malloc(2);
  strcpy (allfile->pattern, "*");
  unknown = make_filetype (app->display, &rank, 
			   unknown16_xpm, regular16_glyph_bits,
			   unknown_xpm, regular32_glyph_bits);
  unknown->pattern = malloc(2);
  strcpy (unknown->pattern, "*");

  folder_open32_glyph = make_icon (app->display, 32, 
				   folderopen_xpm, folderopen32_glyph_bits);
  waste32_glyph = make_icon (app->display, 32, 
			     waste_xpm, waste_glyph_bits);
  cdrom32_glyph = make_icon (app->display, 32, 
			     cdrom_xpm, cdrom_glyph_bits);
  floppy32_glyph = make_icon (app->display, 32, 
			      floppy_xpm, floppy_glyph_bits);
  /* read filetypes from FILETYPE and ~/.filetype */
  rank = read_filetype (app->display, path, rank);
  path = malloc (strlen (HOME) + 10);
  sprintf (path, "%s/.filetype", HOME);
  rank = read_filetype (app->display, path, rank);
  /* for executing file(1) */
  files[0] = malloc (strlen("file") + 1);
  strcpy (files[0], "file");
  /* install the first update routine */
  timer.it_value.tv_sec = 1;
  timer.it_interval.tv_sec = 1;
  notify_set_itimer_func((Notify_client) app->frame, first_notify, 
			 ITIMER_REAL, &timer, NULL);
  /* get path and display files */
  getcwd (app->path, PATH_MAX - 1);
  dir_new (app->path, &props->current_folder);
  change_directory (app, app->path);
} /* workwindow_init */



