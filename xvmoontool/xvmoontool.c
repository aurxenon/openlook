/*

			A Moon for OpenWindows

			     Release 3.0

    Designed and implemented by John Walker in December 1987,
    Revised and updated in February of 1988.
    Revised and updated again in June of 1988 by Ron Hitchens.
    Revised and updated yet again in July/August of 1989 by Ron Hitchens.
    Converted to OpenWindows in December of 1991 by John Walker.

    This  program  is  an OpenWindows tool which displays, as the icon
    for a closed window, the current phase of the Moon.  A subtitle in
    the  icon  gives the age of the Moon in days and hours.  If called
    with the "-t" switch, it rapidly increments forward  through  time
    to display the cycle of phases.

    If you  open  the  window,	additional  information  is  displayed
    regarding  the  Moon.   The  information  is generally accurate to
    within ten minutes.

    The algorithms used in this program to calculate the positions Sun
    and  Moon  as seen from the Earth are given in the book "Practical
    Astronomy With Your Calculator"  by  Peter  Duffett-Smith,  Second
    Edition,  Cambridge  University  Press,  1981.   Ignore  the  word
    "Calculator" in the title;  this  is  an  essential  reference  if
    you're   interested   in   developing  software  which  calculates
    planetary positions, orbits, eclipses, and the  like.   If  you're
    interested	in  pursuing such programming, you should also obtain:

    "Astronomical Formulae  for  Calculators"  by  Jean  Meeus,  Third
    Edition, Willmann-Bell, 1985.  A must-have.

    "Planetary  Programs  and  Tables  from  -4000 to +2800" by Pierre
    Bretagnon and Jean-Louis Simon, Willmann-Bell, 1986.  If you  want
    the  utmost  (outside of JPL) accuracy for the planets, it's here.

    "Celestial BASIC" by Eric Burgess, Revised Edition,  Sybex,  1985.
    Very cookbook oriented, and many of the algorithms are hard to dig
    out of the turgid BASIC code, but you'll probably want it anyway.

    Many  of these references can be obtained from Willmann-Bell, P.O.
    Box 35025, Richmond, VA 23235, USA.  Phone:  (804)	320-7016.   In
    addition  to  their  own  publications,  they  stock  most	of the
    standard references for mathematical and positional astronomy.

    This program was written by:

	John Walker
	Autodesk Neuchbtel
	Avenue des Champs-Montants 14b
	CH-2074 MARIN
	Switzerland
	Usenet: kelvin@Autodesk.com
	Fax:	038/33 88 15
	Voice:	038/33 76 33

    This  program is in the public domain: "Do what thou wilt shall be
    the whole of the law".  I'd appreciate  receiving  any  bug  fixes
    and/or  enhancements, which I'll incorporate in future versions of
    the program.  Please leave the  original  attribution  information
    intact so that credit and blame may be properly apportioned.

	History:
	--------
	June 1988	Version 2.0 posted to usenet by John Walker

	June 1988	Modified by Ron Hitchens
			     ronbo@vixen.uucp
			     ...!uunet!cs.utah.edu!caeco!vixen!ronbo
			     hitchens@cs.utexas.edu
			to produce version 2.1.
			Modified icon generation to show surface texture
			 on visible moon face.
			Added a menu to allow switching in and out of
			 test mode, for entertainment value mostly.
			Modified layout of information in open window display
			 to reduce the amount of pixels modified in each
			 update.

	July 1989	Modified further for color displays.  On a color Sun,
			 four colors will be used for the canvas and the icon.
			 Rather than just show the illuminated portion of
			 the moon, a color icon will also show the darkened
			 portion in a dark blue shade.	The text on the icon
                         will also be drawn in a nice "buff" color, since there
			 was one more color left to use.
                        Add two command line args, "-c" and "-m" to explicitly
			 specify color or monochrome mode.
			Use getopt to parse the args.
			Change the tool menu slightly to use only one item
			 for switching in and out of test mode.

	July 1989	Modified a little bit more a few days later to use 8
			 colors and an accurate grey-scale moon face created
			 by Joe Hitchens on an Amiga.
			Added The Apollo 11 Commemorative Red Dot, to show
			 where Neil and Buzz went on vacation a few years ago.
			Updated man page.

        August 1989     Received version 2.3 of John Walker's original code.
			Rolled in bug fixes to astronomical algorithms:

                         2.1  6/16/88   Bug fix.  Table of phases didn't update
					at the moment of the new moon.	Call on
                                        phasehunt  didn't  convert civil Julian
					date  to  astronomical	 Julian   date.
					Reported by Dag Bruck
					 (dag@control.lth.se).

			 2.2  N/A	(Superseded by new moon icon.)

			 2.3  6/7/89	Bug fix.  Table of phases  skipped  the
					phases	for  July  1989.  This occurred
					due  to  sloppy  maintenance   of   the
					synodic  month index in the interchange
					of information between phasehunt()  and
					meanphase().  I simplified and
					corrected  the	handling  of  the month
					index as phasehunt()  steps  along  and
					removed unneeded code from meanphase().
					Reported by Bill Randle  of  Tektronix.
					 (billr@saab.CNA.TEK.COM).

	January 1990	Ported to OpenWindows by John Walker.
                        All  of Ron Hitchens' additions which were not
			Sun-specific  were   carried   on   into   the
			OpenWindows version.

	September 1993	reported to Motif (as God intended) by Cary Sandvig.
			Some window reformatting was done as I was unable to
			view the existing setup.

	April 1997	Ported to XView under Linux by Joerg Richter.
			Added moon rise/set feature (based on code of
			Marc T. Kaufman)
*/

#include <stdio.h>
#include <time.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/font.h>
#include <xview/cms.h>

#include <xview/pixwin.h>

#include "riseset.h"

#define PI 3.14159265358979323846  /* Assume not near black hole nor in
				      Tennessee */

#include <math.h>

#define MOON_TITLE "XVMoontool   by John Walker   v3.1"

#define UNKNOWN -2

#include "moon_icon"
#include "color.pix"
#include "mini.icons"

static char moon_icon_work[sizeof moon_icon_bits];
static char moon_icon_last[sizeof moon_icon_bits] = "abcd";
static char icon_label_last[20] = "";

#define CLOSED_SECS	120		/* update interval when tool closed */
#define CLOSED_USECS	0
#define OPEN_SECS	1		/* update interval when open */
#define OPEN_USECS	0

