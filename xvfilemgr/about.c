/*
 *      System  : XVFilemanager
 *	File    : $RCSfile: about.c,v $
 *	Author  : $Author: arkenoi $
 *	Date    : $Date: 2005/07/13 18:31:15 $
 *	Purpose : displays the about-panel
 *
 * $Log: about.c,v $
 * Revision 1.1  2005/07/13 18:31:15  arkenoi
 * Initial revision
 *
 * Revision 1.7  1998/12/20 18:23:39  root
 * Changes to (C) window.
 *
 * Revision 1.6  1998/12/20 18:16:56  root
 * Changes to email addresses.
 *
 * Revision 1.5  1998/10/18 01:33:15  root
 * Mods.
 *
 * Revision 1.4  1996/08/16 17:17:58  root
 * changing text in about window concerning the authors of the program.
 *
 * Revision 1.3  1996/07/28 08:23:26  root
 * incorporate changes from Vincent concerning colormaps etc.
 * clean up the code to keep the interface small.
 *
 * Revision 1.1  1995/12/01  15:32:53  root
 * Initial revision
 *
 *
 */

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>

#include "config.h"
#include "global.h"
#include "about.h"
#include "misc.h"

#include "icons/xvfilemgr.xpm"
#include "icons/xvfmgricon"

#define OTHER_ADDRESS 2
#define TO_LEN  64

static Panel_item   about_picture;
static Server_image xvfmgr_image;

struct about_window {
  Frame      frame;
  Panel      panel;
  Textsw     message;
  Panel_item bottom_panel;
  Panel_item mail;
  /* for non-OPEN LOOK window managers */
  Panel_item dismiss;
  char       geometry[20];
};

static struct about_window about_window;

struct feedback_window {
  Frame      frame;
  Panel      panel;
  Panel_item which;
  Panel_item other;
  Textsw     feedback;
  Panel_item bottom_panel;
  Panel_item send;
  Panel_item cancel;
};

static struct feedback_window feedback_window;

static char about_message[] =
"XVFilemgr was written by Christian Koch and Vincent S. Cojot\n\
\n\
XVFilemgr is a clone of Sun\'s famous filemgr from Solaris 2.x \
(pre-motif era).\n\
This project is being written on Linux and is mostly aimed at the \
Linux community but ports to other Xview-capable platforms will \
probably be considered in the future.\n\
\n\
XVFilemgr is still under development, so suggestions (and bug \
reports, of course!) are welcome (although bug reports aren't _quite_ \
so welcome :-). Send mail to us with the \"Send Mail\" button below.\n\
\n\
Vincent S. Cojot (coyote@step.polymtl.ca) wants to hear about bug reports\
 and is trying to contribute to the XVFilemgr project.\n\
Credits:\n\
\n\
This about window is based on code from Larry Wake.\n\
";


static void
add_dismiss (Panel panel, Panel_item first, Panel_item dismiss)
{
  Rect    *first_rect;
  Rect    *dismiss_rect;
  int     width, space, pos;

  width = xv_get(panel, XV_WIDTH);
  first_rect = (Rect *)xv_get(first, XV_RECT);
  xv_set(dismiss,
	 XV_SHOW, FALSE,
	 NULL);
  xv_set(first,
	 XV_X, width/2 - first_rect->r_width/2,
	 XV_SHOW, TRUE,
	 NULL);
} /* add_dismiss */


static void
about_send_proc ()
{
  xv_set(feedback_window.frame,
	 XV_SHOW, TRUE,
	 NULL);
} /* about_send_proc */


static void
feedback_address_proc (Panel_item item, unsigned int value, Event *event)
{
  if (value == OTHER_ADDRESS) {
    xv_set(feedback_window.other,
	   XV_SHOW, TRUE,
	   NULL);
  } else {
    xv_set(feedback_window.other,
	   XV_SHOW, FALSE,
	   NULL);
  }
} /* feedback_adress_proc */

