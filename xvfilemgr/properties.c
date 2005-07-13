/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: properties.c,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:00 $
 *	Purpose : properties
 *
 * $Log: properties.c,v $
 * Revision 1.1.1.1  2005/07/13 18:31:00  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.13  1997/11/14 04:15:02  root
 * Moved the properties Panel_item to a Menu_item of edit_menu.
 *
 * Revision 1.12  1997/05/30 06:53:28  root
 * command part of filetype file can contain spaces (Alessandro Russo).
 *
 * Revision 1.11  1997/03/06 17:53:40  root
 * Fixing some incorrect pointer handling.
 * Thanks to James B. Hiller.
 *
 * Revision 1.10  1996/10/20 15:47:30  root
 * correct saving of properties.
 *
 * Revision 1.9  1996/10/14 19:29:52  root
 * correction in custom command handling.
 *
 * Revision 1.8  1996/08/16 19:18:45  root
 * new function: return number of goto menu labels.
 *
 * Revision 1.7  1996/07/26 19:56:33  root
 * change some constants
 *
 * Revision 1.6  1996/07/22 19:00:39  root
 * Handling of properties fixed.
 *
 * Revision 1.5  1996/07/01 19:25:02  root
 * Update frequency is set during apply.
 *
 * Revision 1.4  1996/05/26 17:41:31  root
 * support for global constant HOME. Delete all occurences of gentenv ("HOME").
 *
 * Revision 1.3  1996/05/26 15:57:30  root
 * correct loading and saving of properties.
 *
 * Revision 1.2  1996/05/26 11:53:30  root
 * bug fix concerning label and directory entry of goto menu.
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>

#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>

#include "config.h"
#include "global.h"
#include "properties.h"

#define CURRENT_FOLDER   0
#define NEW_FOLDER       1
#define GOTO_DEFAULTS    2
#define CUSTOM_COMMANDS  3
#define GENERAL          4
#define ADVANCED         5
#define SHOW           230
#define LOUT           231

static Properties_T props;
static int          category;
static Frame        props_frame;
static Panel        props_panel [6];
static Panel_item   props_cf_lout;   /* current folder settings */
static Panel_item   props_cf_sort;
static Panel_item   props_cf_hide;
static Panel_item   props_cf_disp;
static Panel_item   props_cf_show;
static Panel_item   props_nf_name;   /* new folder settings */
static Panel_item   props_nf_lout;
static Panel_item   props_nf_sort;
static Panel_item   props_nf_hide;
static Panel_item   props_nf_disp;
static Panel_item   props_nf_show;
static Panel_item   props_gt_path;   /* goto menu defaults */
static Panel_item   props_gt_label;
static Panel_item   props_gt_add;
static Menu         props_gt_menu; 
static Menu         props_gt_paste; 
static Panel_item   props_gt_edit;
static Panel_item   props_gt_list;
static Panel_item   props_gt_nfol;
static Panel_item   props_gd_open;   /* general defaults */
static Panel_item   props_gd_delt;
static Panel_item   props_gd_newd;
static Panel_item   props_as_link;   /* advanced settings */ 
static Panel_item   props_as_update;
static Panel_item   props_as_filter;
static Panel_item   props_as_other;
static Panel_item   props_as_edit;
static Panel_item   props_cc_add;   /* custom commands */
static Panel_item   props_cc_command;
static Panel_item   props_cc_label;
static Menu         props_cc_menu; 
static Menu         props_cc_paste; 
static Panel_item   props_cc_edit;
static Panel_item   props_cc_list;
static Panel_item   props_cc_prmwin;
static Panel_item   props_cc_prompt;
static Panel_item   props_cc_outwin;

typedef struct Gotomenu_T * Gotomenu_PTR;

typedef struct Gotomenu_T {
  char *label;
  char *directory;
  Gotomenu_PTR next;
} Gotomenu_T;  

static Gotomenu_T *first_menu = NULL;
static Gotomenu_T *clip_menu = NULL;
static Command_T  *first_command = NULL;
static Command_T  *clip_command = NULL;

static void
get_prop ()
{
  /* current folder properties */
  props.current_folder.lout = (int) xv_get (props_cf_lout, PANEL_VALUE);
  props.current_folder.sort = (int) xv_get (props_cf_sort, PANEL_VALUE);
  props.current_folder.hide = (int) xv_get (props_cf_hide, PANEL_VALUE);
  props.current_folder.icon = (int) xv_get (props_cf_disp, PANEL_VALUE);
  props.current_folder.show = (int) xv_get (props_cf_show, PANEL_VALUE);
  /* new folder properties */
  free (props.new_folder.name);
  props.new_folder.name = malloc (strlen ((char *) 
				 xv_get (props_nf_name, PANEL_VALUE)) + 1);
  strcpy (props.new_folder.name, (char *) xv_get (props_nf_name, PANEL_VALUE));
  props.new_folder.lout = (int) xv_get (props_nf_lout, PANEL_VALUE);
  props.new_folder.sort = (int) xv_get (props_nf_sort, PANEL_VALUE);
  props.new_folder.hide = (int) xv_get (props_nf_hide, PANEL_VALUE);
  props.new_folder.icon = (int) xv_get (props_nf_disp, PANEL_VALUE);
  props.new_folder.show = (int) xv_get (props_nf_show, PANEL_VALUE);
  switch (props.current_folder.icon) {
  case ICON_LARGE:
    props_large_icons ();
    break;
  case ICON_SMALL:
    props_small_icons ();
    break;
  case LIST:
    props_list ();
    break;
  } /* switch */
  /* goto menu defaults */
  props.nfolder = (int) xv_get (props_gt_nfol, PANEL_VALUE);
  /* general defaults */
  free (props.newdoc);
  props.newdoc = malloc (strlen ((char *) 
				 xv_get (props_gd_newd, PANEL_VALUE)) + 1);
  strcpy (props.newdoc, (char *) xv_get (props_gd_newd, PANEL_VALUE));
  props.open = (int) xv_get (props_gd_open, PANEL_VALUE);
  props.delete = (int) xv_get (props_gd_delt, PANEL_VALUE);
  /* advanced settings */
  props.update = (int) xv_get (props_as_update, PANEL_VALUE);
  props.link = (int) xv_get (props_as_link, PANEL_VALUE);
  props.other = (int) xv_get (props_as_other, PANEL_VALUE);
  free (props.edit);
  props.edit = malloc (strlen ((char *) 
			       xv_get (props_as_edit, PANEL_VALUE)) + 1);
  strcpy (props.edit, (char *) xv_get (props_as_edit, PANEL_VALUE));
  free (props.filter);
  props.filter = malloc (strlen ((char *) 
				 xv_get (props_as_filter, PANEL_VALUE)) + 1);
  strcpy (props.filter, (char *) xv_get (props_as_filter, PANEL_VALUE));
} /* get_prop */


