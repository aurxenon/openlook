/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: xvfilemgr.c,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:15 $
 *	Purpose : main program
 *
 * $Log: xvfilemgr.c,v $
 * Revision 1.1  2005/07/13 18:31:15  arkenoi
 * Initial revision
 *
 * Revision 1.18  1999/01/02 12:59:39  root
 * Minor cosmetic change.
 *
 * Revision 1.17  1997/11/14 04:15:02  root
 * Moved the properties Panel_item to a Menu_item of edit_menu.
 *
 * Revision 1.16  1997/11/14 03:39:07  root
 * Small goto menu modification to look more like filemgr.
 *
 * Revision 1.15  1997/03/14 20:28:27  root
 * Fix of the path panel bug.  Thanks to James B. Hiller
 *
 * Revision 1.14  1996/08/17 08:44:41  root
 * minor corrections concerning goto menu.
 *
 * Revision 1.13  1996/07/31 19:15:46  root
 * cdrom support: eject button in cdrom window.
 *
 * Revision 1.12  1996/07/26 19:59:44  root
 * change some constants
 *
 * Revision 1.11  1996/07/22 08:33:15  root
 * changes from vincent concerning quit notification.
 *
 * Revision 1.10  1996/06/27  18:59:12  root
 * Cosmetic change.
 *
 * Revision 1.9  1996/06/21 19:03:48  root
 * hook for remote file transfer installed.
 *
 * Revision 1.8  1996/06/07 16:06:46  root
 * uninitialized goto menu client data.
 *
 * Revision 1.7  1996/06/01 14:02:54  root
 * support for mount module.
 *
 * Revision 1.6  1996/05/27 10:36:25  root
 * menu entry for Select All.
 *
 * Revision 1.5  1996/05/26 17:39:52  root
 * set the HOME constant.
 *
 * Revision 1.4  1996/05/26 15:10:55  root
 * remove files from clipboard before exiting.
 *
 * Revision 1.3  1996/05/19 19:13:01  root
 * edit depend on applications frame.
 *
 * Revision 1.2  1996/05/19  18:13:15  root
 * added support for clipboard.
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/rect.h>
#include <xview/svrimage.h>
#include <xview/font.h>
#include <xview/icon.h>
#include <xview/scrollbar.h>
#include <xview/notify.h>
#include <xview/notice.h>
#include <xview/server.h>
#include <sspkg/canshell.h>

#include "config.h"
#include "global.h"
#include "about.h"
#include "file.h"
#include "directory.h"
#include "find.h"
#include "properties.h"
#include "workwindow.h"
#include "waste.h"
#include "edit.h"
#include "mount.h"
#include "removable.h"
#include "remote.h"

#include "icons/xvfmgricon"
#include "icons/xvfmgrmask"
#include "icons/cdromicon"
#include "icons/cdrommask"
#include "icons/floppyicon"
#include "icons/floppymask"

/* Funktionen für xvfmgr-Frame-Handling */