static void
feedback_send_proc (Panel_item item, Event *event)
{
  FILE *pp;
  char addr[TO_LEN + 1], buf[BUFSIZ+1];
  int addr_sel, last_was_NL = 0;
  Textsw_index cur_pos, next_pos;
  char    *sigfile;
  FILE    *sigfp;
  int             ch;
  
  static char *fb_cmd = NULL;
  
  if ((int)xv_get(feedback_window.feedback, TEXTSW_LENGTH) == 0) {
    
    xv_set(feedback_window.frame,
	   FRAME_LEFT_FOOTER, "No text in message.",
	   NULL);
    xv_set(item,
	   PANEL_NOTIFY_STATUS, XV_ERROR,
	   NULL);
    return;
  }
  
  addr_sel = (int)xv_get(feedback_window.which, PANEL_VALUE);
  if (addr_sel == OTHER_ADDRESS)
    strncpy(addr,
	    (char *)xv_get(feedback_window.other, PANEL_VALUE),
	    TO_LEN);
  else
    strncpy(addr, (char *)xv_get(feedback_window.which,
				 PANEL_CHOICE_STRING, addr_sel), TO_LEN);
  
  if (addr[0] == '\0') {
    
    xv_set(feedback_window.frame,
	   FRAME_LEFT_FOOTER, "No address specified.",
	   NULL);
    xv_set(item, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);
    return;
  }
  if (fb_cmd == NULL)
    fb_cmd = (char *)malloc((unsigned int)(strlen(DEFAULT_MAILER) +
					   TO_LEN + 1));
  
  
  sprintf(fb_cmd, "%s %s", DEFAULT_MAILER, addr);
  if ((pp = popen(fb_cmd, "w")) == NULL) {
    
    xv_set(feedback_window.frame,
	   FRAME_LEFT_FOOTER, "popen error; couldn't send feedback message!",
	   NULL);
    xv_set(item, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);
    return;
  }
  
  fprintf(pp, "XVfilemgr Version %s Comment\n\n", XVFILEMGR_VERSION);
  
  fprintf(pp, "\n");
  
  next_pos = 0;
  cur_pos = next_pos - BUFSIZ;
  while (next_pos == cur_pos + BUFSIZ) {
    cur_pos = next_pos;
    next_pos = (Textsw_index)xv_get(feedback_window.feedback,
				    TEXTSW_CONTENTS, cur_pos, buf, BUFSIZ);
    if ((next_pos - cur_pos) != 0) {
      buf[next_pos - cur_pos] = '\0';
      fprintf(pp, "%s", buf);
      last_was_NL = (buf[next_pos-cur_pos-1] == '\n');
    }
  }
  /*
   *  Force last char out to be a newline
   */
  if (!last_was_NL)
    putc('\n', pp);
  
  sigfile = find_dotfile(".signature");
  if (sigfile) {
    sigfp = fopen(sigfile, "r");
    if (sigfp != NULL) {
      while ((ch = getc(sigfp)) != EOF)
	putc(ch, pp);
      fclose(sigfp);
    }
    free(sigfile);
  }
  
  if (pclose(pp) != 0) {
    xv_set(feedback_window.frame,
	   FRAME_LEFT_FOOTER, "Mail failed -- message not sent!",
	   NULL);
    xv_set(item, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);
    return;
  }
  textsw_reset(feedback_window.feedback, 0, 0);
} /* feedback_send_proc */


static void
feedback_cancel_proc ()
{
  textsw_reset(feedback_window.feedback, 0, 0);
} /* feedback_cancel_proc */


static void
dismiss_about_window ()
{
  xv_set(about_window.frame,
	 XV_SHOW, FALSE,
	 FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_OUT,
	 NULL);
} /* dismiss_about_window */


static void
about_ok (Panel_item item)
{
  Frame about_frame;

  about_frame = (Frame) xv_get (item, PANEL_CLIENT_DATA);
  xv_set (about_frame, XV_SHOW, FALSE, NULL);
  xv_destroy_safe (about_frame);
} /* about_ok */


extern void
about_notify (Panel_item item, Event *event)
{
  Applic_T *app;
  Frame    about_frame;
  Panel    about_panel;
  Rect     rect;
  Rect     rect2;

  app = (Applic_T *) xv_get (item, MENU_CLIENT_DATA);
  textsw_reset (about_window.message, 0, 0);
  textsw_insert (about_window.message, about_message,
		 strlen(about_message));
  textsw_normalize_view (about_window.message, 0);
#ifdef DIALOG_ALWAYS_RIGHT
  frame_get_rect (app->frame, &rect);
  frame_get_rect (about_window.frame, &rect2);
  rect2.r_left = rect.r_left + rect.r_width;
  rect2.r_top = rect.r_top;
  frame_set_rect (about_window.frame, &rect2);
#endif
  xv_set (about_window.frame, XV_SHOW, TRUE, NULL);
} /* about_notify */


