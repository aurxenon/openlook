/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: directory.c,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:13 $
 *	Purpose : all tasks concerning directories, folder view
 *
 * $Log: directory.c,v $
 * Revision 1.1.1.1  2005/07/13 18:31:13  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.15  1999/08/08 19:56:36  root
 * No Idea.
 *
 * Revision 1.14  1999/01/02 19:14:53  root
 * Some cosmetic changes, I really need to break up the large trees.
 *
 * Revision 1.13  1996/10/13 16:53:39  root
 * bug fix.
 *
 * Revision 1.12  1996/10/08 16:34:14  root
 * eliminating all waitpid system calls.
 *
 * Revision 1.11  1996/07/26 19:48:42  root
 * change some constants.
 *
 * Revision 1.10  1996/07/24 20:10:07  root
 * Wait updating the workwindow till new diretory is created.
 *
 * Revision 1.9  1996/06/23 17:12:13  root
 * New error handling.
 *
 * Revision 1.8  1996/06/17 19:36:29  root
 * file menu completed: file->open and file->information added.
 *
 * Revision 1.7  1996/06/07 15:45:00  root
 * activating/deactivating menu items.
 *
 * Revision 1.6  1996/05/27 17:10:44  root
 * update the display window after creating a new empty folder.
 *
 * Revision 1.5  1995/12/04 20:01:51  root
 * add parent to node of folder not corresponding to
 * the root of the directory tree
 *
 * Revision 1.4  1995/12/04  19:21:10  root
 * starting folder view on node other than root
 *
 * Revision 1.3  1995/12/03  19:22:33  root
 * show previously hidden folder
 *
 * Revision 1.2  1995/12/03  17:46:22  root
 * hiding of folder subtree.
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/icon.h>
#include <xview/svrimage.h>
#include <xview/rect.h>
#include <xview/scrollbar.h>
#include <xview/notify.h>
#include <sspkg/canshell.h>
#include <sspkg/rectobj.h>
#include <sspkg/list.h>
#include <sspkg/tree.h>

#include "config.h"
#include "global.h"
#include "directory.h"
#include "file.h"
#include "properties.h"
#include "select.h"
#include "error.h"
#include "find.h"

#include "icons/folderviewicon"
#include "icons/folderviewmask"

#define DIR_NODE 250

#define FOLDER_VIEW_SHOW   3300
#define FOLDER_VIEW_HIDE   3301
#define FOLDER_VIEW_START  3302
#define FOLDER_VIEW_ADD    3303
#define FOLDER_FILE_INFO   3304
#define FOLDER_FILE_OPEN   3305
#define FOLDER_TREE        3306

typedef struct Dir_T *Dir_PTR;

typedef struct Dir_T {
  char         *name;
  FolderProp_T *fprops;
  Dir_PTR      *sub;    /* points to list of subdirectories */
  Dir_PTR      *next;   /* next directory on the same directory layer */
} Dir_T;

static Dir_T         *root;
static Selected_T    *dir_selected; /* list of selected folders */
static Frame         folder_frame;
static Canvas_shell  folder_canvas;
static Tree          folder_tree;
static Rectobj       folder_rootobj;
static Notify_client client = (Notify_client) 13;

static Notify_value
dir_wait (Notify_client client, int pid, union wait *status, 
	  struct rusage *rusage)
{
  if (WIFEXITED(*status)) {
    return NOTIFY_DONE;
  } /* if */
  return NOTIFY_IGNORED;
} /* dir_wait */


static void
dir_folder_view_create (Tree tree, Rectobj parent, Dir_T *root, char *path);

static Dir_T *
new_node (char *name, FolderProp_T *fprops)
{
  Dir_T *node;

  node = (Dir_T *) malloc (sizeof (Dir_T));
  node->fprops = (FolderProp_T *) malloc (sizeof (FolderProp_T));
  memcpy (node->fprops, fprops, sizeof (FolderProp_T));
  node->name = (char *) malloc (strlen (name) +1);
  strcpy (node->name, name);
  node->sub = NULL;
  node->next = NULL;
  return node;
} /* new_node */