static void
set_prop ()
{
  /* current folder properties */
  xv_set (props_cf_lout, PANEL_VALUE, props.current_folder.lout, NULL);
  xv_set (props_cf_sort, PANEL_VALUE, props.current_folder.sort, NULL);
  xv_set (props_cf_hide, PANEL_VALUE, props.current_folder.hide, NULL);
  xv_set (props_cf_disp, PANEL_VALUE, props.current_folder.icon, NULL);
  xv_set (props_cf_show, PANEL_VALUE, props.current_folder.show, NULL);
  /* new folder properties */
  xv_set (props_nf_name, PANEL_VALUE, props.new_folder.name, NULL);
  xv_set (props_nf_lout, PANEL_VALUE, props.new_folder.lout, NULL);
  xv_set (props_nf_sort, PANEL_VALUE, props.new_folder.sort, NULL);
  xv_set (props_nf_hide, PANEL_VALUE, props.new_folder.hide, NULL);
  xv_set (props_nf_disp, PANEL_VALUE, props.new_folder.icon, NULL);
  xv_set (props_nf_show, PANEL_VALUE, props.new_folder.show, NULL);
  /* goto menu defaults */
  xv_set (props_gt_nfol, PANEL_VALUE, props.nfolder, NULL);
  /* general defaults */
  xv_set (props_gd_newd, PANEL_VALUE, props.newdoc, NULL);
  xv_set (props_gd_delt, PANEL_VALUE, props.delete, NULL);
  xv_set (props_gd_open, PANEL_VALUE, props.open, NULL);
  /* advanced settings */
  xv_set (props_as_update, PANEL_VALUE, props.update, NULL);
  xv_set (props_as_link, PANEL_VALUE, props.link, NULL);
  xv_set (props_as_other, PANEL_VALUE, props.other, NULL);
  xv_set (props_as_edit, PANEL_VALUE, props.edit, NULL);
  xv_set (props_as_filter, PANEL_VALUE, props.filter, NULL);
} /* set_prop */


static void
other_editor (Panel_item item)
{
  if ((int) xv_get (item, PANEL_VALUE) == 0)
    xv_set (props_as_edit, XV_SHOW, FALSE, NULL);
  else
    xv_set (props_as_edit, XV_SHOW, TRUE, NULL);
} /* other_editor */


static void
new_display_method (Panel_item item)
{
  int        method;
  Panel_item show;
  Panel_item lout;

  method = (int) xv_get (item, PANEL_VALUE);
  show = (Panel_item) xv_get (item, XV_KEY_DATA, SHOW);
  lout = (Panel_item) xv_get (item, XV_KEY_DATA, LOUT);
  if (method == LIST) {
    xv_set (show, PANEL_INACTIVE, FALSE, NULL);
    xv_set (lout, PANEL_INACTIVE, TRUE, NULL);
  } /* if */
  else {
    xv_set (show, PANEL_INACTIVE, TRUE, NULL);
    xv_set (lout, PANEL_INACTIVE, FALSE, NULL);
  } /* else */
} /* new_display_method */


static void 
new_category (Panel_item item)
{
  Frame frame;
  int   width, height;

  frame = (Frame) xv_get (item, PANEL_CLIENT_DATA);
  xv_set (props_panel [category], XV_SHOW, FALSE, NULL);
  category = (int) xv_get (item, PANEL_VALUE);
  width = xv_get (props_panel [category], XV_WIDTH);
  height = xv_get (props_panel [category], XV_HEIGHT);
  xv_set (props_panel [category], XV_SHOW, TRUE, NULL);
  xv_set (frame, XV_WIDTH, width + 4, NULL);
  xv_set (frame, XV_HEIGHT, height + 50, NULL);
} /* new_category */

/*********************/
/* Gotomenu handling */
/*********************/

extern int
props_menu_no_label ()
{
  Gotomenu_T *first;
  int        no = 0;

  first = first_menu;
  while (first != NULL) {
    first = first->next;
    no++;
  } /* while */
  return no;
} /* props_menu_no_label */


extern char *
props_menu_get (char *label)
{
  Gotomenu_T *current;
  
  current = first_menu;
  while (current != NULL) {
    if (strcmp (current->label, label) == 0)
      break;
    current = current->next;
  } /* while */
  if (current != NULL)
    return current->directory;
  else
    return label;
} /* props_menu_get */


static void
props_menu_notify (Panel_item item)
{
  Gotomenu_T *current;
  int        selected;

  selected = (int) xv_get (props_gt_list, PANEL_LIST_FIRST_SELECTED);
  if (selected != -1) {
    current = (Gotomenu_T *) xv_get (props_gt_list, PANEL_LIST_CLIENT_DATA, 
				     selected);
    xv_set (props_gt_label, PANEL_VALUE, current->label, NULL);
    xv_set (props_gt_path, PANEL_VALUE, current->directory, NULL);
  } /* if */  
} /* props_menu_notify */

extern void
props_menu_set_menu ()
{
  int n = 0;
  Menu_item item, new_item;
  Gotomenu_T *current;
  
  /* delete all goto menu entries concerning props_gt */
  item = (Menu_item) xv_get (goto_menu, MENU_NTH_ITEM, 1);
  while (strlen ((char *) xv_get (item, MENU_STRING)) != 0) {
    xv_set (goto_menu, MENU_REMOVE, 1, NULL);
    item = (Menu_item) xv_get (goto_menu, MENU_NTH_ITEM, 1);
  } /* while */
  item = (Menu_item) xv_get (goto_menu, MENU_NTH_ITEM, 1);
  /* insert entries */
  current = first_menu;
  while (current != NULL) {
    new_item = (Menu_item) xv_create (XV_NULL, MENUITEM,
				      MENU_STRING, current->label,
				      MENU_NOTIFY_PROC, menu_directory,
				      NULL);
    xv_set (goto_menu, MENU_INSERT, n, new_item, NULL);
    n++;
    new_item = item;
    current = current->next;
  } /* while */
} /* props_menu_set_menu */


static void
props_menu_set ()
{
  int n= 0;
  Gotomenu_T *current;

  current = first_menu;
  while (current != NULL) {
    xv_set (props_gt_list, 
	    PANEL_LIST_INSERT, n,
	    PANEL_LIST_STRING, n, current->label,
	    PANEL_LIST_CLIENT_DATA, n, current,
	    NULL);
    n++;
    current = current->next;
  } /* while */
} /* props_menu_set */


static void
props_menu_add (Panel_item item)
{
  Gotomenu_T *new;
  int        c = 0;

  new = (Gotomenu_T *) malloc (sizeof (Gotomenu_T));
  new->label = (char *) malloc (strlen ((char *) 
		        xv_get (props_gt_label, PANEL_VALUE)) + 1);
  strcpy (new->label, (char *) xv_get (props_gt_label, PANEL_VALUE));
  if (strlen (new->label) == 0) {
    xv_set (props_frame, FRAME_LEFT_FOOTER, "You must specify a label.", NULL);
  } /* if */
  else {
    new->directory = (char *) malloc (strlen ((char *) 
			      xv_get (props_gt_path, PANEL_VALUE)) + 1);
    strcpy (new->directory, (char *) xv_get (props_gt_path, PANEL_VALUE));
    if (strlen (new->directory) == 0) {
      xv_set (props_frame, 
	      FRAME_LEFT_FOOTER, "You must specify a directory.", NULL);
    } /* if */
    else {
      xv_set (props_gt_list, 
	      PANEL_LIST_INSERT, 0,
	      PANEL_LIST_STRING, 0, new->label,
	      PANEL_LIST_CLIENT_DATA, 0, new,
	      NULL);
      new->next = first_menu;
      first_menu = new;
    } /* else */
  } /* else */
  /* clear input */
  xv_set (props_gt_path, PANEL_VALUE, "", NULL);
  xv_set (props_gt_label, PANEL_VALUE, "", NULL);
} /* props_menu_add */