/*		    0	    1	    2       3       4	    5	    6       7*/
#define COLOR_R {0x00, 0xe0e0, 0xc0c0, 0x9090, 0xffff, 0x1010, 0x1010, 0x1010}
#define COLOR_G {0x00, 0xf0f0, 0xd0d0, 0xa0a0, 0x0000, 0x1010, 0x1010, 0x1010}
#define COLOR_B {0x00, 0xffff, 0xe0e0, 0xb0b0, 0x0000, 0x9090, 0x7070, 0x5050}

static int c_r[] = COLOR_R;
static int c_g[] = COLOR_G;
static int c_b[] = COLOR_B;

/*  Astronomical constants  */
#define epoch	    2444238.5	   /* 1980 January 0.0 */

/*  Constants defining the Sun's apparent orbit  */
#define elonge	    278.833540	   /* Ecliptic longitude of the Sun
				      at epoch 1980.0 */
#define elongp	    282.596403	   /* Ecliptic longitude of the Sun at
				      perigee */
#define eccent      0.016718       /* Eccentricity of Earth's orbit */
#define sunsmax     1.495985e8     /* Semi-major axis of Earth's orbit, km */
#define sunangsiz   0.533128       /* Sun's angular size, degrees, at
				      semi-major axis distance */

/*  Elements of the Moon's orbit, epoch 1980.0  */
#define mmlong      64.975464      /* Moon's mean lonigitude at the epoch */
#define mmlongp     349.383063	   /* Mean longitude of the perigee at the
				      epoch */
#define mlnode	    151.950429	   /* Mean longitude of the node at the
				      epoch */
#define minc        5.145396       /* Inclination of the Moon's orbit */
#define mecc        0.054900       /* Eccentricity of the Moon's orbit */
#define mangsiz     0.5181         /* Moon's angular size at distance a
				      from Earth */
#define msmax       384401.0       /* Semi-major axis of Moon's orbit in km */
#define mparallax   0.9507	   /* Parallax at distance a from Earth */
#define synmonth    29.53058868    /* Synodic month (new Moon to new Moon) */
#define lunatbase   2423436.0      /* Base date for E. W. Brown's numbered
				      series of lunations (1923 January 16) */
/*  Properties of the Earth  */
#define earthrad    6378.16	   /* Radius of Earth in kilometres */


/*  Handy mathematical functions  */
#define sgn(x) (((x) < 0) ? -1 : ((x) > 0 ? 1 : 0))	  /* Extract sign */
#define abs(x) ((x) < 0 ? (-(x)) : (x)) 		  /* Absolute val */
#define fixangle(a) ((a) - 360.0 * (floor((a) / 360.0)))  /* Fix angle	  */
#define torad(d) ((d) * (PI / 180.0))			  /* Deg->Rad	  */
#define todeg(d) ((d) * (180.0 / PI))			  /* Rad->Deg	  */
#define dsin(x) (sin(torad((x))))			  /* Sin from deg */
#define dcos(x) (cos(torad((x))))			  /* Cos from deg */

static int testmode = FALSE;	      /* Rapid warp through time for debugging */
static int color_mode = UNKNOWN;      /* indicates color/mono mode */
static double faketime = 0.0;	      /* Time increment for test mode */	
static double nptime = 0.0;	      /* Next new moon time */
static int col_vals[16];              /* pixel values for color mode */
static int day_last = 0;	      
static long epoch_seconds_test = 0.0;

extern	char riseset_buf[][64];

static char *moname[] = {
    "January", "February", "March", "April", "May",
    "June", "July", "August", "September",
    "October", "November", "December"
};

static char *labels[] = {
    "Julian date:",
    "Universal time:",
    "Local time:",
    "",
    "Age of moon:",
    "Moon phase:",
    "Moon's distance:",
    "Moon subtends:",
    "",
    "Sun subtends:",
    "Sun's distance:",
    "",
    "Last new moon:",
    "First quarter:",
    "Full moon:",
    "Last quarter:",
    "Next new moon:"
};

#define Nlabels ((sizeof labels) / sizeof(char *))

static char *rlabels[] = {
	"Moon rise   Today:",
	"Tomorrow:",
	"Moon set   Today:",
	"Tomorrow:"
};

#define Rlabels	((sizeof rlabels) / sizeof(char *))

static	Frame		frame;			/* base frame */
static	Display		*display;

static	Panel		panel;			/* main panel */
static  Panel		panel_l;		/* info panel */
static  Panel		panel_u;		/* info panel */
static	Panel		panel_m;		/* set/rise info panel */

static  Panel_item	panel_prt[Nlabels];	/* moon infos */
static  Panel_item	panel_prl[2];		/* moon info lunation */
static	Panel_item	panel_psr[Rlabels];	/* moon info set/rise */
static  Panel_item	paneltoggle;		/* to switch test mode on/off */

static  Server_image	server_image;
static  Server_image	full_img;		/* panelicon fullmoon */
static  Server_image	halfl_img;		/* panelicon first quarter moon */
static  Server_image	halfr_img;		/* panelicon last quarter moon */
static  Server_image	new_img;		/* panelicon new moon */

static  Canvas		canvas;			/* display the moon on main panel */
static  Icon		icon;			/* frame icon */
Pixmap			pixmap;			/* pixmap to swapped into server_image*/
Pixwin	*pw;

Colormap	cmap;
XColor		ctmp;

XGCValues	gcv;
GC		gc;

static struct itimerval kickme = {{CLOSED_SECS, CLOSED_USECS}, {0, 1} };

/*
 * these two variables do nothing except provide unique addresses to be used
 * as client handles for registering with the notifier.
 */
static int	timer_client, sig_client;
/*Notify_client	timer_client, sig_client;*/ 

/*  Forward functions  */

static double		jtime(), phase();
static void		phasehunt(), set_mode(), fmt_phase_time();
static Notify_value	ringgg(), catch_winch(), catch_destroy();

static void		drawmoon(), jyear(), jhms();


