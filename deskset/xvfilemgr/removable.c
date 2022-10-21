/*
	System  : XVFilemgr
	File    : $RCSfile: removable.c,v $
	Author  : $Author: arkenoi $
	Date    : $Date: 2005/07/13 18:31:00 $
	Purpose : handling of cd-roms and floppy disks
*
* $Log: removable.c,v $
* Revision 1.1.1.1  2005/07/13 18:31:00  arkenoi
* Initial import of 0.2g
*
*
* Revision 1.7  2000/02/18 07:13:34  root
* More fixes for xvfilemgr. New code for removeable devices.
*
* Revision 1.6  1998/10/18 01:26:56  root
* FreeBSD mods by Mark Ovens.
*
* Revision 1.5  1997/11/02 16:46:27  root
* Small modifications to floppy mounts.
*
* Revision 1.4  1996/10/08 16:22:40  root
* ajusting includes and definitions.
*
* Revision 1.3  1996/10/08 16:12:14  root
* eliminating all wait system calls.
*
* Revision 1.2  1996/08/17 08:45:26  root
* rudimentary floppy and cdrom support.
*
* Revision 1.1  1996/07/31 19:15:59  root
* Initial revision
*
*
*/


#ifdef __FreeBSD__
#include <sys/param.h>
#endif


#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mount.h>

#ifdef __FreeBSD__
#include <sys/vfs.h>
#include <sys/cdio.h>
#else
#include <linux/cdrom.h>
/* #include <linux/fs.h> removed because this is GLIBC2 is mount.h is OK */
#endif

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/notify.h>

#include "global.h"
#include "config.h"
#include "error.h"
#include "properties.h"



static Notify_client client = (Notify_client) 10;



static Notify_value
remove_wait (Notify_client client, int pid, union wait *status, 
	     struct rusage *rusage)
{
  if (WIFEXITED(*status)) {
    return NOTIFY_DONE;
  } /* if */
  return NOTIFY_IGNORED;
} /* remove_wait */




static Notify_value
remove_wait_cdrom_mount (Notify_client client, int pid, union wait *status, 
	     struct rusage *rusage)
{
  Applic_T      *app;
  Properties_T  *props;

  if ( mount_exists (CDROM_MOUNTPOINT) ) {
    /* succesfully mounted, now open window */
    props = get_properties ();
    app = xvfmgr_new_frame (CDROM_WINDOW);
    app->next = first_app->next;
    (Applic_T *) first_app->next = app;
    strcpy (app->path, CDROM_MOUNTPOINT);
    dir_new (app->path, &props->current_folder);
    change_directory (app, app->path);
  }
  else error_message_name( first_app->frame, "Can't mount CDROM!\n");

  if (WIFEXITED(*status)) {
    return NOTIFY_DONE;
  } /* if */

  return NOTIFY_IGNORED;
} /* remove_wait_cdrom */


extern void
remove_check_for_cdrom (Menu menu, Menu_item menu_item)
{
  Applic_T      *app;
  int           pid;

  app = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA); 

  /* already mounted ? */
  if (mount_exists (CDROM_MOUNTPOINT) == 0) {

    /* no, try to mount */
    switch ( pid=fork()) {

      case -1: /* Error on fork() */
        error_message (app->frame);
        return;
    
      case 0: /* Child */
        execlp ("mount", "mount", CDROM_MOUNTPOINT, (char *) 0);
        error_message (app->frame);
	_exit( -1); /* The son must die here */
      
      default: /* Parent */
        notify_set_wait3_func( client, remove_wait_cdrom_mount, pid);        
    }
  } /* if */

} /* remove_check_for_cdrom */


void eject_mount( char device[] )
{
  int pid;
  int cdrom;
 
  /* eject is a long operation, so can't be made by the main app */
    switch ( pid=fork()) {

      case -1: /* Error on fork() */
        error_message ( first_app->frame);
        return;
    
      case 0: /* Child */
        cdrom = open (CDROM_DEVICE, O_RDONLY);
        if( cdrom >= 0 ){
#ifdef __FreeBSD__
          if (ioctl (cdrom, CDIOCEJECT) < 0)
#else
          if (ioctl (cdrom, CDROMEJECT) < 0)
#endif
            error_message (first_app->frame);

          close( cdrom);
        } 
	_exit( 0); /* The child must die here */
      
      default: /* Parent */
        break;        
    }

}




