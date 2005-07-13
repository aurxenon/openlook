/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: properties.h,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:00 $
 *	Purpose : properties
 *
 * $Log: properties.h,v $
 * Revision 1.1  2005/07/13 18:31:00  arkenoi
 * Initial revision
 *
 * Revision 1.3  1996/08/16 19:18:26  root
 * new function: return the number of goto menu labels.
 *
 * Revision 1.2  1996/07/22 19:00:57  root
 * Handling of properties fixed.
 *
 * Revision 1.1  1995/12/01 15:32:53  root
 * Initial revision
 *
 *
 */

#ifndef __PROP__
#define __PROP__

#include <xview/xview.h>
#include <xview/panel.h>

#define SORT_NAME 0
#define SORT_TYPE 1
#define SORT_SIZE 2
#define SORT_DATE 3
#define LAYOUT_EW 0
#define LAYOUT_NS 1
#define ICON_SMALL 0
#define ICON_LARGE 1
#define LIST       2
#define SHOW_DATE  1
#define SHOW_SIZE  2
#define SHOW_OWNER 4
#define SHOW_GROUP 8
#define SHOW_LINK 16
#define SHOW_PERM 32
#define TO_WASTE   0
#define SAME_WIN   0
#define NEW_WIN    1
#define FOLLOW_LNK 0

typedef struct {
  char *name;     /* name for new folder, applies to new_folder only */
  int  lout;       /* vertcal or horizontal */
  int  sort;       /* by name, type, size or date */
  int  hide;       /* show hidden files */
  int  icon;       /* list, small or large icons */
  int  show;       /* what to show in the list */
} FolderProp_T;

typedef struct {
  FolderProp_T current_folder;
  FolderProp_T new_folder;
  char         *newdoc; /* general defaults */
  int          update;  /* advanced settings */
  int          link;    /* follow symbolic link */
  int          delete;  /* destroy a file or copy it to the wastebasket */
  int          open;    /* open new window for every folder */
  int          nfolder; /* number of chached directory names */
  char         *filter; /* filter pattern */
  int          other;   /* textedit or other editor */
  char         *edit;   /* other text editor */
  int gap_x;            /* settings for the workwidow, not customizable */
  int gap_y;
  int pixel_per_file_x;
  int pixel_per_file_y;
  int pixel_per_folder;
} Properties_T;

typedef struct Command_T * Command_PTR;

typedef struct Command_T {
  char *label;
  char *command;        /* command to execute */
  int  prmwin;          /* prompt for parameter ? */
  char *prompt;         /* which prompt to show */
  int  outwin;          /* create in extra window for output? */
  Command_PTR next;
} Command_T;  

extern Properties_T *
get_properties ();

extern void
set_folder_props (FolderProp_T *fprops);

extern void
properties_notify (Panel_item item);

extern void
props_sort_name ();

extern void
props_sort_type ();

extern void
props_sort_size ();

extern void
props_sort_date ();

extern void
props_small_icons ();

extern void
props_large_icons ();

extern int 
props_menu_no_label();

extern void
props_menu_set_menu ();

extern char *
props_menu_get (char *label);

extern void
props_command_set_menu ();

extern Command_T *
props_command_get (char *label);

extern void
props_list ();

extern void
properties_init ();

#endif /* __PROP__ */