main (argc, argv)
	int argc;
	char **argv;
{
	int		c;
	int		i, j;
	extern int	opterr;
	void canvas_repaint_proc();

	opterr = 0;
        while ((c = getopt (argc, argv, "mt")) != EOF) {
		switch (c) {
                case 't':                       /* jump into test mode */
			testmode = TRUE;
			break;

                case 'm':                       /* force to mono mode */
			color_mode = FALSE;
			break;

			break;
		}
	}
    if (opterr) {
	return 2;
    }

    if (color_mode == UNKNOWN) {
		color_mode = TRUE;	/* work for future releases */
    }

	xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);

	full_img = (Server_image)xv_create(XV_NULL, SERVER_IMAGE,
			XV_WIDTH,	full_width,
			XV_HEIGHT,	full_height,
			SERVER_IMAGE_X_BITS,	full_bits,
			NULL);
	halfl_img = (Server_image)xv_create(XV_NULL, SERVER_IMAGE,
			XV_WIDTH,       halfl_width,
			XV_HEIGHT,      halfl_height,
			SERVER_IMAGE_X_BITS,    halfl_bits,
			NULL);
	halfr_img = (Server_image)xv_create(XV_NULL, SERVER_IMAGE,
			XV_WIDTH,       halfr_width,
			XV_HEIGHT,      halfr_height,
			SERVER_IMAGE_X_BITS,    halfr_bits,
			NULL);
	new_img = (Server_image)xv_create(XV_NULL, SERVER_IMAGE,
			XV_WIDTH,       new_width,
			XV_HEIGHT,      new_height,
			SERVER_IMAGE_X_BITS,    new_bits,
			NULL);		
	
	server_image = (Server_image)xv_create(XV_NULL, SERVER_IMAGE, NULL);

	frame = (Frame)xv_create(XV_NULL, FRAME,
			 XV_LABEL,	MOON_TITLE,
			 XV_WIDTH,	510,
			 XV_HEIGHT,	370,
			 /*FRAME_CLOSED,	TRUE,*/
			 NULL);
	panel = (Panel)xv_create(frame, PANEL, NULL);
	panel_l = (Panel)xv_create(frame, PANEL,
			PANEL_BORDER,		TRUE,
			XV_X,			8,
			XV_Y,			12,
			NULL);
	xv_set (panel_l, PANEL_LAYOUT, PANEL_VERTICAL, NULL);

	panel_m = (Panel)xv_create(frame, PANEL,
			PANEL_BORDER,           TRUE,
			XV_X,                   8,
			NULL);
	xv_set (panel_m, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
	panel_u = (Panel)xv_create(frame, PANEL,
			PANEL_BORDER,		TRUE,
			XV_X,			8,
			NULL);
	xv_set (panel_u, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
	
	display = (Display *)xv_get(frame, XV_DISPLAY);
	
	gcv.function = GXcopy;
	gc = XCreateGC(display, xv_get(frame, XV_XID), 
			GCForeground | GCBackground | GCFunction, &gcv);
	
	if ( color_mode ) {
	
		cmap = DefaultColormap (display, DefaultScreen(display));
	
		for (i=0; i<4; i++) {
			ctmp.red = color_moon_col_r[i];
			ctmp.green = color_moon_col_g[i];
			ctmp.blue = color_moon_col_b[i];
			ctmp.flags = DoRed | DoGreen | DoBlue;
			XAllocColor(display, cmap, &ctmp);
			col_vals[i] = ctmp.pixel;
		}

		for (i=0; i<8; i++) {
			ctmp.red = c_r[i];
	  		ctmp.green = c_g[i];
	  		ctmp.blue = c_b[i];
	  		ctmp.flags = DoRed | DoGreen | DoBlue;
	  		XAllocColor(display, cmap, &ctmp);

			col_vals[i+4] = ctmp.pixel;
       		}

		pixmap = XCreatePixmap(display,
			RootWindow (display, DefaultScreen(display)), 64, 64,
			DefaultDepthOfScreen(DefaultScreenOfDisplay(display)));

		for (i=0; i<64; i++)
	    		for (j=0; j<64; j++) {
			XSetForeground(display, gc,
				col_vals[color_moon_pixels[(64*j)+i]]);
			XDrawPoint(display, pixmap, gc, i, j);
		}
		xv_set(server_image, SERVER_IMAGE_PIXMAP, pixmap, NULL);
	
	} else {
		
		server_image = (Server_image)xv_create(XV_NULL, SERVER_IMAGE,
					XV_WIDTH,	64,
					XV_HEIGHT,	64,
					SERVER_IMAGE_X_BITS,	moon_icon_bits,
					NULL);	
	}
	/* create the info panel */
	for( i=0; i < (Nlabels - 5); i++) {
		
		xv_create(panel_l, PANEL_MESSAGE,
				PANEL_LABEL_STRING,	labels[i],
				PANEL_LABEL_BOLD,	TRUE,
				PANEL_VALUE_X,		140,
				PANEL_NEXT_ROW,		5,
				NULL);
		xv_set(panel_l, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
		panel_prt[i] = xv_create(panel_l, PANEL_MESSAGE, NULL);
		xv_set(panel_l, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
	}
	/* create the second info panel */
	for( i=(Nlabels - 5); i < Nlabels; i++) {
	
		xv_create(panel_u, PANEL_MESSAGE,
				PANEL_LABEL_STRING,     labels[i],
				PANEL_LABEL_BOLD,       TRUE,
				PANEL_VALUE_X,          140,
				PANEL_NEXT_ROW,         5,
				NULL);
		xv_set(panel_u, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
		panel_prt[i] = xv_create(panel_u, PANEL_MESSAGE, NULL);
		if ( i == (Nlabels - 5) ) {
			xv_create(panel_u, PANEL_MESSAGE,
					PANEL_LABEL_STRING,     "Lunation :",
					PANEL_LABEL_BOLD,       TRUE,
					PANEL_VALUE_X,          450,
					NULL);
			panel_prl[1] = xv_create(panel_u, PANEL_MESSAGE, NULL);
		}
		if ( i == (Nlabels - 1) ) {
			xv_create(panel_u, PANEL_MESSAGE,
					PANEL_LABEL_STRING,     "Lunation :",
					PANEL_LABEL_BOLD,       TRUE,
					PANEL_VALUE_X,          450,
					NULL);
			panel_prl[2] = xv_create(panel_u, PANEL_MESSAGE, NULL);
		}
		xv_set(panel_u, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
		
	}
	/* create set/rise panel */
	xv_create(panel_m, PANEL_MESSAGE,
			PANEL_LABEL_STRING,	rlabels[0],
			PANEL_LABEL_BOLD,	TRUE,
			PANEL_VALUE_X,		140,
			PANEL_NEXT_ROW,		5,
			NULL);
	xv_set(panel_m, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
	panel_psr[0] = xv_create(panel_m, PANEL_MESSAGE, NULL);
	xv_create(panel_m, PANEL_MESSAGE,
			PANEL_LABEL_STRING,     rlabels[1],
			PANEL_LABEL_BOLD,       TRUE,
			PANEL_VALUE_X,          350,
			/*PANEL_NEXT_ROW,         5,*/
			NULL);
	xv_set(panel_m, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
	panel_psr[1] = xv_create(panel_m, PANEL_MESSAGE, NULL);
	xv_set(panel_m, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
	xv_create(panel_m, PANEL_MESSAGE,
			PANEL_LABEL_STRING,     rlabels[2],
			PANEL_LABEL_BOLD,       TRUE,
			PANEL_VALUE_X,          140,
			PANEL_NEXT_ROW,         5,
			NULL);
	xv_set(panel_m, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
	panel_psr[2] = xv_create(panel_m, PANEL_MESSAGE, NULL);
	xv_create(panel_m, PANEL_MESSAGE,
			PANEL_LABEL_STRING,     rlabels[3],
			PANEL_LABEL_BOLD,       TRUE,
			PANEL_VALUE_X,          350,
			/*PANEL_NEXT_ROW,         5,*/
			NULL);
	xv_set(panel_m, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);
	panel_psr[3] = xv_create(panel_m, PANEL_MESSAGE, NULL);

	/*for ( i=0; i<Rlabels; i++)
	xv_set(panel_psr[i], PANEL_LABEL_STRING, "12:30 GMT", NULL);
	*/
	
	/* draw the panelicons: new, first quarter, full, last quarter, new */
	xv_create(panel_u, PANEL_MESSAGE,
			PANEL_LABEL_IMAGE, new_img,
			XV_X,   5,
			XV_Y,   (int)xv_get(panel_prt[12], XV_Y),
			NULL);
	xv_create(panel_u, PANEL_MESSAGE,
			PANEL_LABEL_IMAGE, halfr_img,
			XV_X,   5,
			XV_Y,   (int)xv_get(panel_prt[13], XV_Y),
			NULL);
	xv_create(panel_u, PANEL_MESSAGE, 
			PANEL_LABEL_IMAGE, full_img,
			XV_X,	5,
			XV_Y,	(int)xv_get(panel_prt[14], XV_Y),
			NULL);
	xv_create(panel_u, PANEL_MESSAGE,
			PANEL_LABEL_IMAGE, halfl_img,
			XV_X,   5,
			XV_Y,   (int)xv_get(panel_prt[15], XV_Y),
			NULL);
	xv_create(panel_u, PANEL_MESSAGE,
			PANEL_LABEL_IMAGE, new_img,
			XV_X,   5,
			XV_Y,   (int)xv_get(panel_prt[16], XV_Y),
			NULL);	
	
	/* create paneltoggle to switch test mode on/off */
	paneltoggle = (Panel_item)xv_create(panel, PANEL_TOGGLE,
			PANEL_FEEDBACK,	PANEL_MARKED,
			PANEL_LABEL_STRING,	"Test mode",
			XV_X,		400,
			XV_Y,		10,
			PANEL_NOTIFY_PROC,	set_mode,
			PANEL_VALUE,		testmode,
			NULL);
	/* canvas for displaying the moon */
	canvas = (Canvas)xv_create(frame, CANVAS,
			 XV_X,		415,
			 XV_Y,		50,
			 XV_WIDTH,	72,
			 XV_HEIGHT,	72,
			 CANVAS_REPAINT_PROC,	canvas_repaint_proc,
			  NULL);
	/* create the frame icon */
	icon = (Icon)xv_create(frame, ICON,
			XV_HEIGHT,	78,
			ICON_IMAGE, 	server_image,
			WIN_BACKGROUND_COLOR, color_mode?1:0, 
			WIN_FOREGROUND_COLOR, color_mode?0:1, 
			NULL);

	xv_set(frame, FRAME_ICON, icon, NULL);

	/* first loop to adjust the panels */
	ringgg( (Notify_client)NULL, 0);
	window_fit(panel_l);
	xv_set(panel_m, XV_Y, (int)xv_get(panel_l, XV_HEIGHT) + 12 + 8);
	window_fit_height(panel_m);
	xv_set(panel_u, XV_Y,( (int)xv_get(panel_l, XV_HEIGHT) + 12 + 8 + 8
					+ (int)xv_get(panel_m, XV_HEIGHT) ));
	window_fit(panel_u);
	xv_set(panel_m, XV_WIDTH, (int)xv_get(panel_u, XV_WIDTH) );
	
	/* starting iconic */
	xv_set(frame, FRAME_CLOSED, TRUE, NULL);
		
	/* mmh, it dosen't work under Linux, we use notify_set_event_func() */
	/*notify_set_signal_func( (Notify_client)&sig_client, catch_winch, SIGWINCH,
			NOTIFY_SYNC);*/

	notify_set_event_func( (Notify_client)frame, catch_winch, NOTIFY_SAFE);

	notify_interpose_destroy_func (frame, catch_destroy);

	xv_main_loop(frame);
}

void canvas_repaint_proc(canvas, canvas_pw, repaint_area)

Canvas canvas;
Pixwin *canvas_pw;
Rectlist *repaint_area;
{
	xv_rop((Xv_opaque)canvas_pw, 3, 3, 64, 64, PIX_SRC,
				(Pixrect *)server_image, 0, 0);
}


/*ARGSUSED*/
static
void
set_mode (item, event)
	Panel_item	item;
	Event		*event;
{
	testmode = (int)xv_get (paneltoggle, PANEL_VALUE);
	if (testmode) {
	   faketime = 0.0;
	   epoch_seconds_test = 0.0;
	} else
	   nptime = 0.0;		/* force lunation info to update */
	/* fake a sigwinch to modify the timer */
	(void)catch_winch ( &sig_client, SIGWINCH, NOTIFY_SYNC);
}

/*
 * Catch window change events.  We'll get at least one winch each time the
 * window opens or closes, as well as when a portion of it is uncovered.
 * We also get a winch when the tool first appears, we depend on this fact to
 * start the timer running.  Each time we get a winch we check the current open
 * state of the tool and set the timer interval appropriately.	The timer
 * we set has a nearly immediate initial trip which will cause a refresh
 * of the icon or open window, then a periodic trip which depends on whether
 * the window is open or closed.
 */

/*ARGSUSED*/
static
Notify_value
catch_winch (client, signal_num, mode)
	Notify_client		client;
	int			signal_num;
	Notify_signal_mode	mode;
{
	if (testmode) {
		kickme = NOTIFY_POLLING_ITIMER; 	/* run flat out */
	} else {
		if ((int) xv_get(frame, FRAME_CLOSED) == TRUE) {
			kickme.it_interval.tv_sec = CLOSED_SECS;
			kickme.it_interval.tv_usec = CLOSED_USECS;
		} else {
			kickme.it_interval.tv_sec = OPEN_SECS;
			kickme.it_interval.tv_usec = OPEN_USECS;
		}
		kickme.it_value.tv_sec = 0;	/* immediate initial trip */
		kickme.it_value.tv_usec = 10000;
	}

	/* Set/change the timer to the proper interval for new window state */
	notify_set_itimer_func ( (Notify_client)&timer_client, ringgg, ITIMER_REAL,
		&kickme, NULL);
	return (NOTIFY_DONE);
}

/*
 *	Interpose on the destroy function so that we can stop the timer before
 *	the windows go away.  This avoids a bunch of irritating ioctl failure
 *	messages being kicked out under certain circumstances.
 */

static
Notify_value
catch_destroy (frame, status)
	Frame		frame;
	Destroy_status	status;
{
	if (status != DESTROY_CHECKING) {
		notify_set_itimer_func ( (Notify_client)&timer_client, (Notify_value (*)())0,
			ITIMER_REAL, (struct itimerval *)0, NULL);
	}

	return (notify_next_destroy_func (frame, status));
}

/*  ROP  --  Perform raster op on icon.  */

static void rop(sx, sy, lx)
  int sx, sy, lx;
{
    int rowoff = sy * (moon_icon_width / 8), i;

    for (i = sx; i < (sx + lx); i++) {
	moon_icon_work[rowoff + (i / 8)] =
	    (moon_icon_work[rowoff + (i / 8)] & ~(1 << (i & 7))) |
	    (moon_icon_bits[rowoff + (i / 8)] &
		(1 << (i & 7)));
    }
}

static void rop2(sx, sy, lx, pixmap)
  int sx, sy, lx;
  Pixmap pixmap;
{
    int rowoff = sy * (moon_icon_width / 8), i;

    for (i = sx; i < (sx + lx); i++) {
       XSetForeground(display, gc,
		      col_vals[color_moon_pixels[(64*sy)+i]+4]);
       XDrawPoint(display, pixmap, gc, i, sy);
    }
}

/*   DRAWMOON  --  Construct icon for moon, given phase of moon.  */

static void drawmoon(ph, aom_d, aom_h, aom_m)
  double ph;
  int aom_d, aom_h, aom_m;
{
    register int i, lx, rx, j;
    register double cp, xscale;
    static char tbuf[20];


#define RADIUS		27.0
#define IRADIUS 	27
#define OFFSET		28
#define CENTER		32


    if (aom_d == 0) {
        sprintf(tbuf, "%dh %dm", aom_h, aom_m);
    } else {
        sprintf(tbuf, "%dd %dh", aom_d, aom_h);
    }

    if( color_mode == FALSE ) {
       memset(moon_icon_work, 0xFF, sizeof moon_icon_work);

       xscale = cos(2 * PI * ph);
       for (i = 0; i < IRADIUS; i++) {
	  cp = RADIUS * cos(asin((double) i / RADIUS));
	  if (ph < 0.5) {
	     rx = CENTER + cp;
	     lx = CENTER + xscale * cp;
	  } else {
	     lx = CENTER - cp;
	     rx = CENTER - xscale * cp;
	  }

	  /* We  now know the left and right  endpoints of the scan line
	     for this y  coordinate.   We  raster-op  the  corresponding
	     scanlines  from  the  source  pixrect  to  the  destination
	     pixrect, offsetting to properly place it in the pixrect and
	     reflecting vertically. */

	  rop(lx, OFFSET + i, (rx - lx) + 1);
	  if (i != 0) {
	     rop(lx, OFFSET - i, (rx - lx) + 1);
	  }
       }

	  /*We  don't  update  the  icon  unless  it's  actually
	  changed.    This  not  only  saves  time,  it  also  eliminates
	  unnecessary blinking of the icon on the display due to nugatory
	  refreshes. */

       if ((memcmp(moon_icon_work, moon_icon_last, sizeof moon_icon_last) != 0) ) {
	  memcpy(moon_icon_last, moon_icon_work, sizeof moon_icon_last);

	  xv_set(server_image, SERVER_IMAGE_X_BITS, moon_icon_work, NULL);
	  
	  pw = canvas_pixwin(canvas);
	  xv_rop( (Xv_opaque)pw, 3, 3, 64, 64, PIX_SRC,
	  			(Pixrect *)server_image, 0, 0 );
	  			
	  xv_set(icon, ICON_IMAGE, server_image, NULL);
	  
	  xv_set(frame, FRAME_ICON, icon, NULL);
	  
       }
    } else {

       int lval;

       memset(moon_icon_work, 0xFF, sizeof moon_icon_work);

       xscale = cos(2 * PI * ph);
       for (i = 0; i < IRADIUS; i++) {
	  cp = RADIUS * cos(asin((double) i / RADIUS));
	  if (ph < 0.5) {
	     rx = CENTER + cp;
	     lx = CENTER + xscale * cp;
	  } else {
	     lx = CENTER - cp;
	     rx = CENTER - xscale * cp;
	  }

	  /* We  now know the left and right  endpoints of the scan line
	     for this y  coordinate.   We  raster-op  the  corresponding
	     scanlines  from  the  source  pixrect  to  the  destination
	     pixrect, offsetting to properly place it in the pixrect and
	     reflecting vertically. */

	  rop(lx, OFFSET + i, (rx - lx) + 1);
	  if (i != 0) {
	     rop(lx, OFFSET - i, (rx - lx) + 1);
	  }
       }       
       if (memcmp(moon_icon_work, moon_icon_last, sizeof moon_icon_last) != 0) {
	  memcpy(moon_icon_last, moon_icon_work, sizeof moon_icon_last);

          for (i=0; i<64; i++)
	     for (j=0; j<64; j++) {
	        lval = color_moon_pixels[(64*j)+i];
	        XSetForeground(display, gc,
			       col_vals[(lval>0)?lval+8:4]);
	        XDrawPoint(display, pixmap, gc, i, j);
	     } 

          for (i=0; i<IRADIUS; i++) {
	     cp = RADIUS * cos(asin((double)i/RADIUS));
	     if (ph < 0.5) {
	        rx = CENTER + cp;
	        lx = CENTER + xscale * cp;
	     } else {
	        lx = CENTER - cp;
	        rx = CENTER - xscale * cp;
	     }
	     rop2(lx, OFFSET+i, (rx-lx)+1, pixmap);
	     if (i != 0)
	        rop2(lx, OFFSET-i, (rx-lx)+1, pixmap);
          }


          XSetForeground(display, gc, col_vals[8]);
          XDrawPoint(display, pixmap, gc, 41, 29);

          xv_set(server_image, SERVER_IMAGE_PIXMAP, pixmap, NULL);
          
          pw = canvas_pixwin(canvas);
          xv_rop( (Xv_opaque)pw, 3, 3, 64, 64, PIX_SRC,
          	 			(Pixrect *)server_image, 0, 0 );
          
          xv_set(icon, ICON_IMAGE, server_image, NULL);
	  xv_set(frame, FRAME_ICON, icon, NULL);

       }
    }

    /* Update the date of the moon in the icon label if it's changed. */

    if (strcmp(tbuf, icon_label_last) != 0) {
	strcpy(icon_label_last, tbuf);
	xv_set(icon, ICON_LABEL, tbuf, NULL);
    }

}

/*
 * RINGGG  --	Update status on interval timer ticks and redraw
 *		window if needed.
 */
#define prt(y) xv_set(panel_prt[y-1], PANEL_LABEL_STRING, tbuf, NULL)
#define prl(y) xv_set(panel_prl[y], PANEL_LABEL_STRING, tbuf, NULL)

#define EPL(x) (x), (x) == 1 ? "" : "s"
#define APOS(x) (x + 13)

/*ARGSUSED*/
static
Notify_value
ringgg (client, itimer_type)
	Notify_client	client;
	int		itimer_type;
{
	int		lunation;
	int		i, yy, mm, dd, hh, mmm, ss;
	int		aom_d, aom_h, aom_m;
	int		op;
	long		t;
	double		jd, p, aom, cphase, cdist, cangdia, csund, csuang;
	double		phasar [5];
	char		tbuf[80];
	struct tm	*gm;

	(void) time (&t);
	jd = jtime ((gm = gmtime (&t)));
	if (testmode) {
		if (faketime == 0.0) {
			faketime = jd + 1;		/* one day step */
			epoch_seconds_test = t + 86400; /* Sec_per_day */
		} else {
			faketime += 1.0 / 24;		/* one hour step */
			epoch_seconds_test += 3600;	/* sec per hour */
		}
		jd = faketime;
	}

    p = phase(jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
    aom_d = (int) aom;
    aom_h = (int) (24 * (aom - floor(aom)));
    aom_m = (int) (1440 * (aom - floor(aom))) % 60;

    drawmoon(p, aom_d, aom_h, aom_m);

	if ((int) xv_get (frame, FRAME_CLOSED) == TRUE) {
		
		/* If we're iconic, there's nothing more to do. */
		return (NOTIFY_DONE);
	}

    /* Update textual information for open window */

    sprintf(tbuf, "%.5f", jd + 0.5);  /* Julian date */
    prt(1);

    if (testmode) {		      /* Universal time */
	jyear(jd, &yy, &mm, &dd);
	jhms(jd, &hh, &mmm, &ss);
        sprintf(tbuf, "%2d:%02d:%02d %2d %s %d",
		hh, mmm, ss, dd, moname [mm - 1], yy);
    } else {
        sprintf(tbuf, "%2d:%02d:%02d %2d %s 19%d",
		gm->tm_hour, gm->tm_min, gm->tm_sec,
		gm->tm_mday, moname [gm->tm_mon], gm->tm_year);
    }
    prt(2);

    if (testmode == FALSE) {	      /* Ignore local time in test mode */
	gm = localtime(&t);

	/* Local time */

        sprintf(tbuf, "%2d:%02d:%02d %2d %s 19%d",
		gm->tm_hour, gm->tm_min, gm->tm_sec,
		gm->tm_mday, moname [gm->tm_mon], gm->tm_year);
	prt(3);
    }

    /* Information about the Moon */

    /* Age of moon */

    sprintf(tbuf, "%d day%s, %d hour%s, %d minute%s.",
	    EPL(aom_d), EPL(aom_h), EPL(aom_m));
    prt(5);

    /* Moon phase */

    sprintf(tbuf, "%d%%   (0%% = New, 100%% = Full)",
	    (int) (cphase * 100));
    while (strlen(tbuf) < 40) {
        strcat(tbuf, " ");
    }
    prt(6);

    /* Moon distance */

    sprintf(tbuf, "%ld kilometres, %.1f Earth radii.",
	    (long) cdist, cdist / earthrad);
    prt(7);

    /* Moon subtends */

    sprintf(tbuf, "%.4f degrees.", cangdia);
    prt(8);

    /* Information about the Sun */

    /* Sun subtends */
    
    sprintf(tbuf, "%.4f degrees.", csuang);
    prt(10);

    /* Sun's distance */

    sprintf(tbuf, "%.0f kilometres    \n%.3f astronomical units.",
    		csund, csund / sunsmax);
    prt(11);

   /* Calculate times of phases of this lunation.  This is
       sufficiently time-consuming that we only do it once a month. */

    if (jd > nptime) {
	phasehunt(jd + 0.5, phasar);
	lunation = floor(((phasar[0] + 7) - lunatbase) / synmonth) + 1;

	for (i = 0; i < 5; i++) {
	    fmt_phase_time(phasar[i], tbuf);
	    prt(APOS(i));
	}
	nptime = phasar[4];

	/* Edit lunation numbers into cells reserved for them. */

        sprintf(tbuf, "%d", lunation);
	prl(1);
        sprintf(tbuf, "%d", lunation + 1);
	prl(2);
    }
    /* calculate the moon rise/set time for today and tomorrow */
    if (testmode) {	/* show only rise/set for corresponding day */
    	riseset((long)epoch_seconds_test);
    	xv_set(panel_psr[0], PANEL_LABEL_STRING, riseset_buf[B_MRD], NULL);
    	xv_set(panel_psr[1], PANEL_LABEL_STRING, 0, NULL);
    	xv_set(panel_psr[2], PANEL_LABEL_STRING, riseset_buf[B_MSD], NULL);
    	xv_set(panel_psr[3], PANEL_LABEL_STRING, 0, NULL);
    	day_last = 0;
    } else {
    	gm = localtime(&t);
        if ( gm->tm_mday !=  day_last ) {
        	day_last = gm->tm_mday;
    		riseset(t);
    		xv_set(panel_psr[0], PANEL_LABEL_STRING, riseset_buf[B_MRD], NULL);
    		xv_set(panel_psr[1], PANEL_LABEL_STRING, riseset_buf[B_MRT], NULL);
    		xv_set(panel_psr[2], PANEL_LABEL_STRING, riseset_buf[B_MSD], NULL);
    		xv_set(panel_psr[3], PANEL_LABEL_STRING, riseset_buf[B_MST], NULL);
    	}
    }		
    return;
}
#undef APOS


/*  FMT_PHASE_TIME  --	Format	the  provided  julian  date  into  the
			provided  buffer  in  UTC  format  for	screen
			display  */

static void fmt_phase_time(utime, buf)
    double  utime;
    char    *buf;
{
    int yy, mm, dd, hh, mmm, ss;

    jyear(utime, &yy, &mm, &dd);
    jhms(utime, &hh, &mmm, &ss);
    sprintf(buf, "%2d:%02d UTC %2d %s %d",
	hh, mmm, dd, moname [mm - 1], yy);
}


/*  JDATE  --  Convert internal GMT date and time to  Julian  day  and
	       fraction.  */

static long jdate(t)
  struct tm *t;
{
    long c, m, y;

    y = t->tm_year + 1900;
    m = t->tm_mon + 1;
    if (m > 2) {
	m = m - 3;
    } else {
	m = m + 9;
	y--;
    }
    c = y / 100L;		      /* Compute century */
    y -= 100L * c;
    return (t->tm_mday + (c * 146097L) / 4 + (y * 1461L) / 4 +
	   (m * 153L + 2) / 5 + 1721119L);
}


/*  JTIME  --  Convert internal GMT  date  and	time  to  astronomical
	       Julian	time  (i.e. Julian  date  plus	day  fraction,
	       expressed as a double).	*/

static double jtime(t)
  struct tm *t;
{
    return (jdate(t) - 0.5) + 
	   (t->tm_sec + 60 * (t->tm_min + 60 * t->tm_hour)) / 86400.0;
}


/*  JYEAR  --  Convert	Julian	date  to  year,  month, day, which are
	       returned via integer pointers to integers.  */

static void jyear(td, yy, mm, dd)
  double  td;
  int *yy, *mm, *dd;
{
    double j, d, y, m;

    td += 0.5;			      /* Astronomical to civil */
    j = floor(td);
    j = j - 1721119.0;
    y = floor(((4 * j) - 1) / 146097.0);
    j = (j * 4.0) - (1.0 + (146097.0 * y));
    d = floor(j / 4.0);
    j = floor(((4.0 * d) + 3.0) / 1461.0);
    d = ((4.0 * d) + 3.0) - (1461.0 * j);
    d = floor((d + 4.0) / 4.0);
    m = floor(((5.0 * d) - 3) / 153.0);
    d = (5.0 * d) - (3.0 + (153.0 * m));
    d = floor((d + 5.0) / 5.0);
    y = (100.0 * y) + j;
    if (m < 10.0) {
	m = m + 3;
    } else {
	m = m - 9;
	y = y + 1;
    }
    *yy = y;
     *mm = m;
    *dd = d;
}


/*  JHMS  --  Convert Julian time to hour, minutes, and seconds.  */

static void jhms(j, h, m, s)
  double j;
  int *h, *m, *s;
{
    long ij;

    j += 0.5;			      /* Astronomical to civil */
    ij = (j - floor(j)) * 86400.0;
    *h = ij / 3600L;
    *m = (ij / 60L) % 60L;
    *s = ij % 60L;
}


/*  MEANPHASE  --  Calculates  time  of  the mean new Moon for a given
		   base date.  This argument K to this function is the
		   precomputed synodic month index, given by:
  
			  K = (year - 1900) * 12.3685
  
		   where year is expressed as a year and fractional year.  */

static double meanphase(sdate, k)
  double sdate, k;
{
    double t, t2, t3, nt1;

    /* Time in Julian centuries from 1900 January 0.5 */
    t = (sdate - 2415020.0) / 36525;
    t2 = t * t; 		      /* Square for frequent use */
    t3 = t2 * t;		      /* Cube for frequent use */

    nt1 = 2415020.75933 + synmonth * k
	    + 0.0001178 * t2
	    - 0.000000155 * t3
	    + 0.00033 * dsin(166.56 + 132.87 * t - 0.009173 * t2);

    return nt1;
}


/*  TRUEPHASE  --  Given a K value used to determine the mean phase of
		   the new moon, and a phase selector (0.0, 0.25, 0.5,
		   0.75), obtain the true, corrected phase time.  */

static double truephase(k, phase)
  double k, phase;
{
    double t, t2, t3, pt, m, mprime, f;
    int apcor = FALSE;

    k += phase; 		      /* Add phase to new moon time */
    t = k / 1236.85;		      /* Time in Julian centuries from
					 1900 January 0.5 */
    t2 = t * t; 		      /* Square for frequent use */
    t3 = t2 * t;		      /* Cube for frequent use */
    pt = 2415020.75933		      /* Mean time of phase */
	 + synmonth * k
	 + 0.0001178 * t2
	 - 0.000000155 * t3
	 + 0.00033 * dsin(166.56 + 132.87 * t - 0.009173 * t2);

    m = 359.2242                      /* Sun's mean anomaly */
	+ 29.10535608 * k
	- 0.0000333 * t2
	- 0.00000347 * t3;
    mprime = 306.0253                 /* Moon's mean anomaly */
	+ 385.81691806 * k
	+ 0.0107306 * t2
	+ 0.00001236 * t3;
    f = 21.2964                       /* Moon's argument of latitude */
	+ 390.67050646 * k
	- 0.0016528 * t2
	- 0.00000239 * t3;
    if ((phase < 0.01) || (abs(phase - 0.5) < 0.01)) {

       /* Corrections for New and Full Moon */

       pt +=	 (0.1734 - 0.000393 * t) * dsin(m)
		+ 0.0021 * dsin(2 * m)
		- 0.4068 * dsin(mprime)
		+ 0.0161 * dsin(2 * mprime)
		- 0.0004 * dsin(3 * mprime)
		+ 0.0104 * dsin(2 * f)
		- 0.0051 * dsin(m + mprime)
		- 0.0074 * dsin(m - mprime)
		+ 0.0004 * dsin(2 * f + m)
		- 0.0004 * dsin(2 * f - m)
		- 0.0006 * dsin(2 * f + mprime)
		+ 0.0010 * dsin(2 * f - mprime)
		+ 0.0005 * dsin(m + 2 * mprime);
       apcor = TRUE;
    } else if ((abs(phase - 0.25) < 0.01 || (abs(phase - 0.75) < 0.01))) {
       pt +=	 (0.1721 - 0.0004 * t) * dsin(m)
		+ 0.0021 * dsin(2 * m)
		- 0.6280 * dsin(mprime)
		+ 0.0089 * dsin(2 * mprime)
		- 0.0004 * dsin(3 * mprime)
		+ 0.0079 * dsin(2 * f)
		- 0.0119 * dsin(m + mprime)
		- 0.0047 * dsin(m - mprime)
		+ 0.0003 * dsin(2 * f + m)
		- 0.0004 * dsin(2 * f - m)
		- 0.0006 * dsin(2 * f + mprime)
		+ 0.0021 * dsin(2 * f - mprime)
		+ 0.0003 * dsin(m + 2 * mprime)
		+ 0.0004 * dsin(m - 2 * mprime)
		- 0.0003 * dsin(2 * m + mprime);
       if (phase < 0.5)
	  /* First quarter correction */
	  pt += 0.0028 - 0.0004 * dcos(m) + 0.0003 * dcos(mprime);
       else
	  /* Last quarter correction */
	  pt += -0.0028 + 0.0004 * dcos(m) - 0.0003 * dcos(mprime);
       apcor = TRUE;
    }
    if (!apcor) {
	fprintf(stderr,
            "TRUEPHASE called with invalid phase selector.\n");
	abort();
    }
    return pt;
}


/*   PHASEHUNT	--  Find time of phases of the moon which surround the
		    current date.  Five phases are found, starting and
		    ending with the new moons which bound the  current
		    lunation.  */

static void phasehunt(sdate, phases)
  double sdate;
  double phases[5];
{
    double adate, k1, k2, nt1, nt2;
    int yy, mm, dd;

    adate = sdate - 45;

    jyear(adate, &yy, &mm, &dd);
    k1 = floor((yy + ((mm - 1) * (1.0 / 12.0)) - 1900) * 12.3685);

    adate = nt1 = meanphase(adate, k1);
    while (TRUE) {
	adate += synmonth;
	k2 = k1 + 1;
	nt2 = meanphase(adate, k2);
	if (nt1 <= sdate && nt2 > sdate)
	    break;
	nt1 = nt2;
	k1 = k2;
    }
    phases[0] = truephase(k1, 0.0);
    phases[1] = truephase(k1, 0.25);
    phases[2] = truephase(k1, 0.5);
    phases[3] = truephase(k1, 0.75);
    phases[4] = truephase(k2, 0.0);
}


/*  KEPLER  --	 Solve the equation of Kepler.	*/

static double kepler(m, ecc)
  double m, ecc;
{
    double e, delta;
#define EPSILON 1E-6

    e = m = torad(m);
    do {
	delta = e - ecc * sin(e) - m;
	e -= delta / (1 - ecc * cos(e));
    } while (abs(delta) > EPSILON);
    return e;
}

/*  PHASE  --  Calculate phase of moon as a fraction:
  
    The  argument  is  the  time  for  which  the  phase is requested,
    expressed as a Julian date and fraction.  Returns  the  terminator
    phase  angle  as a percentage of a full circle (i.e., 0 to 1), and
    stores into pointer arguments  the	illuminated  fraction  of  the
    Moon's  disc, the Moon's age in days and fraction, the distance of
    the Moon from the centre of the Earth, and	the  angular  diameter
    subtended  by the Moon as seen by an observer at the centre of the
    Earth.
*/

static double phase(pdate, pphase, mage, dist, angdia, sudist, suangdia)
  double  pdate;
  double  *pphase;		      /* Illuminated fraction */
  double  *mage;		      /* Age of moon in days */
  double  *dist;		      /* Distance in kilometres */
  double  *angdia;		      /* Angular diameter in degrees */
  double  *sudist;		      /* Distance to Sun */
  double  *suangdia;                  /* Sun's angular diameter */
{

    double Day, N, M, Ec, Lambdasun, ml, MM, MN, Ev, Ae, A3, MmP,
	   mEc, A4, lP, V, lPP, NP, y, x, Lambdamoon, BetaM,
	   MoonAge, MoonPhase,
	   MoonDist, MoonDFrac, MoonAng, MoonPar,
	   F, SunDist, SunAng;

    /* Calculation of the Sun's position */

    Day = pdate - epoch;		    /* Date within epoch */
    N = fixangle((360 / 365.2422) * Day);   /* Mean anomaly of the Sun */
    M = fixangle(N + elonge - elongp);	    /* Convert from perigee
					    co-ordinates to epoch 1980.0 */
    Ec = kepler(M, eccent);		    /* Solve equation of Kepler */
    Ec = sqrt((1 + eccent) / (1 - eccent)) * tan(Ec / 2);
    Ec = 2 * todeg(atan(Ec));		    /* True anomaly */
    Lambdasun = fixangle(Ec + elongp);      /* Sun's geocentric ecliptic
						    longitude */
    /* Orbital distance factor */
    F = ((1 + eccent * cos(torad(Ec))) / (1 - eccent * eccent));
    SunDist = sunsmax / F;		    /* Distance to Sun in km */
    SunAng = F * sunangsiz;                 /* Sun's angular size in degrees */


    /* Calculation of the Moon's position */

    /* Moon's mean longitude */
    ml = fixangle(13.1763966 * Day + mmlong);

    /* Moon's mean anomaly */
    MM = fixangle(ml - 0.1114041 * Day - mmlongp);

    /* Moon's ascending node mean longitude */
    MN = fixangle(mlnode - 0.0529539 * Day);

    /* Evection */
    Ev = 1.2739 * sin(torad(2 * (ml - Lambdasun) - MM));

    /* Annual equation */
    Ae = 0.1858 * sin(torad(M));

    /* Correction term */
    A3 = 0.37 * sin(torad(M));

    /* Corrected anomaly */
    MmP = MM + Ev - Ae - A3;

    /* Correction for the equation of the centre */
    mEc = 6.2886 * sin(torad(MmP));

    /* Another correction term */
    A4 = 0.214 * sin(torad(2 * MmP));

    /* Corrected longitude */
    lP = ml + Ev + mEc - Ae + A4;

    /* Variation */
    V = 0.6583 * sin(torad(2 * (lP - Lambdasun)));

    /* True longitude */
    lPP = lP + V;

    /* Corrected longitude of the node */
    NP = MN - 0.16 * sin(torad(M));

    /* Y inclination coordinate */
    y = sin(torad(lPP - NP)) * cos(torad(minc));

    /* X inclination coordinate */
    x = cos(torad(lPP - NP));

    /* Ecliptic longitude */
    Lambdamoon = todeg(atan2(y, x));
    Lambdamoon += NP;

    /* Ecliptic latitude */
    BetaM = todeg(asin(sin(torad(lPP - NP)) * sin(torad(minc))));

    /* Calculation of the phase of the Moon */

    /* Age of the Moon in degrees */
    MoonAge = lPP - Lambdasun;

    /* Phase of the Moon */
    MoonPhase = (1 - cos(torad(MoonAge))) / 2;

    /* Calculate distance of moon from the centre of the Earth */

    MoonDist = (msmax * (1 - mecc * mecc)) /
       (1 + mecc * cos(torad(MmP + mEc)));

    /* Calculate Moon's angular diameter */

    MoonDFrac = MoonDist / msmax;
    MoonAng = mangsiz / MoonDFrac;

    /* Calculate Moon's parallax */

    MoonPar = mparallax / MoonDFrac;

    *pphase = MoonPhase;
    *mage = synmonth * (fixangle(MoonAge) / 360.0);
    *dist = MoonDist;
    *angdia = MoonAng;
    *sudist = SunDist;
    *suangdia = SunAng;
    return fixangle(MoonAge) / 360.0;
}

		