static Notify_value
remove_wait_cdrom_umount (Notify_client client, int pid, union wait *status, 
	     struct rusage *rusage)
{
  Applic_T      *app;
  Properties_T  *props;
  int cdrom;

  if (mount_exists (CDROM_MOUNTPOINT) == 1) {

    error_message_name( first_app->frame, "Can't umount CDROM!\nPlease check there isn't an application using it\n");
    
    /* RE-open! */
    props = get_properties ();
    app = xvfmgr_new_frame (CDROM_WINDOW);
    app->next = first_app->next;
    (Applic_T *) first_app->next = app;
    strcpy (app->path, CDROM_MOUNTPOINT);
    dir_new (app->path, &props->current_folder);
    change_directory (app, app->path);
  }
  else eject_mount( CDROM_DEVICE);

  if (WIFEXITED(*status)) {
    return NOTIFY_DONE;
  } /* if */

  return NOTIFY_IGNORED;
} /* remove_wait_cdrom_umount */



extern void
remove_cdrom_umount (Applic_T *app)
{
  int pid;

  if (mount_exists (CDROM_MOUNTPOINT) == 1) {

    /* yes, try to umount */
    switch( pid=fork() ) {
      
      case -1: /* Error on fork() */
        error_message( app->frame);
        return;

      case 0: /* Child */
#ifdef __FreeBSD__
        execlp( "unmount", "unmount", CDROM_MOUNTPOINT, (char *)0 );
#else
        execlp( "umount", "umount", CDROM_MOUNTPOINT, (char *)0 );
#endif
        error_message( app->frame);
        _exit( -1); /* The son must die here */

      default: /* Parent */ ;
        notify_set_wait3_func( client, remove_wait_cdrom_umount, pid);
    }
  } /* if */
} /* remove_cdrom_umount */




extern void
remove_eject_cdrom (Panel_item item)
{
  Applic_T *app;
  int      pid;

  app = (Applic_T *) xv_get (item, XV_KEY_DATA, EJECT_APP);

  /* Close the window */
  xv_destroy_safe (app->frame);

} /* remove_eject_cdrom */




extern void
remove_floppy_umount (Applic_T *app)
{
  if (mount_exists (FLOPPY_MOUNTPOINT) == 1) {
#ifdef __FreeBSD__   /* FreeBSD uses unmount, not umount */
    if (unmount (FLOPPY_DEVICE, MNT_FORCE))
#else
    if (umount (FLOPPY_DEVICE))
#endif
      error_message (app->frame);
  } /* if */
} /* remove_floppy_umount */


extern void
remove_check_for_floppy (Menu menu, Menu_item menu_item)
{
  Applic_T      *app;
  Applic_T      *app2;
  Properties_T  *props;
  int           error = 0;
  int           pid;

  app2 = (Applic_T *) xv_get (menu, MENU_CLIENT_DATA); 
  /* already mounted ? */
  if (mount_exists (FLOPPY_MOUNTPOINT) == 0) {
    /* no, try to mount */
/*    if (mount(FLOPPY_DEVICE, FLOPPY_MOUNTPOINT, "msdos", 
	      MS_MGC_VAL | MS_RDONLY, (char *) 0) < 0) {
	  error_message (app2->frame);
	  error = 1;
	}*/ /* if */
    if ((pid = fork()) < 0)
      error_message (app2->frame);
    else
      if (pid == 0)
	if (execlp ("mount", "mount", FLOPPY_MOUNTPOINT,
				(char *) 0) < 0) {
	  error_message (app2->frame);
	  error = 1;
	} /* if */
    notify_set_wait3_func(client, remove_wait, pid);
  } /* if */
  if (error = 0) {
    /* succesfully mounted, now open window */
    props = get_properties ();
    app = xvfmgr_new_frame (FLOPPY_WINDOW);
    app->next = first_app->next;
    (Applic_T *) first_app->next = app;
    strcpy (app->path, FLOPPY_MOUNTPOINT);
    dir_new (app->path, &props->current_folder);
    change_directory (app, app->path);
  } /* if */
} /* remove_check_for_floppy */


extern void
remove_format_floppy (Menu menu, Menu_item menu_item)
{
} /* remove_format_floppy */

