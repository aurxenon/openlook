/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: file.h,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:30:58 $
 *	Purpose : all tasks concerning a single file
 *
 * $Log: file.h,v $
 * Revision 1.1  2005/07/13 18:30:58  arkenoi
 * Initial revision
 *
 * Revision 1.15  1996/11/23 12:37:18  root
 * file_move handles name conflicts in target directory.
 *
 * Revision 1.14  1996/10/26 12:42:08  root
 * count the number of files in the selection for drag and drop objects.
 *
 * Revision 1.13  1996/07/28 08:54:25  root
 * new file_file function returning the output from file(1).
 *
 * Revision 1.12  1996/07/18 19:34:01  root
 * Changing of the file name in responds to a direct editing of the icon text.
 *
 * Revision 1.11  1996/05/27 20:02:40  root
 * support for special device files and named pipes.
 *
 * Revision 1.10  1996/05/27 19:19:10  root
 * added filetype for a link to a directory.
 *
 * Revision 1.9  1996/05/27 19:07:01  root
 * name conflict with system file.
 *
 * Revision 1.8  1996/05/27 19:01:25  root
 * added filetype for links.
 *
 * Revision 1.7  1996/05/27 16:24:36  root
 * rename of file_move2 becase there is only one.
 *
 * Revision 1.6  1996/05/27 10:35:59  root
 * prototype for file_select_all.
 *
 * Revision 1.5  1996/05/26 15:14:50  root
 * prototype for file_delete2.
 *
 * Revision 1.4  1996/05/26 14:27:16  root
 * added extern file_move for edit.c.
 *
 * Revision 1.3  1996/05/26 13:23:43  root
 * added file_copy and file_link.
 *
 * Revision 1.2  1996/05/19 19:07:55  root
 * bug fix.
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#ifndef __FIL__
#define __FIL__

#include <X11/Xlib.h>

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/dragdrop.h>
#include <xview/sel_pkg.h>
#include <sspkg/canshell.h>
#include <sspkg/rectobj.h>
#include <sspkg/drawobj.h>

#include "global.h"

Filetype_T     *directory;
Filetype_T     *regular;
Filetype_T     *allfile;
Filetype_T     *execute;
Filetype_T     *special;
Filetype_T     *fifo;
Filetype_T     *linkfile;
Filetype_T     *linkdir;
Filetype_T     *unknown;

extern void 
file_copy (Frame frame, char *from, char *to);

extern void 
file_link (Frame frame, char *from, char *to);

extern void 
file_move (Frame frame, char *from, char *to, char *as);

extern void
file_delete2 (Frame frame, char *name);

extern void
file_file (Frame, char *name, char *output);

extern void
file_selected (Applic_T *app);

extern void
nofile_selected (Applic_T *app);

extern void 
file_change_name (Drawtext text);

extern void
file_select (Rectobj object, int selected, Event *event);

extern void
file_select_all (Menu menu, Menu_item menu_item);

extern void
file_open (Xv_Window paint_window, Event *event, 
	   Canvas_shell canvas, Rectobj object);

extern void
file_menu_open (Menu menu, Menu_item menu_item);

extern void
file_menu_gotoarg (Menu menu, Menu_item menu_item);

extern void
file_menu_textedit (Menu menu, Menu_item menu_item);

extern void
file_command (Menu menu, Menu_item menu_item);

extern void
file_duplicate (Menu menu, Menu_item menu_item);

extern void
file_create (Menu menu, Menu_item menu_item);

extern void
file_delete (Menu menu, Menu_item menu_item);

extern int
file_frame_drop (Panel_item item, int value, Event *event);

extern void
file_default_drop (Xv_Window window, Event *event, Canvas_shell canvas,
                   Rectobj object);

extern void
file_drop (Xv_window window, Event *event, Canvas_shell canvas, 
	   Rectobj object);

extern Selection_item
file_selection (Applic_T *app, Dnd dnd_object, int *count);

extern void
file_information (Frame frame, File_T *file, char *path);

extern void
file_info_menu (Menu menu, Menu_item menu_item);

extern void
file_init (Applic_T *app);

#endif /* __FIL__ */