extern Notify_value
xvfmgr_frame_delete (Notify_client client, Destroy_status status)
{
  Frame        frame;
  Xv_Notice    quit_notice;
  Applic_T     *app;
  Applic_T     *app2;
  Properties_T *props;
  struct itimerval timer;
  int one_frame_left = TRUE;
  int quit_confirm = TRUE;
  int notice_result;   

  frame = (Frame) client;

  /* We want to check for the last frame not a WASTE_WINDOW, */
  /* to know if the user is quitting the filemanager or just destroying */
  /* one of its workwindows. If quitting, then delete the Waste Window */
  /* if it still exists. The Waste Window is the only window which must */
  /* not remain alone on the screen. */
  
  
  switch (status){
  case DESTROY_CHECKING: 
  
    app = first_app;
    while (app != NULL) {
      /* If not the Window for which the delete event was received */
      /* and not a Waste Window, then we have at least another Window. */
      if ( (app->frame != frame) &&
           (app->type != WASTE_WINDOW) ) {
          /* printf("FOUND NON-WASTE_WINDOW*\n"); */
        one_frame_left = FALSE;
      } else {
  
        /* Did we find the Waste Window? Keep it handy to use it later. */
        if (app->type == WASTE_WINDOW) {
          /* printf("FOUND WASTE_WINDOW**********\n"); */
        }
      }
      app = (Applic_T *) app->next;
    } /* while */

    /* Only one Window is left. The user is Quitting the filemanager. */ 
    /* Ask the user if we should really quit. */
    /* if ( (one_frame_left) && (first_app->next != NULL) ) */
    if ( (one_frame_left) && (first_app->type != WASTE_WINDOW) )
    {
      
      /* The Frame must go busy when prompting the user with a notice. */
      xv_set(frame, FRAME_BUSY, TRUE, NULL);
  
      /* This small delay (a few Micro-seconds) is needed to get the frame */
      /* look busy just fine, otherwise, it may not look BUSY. */
      XSync(first_app->display, False);
      
      /* Flush the input buffer to prevent unattended exit of xvfilemgr. */
      xv_set ( XVfilemgr_Server,
                                SERVER_SYNC_AND_PROCESS_EVENTS,
                                NULL);
    
      /* Prompt the user with a quit confirmation notice. */
      quit_notice = (Xv_Notice) xv_create(first_app->frame, NOTICE,
                        NOTICE_LOCK_SCREEN, TRUE,
                        NOTICE_MESSAGE_STRINGS,
                          "Are you sure you want to quit File Manager?",
                          NULL,
                        NOTICE_BUTTON_YES, "Quit File Manager",
                        NOTICE_BUTTON_NO, "Cancel",
                        NOTICE_NO_BEEPING, TRUE,
                        NOTICE_STATUS, &notice_result,
                        XV_SHOW, TRUE,
                        NULL);  
                                
      /* The frame must not be busy anymore. */
      xv_set(frame, FRAME_BUSY, FALSE, NULL);
      xv_destroy_safe (quit_notice);

      /* if the user confirms deletion, we can now proceed to find the */
      /* Waste Window, destroy it and let the rest of the function do the */
      /* rest of deleting the last frame. */
      switch(notice_result) {  
      case NOTICE_YES:
        if ((first_app != NULL) && (first_app->next != NULL)) {
                        
          app = first_app;
          if (app->type == WASTE_WINDOW) {
            /* destroy first frame */
            xv_set (app->frame, XV_SHOW, FALSE, NULL);
            xv_destroy_safe (app->frame);
            first_app = (Applic_T *) first_app->next;
            switch (app->type) {
            case CDROM_WINDOW:
              remove_cdrom_umount (app);
              break;
            case FLOPPY_WINDOW:
              remove_floppy_umount (app);
              break;
            } /* switch */   
            free (app);
          } /* if */
          else {
            while (app->type != WASTE_WINDOW) {
              app2 = app;
              app = (Applic_T *) app->next;
            } /* while */
            if (app != NULL) {
              xv_set (app->frame, XV_SHOW, FALSE, NULL);
              xv_destroy_safe (app->frame);
            } /* if */
            app2->next = app->next;  
            switch (app->type) {
            case CDROM_WINDOW:
              remove_cdrom_umount (app);
              break;
            case FLOPPY_WINDOW:
              remove_floppy_umount (app);
              break;
            } /* switch */   
            free (app);
          } /* else */
        } else {
          /* Here we got the notice but we had less than two windows */
          /* or there was an error...*/
        } /* else */
        break;
      case NOTICE_NO:
        notify_veto_destroy(client);
        quit_confirm = FALSE;
        break;
      default:
        notify_veto_destroy(client);
        quit_confirm = FALSE;
        break;
      } /* switch */   
          
    } /* if one_frame_left */
    break;

  case DESTROY_CLEANUP:
    frame = (Frame) client;
    xv_set (frame, XV_SHOW, FALSE, NULL);
    xv_destroy_safe (frame);
    app = first_app;
    if (app->frame == frame) {
      /* destroy first frame */
      first_app = (Applic_T *) first_app->next;
      switch (app->type) {
      case CDROM_WINDOW:
	remove_cdrom_umount (app);
	break;
      case FLOPPY_WINDOW:
	remove_floppy_umount (app);
	break;
      } /* switch */
      free (app);
    } /* if */
    else {
      while (app->frame != frame) {
        app2 = app;
        app = (Applic_T *) app->next;
      } /* while */
      switch (app->type) {
      case CDROM_WINDOW:
	remove_cdrom_umount (app);
	break;
      case FLOPPY_WINDOW:
	remove_floppy_umount (app);
	break;
      } /* switch */
      free (app);
      app2->next = app->next;
    } /* else */
    if (first_app != NULL) {
      /* install new update routine */
      props = get_properties ();
      timer.it_value.tv_sec = 1;
      timer.it_interval.tv_sec = props->update;
      notify_set_itimer_func((Notify_client) first_app->frame, 
                             update_window_notify, 
                             ITIMER_REAL, &timer, NULL);
      return notify_next_destroy_func (client, status);
    } /* if */
    else {
      /* last frame removed */
      exit (0);
    } /* else */
    break;
  default:
    return NOTIFY_DONE;
    break;
  } /* switch */

} /* xvfmgr_frame_delete */



