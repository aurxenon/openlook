/*
	System  : XVfilemgr
	File    : $RCSfile: mount.c,v $
	Author  : $Author: arkenoi $
	Date    : $Date: 2005/07/13 18:31:03 $
	Purpose : handling of mount points
*
* $Log: mount.c,v $
* Revision 1.1.1.1  2005/07/13 18:31:03  arkenoi
* Initial import of 0.2g
*
*
* Revision 1.5  1998/10/18 01:25:17  root
* FreeBSD mods by Mark Ovens.
*
* Revision 1.4  1996/07/31 19:13:09  root
* testing wether a mount point exists added.
*
* Revision 1.3  1996/07/26 19:58:37  root
* change FILENAME_LENGTH constant to PATH_MAX
*
* Revision 1.2  1996/06/22 11:35:45  root
* Functions for free bytes and total bytes.
*
* Revision 1.1  1996/06/01 14:02:27  root
* Initial revision
*
*
*/

#ifdef __FreeBSD__
#include <sys/param.h>
#include <sys/types.h>
#endif

#include <sys/vfs.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "mount.h"


typedef struct Mount_T * Mount_PTR;

typedef struct Mount_T {
  char      *point; /* where it is mounted */
  char      *from;  /* device */
  Mount_PTR next;
} Mount_T;

static Mount_T *first;

extern char *
mount_get_point (char *path)
{
  Mount_T *mount = first;

  /* return mount point for this file/path */
  while (mount != NULL) {
    /* this works because the list of mount point is invers. */
    if (strncmp (path, mount->point, strlen (mount->point)) == 0)
      return mount->point;
    mount = mount->next;
  } /* while */
  return NULL;
} /* mount_get_point */


extern char *
mount_get_from (char *path)
{
  Mount_T *mount = first;

  /* return path of special device file for this mount point */
  while (mount != NULL) {
    /* this works because the list of mount point is invers. */
    if (strncmp (path, mount->point, strlen (mount->point)) == 0)
      return mount->from;
    mount = mount->next;
  } /* while */
  return NULL;
} /* mount_get_from */


extern long
mount_get_free_bytes (char *path)
{
  struct statfs buf;

  statfs (path, &buf);
  return (buf.f_bavail * buf.f_bsize);
} /* mount_get_free_bytes */


extern long
mount_get_total_bytes (char *path)
{
  struct statfs buf;

  statfs (path, &buf);
  return (buf.f_blocks * buf.f_bsize);
} /* mount_get_total_bytes */


static void
mount_update ()
{
  FILE    *fp;
  char    buf [PATH_MAX];
  Mount_T *mount, *mount2;
  char    *point, *from;

  /* remove old mount list */
  if (first != NULL) {
    mount = first;
    while (mount != NULL) {
      mount2 = mount->next;
      free (mount->point);
      free (mount->from);
      free (mount);
      mount = mount2;
    } /* while */
  } /* if */
  first = NULL;
  /* scan /proc/mounts to get mount point information.  The advantage of this
     solutions that not only root can access /proc */
  if ((fp = fopen ("/proc/mounts", "r")) != NULL) {
    while (fgets (buf, sizeof (buf) - 1, fp) != NULL) {
      mount = malloc (sizeof (Mount_T));
      from = strtok (buf, " ");
      point = strtok (NULL, " ");
      mount->from = malloc (strlen (from) + 1);
      strcpy (mount->from, from);
      mount->point = malloc (strlen (point) + 1);
      strcpy (mount->point, point);
      /* create list of mount point in invers order */
      mount->next = first;
      first = mount;
    } /* while */
    fclose (fp);
  } /* if */
} /* mount_update */


extern int
mount_exists (char *path)
{
  Mount_T *mount;
  int exist = 0;
  
  mount_update ();
  mount = first;
  while (mount != NULL) {
    if (strcmp (mount->point , path) == 0) {
      exist = 1;
      break;
    } /* if */
    mount = mount->next;
  } /* while */
  return exist;
} /* mount_exists */


extern void
mount_init ()
{
  mount_update();
} /* mount_init */
