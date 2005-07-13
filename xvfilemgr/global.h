/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: global.h,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:13 $
 *	Purpose : global definitions
 *
 * $Log: global.h,v $
 * Revision 1.1  2005/07/13 18:31:13  arkenoi
 * Initial revision
 *
 * Revision 1.11  1998/12/20 16:21:37  root
 * GLIBC2 needs limits.h which libc5 supports, we're including it.
 *
 * Revision 1.10  1997/03/14 20:29:06  root
 * Fix of the path panle bug.  Thanks to James B. Hiller.
 *
 * Revision 1.9  1996/08/16 17:14:17  root
 * new XV_KEY_DATA defines.
 *
 * Revision 1.8  1996/07/28 08:57:22  root
 * extended Filetype_T to reflect new filetype format.
 * change of constants.
 *
 * Revision 1.7  1996/07/22 08:35:35  root
 * chnages from vincent
 *
 * Revision 1.5  1996/07/01 19:25:50  root
 * New prototype for update_window_notify so the uppdate frequency can be
 * manipulated during properties dialog.
 *
 * Revision 1.4  1996/05/26 17:35:14  root
 * change comment.
 *
 * Revision 1.3  1996/05/26 17:34:29  root
 * corrections. another "constant".
 *
 * Revision 1.2  1996/05/26 14:22:05  root
 * added "constant".
 *
 * Revision 1.1  1995/12/01 15:32:53  root
 * Initial revision
 *
 *
 */

#ifndef __GLOBAL__
#define __GLOBAL__

#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include <xview/icon.h>
#include <xview/font.h>
#include <xview/openmenu.h>
#include <xview/notify.h>
#include <sspkg/drawobj.h>
#include <sspkg/canshell.h>

#include "config.h"

/* keys for XV_KEY_DATA */
#define PATH        10     /* full path name for rectobjects */
#define GOTO_MENU   101
#define GOTO_FILE   102
#define FILE_MSG    103
#define COMMAND_APP 120    /* pointer to the Applic_T struct */
#define EDIT_APP    130    /* pointer to the Applic_T struct */
#define GOTO_APP    140    /* pointer to the Applic_T struct */
#define GOTO_PATH   143    /* path name in goto panel text */
#define EMPTY_APP   145    /* pointer to the Applic_T struct */
#define CANVAS_APP  150    /* pointer to the Applic_T struct */
#define PANEL_APP   155    /* pointer to the Applic_T struct */
#define FILE_APP    160    /* pointer to the Applic_T struct */
#define FILE_FILE   161    /* pointer to the File_T struct */
#define TEXT_APP    165    /* pointer to the Applic_T struct */
#define TEXT_FILE   166    /* pointer to the File_T struct */
#define SEL_APP     170    /* pointer to the Applic_T struct */
#define EJECT_APP   175    /* pointer to the Applic_T struct */

/* window types */
#define FOLDER_WINDOW 1
#define WASTE_WINDOW  2
#define CDROM_WINDOW  3
#define FLOPPY_WINDOW 4

#define MAX_PATH      30

/* global for all modules */
char *CLIPBOARD;           /* name of hidden directory for cut files */
char *HOME;                /* home directory, set in xvfilemgr.c */

/* global for all frames */
Xv_Font      list_font;
Menu         command_menu; /* because this is shared by all frames */
Menu         goto_menu;    /* because this is shared by all frames */
Menu         edit_menu;    /* because this is shared by all frames */

typedef struct Selected_T * Selected_PTR;

typedef struct Selected_T {
  Rectobj      object;
  Selected_PTR next;
} Selected_T;  

typedef struct Filetype_T * Filetype_PTR;

typedef struct Filetype_T {
  char         *pattern;         /* normal file matching pattern */
  char         *file;            /* text output from file command */
  int          rank;             /* for sorting by type */
  Server_image small_icon;
  Server_image large_icon;
/*  Cursor       small_dnd_cursor;
    Cursor       large_dnd_cursor;*/
  char         *open;
  Filetype_PTR next;
} Filetype_T;

typedef struct File_T *File_PTR;

typedef struct File_T {
  char        *name;
  struct stat stat;
  Drawicon    icon;
  Drawtext    text;
  Filetype_T  *type;
  File_PTR    next;
} File_T;

typedef struct Applic_T * Applic_PTR;

typedef struct Applic_T {
  Frame        frame;                  /* base frame of the application */

  Canvas_shell path_canvas;            /* where to display the path */
  Canvas_shell file_canvas;            /* where to display the files */
  Scrollbar    path_scroll;            /* scrollbars attached to the canvas */
  Scrollbar    file_scroll_h;
  Scrollbar    file_scroll_v;
  Icon         icon;                   /* icon of the base frame */
  Menu_item    file_open;              /* for activating/deactivating items */
  Menu_item    file_duplicate;
  Menu_item    file_information;
  Menu_item    edit_cut;
  Menu_item    edit_copy;
  Menu_item    edit_link;
  Menu_item    edit_paste;
  Menu_item    edit_delete;
  Display      *display;
  int          changed_dir;
  int          type;                   /* wastebasket, cdrom, normal folder */
  char         path [PATH_MAX];        /* current dir to display */
  time_t       last_update;            /* time of last access to dir */
  Selected_T   *select;                /* linked list of selected icons */
  File_T       *files;                 /* linked list of files in canvas */
  Rectobj      pathicon[MAX_PATH];     /* maximum no of components in a path */
  Applic_PTR   *next;                  /* linked list of all applications */
} Applic_T;

Applic_T *first_app;
Xv_Server XVfilemgr_Server;            /* The X server we are using */
Xv_Screen XVfilemgr_Screen;            /* The Screen on the X server */

extern void                            /* update the canvas file window */
update_window ();

extern Notify_value                    /* callback for notifier */
update_window_notify (Notify_client client, int which);

extern void
new_directory (Xv_Window paint_window, Event *event, 
	       Canvas_shell canvas, Rectobj object);

extern void
menu_directory (Menu menu, Menu_item menu_item);

extern void
file_command (Menu menu, Menu_item menu_item);

extern void
start_drag (Xv_window window, Event *event, Canvas_shell canvas,
	    Rectobj object, int x, int y, int adjust);

extern void
free_data (Xv_object object, int key, caddr_t data);

extern Applic_T *
xvfmgr_new_frame (int type);

#endif /* __GLOBAL__ */