static void
props_menu_delete (Menu menu, Menu_item item)
{
  Gotomenu_T *current;
  Gotomenu_T *delete;
  int        selected;

  selected = (int) xv_get (props_gt_list, PANEL_LIST_FIRST_SELECTED);
  if (selected != -1) {
    delete = (Gotomenu_T *) xv_get (props_gt_list, PANEL_LIST_CLIENT_DATA, 
				    selected);
    if (strcmp (delete->label, "Home") == 0) 
      xv_set (props_frame, 
	      FRAME_LEFT_FOOTER, "You can't delete the Home entry.", 
	      NULL);
    else {
      current = first_menu;
      if (first_menu == delete) {
	/* destroy first entry */
	first_menu = (Gotomenu_T *) first_menu->next;
	free (delete);
      } /* if */
      else {
	while (current->next != delete) {
	  current = (Gotomenu_T *) current->next;
	} /* while */
	current->next = delete->next;
	free (delete);
      } /* else */
      xv_set (props_gt_list, PANEL_LIST_DELETE, selected, NULL);
    } /* if */
    xv_set (props_gt_path, PANEL_VALUE, "", NULL);
    xv_set (props_gt_label, PANEL_VALUE, "", NULL);
  } /* else */
} /* props_menu_delete */


static Gotomenu_T *
copy_menu (Gotomenu_T *current) 
{
  Gotomenu_T *c2;
  
  c2 = (Gotomenu_T *) malloc (sizeof (Gotomenu_T));
  c2->label = (char *) malloc (strlen (current->label) + 1);
  strcpy (c2->label, current->label);
  c2->directory = (char *) malloc (strlen (current->directory) + 1);
  strcpy (c2->directory, current->directory);
  return c2;
} /* new_menu */

static void
props_menu_copy (Menu menu, Menu_item item)
{
  Gotomenu_T *current;
  int        selected;

  selected = (int) xv_get (props_gt_list, PANEL_LIST_FIRST_SELECTED);
  if (selected != -1) {
    current = (Gotomenu_T *) xv_get (props_gt_list, PANEL_LIST_CLIENT_DATA,
				     selected);
    if (clip_menu != NULL)
      free (clip_menu);
    clip_menu = copy_menu (current);
  } /* if */
} /* props_menu_copy */


static void
props_menu_cut (Menu menu, Menu_item item)
{
  props_menu_copy (menu, item);
  props_menu_delete (menu, item);
} /* props_menu_cut */


static void
props_menu_paste_before (Menu menu, Menu_item item)
{
  Gotomenu_T *new;
  Gotomenu_T *current;
  Gotomenu_T *c2;
  int        selected;

  if (clip_menu != NULL) {
    new = copy_menu (clip_menu);
    selected = (int) xv_get (props_gt_list, PANEL_LIST_FIRST_SELECTED);
    current = (Gotomenu_T *) xv_get (props_gt_list, PANEL_LIST_CLIENT_DATA,
				     selected);
    if (first_menu == current) {
      /* before first entry */
      new->next = first_menu;
      first_menu = new;
    } /* if */
    else {
      c2 = first_menu;
      while (c2->next != current) {
	c2 = (Gotomenu_T *) c2->next;
      } /* while */
      new->next = current->next;
      current->next = new;
    } /* else */
    xv_set (props_gt_list,
	    PANEL_LIST_INSERT, selected,
	    PANEL_LIST_STRING, selected, new->label,
	    PANEL_LIST_CLIENT_DATA, selected, new,
	    NULL);
    free (clip_menu);
    clip_menu = NULL;
  } /* if */
} /* props_menu_paste_before */


static void
props_menu_paste_after (Menu menu, Menu_item item)
{
  Gotomenu_T *new;
  Gotomenu_T *current;
  int        selected;

  if (clip_menu != NULL) {
    new = copy_menu (clip_menu);
    selected = (int) xv_get (props_gt_list, PANEL_LIST_FIRST_SELECTED);
    current = (Gotomenu_T *) xv_get (props_gt_list, PANEL_LIST_CLIENT_DATA,
				     selected);
    new->next = current->next;
    current->next = new;
    xv_set (props_gt_list,
	    PANEL_LIST_INSERT, selected + 1,
	    PANEL_LIST_STRING, selected + 1, new->label,
	    PANEL_LIST_CLIENT_DATA, selected + 1, new,
	    NULL);
    free (clip_menu);
    clip_menu = NULL;
  } /* if */
} /* props_menu_paste_after */


static void
props_menu_paste_top (Menu menu, Menu_item item)
{
  Gotomenu_T *new;

  if (clip_menu != NULL) {
    new = copy_menu (clip_menu);
    new->next = first_menu;
    first_menu = new;
    xv_set (props_gt_list,
	    PANEL_LIST_INSERT, 0,
	    PANEL_LIST_STRING, 0, new->label,
	    PANEL_LIST_CLIENT_DATA, 0, new,
	    NULL);
    free (clip_menu);
    clip_menu = NULL;
  } /* if */
} /* props_menu_paste_top */


static void
props_menu_paste_bottom (Menu menu, Menu_item item)
{
  Gotomenu_T *new;
  Gotomenu_T *current;
  int        n;

  if (clip_menu != NULL) {
    new = copy_menu (clip_menu);
    current = first_menu;
    if (first_menu == NULL)
      /* list is empty */
      first_menu = new;
    else {
      while (current->next != NULL)
	current = current->next;
      current->next = new;
    } /* else */
    new->next = NULL;
    n = (int) xv_get (props_gt_list, PANEL_LIST_NROWS);
    xv_set (props_gt_list,
	    PANEL_LIST_INSERT, n,
	    PANEL_LIST_STRING, n, new->label,
	    PANEL_LIST_CLIENT_DATA, n, new,
	    NULL);
    free (clip_menu);
    clip_menu = NULL;
  } /* if */
} /* props_menu_paste_bottom */


/************************/
/* Commandmenu handling */
/************************/


extern Command_T *
props_command_get (char *label)
{
  Command_T *current;
  
  current = first_command;
  while (current != NULL) {
    if (strcmp (current->label, label) == 0)
      break;
    current = current->next;
  } /* while */
  if (current != NULL)
    return current;
  else
    return NULL;
} /* props_command_get */


static void
props_command_notify (Panel_item item)
{
  Command_T *current;
  int        selected;

  selected = (int) xv_get (props_cc_list, PANEL_LIST_FIRST_SELECTED);
  if (selected != -1) {
    current = (Command_T *) xv_get (props_cc_list, PANEL_LIST_CLIENT_DATA, 
				     selected);
    xv_set (props_cc_label, PANEL_VALUE, current->label, NULL);
    xv_set (props_cc_command, PANEL_VALUE, current->command, NULL);
    xv_set (props_cc_prompt, PANEL_VALUE, current->prompt, NULL);
    xv_set (props_cc_outwin, PANEL_VALUE, current->outwin, NULL);
    xv_set (props_cc_prmwin, PANEL_VALUE, current->prmwin, NULL);
  } /* if */  
} /* props_command_notify */

extern void
props_command_set_menu ()
{
  int n;
  int i;
  Menu_item new_item;
  Command_T *current;
  
  /* delete all command menu entries concerning props_cc */
  n = (int) xv_get (command_menu, MENU_NITEMS);
  for (i = 0; i < n; i++) {
    xv_set (command_menu, MENU_REMOVE, 1, NULL);
  } /* for */
  /* insert entries */
  current = first_command;
  while (current != NULL) {
    new_item = (Menu_item) xv_create (XV_NULL, MENUITEM,
				      MENU_STRING, current->label,
				      MENU_NOTIFY_PROC, file_command,
				      NULL);
    xv_set (command_menu, MENU_APPEND_ITEM, new_item, NULL);
    n++;
    current = current->next;
  } /* while */
} /* props_command_set_menu */


static void
props_command_set ()
{
  int n= 0;
  Command_T *current;

  current = first_command;
  while (current != NULL) {
    xv_set (props_cc_list, 
	    PANEL_LIST_INSERT, n,
	    PANEL_LIST_STRING, n, current->label,
	    PANEL_LIST_CLIENT_DATA, n, current,
	    NULL);
    n++;
    current = current->next;
  } /* while */
} /* props_command_set */


