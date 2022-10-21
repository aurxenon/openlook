/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: select.c,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:03 $
 *	Purpose : management of selected objects
 *
 * $Log: select.c,v $
 * Revision 1.1.1.1  2005/07/13 18:31:03  arkenoi
 * Initial import of 0.2g
 *
 *
 * Revision 1.1  1995/12/01 15:32:53  root
 * Initial revision
 *
 *
 */

#include <sspkg/rectobj.h>

#include "global.h"
#include "select.h"


extern Selected_T *
select_add (Selected_T *select, Rectobj object)
{
  Selected_T *s;

  s = malloc (sizeof (Selected_T));
  s->object = object;
  s->next = select;
  return s;
} /* select_add */


extern Selected_T *
select_delete (Selected_T *select, Rectobj object)
{
  Selected_T *s;
  Selected_T *t = NULL;

  s = select;
  while (s->object != object) {
    t = s;
    s = s->next;
  } /* while */
  if (t != NULL) {
    t->next = s->next;
    free (s);
  } /* if */
  else {
    /* first element in the list */
    select = s->next;
    free (s);
  } /* else */
  return select;
} /* select_delete */


extern Selected_T *
select_get (Selected_T *select, Rectobj object)
{
  Selected_T *s;

  s = select;
  while (s != NULL) {
    if (s->object == object)
      return s;
  } /* while */
  return NULL;
} /* select_get */