static Dir_T *
search_node (Dir_T *node, char *name, FolderProp_T *fprops)
{
  Dir_T *n2;
  Dir_T *n3;

  /* are there already some subdirs? */
  if (node->sub != NULL) {
    node = (Dir_T *) node->sub;
    while (node != NULL) {
      if (strcmp (node->name, name) == 0)
	return node;
      n2 = node;
      node = (Dir_T *) node->next;
    } /* while */
    (Dir_T *) n2->next = new_node (name, fprops);
    n3 = (Dir_T *) n2->next;
  } /* if */
  else {
    /* create subdir */
    (Dir_T *) node->sub = new_node (name, fprops);
    n3 = (Dir_T *) node->sub;
  }
  return n3;
} /* search_node */


static Dir_T *
find_node (Dir_T *node, char *name)
{
  /* search for name in subdirs of node */
  if (node->sub != NULL) {
    node = (Dir_T *) node->sub;
    while (node != NULL) {
      if (strcmp (node->name, name) == 0)
	return node;
      node = (Dir_T *) node->next;
    } /* while */
  } /* if */
  return NULL;
} /* find_node */


static Dir_T *
find_root_node (Dir_T *n, Dir_T *node)
{
  Dir_T *n2;

  /* search for root node of n */
    while (node != NULL) {
      if (n == (Dir_T *) node->sub)
	return node;
      if (node->sub != NULL) {
	n2 = find_root_node (n, (Dir_T *) node->sub);
	if (n2 != NULL)
	  return n2;
      } /* if */
      node = (Dir_T *) node->next;
    } /* while */
  return NULL;
} /* find_root_node */


extern void
dir_new (char *name, FolderProp_T *fprops)
{
  char  *path;
  char  *p1;
  char  *p2;
  Dir_T *node;

  node = root;
  path = malloc (strlen (name) + 1);
  strcpy (path, name);
  p1 = path;
  p2 = strchr (path, '/');
  while (p2 != NULL) {
    *p2 = '\0';
    /* avoid adding a second node for the root dir */
    if (strlen (p1) > 0)
      node = search_node (node, p1, fprops);
    *p2 = '/';
    p2++;
    p1 = p2;
    p2 = strchr (p2, '/');
  } /* while */
  node = search_node (node, p1, fprops);
  free (path);
} /* dir_new */


extern void
dir_delete (char *name)
{
} /* dir_delete */


extern FolderProp_T *
dir_get (char *name)
{
  char  *path;
  char  *p1;
  char  *p2;
  Dir_T *node;

  node = root;
  path = malloc (strlen (name) + 1);
  strcpy (path, name);
  p1 = path;
  p2 = strchr (path, '/');
  while (p2 != NULL) {
    *p2 = '\0';
    /* avoid searching for root dir */
    if (strlen (p1) > 0) 
      node = find_node (node, p1);
    *p2 = '/';
    p2++;
    p1 = p2;
    p2 = strchr (p2, '/');
  } /* while */
  node = find_node (node, p1);
  free (path);
  return node->fprops;
} /* dir_get */


extern void
dir_set (char *name, FolderProp_T *fprops)
{
  char  *path;
  char  *p1;
  char  *p2;
  Dir_T *node;

  node = root;
  path = malloc (strlen (name) + 1);
  strcpy (path, name);
  p1 = path;
  p2 = strchr (path, '/');
  while (p2 != NULL) {
    *p2 = '\0';
    /* avoid searching for root dir */
    if (strlen (p1) > 0) 
      node = find_node (node, p1);
    *p2 = '/';
    p2++;
    p1 = p2;
    p2 = strchr (p2, '/');
  } /* while */
  node = find_node (node, p1);
  if (node != NULL) {
    free (node->fprops); 
    node->fprops = (FolderProp_T *) malloc (sizeof (FolderProp_T));
    memcpy (node->fprops, fprops, sizeof (FolderProp_T));
  } /* if */
  free (path);
} /* dir_set */