extern void
about_init (Applic_T *app)
{
  Rect    *first_rect;
  Rect    *butrect;
  Pixmap     pixmap;
  Pixmap     pixmask;
  int     width = 500;
  int     height = 350;

  if (XpmCreatePixmapFromData (app->display, 
		       RootWindow (app->display, DefaultScreen (app->display)),
		       xvfilemgr_xpm, &pixmap, &pixmask, NULL) == 0) {
    xvfmgr_image = (Server_image) xv_create ((int) NULL, SERVER_IMAGE,
					     XV_WIDTH, 64,
					     XV_HEIGHT, 64,
					     SERVER_IMAGE_DEPTH, 4,
					     SERVER_IMAGE_PIXMAP, pixmap,
					     NULL);
  } /* if */
  else {
    /* We were not able to allocate colors correctly. */
    /* Try to replace the colors pixmap we were trying to load */
    /* by an icon made of an X bitmap. */
    xvfmgr_image = (Server_image) xv_create (XV_NULL, SERVER_IMAGE,  
				     XV_WIDTH, xvfmgricon_width,
				     XV_HEIGHT, xvfmgricon_height,
				     SERVER_IMAGE_X_BITS, xvfmgricon_bits,
				     NULL);
  } /* else */

  about_window.frame = xv_create(app->frame,
				 FRAME_CMD,
				 XV_LABEL, "XVfilemgr: About XVfilemgr",
				 FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_IN,
				 XV_WIDTH, width,
				 XV_HEIGHT, height,
				 NULL);
  
  about_window.panel = xv_get(about_window.frame, FRAME_CMD_PANEL);
  
  xv_set(about_window.panel,
	 PANEL_LAYOUT, PANEL_VERTICAL,
	 NULL);
  
  xv_create(about_window.panel,
	    PANEL_MESSAGE,
	    XV_X, 150,
	    PANEL_LABEL_STRING, "XVFilemanager",
	    PANEL_LABEL_BOLD, TRUE,
	    NULL);
  
  xv_create(about_window.panel,
	    PANEL_MESSAGE,
	    XV_X, 150,
	    PANEL_LABEL_STRING, "(C) 1996,97,98 by Christian Koch",
	    PANEL_LABEL_BOLD, TRUE,
	    NULL);
  
  xv_create(about_window.panel,
	    PANEL_MESSAGE,
	    XV_X, 150,
	    PANEL_LABEL_STRING, "Maintained by Vincent S. Cojot",
	    PANEL_LABEL_BOLD, TRUE,
	    NULL);
  
  xv_create(about_window.panel,
	    PANEL_MESSAGE,
	    XV_X, 150,
	    PANEL_LABEL_STRING, "",
	    PANEL_LABEL_BOLD, TRUE,
	    NULL);
  
  window_fit_height(about_window.panel);
  
  about_window.message = xv_create(about_window.frame,
				   TEXTSW,
				   XV_X, 0,
				   XV_WIDTH, WIN_EXTEND_TO_EDGE,
				   XV_HEIGHT, 136,
				   OPENWIN_SHOW_BORDERS, TRUE,
				   TEXTSW_BROWSING, TRUE,
				   TEXTSW_DISABLE_LOAD, TRUE,
				   NULL);
  
  about_window.bottom_panel = xv_create(about_window.frame,
					PANEL,
					XV_X, 0,
					NULL);
  
  about_window.mail = xv_create(about_window.bottom_panel,
				PANEL_BUTTON,
				PANEL_LABEL_STRING, "Send Mail...",
				PANEL_NOTIFY_PROC, about_send_proc,
				NULL);
  
  xv_set(about_window.panel,
	 PANEL_LAYOUT, PANEL_HORIZONTAL,
	 NULL);
  
  about_window.dismiss = xv_create(about_window.bottom_panel,
				   PANEL_BUTTON,
				   PANEL_LABEL_STRING, "Dismiss",
				   PANEL_NOTIFY_PROC, dismiss_about_window,
				   NULL);
  
  xv_set(about_window.panel,
	 PANEL_LAYOUT, PANEL_VERTICAL,
	 NULL);
  
  window_fit_height(about_window.bottom_panel);
  
  butrect = (Rect *)xv_get(about_window.mail, XV_RECT);
  
  xv_set(about_window.mail,
	 XV_X, (int)xv_get(about_window.bottom_panel, XV_WIDTH)/2
	 - butrect->r_width,
	 NULL);
  
  
  window_fit(about_window.frame);
  
  xv_create(about_window.panel, PANEL_MESSAGE,
	    XV_X, 2,
	    XV_Y, xv_get(about_window.panel, XV_HEIGHT)/2 - 32,
	    PANEL_LABEL_IMAGE, xvfmgr_image,
	    NULL);
  
  
  width = xv_get(about_window.panel, XV_WIDTH);
  
  first_rect = (Rect *)xv_get(about_window.mail, XV_RECT);
  
  add_dismiss(about_window.panel, about_window.mail,
	      about_window.dismiss);
} /* about_init */