static void
props_command_add (Panel_item item)
{
  Command_T *new;
  int        c = 0;

  new = (Command_T *) malloc (sizeof (Command_T));
  new->label = (char *) malloc (strlen ((char *) 
		        xv_get (props_cc_label, PANEL_VALUE)) + 1);
  strcpy (new->label, (char *) xv_get (props_cc_label, PANEL_VALUE));
  if (strlen (new->label) == 0) {
    xv_set (props_frame, FRAME_LEFT_FOOTER, "No label specified.", NULL);
  } /* if */
  else {
    new->command = (char *) malloc (strlen ((char *) 
			    xv_get (props_cc_command, PANEL_VALUE)) + 1);
    strcpy (new->command, (char *) xv_get (props_cc_command, PANEL_VALUE));
    new->prmwin = (int) xv_get (props_cc_prmwin, PANEL_VALUE);
    if (new->prmwin == 0) {
      new->prompt = (char *) malloc (strlen ((char *) 
			     xv_get (props_cc_prompt, PANEL_VALUE)) + 1);
      strcpy (new->prompt, (char *) xv_get (props_cc_prompt, PANEL_VALUE));
    } /* if */
    new->outwin = (int) xv_get (props_cc_outwin, PANEL_VALUE);
    xv_set (props_cc_list, 
	    PANEL_LIST_INSERT, 0,
	    PANEL_LIST_STRING, 0, new->label,
	    PANEL_LIST_CLIENT_DATA, 0, new,
	    NULL);
    new->next = first_command;
    first_command = new;
  } /* else */
  /* clear input */
  xv_set (props_cc_command, PANEL_VALUE, "", NULL);
  xv_set (props_cc_label, PANEL_VALUE, "", NULL);
  xv_set (props_cc_prompt, PANEL_VALUE, "", NULL);
  xv_set (props_cc_prmwin, PANEL_VALUE, 1, NULL);
  xv_set (props_cc_outwin, PANEL_VALUE, 1, NULL);
} /* props_command_add */


static void
props_command_delete (Menu menu, Menu_item item)
{
  Command_T *current;
  Command_T *delete;
  int        selected;

  selected = (int) xv_get (props_cc_list, PANEL_LIST_FIRST_SELECTED);
  if (selected != -1) {
    delete = (Command_T *) xv_get (props_cc_list, PANEL_LIST_CLIENT_DATA, 
				    selected);
    current = first_command;
    if (first_command == delete) {
      /* destroy first entry */
      first_command = (Command_T *) first_command->next;
      free (delete);
    } /* if */
    else {
      while (current->next != delete) {
	current = (Command_T *) current->next;
      } /* while */
      current->next = delete->next;
      free (delete);
    } /* else */
    xv_set (props_cc_list, PANEL_LIST_DELETE, selected, NULL);
  } /* if */
  xv_set (props_cc_command, PANEL_VALUE, "", NULL);
  xv_set (props_cc_label, PANEL_VALUE, "", NULL);
  xv_set (props_cc_prompt, PANEL_VALUE, "", NULL);
  xv_set (props_cc_prmwin, PANEL_VALUE, 1, NULL);
  xv_set (props_cc_outwin, PANEL_VALUE, 1, NULL);
} /* props_command_delete */


static Command_T *
copy_command (Command_T *current) 
{
  Command_T *c2;
  
  c2 = (Command_T *) malloc (sizeof (Command_T));
  c2->label = (char *) malloc (strlen (current->label) + 1);
  strcpy (c2->label, current->label);
  c2->command = (char *) malloc (strlen (current->command) + 1);
  strcpy (c2->command, current->command);
  c2->prompt = (char *) malloc (strlen (current->prompt) + 1);
  strcpy (c2->prompt, current->prompt);
  c2->prmwin = current->prmwin;
  c2->outwin = current->outwin;
  return c2;
} /* copy_command */

static void
props_command_copy (Menu menu, Menu_item item)
{
  Command_T *current;
  int        selected;

  selected = (int) xv_get (props_cc_list, PANEL_LIST_FIRST_SELECTED);
  if (selected != -1) {
    current = (Command_T *) xv_get (props_cc_list, PANEL_LIST_CLIENT_DATA,
				     selected);
    if (clip_command != NULL)
      free (clip_command);
    clip_command = copy_command (current);
  } /* if */
} /* props_command_copy */


static void
props_command_cut (Menu menu, Menu_item item)
{
  props_command_copy (menu, item);
  props_command_delete (menu, item);
} /* props_command_cut */


static void
props_command_paste_before (Menu menu, Menu_item item)
{
  Command_T *new;
  Command_T *current;
  Command_T *c2;
  int        selected;

  if (clip_command != NULL) {
    new = copy_command (clip_command);
    selected = (int) xv_get (props_cc_list, PANEL_LIST_FIRST_SELECTED);
    current = (Command_T *) xv_get (props_cc_list, PANEL_LIST_CLIENT_DATA,
				     selected);
    if (first_command == current) {
      /* before first entry */
      new->next = first_command;
      first_command = new;
    } /* if */
    else {
      c2 = first_command;
      while (c2->next != current) {
	c2 = (Command_T *) c2->next;
      } /* while */
      new->next = current->next;
      current->next = new;
    } /* else */
    xv_set (props_cc_list,
	    PANEL_LIST_INSERT, selected,
	    PANEL_LIST_STRING, selected, new->label,
	    PANEL_LIST_CLIENT_DATA, selected, new,
	    NULL);
    free (clip_command);
    clip_command = NULL;
  } /* if */
} /* props_command_paste_before */


static void
props_command_paste_after (Menu menu, Menu_item item)
{
  Command_T *new;
  Command_T *current;
  int        selected;

  if (clip_command != NULL) {
    new = copy_command (clip_command);
    selected = (int) xv_get (props_cc_list, PANEL_LIST_FIRST_SELECTED);
    current = (Command_T *) xv_get (props_cc_list, PANEL_LIST_CLIENT_DATA,
				     selected);
    new->next = current->next;
    current->next = new;
    xv_set (props_cc_list,
	    PANEL_LIST_INSERT, selected + 1,
	    PANEL_LIST_STRING, selected + 1, new->label,
	    PANEL_LIST_CLIENT_DATA, selected + 1, new,
	    NULL);
    free (clip_command);
    clip_command = NULL;
  } /* if */
} /* props_command_paste_after */


static void
props_command_paste_top (Menu menu, Menu_item item)
{
  Command_T *new;

  if (clip_command != NULL) {
    new = copy_command (clip_command);
    new->next = first_command;
    first_command = new;
    xv_set (props_cc_list,
	    PANEL_LIST_INSERT, 0,
	    PANEL_LIST_STRING, 0, new->label,
	    PANEL_LIST_CLIENT_DATA, 0, new,
	    NULL);
    free (clip_command);
    clip_command = NULL;
  } /* if */
} /* props_command_paste_top */


static void
props_command_paste_bottom (Menu menu, Menu_item item)
{
  Command_T *new;
  Command_T *current;
  int        n;

  if (clip_command != NULL) {
    new = copy_command (clip_command);
    current = first_command;
    if (first_command == NULL)
      /* list is empty */
      first_command = new;
    else {
      while (current->next != NULL)
	current = current->next;
      current->next = new;
    } /* else */
    new->next = NULL;
    n = (int) xv_get (props_cc_list, PANEL_LIST_NROWS);
    xv_set (props_cc_list,
	    PANEL_LIST_INSERT, n,
	    PANEL_LIST_STRING, n, new->label,
	    PANEL_LIST_CLIENT_DATA, n, new,
	    NULL);
    free (clip_command);
    clip_command = NULL;
  } /* if */
} /* props_command_paste_bottom */