extern void
dir_create (Menu menu, Menu_item menu_item)
{
  Applic_T     *app;
  Properties_T *props;
  struct stat  buf;
  pid_t        pid;
  char         name [PATH_MAX];
  int          count = 0;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA);
  props = get_properties ();
  sprintf (name, "%s/%s", app->path, props->new_folder.name);
  while (stat (name, &buf) == 0) {
    count++;
    sprintf (name, "%s.%d", props->new_folder.name, count);
  } /* while */
  if ((pid = fork ()) < 0)
    error_message (app->frame);
  else 
    if (pid == 0)
      if (execlp ("mkdir", "mkdir", name, (char *) 0) < 0)
	error_message_name (app->frame, name);
  notify_set_wait3_func(client, dir_wait, pid);
  update_window ();
} /* dir_create */


static void
dir_select (Rectobj object, int selected, Event *event)
{
  Tree      tree;
  Menu_item show, hide, start, add, open, info;

  if (selected == TRUE) {
    /* add the selected object to the  selected list of the Applic_T struct */
    dir_selected = select_add (dir_selected, object);
  } /* if */
  else {
    /* delete the selected item */
    dir_selected = select_delete (dir_selected, object);
  } /* else */
  /* get the menu items of the frame */
  tree = (Tree) xv_get (object, XV_KEY_DATA, FOLDER_TREE);
  show = (Menu_item) xv_get (tree, XV_KEY_DATA, FOLDER_VIEW_SHOW);
  hide = (Menu_item) xv_get (tree, XV_KEY_DATA, FOLDER_VIEW_HIDE);
  start = (Menu_item) xv_get (tree, XV_KEY_DATA, FOLDER_VIEW_START);
  add = (Menu_item) xv_get (tree, XV_KEY_DATA, FOLDER_VIEW_ADD);
  open = (Menu_item) xv_get (tree, XV_KEY_DATA, FOLDER_FILE_OPEN);
  info = (Menu_item) xv_get (tree, XV_KEY_DATA, FOLDER_FILE_INFO);
  if (dir_selected == NULL) {
    /* no folder selected */
    xv_set (show, MENU_INACTIVE, TRUE, NULL);
    xv_set (hide, MENU_INACTIVE, TRUE, NULL);
    xv_set (start, MENU_INACTIVE, TRUE, NULL);
    xv_set (add, MENU_INACTIVE, TRUE, NULL);
    xv_set (open, MENU_INACTIVE, TRUE, NULL);
    xv_set (info, MENU_INACTIVE, TRUE, NULL);
  } /* if */
  else {
    /* one or more folders selected */
    xv_set (show, MENU_INACTIVE, FALSE, NULL);
    xv_set (hide, MENU_INACTIVE, FALSE, NULL);
    xv_set (start, MENU_INACTIVE, FALSE, NULL);
    xv_set (add, MENU_INACTIVE, FALSE, NULL);
    xv_set (open, MENU_INACTIVE, FALSE, NULL);
    xv_set (info, MENU_INACTIVE, FALSE, NULL);
  } /* else */
} /* dir_select */


static void
dir_information (Menu menu, Menu_item menu_item)
{
  File_T  *file;
  Dir_T   *dir;
  Rectobj object;
  char    *path;
  char    *path2;
  struct stat statbuf;
  
  file = malloc (sizeof (file));
  if (dir_selected != NULL) {
    object = dir_selected->object;
    dir = (Dir_T *) xv_get (object, XV_KEY_DATA, DIR_NODE);
    path = (char *) malloc (strlen ((char *) 
                            xv_get (object, XV_KEY_DATA, PATH)) + 1);
    strcpy (path, (char *) xv_get (object, XV_KEY_DATA, PATH));
    file->name = dir->name;
    file->icon = object;
    file->type = directory;
    if (stat (path, &statbuf) < 0)
      ;
    else {
      file->stat = statbuf;
      /* only the path to dir */
      path [strlen(path) - strlen (dir->name) - 1] = '\0';
      path2 = malloc (strlen (path) + 1);
      strcpy (path2, path);
      /* avoid memory leaks */
      path [strlen(path) - strlen (dir->name) - 1] = '/';
      free (path);
      file_information (folder_frame, file, path2);
    } /* else */
  } /* if */
} /* dir_information */


