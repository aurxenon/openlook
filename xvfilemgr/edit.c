/*
	System  : XVfilemgr
	File    : $RCSfile: edit.c,v $
	Author  : $Author: arkenoi $
	Date    : $Date: 2005/07/13 18:31:02 $
	Purpose : Cut, copy, paste and link
*
* $Log: edit.c,v $
* Revision 1.1  2005/07/13 18:31:02  arkenoi
* Initial revision
*
* Revision 1.7  1996/11/23 13:56:52  root
* new prototype for file_move.
*
* Revision 1.6  1996/05/27 16:25:13  root
* rename of file_move2.
*
* Revision 1.5  1996/05/27 10:20:38  root
* no cut and copy if no file is selected.
*
* Revision 1.4  1996/05/26 15:11:29  root
* really doing something when cut, copy, paste and link action is
* triggered.
*
* Revision 1.3  1996/05/26 12:57:38  root
* basic of getting a selection.
*
 * Revision 1.2  1996/05/20  19:27:19  root
 * initial version.
 *
 * Revision 1.1  1996/05/19  18:01:46  root
 * Initial revision
 *
*
*/

#include <sys/stat.h>

#include <xview/openmenu.h>
#include <xview/sel_pkg.h>

#include "global.h"
#include "select.h"
#include "file.h"

static Selection_owner     owner;
static Selection_requestor requestor;
static Selection_item      item;


static void
edit_selection_lose (Selection_owner owner)
{
  Applic_T *app;
  char *names, *source;

  /* remove all files previously selected from ~/.xvfilemgr/ */
  if (item != 0) {
    names = (char *) xv_get (item, SEL_DATA);
    app = (Applic_T *) xv_get (owner, XV_KEY_DATA, SEL_APP);
    source = strtok (names, "\n");
    if (strcmp (source, "cut") == 0) {
      while ((source = strtok (NULL, "\n")) != NULL)
	file_delete2 (app->frame, source);
    } /* if */
  } /* if */
} /* edit_selection_lose */


extern void 
edit_cut (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Selected_T   *sel;
  File_T       *file;
  int          length = 0, path;
  char         *names, *target, *full_name;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  /* give the lose proc a chance to clean up */
  xv_set (owner, SEL_OWN, FALSE, NULL);
  xv_set (owner, 
	  SEL_OWN, TRUE, 
	  XV_KEY_DATA, SEL_APP, app,
	  NULL);
  xv_destroy_safe (item);
  if (app->select != NULL) {
    item = (Selection_item) xv_create (owner, SELECTION_ITEM,
				       SEL_TYPE_NAME, "STRING",
				       NULL);
    /* iterate all selected files/dirs and move them to .xvfilemgr/.
       Determine the length of all path names. */
    path = strlen (app->path);
    sel = (Selected_T *) app->select;
    while (sel != NULL) {
      file = (File_T *) xv_get (sel->object, XV_KEY_DATA, FILE_FILE);
      /* create a selection with all names */
      length = length + strlen (file->name) + path + 6;
      full_name = malloc (strlen (app->path) + strlen (file->name) + 2);
      sprintf (full_name, "%s/%s", app->path, file->name);
      file_move (app->frame, full_name, CLIPBOARD, NULL);
      free (full_name);
      sel = sel->next;
    } /* while */
    /* set selection item data */
    names = malloc (++length);
    target = names;
    /* indication for the lose proc that the data in the clipboard directory
       have to be deleted */
    strcpy (target, "cut\n");
    target = target + 4;
    sel = (Selected_T *) app->select;
    while (sel != NULL) {
      file = (File_T *) xv_get (sel->object, XV_KEY_DATA, FILE_FILE);
      full_name = malloc (strlen (CLIPBOARD) + strlen (file->name) + 2);
      sprintf (full_name, "%s/%s\n", CLIPBOARD, file->name);
      strcpy (target, full_name);
      target = target + strlen (full_name);
      free (full_name);
      sel = sel->next;
    } /* while */
    xv_set (item, 
	    SEL_DATA, names,
	    SEL_LENGTH, length,
	    SEL_COPY, TRUE,
	    NULL);
    free (names);
    update_window ();
  } /* if */
} /* edit_cut */