static void
props_load ()
{
  FILE *fp;
  char *name;
  char buf [NAME_MAX];
  Gotomenu_T *current;
  Command_T  *command;
  int        count = 0;

  name = malloc (strlen (HOME) + 11);
  strcpy (name, HOME);
  strcat (name, "/.xvfmgrrc");
  if ((fp = fopen (name, "r")) != NULL) {
    /* current folder properties */
    fscanf (fp, "%d\n", &props.current_folder.lout);
    fscanf (fp, "%d\n", &props.current_folder.sort);
    fscanf (fp, "%d\n", &props.current_folder.hide);
    fscanf (fp, "%d\n", &props.current_folder.icon);
    fscanf (fp, "%d\n", &props.current_folder.show);
    /* new folder properties */
    fscanf (fp, "%s\n", &buf);
    props.new_folder.name = (char *) malloc (strlen (buf) + 1);
    strcpy (props.new_folder.name, buf);
    fscanf (fp, "%d\n", &props.new_folder.lout);
    fscanf (fp, "%d\n", &props.new_folder.sort);
    fscanf (fp, "%d\n", &props.new_folder.hide);
    fscanf (fp, "%d\n", &props.new_folder.icon);
    fscanf (fp, "%d\n", &props.new_folder.show);
    /* goto menu defaults */
    fscanf (fp, "%d\n", &props.nfolder);
    free (first_menu);
    current = NULL;
    do {
      fgets (buf, sizeof (buf) - 1, fp);
      if (buf [0] == '@')
	break;
      if (current == NULL) {
	current = (Gotomenu_T *) malloc (sizeof (Gotomenu_T));
	first_menu = current;
      } /* if */
      else {
	current->next = (Gotomenu_T *) malloc (sizeof (Gotomenu_T));
	current = current->next;
      } /* else */
      count++;
      current->label = (char *) malloc (strlen (buf));
      strncpy (current->label, buf, strlen (buf) - 1);
      current->label [strlen (buf) - 1] = '\0';
      fgets (buf, sizeof (buf) - 1, fp);
      current->directory = (char *) malloc (strlen (buf));
      strncpy (current->directory, buf, strlen (buf) - 1);
      current->directory [strlen (buf) - 1] = '\0';
    } while (!feof (fp));
    if (count > 0)
      current->next = NULL;
    /* custom commands */
    count = 0;
    free (first_command);
    command = NULL;
    do {
      fgets (buf, sizeof (buf) - 1, fp);
      if (buf [0] == '@')
	break;
      if (command == NULL) {
	command = (Command_T *) malloc (sizeof (Command_T));
	first_command = command;
      } /* if */
      else {
	command->next = (Command_T *) malloc (sizeof (Command_T));
	command = command->next;
      } /* else */
      count++;
      command->label = (char *) malloc (strlen (buf));
      strncpy (command->label, buf, strlen (buf) - 1);
      command->label [strlen (buf) - 1] = '\0';
      fgets (buf, sizeof (buf) - 1, fp);
      command->command = (char *) malloc (strlen (buf));
      strncpy (command->command, buf, strlen (buf) - 1);
      command->command [strlen (buf) - 1] = '\0';
      fgets (buf, sizeof (buf) - 1, fp);
      if (strncmp (buf, "(null)", 6) == 0) 
	command->prompt = NULL;
      else {
	command->prompt = (char *) malloc (strlen (buf));
	strncpy (command->prompt, buf, strlen (buf) - 1);
	command->prompt [strlen (buf) - 1] = '\0';
      } /* else */
      fscanf (fp, "%d\n", &command->prmwin);
      fscanf (fp, "%d\n", &command->outwin);
    } while (!feof (fp));
    if (count > 0)
      command->next = NULL;
    /* general defaults */
    fscanf (fp, "%s\n", &buf);
    props.newdoc = (char *) malloc (strlen (buf) + 1);
    strcpy (props.newdoc, buf);
    fscanf (fp, "%d\n", &props.open);
    fscanf (fp, "%d\n", &props.delete);
    /* advanced settings */
    fscanf (fp, "%d\n", &props.update);
    fscanf (fp, "%d\n", &props.link);
    fscanf (fp, "%d\n", &props.other);
    fscanf (fp, "%[^\n]", &buf);
    if (strcmp (buf, "(null)") == 0)
      props.edit = NULL;
    else {
      props.edit = (char *) malloc (strlen (buf) + 1);
      strcpy (props.edit, buf);
    } /* else */
    fscanf (fp, "%s\n", &buf);
    if (strcmp (buf, "(null)") == 0)
      props.filter = NULL;
    else {
      props.filter = (char *) malloc (strlen (buf) + 1);
      strcpy (props.filter, buf);
    } /* else */
    fclose (fp);
  } /* if */
} /* props_load */


static void 
props_save (Panel_item item)
{
  FILE *fp;
  char *name;
  Gotomenu_T *current;
  Command_T  *command;

  name = malloc (strlen (HOME) + 11);
  strcpy (name, HOME);
  strcat (name, "/.xvfmgrrc");
  if ((fp = fopen (name, "w")) != NULL) {
    /* current folder properties */
    fprintf (fp, "%d\n", props.current_folder.lout);
    fprintf (fp, "%d\n", props.current_folder.sort);
    fprintf (fp, "%d\n", props.current_folder.hide);
    fprintf (fp, "%d\n", props.current_folder.icon);
    fprintf (fp, "%d\n", props.current_folder.show);
    /* new folder properties */
    fprintf (fp, "%s\n", props.new_folder.name);
    fprintf (fp, "%d\n", props.new_folder.lout);
    fprintf (fp, "%d\n", props.new_folder.sort);
    fprintf (fp, "%d\n", props.new_folder.hide);
    fprintf (fp, "%d\n", props.new_folder.icon);
    fprintf (fp, "%d\n", props.new_folder.show);
    /* goto menu defaults */
    fprintf (fp, "%d\n", props.nfolder);
    current = first_menu;
    while (current != NULL) {
      fprintf(fp, "%s\n", current->label);
      fprintf(fp, "%s\n", current->directory);
      current = current->next;
    } /* while */
    fprintf(fp, "@\n");
    /* custom commands */
    command = first_command;
    while (command != NULL) {
      fprintf(fp, "%s\n", command->label);
      fprintf(fp, "%s\n", command->command);
      fprintf(fp, "%s\n", command->prompt);
      fprintf(fp, "%d\n", command->prmwin);
      fprintf(fp, "%d\n", command->outwin);
      command = command->next;
    } /* while */
    fprintf(fp, "@\n");
    /* general defaults */
    fprintf (fp, "%s\n", props.newdoc);
    fprintf (fp, "%d\n", props.open);
    fprintf (fp, "%d\n", props.delete);
    /* advanced settings */
    fprintf (fp, "%d\n", props.update);
    fprintf (fp, "%d\n", props.link);
    fprintf (fp, "%d\n", props.other);
    if ((props.edit == NULL) || (strlen (props.edit) == 0))
      fprintf (fp, "(null)\n");
    else
      fprintf (fp, "%s\n", props.edit);
    if ((props.filter == NULL) || (strlen (props.filter) == 0))
      fprintf (fp, "(null)\n");
    else
    fprintf (fp, "%s\n", props.filter);
    fclose (fp);
  } /* if */
} /* props_save */