static void
dir_open (Menu menu, Menu_item menu_item)
{
  Rectobj object;

  /* open the selected directory */
  if (dir_selected != NULL) {
    object = dir_selected->object;
    new_directory ((Xv_Window) 0, (Event *) 0, folder_canvas, object);
  } /* if */  
} /* dir_open */


static void
dir_folder_free_data (Xv_object object, int key, caddr_t data)
{
  free (data);
} /* dir_folder_free_data */


static void
dir_folder_abort (Frame frame)
{
  xv_set (folder_frame, XV_SHOW, FALSE, NULL);
  xv_destroy_safe (folder_frame);
} /* dir_folder_abort */


static void
folder_dimensions ()
{
  /* set canvas dimensions according to the tree dimensions */
  xv_set (folder_canvas,
	  CANVAS_WIDTH, (int) xv_get (folder_tree, XV_WIDTH),
	  NULL);
  xv_set (folder_canvas,
	  CANVAS_HEIGHT, (int) xv_get (folder_tree, XV_HEIGHT),
	  NULL);
} /* folder_dimensions */


static void
folder_view_orientation (Menu menu, Menu_item menu_item)
{
  if (xv_get (folder_tree, TREE_LAYOUT) == TREE_LAYOUT_HORIZONTAL) {
    xv_set (folder_tree, TREE_LAYOUT, TREE_LAYOUT_VERTICAL, NULL);
    xv_set (menu_item, MENU_STRING, "Show Horizontal", NULL);
  } /* if */
  else {
    xv_set (folder_tree, TREE_LAYOUT, TREE_LAYOUT_HORIZONTAL, NULL);
    xv_set (menu_item, MENU_STRING, "Show Vertical", NULL);
  } /* else */
  folder_dimensions ();
} /* folder_view_orientation */


static Rectobj
dir_folder_add_single (Tree tree, Rectobj parent, char *name, char *full_path,
		       Dir_T *node)
{
  Rectobj object;

  object = (Rectobj) xv_create (tree, DRAWICON,
				DRAWTEXT_STRING, name,
				DRAWIMAGE_IMAGE1, folder_image,
				DRAWICON_LAYOUT_VERTICAL, TRUE,
				RECTOBJ_DRAGGABLE, TRUE,
				RECTOBJ_START_DRAG_PROC, start_drag,
				RECTOBJ_DBL_CLICK_PROC, new_directory,
				RECTOBJ_SELECTION_PROC, dir_select,
				RECTOBJ_ACCEPTS_DROP, TRUE,
				RECTOBJ_DROP_PROC, file_drop,
				XV_KEY_DATA, DIR_NODE, node,
				XV_KEY_DATA, PATH, full_path,
				XV_KEY_DATA, FOLDER_TREE, tree,
				XV_KEY_DATA_REMOVE_PROC, PATH, free_data,
				NULL);
  if ((Rectobj) tree == parent)
    /* this is the root object of the folder_tree */
    folder_rootobj = object;
  xv_set (tree, TREE_ADD_LINK, parent, object, NULL);

/* Set the drawline attributes for the links */ /*
  xv_set ( (Drawline) xv_get(tree, TREE_DRAWLINE, object),
  DRAWLINE_ARROW_STYLE, ARROW_FILLED,
  NULL ); */

  return object;
} /* dir_folder_add_single */