static void
xvfilemgr_quit (Menu menu, Menu_item menu_item)
{
  Applic_T *app;
  Xv_Notice    quit_notice;
  int notice_result;

  /* The Frame must go busy when prompting the user with a notice. */
  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  xv_set(app->frame, FRAME_BUSY, TRUE, NULL);

  /* This small delay (a few Micro-seconds) is needed to get the frame */
  /* look busy just fine, otherwise, it may not look BUSY. */
  XSync(app->display, False);

  /* Flush the input buffer to prevent unattended use of the notice. */
  xv_set ( XVfilemgr_Server,
                            SERVER_SYNC_AND_PROCESS_EVENTS,
                            NULL);

  /* Prompt the user with a quit confirmation notice. */
  quit_notice = (Xv_Notice) xv_create( first_app->frame, NOTICE,
                    NOTICE_LOCK_SCREEN, TRUE,
                    NOTICE_MESSAGE_STRINGS,
                      "Are you sure you want to quit File Manager?",
                      NULL,
                    NOTICE_BUTTON_YES, "Quit File Manager",
                    NOTICE_BUTTON_NO, "Cancel",
                    NOTICE_NO_BEEPING, TRUE,
                    NOTICE_STATUS, &notice_result,
                    XV_SHOW, TRUE,
                    NULL);

  /* The frame must not be busy anymore. */
  xv_set(app->frame, FRAME_BUSY, FALSE, NULL);

  switch(notice_result) {
  case NOTICE_YES:
    edit_exit ();
    exit (0);
    break;
  default:
    break;
  }

  xv_destroy_safe (quit_notice);
} /* xvfilemgr_quit */



static void
command_notify (Panel_item item)
{
  Applic_T    *app;

  app = (Applic_T *) xv_get (item, XV_KEY_DATA, COMMAND_APP);
  xv_set (command_menu, 
	  MENU_CLIENT_DATA, app,
	  NULL);
} /* command_notify */


static void
goto_notify (Panel_item item)
{
  struct stat statbuf;
  Applic_T    *app;
  char        *path;
  char        *dir = NULL;
  Panel_item  goto_file;

  app = (Applic_T *) xv_get (item, XV_KEY_DATA, GOTO_APP);
  goto_file = (Panel_item) xv_get (item, XV_KEY_DATA, GOTO_FILE);
  xv_set (goto_menu, 
	  MENU_CLIENT_DATA, app,
/*	  MENU_GEN_PIN_WINDOW, app->frame, "Goto",*/
	  NULL);

  /* Retrieve the Path for the Text Panel...*/
  path = (char *) xv_get (goto_file, PANEL_VALUE);

  /* Expand the Path to look for paths like "~userid" or "$ENV" */
  dir = (char *) expand_dirname(path, app);
  if (dir == NULL) {
    /* fprintf(stderr,
    "XVFilemgr: out of memory analysing path or other error.\n"); */
    return;
  } else {
    strcpy(path,dir);
  } /* else */

  if (stat (path, &statbuf) == 0) {
    change_directory (app, path);
    xv_set (goto_file, PANEL_VALUE, "" , NULL);
    app->changed_dir = 1;
  } else {
    /* xv_set(app->frame,
            FRAME_LEFT_FOOTER, "Cannot stat requested path.",
            NULL); */
    app->changed_dir = 0;
    return;
  } /* else */
} /* goto_notify */