static void 
props_apply (Panel_item item)
{
  static struct itimerval timer;

  get_prop ();
  props_menu_set_menu ();
  props_command_set_menu ();
  /* install the update routine with the new value */
  timer.it_value.tv_sec = props.update;
  timer.it_interval.tv_sec = props.update;
  notify_set_itimer_func((Notify_client) first_app->frame, 
			 update_window_notify, 
			 ITIMER_REAL, &timer, NULL);
  update_window ();
} /* props_apply */


static void 
props_reset (Panel_item item)
{
  set_prop ();
  update_window ();
} /* props_reset */


static void
create_buttons (Frame frame, Panel panel, int width, int y)
{
  int        width1, width2, width3;
  Panel_item button1, button2, button3;

  button1 = xv_create (panel, PANEL_BUTTON,
		       PANEL_LABEL_STRING, "Apply",
		       PANEL_NOTIFY_PROC, props_apply,
		       PANEL_CLIENT_DATA, frame,
		       XV_Y, y,
		       NULL);

  button2 = xv_create (panel, PANEL_BUTTON,
		       PANEL_LABEL_STRING, "Save As Defaults",
		       PANEL_NOTIFY_PROC, props_save,
		       PANEL_CLIENT_DATA, frame,
		       XV_Y, y,
		       NULL);
  button3 = xv_create (panel, PANEL_BUTTON,
		       PANEL_LABEL_STRING, "Reset",
		       PANEL_NOTIFY_PROC, props_reset,
		       PANEL_CLIENT_DATA, frame,
		       XV_Y, y,
		       NULL);
  width1 = (int) xv_get (button1, XV_WIDTH) + 20;
  width2 = (int) xv_get (button2, XV_WIDTH) + 20;
  width3 = (int) xv_get (button3, XV_WIDTH);
  xv_set (button1, XV_X, (width - width1 - width2 - width3) / 2, NULL);
  xv_set (button2, XV_X, xv_get (button1, XV_X) + width1, NULL);
  xv_set (button3, XV_X, xv_get (button2, XV_X) + width2, NULL);
} /* create_buttons */