static int
folder_find_node (Rectobj_list *list, char *name)
{
  /* recursivly traverse the folder tree and return
     true if name has an corresponding object on screen
     false otherwise */
  if (list != NULL) {
    list_for (list) {
      if (folder_find_node ((Rectobj_list *) 
			    xv_get (folder_tree, TREE_LINK_TO_LIST, 
				    RECTOBJ_LIST_HANDLE (list)),
			    name) == TRUE)
	return TRUE;
      if (strcmp (name, (char *) xv_get (RECTOBJ_LIST_HANDLE (list), 
					 DRAWTEXT_STRING)) == 0)
	return TRUE;
    } /* list_for */
  } /* if */
  return FALSE;
} /* folder_find_node */


static void
folder_show_all (Menu menu, Menu_item menu_item)
{
  Properties_T  *props;
  Selected_T    *select;
  Dir_T         *dir;
  struct stat   statbuf;
  struct dirent *dirp;
  DIR           *dp;
  char          *path;
  char          *new_name;

  select = dir_selected;
  props = get_properties ();
  while (select != NULL) {
    dir = (Dir_T *) xv_get (select->object, XV_KEY_DATA, DIR_NODE);
    path = (char *) xv_get (select->object, XV_KEY_DATA, PATH);
    if ((dp = opendir (path)) == NULL)
      error_message (folder_frame);
    else {
      while ((dirp = readdir (dp)) != NULL) {
	if (strcmp (dirp->d_name, ".") == 0 ||
	    strcmp (dirp->d_name, "..") == 0)
	  continue;
	new_name = malloc (strlen (path) + strlen (dirp->d_name) + 2);
	if (strcmp (path, "/") == 0)
	  sprintf (new_name, "%s%s", path, dirp->d_name);
	else
	  sprintf (new_name, "%s/%s", path, dirp->d_name);
	if (stat (new_name, &statbuf) < 0)
	  ;
	else {
	  /* directory ? */
	  if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
	    /* allready there ? */
	    if (find_node (dir, dirp->d_name) == NULL) {
	      dir_new (new_name, &props->new_folder);
	      (void) dir_folder_add_single (folder_tree, select->object, 
					    dirp->d_name, new_name, 
					    find_node (dir, dirp->d_name));
	    } /* if */
	    else {
	      /* directory object created ? */
	      if (folder_find_node ((Rectobj_list *) 
				    xv_get (folder_tree, TREE_LINK_TO_LIST, 
					    folder_rootobj),
				    dirp->d_name) == FALSE)
		(void) dir_folder_add_single (folder_tree, select->object, 
					      dirp->d_name, new_name, 
					      find_node (dir, dirp->d_name));
	    } /* else */
	  } /* if */
	} /* else */
      }/* while */
    } /* if */
    select = select->next;
  } /* while */
  folder_dimensions ();
} /* folder_show_all */


static void
folder_hide_subtree (Rectobj_list *list)
{
  /* recursivly traverse the folder tree and delete all nodes */
  if (list != NULL) {
    list_for (list) {
      folder_hide_subtree ((Rectobj_list *) 
			   xv_get (folder_tree, TREE_LINK_TO_LIST, 
				   RECTOBJ_LIST_HANDLE (list)));
      xv_set (RECTOBJ_LIST_HANDLE (list), XV_SHOW, FALSE, NULL);
      xv_destroy_safe (RECTOBJ_LIST_HANDLE (list));
    } /* list_for */
  } /* if */
} /* folder_hide_subtree */


static void
folder_hide_all (Menu menu, Menu_item menu_item)
{
  Selected_T *select;
  Dir_T      *dir;

  select = dir_selected;
  dir = (Dir_T *) xv_get (select->object, XV_KEY_DATA, DIR_NODE);
  while (select != NULL) {
    dir = (Dir_T *) xv_get (select->object, XV_KEY_DATA, DIR_NODE);
    /* destroy all objects */
    folder_hide_subtree ((Rectobj_list *) xv_get (folder_tree, 
			 TREE_LINK_TO_LIST, select->object));
    select = select->next;
  } /* while */
  folder_dimensions ();
} /* folder_hide_all */