static void
goto_notify_path(Panel_item item)
{
  char    *dir;
  struct stat statbuf;
  Applic_T    *app; 
  char        *path;
  Panel_item  goto_file;

  app = (Applic_T *) xv_get (item, XV_KEY_DATA, GOTO_APP);
  goto_file = (Panel_item) xv_get (app->frame, XV_KEY_DATA, GOTO_FILE);  
  xv_set (goto_menu, 
	  MENU_CLIENT_DATA, app,
/*	  MENU_GEN_PIN_WINDOW, app->frame, "Goto",*/
	  NULL);

  xv_set(app->frame,
	  FRAME_LEFT_FOOTER, "",
	  NULL);

  /* Retrieve the Path for the Text Panel...*/
  path = (char *) xv_get (goto_file, PANEL_VALUE);

  /* Expand the Path to look for paths like "~userid" or "$ENV" */
  dir = (char *) expand_dirname(path, app);
  if (dir == NULL) {
    /* fprintf(stderr,
    "XVFilemgr: out of memory analysing path or other error.\n"); */
    return;
  } else {
    strcpy(path,dir);
  } /* else */

  /* Do not allow the user to enter a null path. */
  if (*path == '\0') {
    xv_set(app->frame,
	    FRAME_LEFT_FOOTER, "Please type in a pathname first.",
	    NULL);
    return;
  }
  if (stat (path, &statbuf) == 0) {
    change_directory (app, path);
    xv_set (goto_file, PANEL_VALUE, "", NULL);
    app->changed_dir = 1;
  } else {
    /* xv_set(app->frame,
            FRAME_LEFT_FOOTER, "Cannot stat requested path.",
            NULL); */
    app->changed_dir = 0;
    return;
  } /* else */
} /* goto_notify_path */



static void
notify_stub (Menu menu, Menu_item menu_item)
{
} 


