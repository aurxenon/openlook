/*
	System  : XVFilemgr
	File    : $RCSfile: misc.c,v $
	Author  : $Author: arkenoi $
	Date    : $Date: 2005/07/13 18:31:02 $
	Purpose : miscellaneous things
*
* $Log: misc.c,v $
* Revision 1.1.1.1  2005/07/13 18:31:02  arkenoi
* Initial import of 0.2g
*
*
* Revision 1.2  1996/08/16 17:11:27  root
* expandation of filename expressions.
*
* Revision 1.1  1996/07/27 08:52:30  root
* Initial revision
*
*
*/

#include <pwd.h>
#include <sys/types.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>

#include "config.h"
#include "global.h"
#include "misc.h"

extern char *
find_dotfile (char *dotfile)
{
        char    *home;
        char    *filename = NULL;

        home = getenv("HOME");
        if (home != NULL && home[0] != '\0') {
                /* try $HOME/dotfile */
                filename = malloc((unsigned int)(strlen(home) + 1 +
                    strlen(dotfile) + 1));
                if (filename == NULL)
                        return (NULL);
                sprintf(filename, "%s/%s", home, dotfile);
                if (access(filename, F_OK) == -1) {
                        free(filename);
                        return (NULL);
                }
                /* found it */
                return (filename);
        }
        filename = strdup(dotfile);
        if (filename == NULL)
                return (NULL);
        if (access(filename, F_OK) == -1) {
                free(filename);
                return (NULL);
        }

        return (filename);
} /* find_dotfile */

extern char *
expand_dirname(char *arg, Applic_T *app)
{
        char    *slash;
        char    *lastpart = "";
        char    *path;
        struct passwd *pwd;
        char    *firstpart = "";
        char    *s;
        char    message[PATH_MAX + NAME_MAX + 1];

        if (arg[0] == '/' || (arg[0] != '~' && arg[0] != '$')) {
                path = strdup(arg);
                return (path);
        }
        s = strdup(arg);
        if (s == NULL)
                return (NULL);
        if ((slash = index(s, '/')) != NULL) {
                *slash = 0;
                lastpart = slash + 1;
        }
        switch (s[0]) {
        case '~': /* ~ or ~user */
                if (s[1] == '\0') {
                        pwd = getpwuid(getuid());
                        if (pwd == NULL) {
                                xv_set(app->frame,
                                FRAME_LEFT_FOOTER,
                                "Your userid is unknown to the system.",
                                NULL);
                                free(s);
                                return (NULL);
                        }
                } else {
                        pwd = getpwnam(&s[1]);
                        if (pwd == NULL) {
                                sprintf(message, "Unknown user %s.", &s[1]);
                                xv_set(app->frame,
                                FRAME_LEFT_FOOTER, message,
                                NULL);
                                free(s);
                                return (NULL);
                        }
                }
                firstpart = pwd->pw_dir;
                break;
        case '$': /* Environment variable */
                firstpart = getenv(&s[1]);
                if (firstpart == NULL) {
                        sprintf(message, "Unknown variable %s.", &s[1]);
                        xv_set(app->frame,
                        FRAME_LEFT_FOOTER, message,
                        NULL);
                        free(s);
                        return (NULL);
                }
                break;
        }
        path = (char *)malloc((unsigned int)(strlen(firstpart) +
            1 + strlen(lastpart) + 1));
        if (path == NULL) {
                xv_set(app->frame,
                FRAME_LEFT_FOOTER, "Memory allocation failed.",
                NULL);
                free(s);
                return (NULL);
        }
        if (lastpart[0] != '\0')
                sprintf(path, "%s/%s", firstpart, lastpart);
        else
                strcpy(path, firstpart);
        free(s);
        return (path);
}