static void
folder_start_here (Menu menu, Menu_item menu_item)
{
  Dir_T      *dir;
  char       *path;
  Selected_T *select;
  Rectobj    object;

  select = dir_selected;
  if (select->object != folder_rootobj) {
    dir = (Dir_T *) xv_get (select->object, XV_KEY_DATA, DIR_NODE);
    path = malloc (strlen ((char *) xv_get (select->object, 
					    XV_KEY_DATA, PATH)) + 1);
    strcpy (path, (char *) xv_get (select->object, XV_KEY_DATA, PATH));
    /* hide complete tree */
    folder_hide_subtree ((Rectobj_list *) xv_get (folder_tree, 
				  TREE_LINK_TO_LIST, folder_rootobj));
    xv_destroy_safe (folder_rootobj);
    /* create new tree beginning with new rootobj select */
    object = dir_folder_add_single (folder_tree, folder_tree, 
				    dir->name, path, dir);
    dir_folder_view_create (folder_tree, object, (Dir_T *) dir->sub, path);
    folder_dimensions ();
  } /* if */
} /* folder_start_here */


static void
folder_add_parent (Menu menu, Menu_item menu_item)
{
  Selected_T *select;
  Rectobj    object;
  Dir_T      *dir;
  Dir_T      *dir2;
  char       *path;
  char       *path2;
  int        c;

  select = dir_selected;
  if (select->object == folder_rootobj) {
    /* only add parent to root of tree */
    dir = (Dir_T *) xv_get (select->object, XV_KEY_DATA, DIR_NODE);
    if (dir != root) {
      /* find root of dir */
      dir2 = find_root_node (dir, root);
      /* get the right path to the new root */
      path = (char *) xv_get (select->object, XV_KEY_DATA, PATH);
      c = strlen (path);
      while (path [c] != '/')
	c--;
      path [c] = '\0';
      path2 = malloc (strlen (path) + 1);
      strcpy (path2, path);
      path [c] = '/';
      /* hide complete tree */
      folder_hide_subtree ((Rectobj_list *) xv_get (folder_tree, 
				    TREE_LINK_TO_LIST, folder_rootobj));
      xv_destroy_safe (folder_rootobj);
      /* create new tree beginning with new rootobj select */
      object = dir_folder_add_single (folder_tree, folder_tree, 
				      dir2->name, path2, dir2);
      dir_folder_view_create (folder_tree, object, (Dir_T *) dir2->sub, path2);
      folder_dimensions ();
    } /* if */
  } /* if */
} /* folder_add_parent */


static void
dir_folder_view_create (Tree tree, Rectobj parent, Dir_T *root, char *path)
{
  Dir_T   *node;
  Rectobj object;
  char    *new_path;
  char    *root_dir = "/";
  char    *name;

  node = root;
  while (node != NULL) {
    new_path = (char *) malloc (strlen (path) + strlen (node->name) + 2);
    if (strlen (node->name) > 0) {
      if (strcmp (path, "/") == 0)
	sprintf (new_path, "%s%s", path, node->name);
      else
	sprintf (new_path, "%s/%s", path, node->name);
      name = node->name;
    } /* if */
    else {
      name = root_dir;
      strcpy (new_path, root_dir);
    } /* else */
    object = dir_folder_add_single (tree, parent, name, new_path, node);
    if (node->sub != NULL) 
      dir_folder_view_create (tree, object, (Dir_T *) node->sub, new_path);
    node = (Dir_T *) node->next;
  } /* while */
  folder_dimensions ();
} /* dir_folder_view_create */