extern Applic_T *
xvfmgr_new_frame (int type)
{
  /* Variablen für xvfmgr-Panel */
  Panel      xvfmgr_panel;
  Panel_item about;
  Panel_item file;
  Panel_item file_msg;
  Panel_item view;
  Panel_item edit;
  Panel_item prop;
  Panel_item empty;      /* only for wastebasket */
  Panel_item eject;      /* only for cdrom */
  Panel_item got;
  Panel_item goto_file;
  Menu       open_menu;
  Menu       file_menu;
  Menu       view_menu;
  Menu_item  blank_item;
  Rect       label_rect;
  Display    *display;
  Server_image image;
  Server_image imagemask;
  Applic_T     *app;
  Properties_T *props;
  Xv_Window    window;
  int        i;
  
  props = get_properties ();
  app = malloc (sizeof(Applic_T) + 1);
  app->type = type;

  app->frame = (Frame) xv_create (XV_NULL, FRAME,
				  FRAME_MIN_SIZE, 420, 310,
				  FRAME_SHOW_FOOTER, TRUE,
				  FRAME_LEFT_FOOTER, "",
				  FRAME_RIGHT_FOOTER, "",
				  FRAME_LABEL, "File Manager Initializing...",
				  XV_KEY_DATA_REMOVE_PROC, free_data,
				  XV_HEIGHT, 320,
				  XV_WIDTH, 620,
				  NULL);
  switch (type) {
  case FOLDER_WINDOW:
    rect_construct (&label_rect, 0, 54, 64, 10);
    image = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				      XV_WIDTH, xvfmgricon_width,
				      XV_HEIGHT, xvfmgricon_height,
				      SERVER_IMAGE_X_BITS, xvfmgricon_bits,
				      NULL);
    imagemask = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
					  XV_WIDTH, xvfmgrmask_width,
					  XV_HEIGHT, xvfmgrmask_height,
					  SERVER_IMAGE_X_BITS, xvfmgrmask_bits,
					  NULL);
    app->icon = (Icon) xv_create (app->frame, ICON,
				  ICON_IMAGE, image,
				  ICON_MASK_IMAGE, imagemask,
				  ICON_LABEL_RECT, &label_rect,
				  XV_LABEL, "Xvfmgr",
				  NULL);

    break;
  case WASTE_WINDOW:
    app->icon = wasteicon;
    xv_set (app->frame, FRAME_CLOSED, TRUE, NULL);
    break;
  case CDROM_WINDOW:
    rect_construct (&label_rect, 0, 54, 64, 10);
    image = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				      XV_WIDTH, cdromicon_width,
				      XV_HEIGHT, cdromicon_height,
				      SERVER_IMAGE_X_BITS, cdromicon_bits,
				      NULL);
    imagemask = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
					  XV_WIDTH, cdrommask_width,
					  XV_HEIGHT, cdrommask_height,
					  SERVER_IMAGE_X_BITS, cdrommask_bits,
					  NULL);
    app->icon = (Icon) xv_create (app->frame, ICON,
				  ICON_IMAGE, image,
				  ICON_MASK_IMAGE, imagemask,
				  ICON_LABEL_RECT, &label_rect,
				  XV_LABEL, "CD-ROM",
				  NULL);

    break;
  case FLOPPY_WINDOW:
    rect_construct (&label_rect, 0, 54, 64, 10);
    image = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				      XV_WIDTH, floppyicon_width,
				      XV_HEIGHT, floppyicon_height,
				      SERVER_IMAGE_X_BITS, floppyicon_bits,
				      NULL);
    imagemask = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
					  XV_WIDTH, floppymask_width,
					  XV_HEIGHT, floppymask_height,
					  SERVER_IMAGE_X_BITS, floppymask_bits,
					  NULL);
    app->icon = (Icon) xv_create (app->frame, ICON,
				  ICON_IMAGE, image,
				  ICON_MASK_IMAGE, imagemask,
				  ICON_LABEL_RECT, &label_rect,
				  XV_LABEL, "Floppy",
				  NULL);
    break;
  } /* switch */

  xv_set (app->frame, FRAME_ICON, app->icon, NULL);

  xvfmgr_panel = (Panel) xv_create (app->frame, PANEL, NULL);

  if (type != WASTE_WINDOW) {
    open_menu =  (Menu) xv_create (XV_NULL, MENU,
				   MENU_CLIENT_DATA, app,
				   MENU_ITEM,
				     MENU_STRING, "File",
				     MENU_NOTIFY_PROC, file_menu_open,
				   NULL,
				   MENU_ITEM,
				     MENU_STRING, "With 'Goto' Arguments",
				     MENU_NOTIFY_PROC, file_menu_gotoarg,
				   NULL,
				   MENU_ITEM,
				     MENU_STRING, "In Document Editor",
				     MENU_NOTIFY_PROC, file_menu_textedit,
				   NULL,
				   NULL);
  
    file_menu =  (Menu) xv_create (XV_NULL, MENU,
				   MENU_CLIENT_DATA, app,
				   MENU_GEN_PIN_WINDOW, app->frame, "File",
				   NULL);
    app->file_open = (Menu_item) xv_create (XV_NULL, MENUITEM,
					    MENU_STRING, "Open",
					    MENU_PULLRIGHT, open_menu,
					    NULL);
    app->file_duplicate = (Menu_item) xv_create (XV_NULL, MENUITEM,
					 MENU_STRING, "Duplicate",
					 MENU_NOTIFY_PROC, file_duplicate,
					 NULL);
    app->file_information = (Menu_item) xv_create (XV_NULL, MENUITEM,
					   MENU_STRING, "Information...",
					   MENU_NOTIFY_PROC, file_info_menu,
					   NULL);
    xv_set (file_menu,
	    MENU_APPEND_ITEM, app->file_open, 
	    NULL);
    if (type != CDROM_WINDOW) {
      /* CDROMs are read only so there is no need for this */
      xv_set (file_menu,
	      MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
					MENU_STRING, "Create Folder",
					MENU_NOTIFY_PROC, dir_create,
					NULL),
	      MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
					MENU_STRING, "Create Document",
					MENU_NOTIFY_PROC, file_create,
					NULL),
	      MENU_APPEND_ITEM, app->file_duplicate,
	      MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
						       MENU_STRING, "",
						       MENU_INACTIVE, TRUE,
						       NULL),
	      NULL);
    } /* if */
    xv_set (file_menu,
	    MENU_APPEND_ITEM, app->file_information,
	    MENU_APPEND_ITEM, (Menu_item) xv_create(XV_NULL, MENUITEM,
				       MENU_STRING, "Find...",
				       MENU_NOTIFY_PROC, find_menu,
				       NULL),
	    MENU_APPEND_ITEM, (Menu_item) xv_create(XV_NULL, MENUITEM,
				       MENU_STRING, "Remote Transfer",
				       MENU_NOTIFY_PROC, remote_menu,
				       NULL),
	    MENU_APPEND_ITEM, (Menu_item) xv_create(XV_NULL, MENUITEM,
				       MENU_STRING, "Commands",
				       MENU_PULLRIGHT, command_menu,
				       NULL),
	    NULL);

    if (type == FOLDER_WINDOW) {
      xv_set (file_menu,
	      MENU_CLIENT_DATA, app,
	      MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
						       MENU_STRING, "",
						       MENU_INACTIVE, TRUE,
						       NULL),
	      MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
			       MENU_STRING, "Check For CD-Rom",
			       MENU_NOTIFY_PROC, remove_check_for_cdrom,
			       NULL),
	      MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
			       MENU_STRING, "Check For Floppy",
			       MENU_NOTIFY_PROC, remove_check_for_floppy,
			       NULL),
	      MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
			       MENU_STRING, "Format Floppy",
			       MENU_NOTIFY_PROC, remove_format_floppy,
			       NULL),
	      NULL);
    } /* if */
    xv_set (file_menu,
	    MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
						     MENU_STRING, "",
						     MENU_INACTIVE, TRUE,
						     NULL),
	    MENU_APPEND_ITEM, (Menu_item) xv_create(XV_NULL, MENUITEM,
				       MENU_STRING, "Credits...",
				       MENU_CLIENT_DATA, app,
				       MENU_NOTIFY_PROC, about_notify,
				       NULL),
	    MENU_APPEND_ITEM, (Menu_item) xv_create(XV_NULL, MENUITEM,
				       MENU_STRING, "Quit File Manager",
				       MENU_NOTIFY_PROC, xvfilemgr_quit,
				       NULL),
	    NULL);
    nofile_selected (app);
    
    file = xv_create (xvfmgr_panel, PANEL_BUTTON,
		      PANEL_LABEL_STRING, "File",
		      PANEL_ITEM_MENU, file_menu,
		      PANEL_NOTIFY_PROC, command_notify,
		      XV_KEY_DATA, COMMAND_APP, app,
		      NULL);
  } /* if */

  view_menu =  (Menu) xv_create (XV_NULL, MENU,
			     MENU_CLIENT_DATA, app,
			     MENU_GEN_PIN_WINDOW, app->frame, "View",
			     MENU_ITEM,
			       MENU_STRING, "Open Folder View",
			       MENU_NOTIFY_PROC, dir_folder_view,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "",
			       MENU_INACTIVE, TRUE,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "Small Icons",
			       MENU_NOTIFY_PROC, ww_small_icons,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "Large Icons",
			       MENU_NOTIFY_PROC, ww_large_icons,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "",
			       MENU_INACTIVE, TRUE,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "Icon by Name",
			       MENU_NOTIFY_PROC, ww_sort_name,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "Icon by Type",
			       MENU_NOTIFY_PROC, ww_sort_type,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "",
			       MENU_INACTIVE, TRUE,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "List by Name",
			       MENU_NOTIFY_PROC, ww_list_name,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "List by Type",
			       MENU_NOTIFY_PROC, ww_list_type,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "List by Size",
			       MENU_NOTIFY_PROC, ww_list_size,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "List by Date",
			       MENU_NOTIFY_PROC, ww_list_date,
			       NULL,
			     NULL);

  view = xv_create (xvfmgr_panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "View",
		    PANEL_ITEM_MENU, view_menu,
		    NULL);

  edit_menu =  (Menu) xv_create (XV_NULL, MENU,
				 MENU_CLIENT_DATA, app,
				 MENU_ITEM,
				   MENU_STRING, "Select All",
				   MENU_NOTIFY_PROC, file_select_all,
				   NULL,
				 MENU_ITEM,
				   MENU_STRING, "",
				   MENU_INACTIVE, TRUE,
				   NULL,
				 NULL);
  app->edit_cut = (Menu_item) xv_create (XV_NULL, MENUITEM,
					 MENU_STRING, "Cut",
					 MENU_NOTIFY_PROC, edit_cut,
					 NULL);
  app->edit_copy = (Menu_item) xv_create (XV_NULL, MENUITEM,
					  MENU_STRING, "Copy",
					  MENU_NOTIFY_PROC, edit_copy,
					  NULL);
  app->edit_link = (Menu_item) xv_create (XV_NULL, MENUITEM,
					  MENU_STRING, "Link",
					  MENU_NOTIFY_PROC, edit_link,
					  NULL);
  app->edit_paste = (Menu_item) xv_create (XV_NULL, MENUITEM,
					   MENU_STRING, "Paste",
					   MENU_NOTIFY_PROC, edit_paste,
					   NULL);
  app->edit_delete = (Menu_item) xv_create (XV_NULL, MENUITEM,
					    MENU_STRING, "Delete",
					    MENU_NOTIFY_PROC, file_delete,
					    NULL);

  prop = (Menu_item) xv_create (XV_NULL, MENUITEM,
		      MENU_STRING, "Properties...",
		      MENU_NOTIFY_PROC, properties_notify,
		      MENU_CLIENT_DATA, app,
		      NULL);
  xv_set (edit_menu,
	  MENU_APPEND_ITEM, app->edit_cut,
	  MENU_APPEND_ITEM, app->edit_copy,
	  MENU_APPEND_ITEM, app->edit_link,
	  MENU_APPEND_ITEM, app->edit_paste,
	  MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
						   MENU_STRING, "",
						   MENU_INACTIVE, TRUE,
						   NULL),
	  MENU_APPEND_ITEM, app->edit_delete,
	  MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
						   MENU_STRING, "",
						   MENU_INACTIVE, TRUE,
						   NULL),
	  MENU_APPEND_ITEM, prop,
	  NULL);

  edit = xv_create (xvfmgr_panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Edit",
		    PANEL_ITEM_MENU, edit_menu,
		    NULL);

  if (type != WASTE_WINDOW) {
/*    prop = xv_create (xvfmgr_panel, PANEL_BUTTON,
		      PANEL_LABEL_STRING, "Properties...",
		      PANEL_NOTIFY_PROC, properties_notify,
		      PANEL_CLIENT_DATA, app,
		      NULL); */

    got = xv_create (xvfmgr_panel, PANEL_BUTTON,
		     PANEL_LABEL_STRING, "Go To:",
		     PANEL_ITEM_MENU, goto_menu,
		     PANEL_NOTIFY_PROC, goto_notify,
		     XV_KEY_DATA, GOTO_APP, app,
		     NULL);
    
    goto_file = xv_create (xvfmgr_panel, PANEL_TEXT,
			   PANEL_LABEL_STRING, "",
			   PANEL_VALUE, "",
			   PANEL_VALUE_DISPLAY_LENGTH, 20,
			   PANEL_VALUE_STORED_LENGTH, PATH_MAX,
			   PANEL_READ_ONLY, FALSE,
			   PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
			   PANEL_NOTIFY_PROC, goto_notify_path,
			   XV_KEY_DATA, GOTO_APP, app,
			   NULL);
    xv_set (got, XV_KEY_DATA, GOTO_FILE, goto_file, NULL);
  } /* if */
  else {
    (void) xv_create (xvfmgr_panel, PANEL_BUTTON,
		      PANEL_LABEL_STRING, "Empty Waste",
		      PANEL_NOTIFY_PROC, waste_empty, 
		      XV_KEY_DATA, EMPTY_APP, app,
		      NULL);
  } /* else */
  if (type == CDROM_WINDOW)
    (void) xv_create (xvfmgr_panel, PANEL_BUTTON,
		      PANEL_LABEL_STRING, "Eject",
		      PANEL_NOTIFY_PROC, remove_eject_cdrom, 
		      XV_KEY_DATA, EJECT_APP, app,
		      XV_X, 475,
		      XV_Y, 107,
		      NULL);

  /* create an invisible drop target which receives default drops */
  (void) xv_create (xvfmgr_panel, PANEL_DROP_TARGET,
		    PANEL_NOTIFY_PROC, file_frame_drop,
		    PANEL_DROP_SITE_DEFAULT, TRUE,
		    PANEL_DROP_WIDTH, 1,
		    PANEL_DROP_HEIGHT, 1,
		    XV_KEY_DATA, PANEL_APP, app,
		    XV_X, 0,
		    XV_Y, 30,
		    NULL);

  app->path_canvas = (Canvas_shell) xv_create (app->frame, CANVAS_SHELL,
				       CANVAS_AUTO_EXPAND, FALSE,
				       CANVAS_AUTO_SHRINK, FALSE,
				       CANVAS_SHELL_BATCH_REPAINT, TRUE,
				       CANVAS_SHELL_AUTO_DROP_SITE, TRUE,
				       RECTOBJ_ACCEPTS_DROP, TRUE,
				       RECTOBJ_DROP_PROC, file_default_drop,
				       WIN_RETAINED, TRUE,
				       XV_HEIGHT, 75,
				       XV_X, 0,
				       XV_Y, 30,
				       XV_KEY_DATA, CANVAS_APP, app,
				       NULL);
  app->path_scroll = (Scrollbar) xv_create (app->path_canvas, SCROLLBAR,
			       SCROLLBAR_DIRECTION, 
			       SCROLLBAR_HORIZONTAL,
			       SCROLLBAR_SPLITTABLE, FALSE,
			       NULL);

  file_msg = xv_create (xvfmgr_panel, PANEL_MESSAGE,
			PANEL_LABEL_BOLD, FALSE,
			PANEL_LABEL_STRING, " ",
			XV_X, 10,
			XV_Y, 110,
			NULL);
  app->file_canvas = (Canvas_shell) xv_create (app->frame, CANVAS_SHELL,
				  CANVAS_AUTO_EXPAND, FALSE,
				  CANVAS_AUTO_SHRINK, FALSE,
				  CANVAS_SHELL_BATCH_REPAINT, TRUE,
			          CANVAS_SHELL_AUTO_DROP_SITE, TRUE,
				  WIN_RETAINED, TRUE,
                                  RECTOBJ_MENU, edit_menu,
				  RECTOBJ_ACCEPTS_DROP, TRUE,
				  RECTOBJ_DROP_PROC, file_default_drop,
				  XV_X, 0,
				  XV_Y, 135,
				  XV_KEY_DATA, CANVAS_APP, app,
				  XV_KEY_DATA, FILE_MSG, file_msg,
				  NULL);
  app->file_scroll_v = (Scrollbar) xv_create (app->file_canvas, SCROLLBAR,
				 SCROLLBAR_DIRECTION, 
				 SCROLLBAR_VERTICAL,
				 NULL);
  app->file_scroll_h = (Scrollbar) xv_create (app->file_canvas, SCROLLBAR,
				 SCROLLBAR_DIRECTION, 
				 SCROLLBAR_HORIZONTAL,
				 NULL);

  notify_interpose_destroy_func (app->frame, xvfmgr_frame_delete);

  app->display = (Display *) xv_get (app->frame, XV_DISPLAY);

  window_fit (app->frame); 

  xv_set (app->frame, 
	  XV_KEY_DATA, GOTO_MENU, goto_menu, 
	  XV_KEY_DATA, GOTO_FILE, goto_file, 
	  NULL);
  xv_set (app->frame, XV_SHOW, TRUE, NULL);
  /* initialize path icon objects */
  for (i = 0; i < MAX_PATH; i++)
    app->pathicon[i] = (Rectobj) NULL;
  app->changed_dir = 0;
  app->select = NULL;  /* no selected files */
  app->next = NULL;    /* first application frame */
  app->files = NULL;

  return app;
} /* xvfmgr_new_frame */