extern void
properties_notify (Panel_item item)
{
  Applic_T   *app;
  Panel      props_parent;
  Panel_item props_categ;
  Rect       rect;
  int        width, height;

  app = (Applic_T *) xv_get (item, MENU_CLIENT_DATA);
  props_frame = (Frame) xv_create (app->frame, FRAME_CMD,
				   FRAME_LABEL, "XVFilemgr: Properties",
				   FRAME_SHOW_RESIZE_CORNER, FALSE,
				   FRAME_SHOW_FOOTER, TRUE,
				   FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_IN,
				   NULL);
#ifdef DIALOG_ALWAYS_RIGHT
  frame_get_rect (app->frame, &rect);
  rect.r_left += rect.r_width;
  frame_set_rect (props_frame, &rect);
#endif
  props_parent = (Panel) xv_get (props_frame, FRAME_CMD_PANEL);

  props_categ = xv_create (props_parent, PANEL_CHOICE_STACK,
			   PANEL_LABEL_STRING, "Category: ",
			   PANEL_CHOICE_STRINGS, "Current Folder Settings",
			   "New Folder Settings", "Goto Menu Defaults",
			   "Custom Commands", "General Defaults", 
			   "Advanced Settings", NULL,
			   PANEL_NOTIFY_PROC, new_category,
			   PANEL_VALUE, 0,
			   PANEL_CLIENT_DATA, props_frame,
			   NULL);

  for (category = 0; category < 6; category++) {
    props_panel [category] = xv_create (props_parent, PANEL, 
					PANEL_BORDER, TRUE,
					XV_Y, 35,
					NULL);
    switch (category) {
    case CURRENT_FOLDER:
      props_cf_lout = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Icon Layout:",
				 PANEL_CHOICE_STRINGS, "Vertical", 
				 "Horizontal", NULL,
				 PANEL_VALUE_X, 140,
				 XV_X, 5,
				 XV_Y, 7,
				 NULL);
      props_cf_sort = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Sort By:",
				 PANEL_CHOICE_STRINGS, "Name", "Type",
				 "Size", "Date", NULL,
				 PANEL_VALUE_X, 140,
				 XV_X, 5,
				 NULL);
      props_cf_hide = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Hidden Files:",
				 PANEL_CHOICE_STRINGS, "Hide", "Show", NULL,
				 PANEL_VALUE_X, 140,
				 XV_X, 5,
				 NULL);
      props_cf_disp = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Display Files As:",
				 PANEL_CHOICE_STRINGS, "Small Icon", 
				 "Large Icon", "List", 
				 NULL,
				 PANEL_NOTIFY_PROC, new_display_method,
				 PANEL_VALUE_X, 140,
				 XV_X, 5,
				 NULL);
      props_cf_show = xv_create (props_panel [category], PANEL_TOGGLE,
				 PANEL_LABEL_STRING, "Show in List:",
				 PANEL_LAYOUT, PANEL_VERTICAL,
				 PANEL_CHOICE_STRINGS, "Date", "Size", "Owner",
				 "Group", "Links", "Permissions", NULL,
				 PANEL_CHOICE_NROWS, 3,
				 XV_X, 18,
				 NULL);
      xv_set (props_cf_disp, 
	      XV_KEY_DATA, SHOW, props_cf_show, 
	      XV_KEY_DATA, LOUT, props_cf_lout, 
	      NULL);
      (void) xv_create (props_panel [category], PANEL_MESSAGE,
			PANEL_LABEL_STRING, 
			"These settings only apply to the active folder",
			XV_X, 25,
			NULL);
      break;
    case NEW_FOLDER:
      props_nf_name = xv_create (props_panel [category], PANEL_TEXT,
				 PANEL_LABEL_STRING, "Default Name:",
				 PANEL_VALUE_DISPLAY_LENGTH, 30,
				 PANEL_VALUE_STORED_LENGTH, NAME_MAX,
				 PANEL_VALUE_X, 140,
				 PANEL_VALUE, "NewFolder",
				 XV_X, 5,
				 XV_Y, 7,
				 NULL);
      props_nf_lout = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Icon Layout:",
				 PANEL_CHOICE_STRINGS, "Vertical", 
				 "Horizontal", NULL,
				 PANEL_VALUE_X, 140,
				 XV_X, 5,
				 NULL);
      props_nf_sort = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Sort By:",
				 PANEL_CHOICE_STRINGS, "Name", "Type",
				 "Size", "Date", NULL,
				 PANEL_VALUE_X, 140,
				 XV_X, 5,
				 NULL);
      props_nf_hide = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Hidden Files:",
				 PANEL_CHOICE_STRINGS, "Hide", "Show", NULL,
				 PANEL_VALUE_X, 140,
				 XV_X, 5,
				 NULL);
      props_nf_disp = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Display Files As:",
				 PANEL_CHOICE_STRINGS, "Small Icon", 
				 "Large Icon", "List", 
				 NULL,
				 PANEL_NOTIFY_PROC, new_display_method,
				 PANEL_VALUE_X, 140,
				 XV_X, 5,
				 NULL);
      props_nf_show = xv_create (props_panel [category], PANEL_TOGGLE,
				 PANEL_LABEL_STRING, "Show in List:",
				 PANEL_LAYOUT, PANEL_VERTICAL,
				 PANEL_CHOICE_STRINGS, "Date", "Size", "Owner",
				 "Group", "Links", "Permissions", NULL,
				 PANEL_CHOICE_NROWS, 3,
				 XV_X, 18,
				 NULL);
      xv_set (props_nf_disp, 
	      XV_KEY_DATA, SHOW, props_nf_show, 
	      XV_KEY_DATA, LOUT, props_nf_lout, 
	      NULL);
      break;
    case GOTO_DEFAULTS:
      (void) xv_create (props_panel [category], PANEL_MESSAGE,
			PANEL_LABEL_STRING, "Type in the folder's name and click add",
			XV_X, 115,
			NULL);
      props_gt_path = xv_create (props_panel [category], PANEL_TEXT,
				 PANEL_LABEL_STRING, "Pathname:",
				 PANEL_VALUE_DISPLAY_LENGTH, 37,
				 PANEL_VALUE_STORED_LENGTH, PATH_MAX,
				 PANEL_VALUE_X, 120,
				 NULL);
      props_gt_add = xv_create (props_panel [category], PANEL_BUTTON,
				PANEL_LABEL_STRING, "Add Item",
				PANEL_NOTIFY_PROC, props_menu_add,
				XV_X, 440,
				XV_Y, 34,
				NULL);
      props_gt_label = xv_create (props_panel [category], PANEL_TEXT,
				  PANEL_LABEL_STRING, "Menu Label:",
				  PANEL_VALUE_DISPLAY_LENGTH, 37,
				  PANEL_VALUE_STORED_LENGTH, NAME_MAX,
				  PANEL_VALUE_X, 120,
				  NULL);
      props_gt_paste = (Menu) xv_create (XV_NULL, MENU,
					MENU_ITEM,
					  MENU_STRING, "Before",
				          MENU_NOTIFY_PROC, 
					    props_menu_paste_before,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "After",
					  MENU_NOTIFY_PROC, 
					    props_menu_paste_after,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "Top",
					  MENU_NOTIFY_PROC, 
					    props_menu_paste_top,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "Bottom",
					  MENU_NOTIFY_PROC, 
					    props_menu_paste_bottom,
					  NULL,
					NULL);
      props_gt_menu = (Menu) xv_create (XV_NULL, MENU,
					MENU_ITEM,
					  MENU_STRING, "Copy",
					  MENU_NOTIFY_PROC, props_menu_copy,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "Cut",
					  MENU_NOTIFY_PROC, props_menu_cut,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "Paste",
					  MENU_PULLRIGHT, props_gt_paste,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "Delete",
					  MENU_NOTIFY_PROC, props_menu_delete,
					  NULL,
					NULL);
      props_gt_edit = xv_create (props_panel [category], PANEL_BUTTON,
				 PANEL_LABEL_STRING, "Edit",
				 PANEL_ITEM_MENU, props_gt_menu,
				 XV_X, 440,
				 XV_Y, 130,
				 NULL);
      (void) xv_create (props_panel [category], PANEL_MESSAGE,
			PANEL_LABEL_STRING, "Goto Menu Items",
			PANEL_LABEL_BOLD, TRUE,
			XV_X, 30,
			XV_Y, 115,
			NULL);
      props_gt_list = xv_create (props_panel [category], PANEL_LIST,
				 PANEL_LIST_DISPLAY_ROWS, 5,
				 PANEL_LIST_WIDTH, 370,
				 PANEL_NOTIFY_PROC, props_menu_notify,
				 XV_X, 30,
				 XV_Y, 130,
				 NULL);
      (void) xv_create (props_panel [category], PANEL_MESSAGE,
			PANEL_LABEL_STRING, "The Goto Menu displays the last folders visited",
			XV_X, 115,
			NULL);
      props_gt_nfol = xv_create (props_panel [category], PANEL_NUMERIC_TEXT,
				 PANEL_LABEL_STRING, "Display:",
				 PANEL_VALUE_DISPLAY_LENGTH, 3,
				 XV_X, 160,
				 XV_Y, 282,
				 NULL);
      (void) xv_create (props_panel [category], PANEL_MESSAGE,
			PANEL_LABEL_STRING, "Folders",
			PANEL_LABEL_BOLD, TRUE,
			XV_X, 298,
			XV_Y, 282,
			NULL);
      props_menu_set ();
      break;
    case CUSTOM_COMMANDS:
      (void) xv_create (props_panel [category], PANEL_MESSAGE,
		PANEL_LABEL_STRING, 
		"Type in the command to be listed in the Custom Command Menu.",
		XV_X, 100,
		NULL);
      props_cc_command = xv_create (props_panel [category], PANEL_TEXT,
				    PANEL_LABEL_STRING, "UNIX Command:",
				    PANEL_VALUE_DISPLAY_LENGTH, 37,
				    PANEL_VALUE_STORED_LENGTH, NAME_MAX,
				    PANEL_VALUE_X, 140,
				    NULL);
      props_cc_label = xv_create (props_panel [category], PANEL_TEXT,
				  PANEL_LABEL_STRING, "Menu Label:",
				  PANEL_VALUE_DISPLAY_LENGTH, 37,
				  PANEL_VALUE_STORED_LENGTH, NAME_MAX,
				  PANEL_VALUE_X, 140,
				  NULL);
      props_cc_add = xv_create (props_panel [category], PANEL_BUTTON,
				PANEL_LABEL_STRING, "Add Item",
				PANEL_NOTIFY_PROC, props_command_add,
				XV_X, 470,
				XV_Y, 34,
				NULL);
      props_cc_paste = (Menu) xv_create (XV_NULL, MENU,
					MENU_ITEM,
					  MENU_STRING, "Before",
				          MENU_NOTIFY_PROC, 
					    props_command_paste_before,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "After",
					  MENU_NOTIFY_PROC, 
					    props_command_paste_after,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "Top",
					  MENU_NOTIFY_PROC, 
					    props_command_paste_top,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "Bottom",
					  MENU_NOTIFY_PROC, 
					    props_command_paste_bottom,
					  NULL,
					NULL);
      props_cc_menu = (Menu) xv_create (XV_NULL, MENU,
					MENU_ITEM,
					  MENU_STRING, "Copy",
					  MENU_NOTIFY_PROC, props_command_copy,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "Cut",
					  MENU_NOTIFY_PROC, props_command_cut,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "Paste",
					  MENU_PULLRIGHT, props_cc_paste,
					  NULL,
					MENU_ITEM,
					  MENU_STRING, "Delete",
					  MENU_NOTIFY_PROC, 
					    props_command_delete,
					  NULL,
					NULL);
      props_cc_edit = xv_create (props_panel [category], PANEL_BUTTON,
				 PANEL_LABEL_STRING, "Edit",
				 PANEL_ITEM_MENU, props_cc_menu,
				 XV_X, 470,
				 XV_Y, 130,
				 NULL);
      (void) xv_create (props_panel [category], PANEL_MESSAGE,
			PANEL_LABEL_STRING, "Custom Commands Menu Items",
			PANEL_LABEL_BOLD, TRUE,
			XV_X, 30,
			XV_Y, 115,
			NULL);
      props_cc_list = xv_create (props_panel [category], PANEL_LIST,
				 PANEL_LIST_DISPLAY_ROWS, 5,
				 PANEL_LIST_WIDTH, 405,
				 PANEL_NOTIFY_PROC, props_command_notify,
				 XV_X, 30,
				 XV_Y, 130,
				 NULL);
      props_cc_prmwin = xv_create (props_panel [category], PANEL_CHOICE,
				   PANEL_LABEL_STRING, "Prompt Window:",
				   PANEL_CHOICE_STRINGS, "Yes", "No", NULL,
				   PANEL_VALUE_X, 140,
				   XV_X, 5,
				   NULL);
      props_cc_prompt = xv_create (props_panel [category], PANEL_TEXT,
				   PANEL_LABEL_STRING, "Prompt:",
				   PANEL_VALUE_DISPLAY_LENGTH, 37,
				   PANEL_VALUE_STORED_LENGTH, NAME_MAX,
				   PANEL_VALUE_X, 140,
				   NULL);
      props_cc_outwin = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Output Window:",
				 PANEL_CHOICE_STRINGS, "Yes", "No", NULL,
				 PANEL_VALUE_X, 140,
				 XV_X, 5,
				 NULL);
      props_command_set ();
      break;
    case GENERAL:
      props_gd_open = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Open Folder Into:",
				 PANEL_LAYOUT, PANEL_VERTICAL,
				 PANEL_CHOICE_STRINGS, "Same Window",
				 "Seperate Window", NULL,
				 PANEL_VALUE, 0,
				 XV_X, 30,
				 XV_Y, 7,
				 NULL);
      props_gd_delt = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Delete Menu Item Is",
				 PANEL_LAYOUT, PANEL_VERTICAL,
				 PANEL_CHOICE_STRINGS, "Delete",
				 "Destroy", NULL,
				 XV_X, 30,
				 XV_Y, 110,
				 NULL);
      (void) xv_create (props_panel [category], PANEL_MESSAGE,
			PANEL_LABEL_STRING, 
			"File is recoverable from Waste window.",
			XV_X, 115,
			XV_Y, 135,
			NULL);
      (void) xv_create (props_panel [category], PANEL_MESSAGE,
			PANEL_LABEL_STRING, "File is not recoverable.",
			XV_X, 115,
			XV_Y, 158,
			NULL);
      props_gd_newd = xv_create (props_panel [category], PANEL_TEXT,
				 PANEL_LABEL_STRING, "Default Document Name:",
				 PANEL_VALUE_DISPLAY_LENGTH, 25,
				 PANEL_VALUE_STORED_LENGTH, NAME_MAX,
				 XV_X, 30,
				 XV_Y, 215,
				 NULL);
      break;
    case ADVANCED:
      props_as_update = xv_create (props_panel [category], PANEL_NUMERIC_TEXT,
				   PANEL_LABEL_STRING, "Update Window Every",
				   PANEL_LAYOUT, PANEL_VERTICAL,
				   PANEL_VALUE_DISPLAY_LENGTH, 5,
				   XV_X, 30,
				   XV_Y, 5,
				   NULL);
      props_as_link = xv_create (props_panel [category], PANEL_CHOICE,
				 PANEL_LABEL_STRING, "Follow Symbolic Link:",
				 PANEL_CHOICE_STRINGS, "Yes", "No", NULL,
				 PANEL_CHOICE_NROWS, 1,
				 PANEL_LAYOUT, PANEL_VERTICAL,
				 XV_X, 200, 
				 XV_Y, 5,
				 NULL);
      props_as_filter = xv_create (props_panel [category], PANEL_TEXT,
				   PANEL_LABEL_STRING, "View Filter Pattern:",
				   PANEL_LAYOUT, PANEL_VERTICAL,
				   PANEL_VALUE_DISPLAY_LENGTH, 25,
				   PANEL_VALUE_STORED_LENGTH, 50,
				   XV_X, 30, 
				   NULL);
      props_as_other = xv_create (props_panel [category], PANEL_CHOICE,
				  PANEL_LABEL_STRING, "Default Editor:",
				  PANEL_CHOICE_STRINGS, "Text Editor",
				  "Other:", NULL,
				  PANEL_NOTIFY_PROC, other_editor,
				  XV_X, 30, 
				  NULL);
      props_as_edit = xv_create (props_panel [category], PANEL_TEXT,
				 PANEL_VALUE_DISPLAY_LENGTH, 25,
				 PANEL_VALUE_STORED_LENGTH, NAME_MAX,
				 XV_X, 280,
				 XV_Y, 102,
				 NULL);
      other_editor (props_as_other);
      break;
    } /* switch */
    window_fit (props_panel [category]);
    width = (int) xv_get (props_panel [category], XV_WIDTH);
    height = (int) xv_get (props_panel [category], XV_HEIGHT);
    create_buttons (props_frame, props_panel [category], width, height);
    window_fit (props_panel [category]);
  } /* for */
  set_prop ();
  new_display_method (props_cf_disp);
  new_display_method (props_nf_disp);

  window_fit (props_frame);
  xv_set (props_frame, XV_SHOW, TRUE, NULL);
  for (category = 1; category < 6; category++)
    xv_set (props_panel [category], XV_SHOW, FALSE, NULL);
  category = 0;
  xv_set (props_panel [category], XV_SHOW, TRUE, NULL);
  new_category (props_categ);
} /* props_notify */