extern void
dir_folder_view (Menu menu, Menu_item menu_item)
{
  Panel        folder_panel;
  Panel_item   folder_file;
  Panel_item   folder_view;
  Panel_item   folder_goto;
  Menu         folder_file_menu;
  Menu         folder_view_menu;
  Menu_item    folder_file_open;
  Menu_item    folder_file_info;
  Menu_item    folder_view_show;
  Menu_item    folder_view_hide;
  Menu_item    folder_view_start;
  Menu_item    folder_view_add;
  Icon         folder_icon;
  Scrollbar    folder_horizontal;
  Scrollbar    folder_vertical;
  Rect         label_rect;
  Display      *display;
  Server_image image;
  Server_image imagemask;
  Applic_T     *app;

  app = (Applic_T *) xv_get (menu,  MENU_CLIENT_DATA);
  rect_construct (&label_rect, 0, 54, 64, 10);
  image = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				    XV_WIDTH, folderviewicon_width,
				    XV_HEIGHT, folderviewicon_height,
				    SERVER_IMAGE_X_BITS, folderviewicon_bits,
				    NULL);
  imagemask = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,
				XV_WIDTH, folderviewmask_width,
				XV_HEIGHT, folderviewmask_height,
				SERVER_IMAGE_X_BITS, folderviewmask_bits,
				NULL);

  folder_frame = (Frame) xv_create (XV_NULL, FRAME,
				    FRAME_MIN_SIZE, 420, 310,
				    FRAME_LABEL, "Folder View",
				    FRAME_SHOW_FOOTER, TRUE,
				    FRAME_LEFT_FOOTER, "",
				    FRAME_RIGHT_FOOTER, "",
				    FRAME_DONE_PROC, dir_folder_abort,
				    XV_KEY_DATA_REMOVE_PROC, 
				    dir_folder_free_data,
				    XV_HEIGHT, 320,
				    XV_WIDTH, 620,
				    NULL);

  folder_icon = (Icon) xv_create (folder_frame, ICON,
				  ICON_IMAGE, image,
				  ICON_MASK_IMAGE, imagemask,
				  ICON_LABEL_RECT, &label_rect,
				  XV_LABEL, "View",
				  NULL);

  xv_set (folder_frame, FRAME_ICON, folder_icon, NULL);

  folder_panel = (Panel) xv_create (folder_frame, PANEL, NULL);

  folder_file_open = (Menu_item) xv_create (XV_NULL, MENUITEM,
					    MENU_STRING, "Open",
					    MENU_NOTIFY_PROC, dir_open,
					    MENU_INACTIVE, TRUE,
					    NULL);
  folder_file_info = (Menu_item) xv_create (XV_NULL, MENUITEM,
					    MENU_STRING, "Information",
					    MENU_NOTIFY_PROC, dir_information,
					    MENU_INACTIVE, TRUE,
					    NULL);
  folder_file_menu =  (Menu) xv_create (XV_NULL, MENU, NULL);
  xv_set (folder_file_menu,
	  MENU_APPEND_ITEM, folder_file_open,
	  MENU_APPEND_ITEM, folder_file_info,
	  MENU_APPEND_ITEM, 
	  (Menu_item) xv_create (XV_NULL, MENUITEM,
				 MENU_STRING, "",
				 MENU_INACTIVE, TRUE,
				 NULL),
	  MENU_APPEND_ITEM,
	  (Menu_item) xv_create (XV_NULL, MENUITEM,
				 MENU_STRING, "Find",
				 MENU_NOTIFY_PROC, find_menu,
				 NULL),
	  NULL);
  folder_file = xv_create (folder_panel, PANEL_BUTTON,
			   PANEL_LABEL_STRING, "File",
			   PANEL_ITEM_MENU, folder_file_menu,
			   NULL);

  folder_view_show = (Menu_item) xv_create (XV_NULL, MENUITEM,
				    MENU_STRING, "Show All Subdirectories",
				    MENU_NOTIFY_PROC, folder_show_all,
				    MENU_INACTIVE, TRUE,
				    NULL);
  folder_view_hide = (Menu_item) xv_create (XV_NULL, MENUITEM,
				    MENU_STRING, "Hide All Subdirectories",
				    MENU_NOTIFY_PROC, folder_hide_all,
				    MENU_INACTIVE, TRUE,
				    NULL);
  folder_view_start = (Menu_item) xv_create (XV_NULL, MENUITEM,
				     MENU_STRING, "Start View here",
				     MENU_NOTIFY_PROC, folder_start_here,
				     MENU_INACTIVE, TRUE,
				     NULL);
  folder_view_add = (Menu_item) xv_create (XV_NULL, MENUITEM,
				   MENU_STRING, "Add Parent",
				   MENU_NOTIFY_PROC, folder_add_parent,
				   MENU_INACTIVE, TRUE,
				   NULL);

  folder_view_menu =  (Menu) xv_create (XV_NULL, MENU,
			     MENU_ITEM,
			       MENU_STRING, "Show Horizontal",
			       MENU_NOTIFY_PROC, folder_view_orientation,
			       NULL,
			     MENU_ITEM,
			       MENU_STRING, "",
			       MENU_INACTIVE, TRUE,
			       NULL,
			     NULL);
  xv_set (folder_view_menu, 
	  MENU_APPEND_ITEM, folder_view_show,
	  MENU_APPEND_ITEM, folder_view_hide,
	  MENU_APPEND_ITEM, (Menu_item) xv_create (XV_NULL, MENUITEM,
			       MENU_STRING, "",
			       MENU_INACTIVE, TRUE,
			       NULL),
	  MENU_APPEND_ITEM, folder_view_start,
	  MENU_APPEND_ITEM, folder_view_add,
	  NULL);

  folder_view = xv_create (folder_panel, PANEL_BUTTON,
			   PANEL_LABEL_STRING, "View",
			   PANEL_ITEM_MENU, folder_view_menu,
			   NULL);

  folder_goto = xv_create (folder_panel, PANEL_BUTTON,
			   PANEL_LABEL_STRING, "Goto",
			   PANEL_ITEM_MENU, goto_menu,
			   NULL);

  folder_canvas = (Canvas_shell) xv_create (folder_frame, CANVAS_SHELL,
					    CANVAS_AUTO_EXPAND, FALSE,
					    CANVAS_AUTO_SHRINK, FALSE,
					    CANVAS_SHELL_BATCH_REPAINT, TRUE,
					    CANVAS_SHELL_AUTO_DROP_SITE, TRUE,
					    WIN_RETAINED, TRUE,
					    XV_KEY_DATA, CANVAS_APP, first_app,
					    XV_X, 0,
					    XV_Y, 30,
					    NULL);
  folder_horizontal = (Scrollbar) xv_create (folder_canvas, SCROLLBAR,
				   SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
				   SCROLLBAR_SPLITTABLE, FALSE,
				   SCROLLBAR_PIXELS_PER_UNIT, 40,
				   NULL);
  folder_vertical = (Scrollbar) xv_create (folder_canvas, SCROLLBAR,
				   SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
				   SCROLLBAR_SPLITTABLE, FALSE,
				   SCROLLBAR_PIXELS_PER_UNIT, 48,
				   NULL);

  folder_tree = (Tree) xv_create (folder_canvas, TREE,
				  TREE_LAYOUT, TREE_LAYOUT_VERTICAL,
/*				  TREE_BORDER, 1,
				  TREE_PARENT_DISTANCE, 20, */
				  NULL);
  dir_folder_view_create (folder_tree, folder_tree, root, root->name);
  xv_set (folder_tree, 
	  XV_KEY_DATA, FOLDER_FILE_OPEN, folder_file_open,
	  XV_KEY_DATA, FOLDER_FILE_INFO, folder_file_info,
	  XV_KEY_DATA, FOLDER_VIEW_SHOW, folder_view_show,
	  XV_KEY_DATA, FOLDER_VIEW_HIDE, folder_view_hide,
	  XV_KEY_DATA, FOLDER_VIEW_START, folder_view_start,
	  XV_KEY_DATA, FOLDER_VIEW_ADD, folder_view_add,
	  NULL);
  xv_set (folder_frame, XV_SHOW, TRUE, NULL);
} /* dir_folder_view */


extern void
dir_init ()
{
  root = (Dir_T *) malloc (sizeof (Dir_T));
  root->name = malloc (1);
  *root->name = '\0';
  root->sub = NULL;
  root->next = NULL;
} /* dir_init */