static Applic_T *
xvfmgr_init ()
{
  Applic_T *app;

  goto_menu =  (Menu) xv_create (XV_NULL, MENU,
				 MENU_ITEM,
			           MENU_STRING, "",
			           MENU_INACTIVE, TRUE,
			           NULL,
				 NULL);
  command_menu =  (Menu) xv_create (XV_NULL, MENU, NULL);
  props_menu_set_menu ();
  props_command_set_menu ();
  app = xvfmgr_new_frame (FOLDER_WINDOW);
  /* initialize client dat for goto menu */
  xv_set (goto_menu, MENU_CLIENT_DATA, app, NULL);
  list_font = (Xv_Font) xv_find (app->frame, FONT, 
			 FONT_NAME , 
			 "-*-lucidatypewriter-medium-*-*-*-12-*-*-*-*-*-*-*",
			 NULL);
  if (!list_font) {
    fprintf (stderr, 
     "XVFilemgr: Can't find font lucidatypewriter-12, using default\n");
  } /* if */
  workwindow_init (app);
  file_init (app);
  find_init (app);
  remote_init (app);
  edit_init (app);
  about_init (app);
  feedback_init (app);
  return app;
} /* xvfmgr_init */


int
main (int argc, char *argv[])
{
  xv_init (XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);
  HOME = malloc (strlen (getenv ("HOME")) + 1);
  strcpy (HOME, getenv ("HOME"));
  first_app = NULL;
  properties_init ();
  dir_init ();
  waste_init ();
  mount_init ();
  first_app = xvfmgr_init ();
  XVfilemgr_Screen = (Xv_Screen) xv_get (first_app->frame, XV_SCREEN);
  XVfilemgr_Server = (Xv_Server) xv_get (XVfilemgr_Screen, SCREEN_SERVER);
  xv_main_loop (first_app->frame);
  exit (0);
} /* main */