extern void
feedback_init (Applic_T *app)
{
  Rect    *butrect;
  Pixmap  pixmask;
  int     width = 500;
  int     height = 350;

  feedback_window.frame = xv_create(app->frame,
				    FRAME_CMD,
				    XV_LABEL, "XVFilemgr: Send Feedback",
				    FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_OUT,
				    XV_WIDTH, width,
				    XV_HEIGHT, height,
				    NULL);
  
  feedback_window.panel = xv_get(feedback_window.frame, FRAME_CMD_PANEL);
  
  xv_set(feedback_window.panel,
	 PANEL_LAYOUT, PANEL_VERTICAL,
	 NULL);
  
  xv_create(feedback_window.panel,
	    PANEL_MESSAGE,
	    PANEL_LABEL_STRING,
	    "Your feedback is appreciated. To insure that your message arrives,",
	    NULL);
  
  xv_create(feedback_window.panel,
	    PANEL_MESSAGE,
	    PANEL_LABEL_STRING,
	    "check that the \"To\" address is valid for your site before clicking \"Send.\"",
	    NULL);
  
  feedback_window.which = xv_create(feedback_window.panel,
				    PANEL_CHOICE_STACK,
				    PANEL_CHOICE_NROWS, 1,
				    PANEL_LAYOUT, PANEL_HORIZONTAL,
				    PANEL_LABEL_STRING, "To:",
				    PANEL_CHOICE_STRINGS,
				    "coyote@step.polymtl.ca",
				    "root@obelix.shnet.org",
				    "Other Address",
				    NULL,
				    PANEL_NOTIFY_PROC, feedback_address_proc,
				    NULL);
  
  feedback_window.other = xv_create(feedback_window.panel, PANEL_TEXT,
				    PANEL_LABEL_STRING, "Other:  ",
				    PANEL_LABEL_BOLD, TRUE,
				    PANEL_VALUE_DISPLAY_LENGTH, 40,
				    PANEL_VALUE_STORED_LENGTH, 64,
				    PANEL_VALUE, "",
				    PANEL_READ_ONLY, FALSE,
				    XV_SHOW, FALSE,
				    NULL);
  
  window_fit_height(feedback_window.panel);
  
  feedback_window.feedback = xv_create(feedback_window.frame,
				       TEXTSW,
				       XV_X, 0,
				       XV_WIDTH, WIN_EXTEND_TO_EDGE,
				       XV_HEIGHT, 188,
				       OPENWIN_SHOW_BORDERS, TRUE,
				       NULL);
  
  feedback_window.bottom_panel = xv_create(feedback_window.frame,
					   PANEL,
					   XV_X, 0,
					   PANEL_LAYOUT, PANEL_HORIZONTAL,
					   NULL);
  feedback_window.send = xv_create(feedback_window.bottom_panel,
				   PANEL_BUTTON,
				   PANEL_LABEL_STRING, "Send",
				   PANEL_NOTIFY_PROC, feedback_send_proc,
				   NULL);
  
  window_fit_height(feedback_window.bottom_panel);
  
  butrect = (Rect *)xv_get(feedback_window.send, XV_RECT);
  
  xv_set(feedback_window.send,
	 XV_X, (int)xv_get(feedback_window.bottom_panel, XV_WIDTH)/2
	 - butrect->r_width,
	 NULL);
  
  feedback_window.cancel = xv_create(feedback_window.bottom_panel,
				     PANEL_BUTTON,
				     PANEL_LABEL_STRING, "Cancel",
				     PANEL_NOTIFY_PROC, feedback_cancel_proc,
				     NULL);
  
  
  window_fit(feedback_window.frame);
} /* feedback_init */