extern void
props_sort_name ()
{
  props.current_folder.sort = SORT_NAME;
} /* props_sort_name */


extern void
props_sort_type ()
{
  props.current_folder.sort = SORT_TYPE;
} /* props_sort_type */


extern void
props_sort_size ()
{
  props.current_folder.sort = SORT_SIZE;
} /* props_sort_size */


extern void
props_sort_date ()
{
  props.current_folder.sort = SORT_DATE;
} /* props_sort_date */


extern void
props_small_icons ()
{
  props.current_folder.icon = ICON_SMALL;
  props.pixel_per_folder = 70;
  props.pixel_per_file_x = 141;
  props.pixel_per_file_y = 20;
  props.gap_x = 5;
  props.gap_y = 3;
} /* props_small_icons */


extern void
props_large_icons ()
{
  props.current_folder.icon = ICON_LARGE;
  props.pixel_per_folder = 70;
  props.pixel_per_file_x = 125;
  props.pixel_per_file_y = 55;
  props.gap_x = 10;
  props.gap_y = 3;
} /* props_large_icons */


extern void
props_list ()
{
  props.current_folder.icon = LIST;
  props.pixel_per_folder = 70;
  props.pixel_per_file_x = 141;
  props.pixel_per_file_y = 20;
  props.gap_x = 5;
  props.gap_y = 3;
} /* props_list */


extern Properties_T *
get_properties ()
{
  return &props;
} /* get_properties */


extern void
set_folder_props (FolderProp_T *fprops)
{
  props.current_folder.show = fprops->show;
  props.current_folder.hide = fprops->hide;
  props.current_folder.lout = fprops->lout;
  props.current_folder.icon = fprops->icon;
  props.current_folder.sort = fprops->sort;
  switch (props.current_folder.icon) {
  case ICON_LARGE:
    props_large_icons ();
    break;
  case ICON_SMALL:
    props_small_icons ();
    break;
  case LIST:
    props_list ();
    break;
  } /* switch */
} /* set_folder_props */


extern void
properties_init ()
{
  props.current_folder.sort = SORT_TYPE;
  props.current_folder.show = SHOW_SIZE + SHOW_DATE + SHOW_OWNER + SHOW_GROUP;
  props.current_folder.lout = LAYOUT_EW;
  props.current_folder.hide = 0;
  props.current_folder.icon = ICON_LARGE;
  props.new_folder.name = malloc (10);
  strcpy (props.new_folder.name, "NewFolder");
  props.new_folder.sort = SORT_TYPE;
  props.new_folder.show = SHOW_SIZE + SHOW_DATE + SHOW_OWNER + SHOW_GROUP;
  props.new_folder.lout = LAYOUT_EW;
  props.new_folder.hide = 0;
  props.new_folder.icon = ICON_LARGE;
  props_large_icons ();
  props.newdoc = malloc (12);
  strcpy (props.newdoc, "NewDocument");
  props.update = 10;
  props.nfolder = 10;
  first_menu = (Gotomenu_T *) malloc (sizeof (Gotomenu_T));
  first_menu->label = malloc (5); /* length of "Home" */
  strcpy (first_menu->label, "Home");
  first_menu->directory = malloc (strlen (HOME) + 1);
  strcpy (first_menu->directory, HOME);
  first_command = NULL;
  first_menu->next = NULL;
  props.delete = TO_WASTE;
  props_load ();
} /* properties_init */




