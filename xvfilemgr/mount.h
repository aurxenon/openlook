#ifndef __MOUNT__
#define __MOUNT__
/*
	System  : XVfilemgr
	File    : $RCSfile: mount.h,v $
	Author  : $Author: arkenoi $
	Date    : $Date: 2005/07/13 18:31:00 $
	Purpose : hamndling of mount points
*
* $Log: mount.h,v $
* Revision 1.1.1.1  2005/07/13 18:31:00  arkenoi
* Initial import of 0.2g
*
*
* Revision 1.3  1996/07/31 19:13:26  root
* testing wether a mount point exists.
*
* Revision 1.2  1996/06/22 11:36:00  root
* Functions for free and total bytes.
*
* Revision 1.1  1996/06/01 14:02:37  root
* Initial revision
*
*
*/

extern char *
mount_get_point (char *path);

extern char *
mount_get_from (char *path);

extern long
mount_get_free_bytes (char *path);

extern long
mount_get_total_bytes (char *path);

extern int
mount_exists (char *path);

extern void
mount_init ();

#endif /* __MOUNT__ */
