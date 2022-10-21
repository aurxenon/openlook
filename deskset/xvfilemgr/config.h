/* config.h.  Generated automatically by configure.  */
/* Version of XVfilemgr. */
#define XVFILEMGR_VERSION "0.2g ALPHA"

/* location of the system wide filetype file                         */
#define FILETYPE "/etc/filetype"

/* Maximum number of bytes in output window                          */
#define MAX_LOG_LENGTH 400000

/* Mail program to use to send feedback. Must me command line driven */
#define DEFAULT_MAILER "/bin/mail"

/* Defines directories where icons named in "filetype" will be searched 
   for. This allows the user to reference the filenames only and not 
   the entire path to the icon files. Absolute filenames are searched
   first, then filenames within the directories mentionned below. 
   IMPORTANT: if you want to use "make install" then make sure those
   paths reflect the values set in the Imakefile/Makefile. They are set
   to /usr/openwin/lib/pixmaps by default. */
#define IMAGESDIR "/usr/openwin/lib/pixmaps"

/* Define this to let all dialogs appear at the right side of the 
   application window.  If you undefine this the window manager decides
   where to put the dialogs itself.  
   Defining DIALOG_ALWAYS_RIGHT could lead to some problems when using
   ol[v]wm.  Sometimes the application window gets invisible when moving
   the dialog around.  In this case the application window can be mapped
   again by dragging it in the virtual desktop or by refreshing the
   desktop.                                                          */
/* #undef DIALOG_ALWAYS_RIGHT */

/* These commands are hard-coded for now but this is subject to change */
#define DEFAULT_MV_CMD "/bin/mv"
#define DEFAULT_CP_CMD "/bin/cp"
#define DEFAULT_RM_CMD "/bin/rm"

/* Define default text editor, by default xvfilemgr will try
   /usr/openwin/bin/textedit */
#define DEFAULT_EDITOR "/usr/openwin/bin/textedit"

/* If you have a cd rom specify the point where it should be mounted and
   the device file.  The mount point is totaly transparent to the user
   of XVFilemgr. This is subject to change. Xvfilemgr will use runtime 
   analysis of your /etc/fstab and /etc/mtab.                        */
#define CDROM_DEVICE     "/dev/cdrom"
#define CDROM_MOUNTPOINT "/mnt/cdrom0"

/* Same as above but for flopy discs.                                */
#define FLOPPY_DEVICE     "/dev/fd0"
#define FLOPPY_MOUNTPOINT "/mnt0"