extern void 
edit_copy (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Selected_T   *sel;
  File_T       *file;
  int          length = 0, path;
  char         *names, *target, *full_name;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  /* give the lose proc a chance to clean up */
  xv_set (owner, SEL_OWN, FALSE, NULL);
  xv_set (owner, 
	  SEL_OWN, TRUE, 	  
	  XV_KEY_DATA, SEL_APP, app,
	  NULL);
  xv_destroy_safe (item);
  if (app->select != NULL) {
    item = (Selection_item) xv_create (owner, SELECTION_ITEM,
				       SEL_TYPE_NAME, "STRING",
				       NULL);
    /* iterate all selected files/dirs and copy them to .xvfilemgr/ using 
       the absolute path name without the leading /. */
    path = strlen (app->path);
    sel = (Selected_T *) app->select;
    while (sel != NULL) {
      file = (File_T *) xv_get (sel->object, XV_KEY_DATA, FILE_FILE);
      /* create a selection with all names */
      length = length + strlen (file->name) + path + 7;
      sel = sel->next;
    } /* while */
    /* set selection item data */
    names = malloc (++length);
    target = names;
    /* indication for the lose proc that the data can stay in the 
       clipboard directory */
    strcpy (target, "copy\n");
    target = target + 5;
    sel = (Selected_T *) app->select;
    while (sel != NULL) {
      file = (File_T *) xv_get (sel->object, XV_KEY_DATA, FILE_FILE);
      full_name = malloc (strlen (app->path) + strlen (file->name) + 2);
      sprintf (full_name, "%s/%s\n", app->path, file->name);
      strcpy (target, full_name);
      target = target + strlen (full_name);
      free (full_name);
      sel = sel->next;
    } /* while */
    xv_set (item, 
	    SEL_DATA, names,
	    SEL_LENGTH, length,
	    SEL_COPY, TRUE,
	    NULL);
    free (names);
  } /* if */
} /* edit_copy */

extern void 
edit_paste (Menu menu, Menu_item menu_item)
{
  Applic_T      *app;
  int           format;
  unsigned long length;
  char          *names, *source;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  /* get filenames from selection and copy them to the current directory */
  names = (char *) xv_get (requestor, SEL_DATA, &length, &format);
  /* indicates wether the data was cut or copyied */
  source = strtok (names, "\n");
  while ((source = strtok (NULL, "\n")) != NULL)
    file_copy (app->frame, source, app->path);
  update_window ();
} /* edit_paste */

extern void 
edit_link (Menu menu, Menu_item menu_item)
{
  Applic_T      *app;
  int           format;
  unsigned long length;
  char          *names, *source;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  /* get filenames from selection and link them to the current directory */
  names = (char *) xv_get (requestor, SEL_DATA, &length, &format);
  source = strtok (names, "\n");
  if (strcmp (source, "copy")) {
    file_link (app->frame, source, app->path);
    while ((source = strtok (NULL, "\n")) != NULL)
      file_link (app->frame, source, app->path);
  } /* if */
  update_window ();
} /* edit_link */

extern void
edit_init (Applic_T *app)
{
  char *home;

  /* create hidden directory for files that have been cut */
  home = getenv ("HOME");
  CLIPBOARD = malloc (strlen (home) + 12);
  sprintf (CLIPBOARD, "%s/.xvfilemgr", home);
  /* don't care if it already exists */
  mkdir (CLIPBOARD, S_IRWXU);
  /* create objects for selection */
  owner = (Selection_owner) xv_create (app->frame, SELECTION_OWNER,
				       SEL_OWN, TRUE,
				       SEL_RANK_NAME, "CLIPBOARD",
				       SEL_LOSE_PROC, edit_selection_lose,
				       NULL);
  requestor = (Selection_requestor) xv_create (app->frame, SELECTION_REQUESTOR,
				       SEL_RANK_NAME, "CLIPBOARD",
				       NULL);
} /* edit_init */

extern void
edit_exit ()
{
  /* clean up the files in CLIPBOARD */
  edit_selection_lose (owner);
} /* edit_exit */
