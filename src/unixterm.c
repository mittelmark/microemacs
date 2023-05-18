/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * unixterm.c - Unix X-Term and Termcap support routines.
 *
 * Copyright (C) 1993-2001 Steven Phillips
 * Copyright (C) 2002-2010 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Created:     1993
 * Synopsis:    Unix X-Term and Termcap support routines.
 * Authors:     Steven Phillips
 * Description:
 *    This implementation of unix support currently only supports Unix v5 (_USG),
 *    there is no current support for V7 or VMS.
 *    Implemented XTerm support combined with shell termcap.
 */

#include "emain.h"
#include "evers.h"                      /* Version information */
#include "efunc.h"
#include "eskeys.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#ifdef _CYGWIN
#include <sys/socket.h>
#include <termcap.h>
#endif

#if (defined _SUNOS5) && (defined _XTERM)
#include <X11/Sunkeysym.h>
#endif

/* For HP-UX then include the system headers for pstat which are used to
 * determine the location of the executable. */
#ifdef _HPUX
#include <sys/pstat.h>
#include <sys/param.h>
#endif

#ifdef _USG                     /* System V */
/* We need this stuff to do the pipes properly. */
#ifdef _TERMIOS
#include <termios.h>
struct  termios otermio;        /* original terminal characteristics */
struct  termios ntermio;        /* charactoristics to use inside */
typedef struct termios *meTermio ;
#else
#include <termio.h>
struct  termio  otermio;        /* original terminal characteristics */
struct  termio  ntermio;        /* charactoristics to use inside */
typedef struct termio *meTermio ;
#endif /* _TERMIOS */
#endif /* _USG */

#ifdef _BSD                             /* BSD */
#include <sgtty.h>                      /* For stty/gtty functions */
struct sgttyb  osgttyb;                 /* Old tty context */
struct sgttyb  nsgttyb;                 /* New tty context */
struct tchars  otchars;                 /* Old terminal context */
struct tchars  ntchars;                 /* New terminal context */
struct ltchars oltchars;                /* Old line discipline context */
struct ltchars nltchars;                /* New line discipline context */
int    olwmode;                         /* Old local word mode */
int    nlwmode;                         /* New local word mode */
/* Terminal output buffer */
#define TERMOUT_BUFSIZ   256            /* Size of the terminal o/p buffer */
static char termOutBuf [TERMOUT_BUFSIZ];
#endif /* _BSD */

#ifdef _USEPOLL
/* stdin character polling performed with poll() */
#include <poll.h>
#endif

#if MEOPT_COLOR
meUInt *colTable=NULL ;
#endif

#ifdef _ME_CONSOLE
#ifdef _TCAP
/**************************************************************************
* TERMCAP Definitions                                                     *
**************************************************************************/
#ifndef _CYGWIN
/* Termcap prototyes - really we should include the appropriate header */
extern int      tgetnum(char *);
extern int      tgetent(char *,char *);
extern char *   tgetstr(char *,char **);
extern int      tputs(char *,int, int (*)(int));
extern int      tgetflag(char *);
#endif /* _CYGWIN */
#ifdef _USETPARM
extern char *   tparm(const char *, ...);
#define meTCAPParm tparm
#else
#define meTCAPParm meTParm
#endif

#define putpad(s)  tputs(s, 1, putchar)
#define TCAPSLEN 512

#define TERMCAP_END          0          /* End of the termcap table */
#define TERMCAP_BOOLN        1          /* Boolean termcap value */
#define TERMCAP_DIGIT        2          /* Integer termcap value */
#define TERMCAP_STRING       3          /* String termcap value */

/* TCAPkey; The termcap code table for the information that we extract from
 * termcap */
struct TCAPkey
{
    char capKey[3];                     /* Termcap key (really only need 2) */
    char type;                          /* Type of variable */
    union
    {
        int value;                      /* Integer value back */
        char *str;                      /* String value back */
    } code;                             /* Termcap code */
};

/* Define the TERMCAP entry table from the definition file */
struct TCAPkey tcaptab [] =
{
#define TCAP_BOOLN(var,c1,c2,c3)  {{c1,c2,c3},TERMCAP_BOOLN,{0}},
#define TCAP_DIGIT(var,c1,c2,c3)  {{c1,c2,c3},TERMCAP_DIGIT,{0}},
#define TCAP_STRING(var,c1,c2,c3) {{c1,c2,c3},TERMCAP_STRING,{0}},
#include "etermcap.def"
#undef TCAP_BOOLN
#undef TCAP_DIGIT
#undef TCAP_STRING
    {"", TERMCAP_END, {0}}
};

/* Define the TERMCAP index entries */
enum
{
#define TCAP_BOOLN(var,c1,c2,c3)  var,
#define TCAP_DIGIT(var,c1,c2,c3)  var,
#define TCAP_STRING(var,c1,c2,c3) var,
#include "etermcap.def"
#undef TCAP_BOOLN
#undef TCAP_DIGIT
#undef TCAP_STRING
    TCAPmax
};

/* Define the local termcap constants that we require */
unsigned  short  TTnewWid, TTnewHig  ;
static    int    TTcursorVisible = 1;   /* Cursor is visible */
static    int    TTaMarginsDisabled = 0;/* Automatic margins disabled */
static    char   tcapbuf[TCAPSLEN];

/**************************************************************************
* Special termcal color definitions                                       *
**************************************************************************/
#if MEOPT_COLOR
#define tcapNumColors 8
static meUByte tcapColors[tcapNumColors*3] =
{
    0,    0,  0,                        /* Black */
    200,  0,  0,                        /* Red */
    0,  200,  0,                        /* Green */
    200,200,  0,                        /* Yellow */
    0,    0,200,                        /* Blue */
    200,  0,200,                        /* Magenta */
    0,  200,200,                        /* Cyan */
    200,200,200,                        /* White */
} ;

/* Set up the default ANSI colors. Many Termcap entries omit these when they
 * should not - we provide our own in the hope that they may be used. */

/* exit_attribute_mode/srg0/me/Turn off all attributes */
static char RCOLSTR[] =
{ 27, 91, 48,     109, 0 } ;   /* me - ESC[0m */
/* set_a_foreground/setaf/AF/Set foreground color using ANSI escape */
static char FCOLSTR[] =
{ 27, 91, 51, 65, 109, 0 } ;   /* AF - ESC[3Am */
/* set_a_background/setab/AB/Set background color using ANSI escape */
static char BCOLSTR[] =
{ 27, 91, 52, 65, 109, 0 } ;   /* AB - ESC[4Am */
#endif /* MEOPT_COLOR */

#define	DEFSKEY(s,i,d,t) i,

static char *tcapSpecKeyStrs[]=
{
#include	"eskeys.def"
};
#undef	DEFSKEY

#define	DEFSKEY(s,i,d,t) (meUByte *) d,

static meUByte *tcapSpecKeyDefs[]=
{
#include	"eskeys.def"
};
#undef	DEFSKEY

#endif /* _TCAP */
#endif /* _ME_CONSOLE */

/**************************************************************************
* X-Windows                                                               *
**************************************************************************/
#ifdef _XTERM

#include <X11/Xatom.h>
#include <X11/cursorfont.h>

/* X11 Drag and drop */
#ifdef _DRAGNDROP
static void xdndInitialize (Display * display);
static void xdndAware (Window window);
static int xdndSelectionNotify (XEvent *xevent, meFrame *frame);
static int xdndClientMessage (XEvent *xevent, meFrame *frame);
#else
/* Disablers when not enabled. */
#define xdndInitialize(display)             /* NULL */
#define xdndAware(window)                   /* NULL */
#define xdndSelectionNotify(xevent,frame)   /* NULL */
#define xdndClientMessage(xevent, frame)    /* NULL */
#endif /* (defined _DRAGNDROP) */

/* Setup the xterm variables and things */
meCellMetrics mecm ;

static int disableResize = 0;           /* Flag to disable screen resize */

Colormap xcmap ;
int      xscreen ;
XSizeHints sizeHints ;
int meStdin ;
#define meATOM_WM_DELETE_WINDOW 0
#define meATOM_WM_SAVE_YOURSELF 1
#define meATOM_WM_PROTOCOLS     2
#define meATOM_COPY_TEXT        3
#define meATOM_INCR             4
#define meATOM_MULTIPLE         5
#define meATOM_TARGETS          6
#define meATOM_STRING           7
static int TTdefaultPosX, TTdefaultPosY ;
static Atom meAtoms[8]={0};
char *meName=ME_FULLNAME ;
char *meIconName=ME_FULLNAME ;

/* Forward reference the icon assignment */
static void meSetIconState (Display *display, Window window);

#else  /* _XTERM */
#define meStdin 0
#endif /* _XTERM */

/**************************************************************************
* Mouse                                                                   *
**************************************************************************/
#if MEOPT_MOUSE
/* Local definitions for mouse handling code */
/* mouseState
 * A integer interpreted as a bit mask that holds the current state of
 * the mouse interaction. */
#define MOUSE_STATE_LEFT         0x0001 /* Left mouse button is pressed */
#define MOUSE_STATE_MIDDLE       0x0002 /* Middle mouse button is pressed */
#define MOUSE_STATE_RIGHT        0x0004 /* Right mouse button is pressed */
#define MOUSE_STATE_BUT4         0x0008 /* Button 4 (can be used by wheel) */
#define MOUSE_STATE_BUT5         0x0010 /* Button 5 (can be used by wheel) */
#define MOUSE_STATE_BUTTONS      (MOUSE_STATE_LEFT|MOUSE_STATE_MIDDLE|MOUSE_STATE_RIGHT|MOUSE_STATE_BUT4|MOUSE_STATE_BUT5)

int  mouseState = 0;             /* State of the mouse. */

/* mouseKeyState
 * The state of the control keys when the mouse was pressed. */
meUShort mouseKeyState;          /* State of keyboard control */

static meUShort mouseKeys[8] = { 0, 1, 2, 3, 4, 5 } ;
#define mouseButtonPick(bb) (mouseState |=  (1<<((bb)-1)))
#define mouseButtonDrop(bb) (mouseState &= ~(1<<((bb)-1)))

#define mouseButtonGetPick()                                                   \
((mouseState == 0)                 ? 0:                                      \
 (mouseState & MOUSE_STATE_LEFT)   ? 1:                                      \
 (mouseState & MOUSE_STATE_MIDDLE) ? 2:                                      \
 (mouseState & MOUSE_STATE_RIGHT)  ? 3:                                      \
 (mouseState & MOUSE_STATE_BUT4)   ? 4:                                      \
 (mouseState & MOUSE_STATE_BUT5)   ? 5:0)

#endif

/** 
 * Get the executable file name.
 * 
 * @param [in] argname The argument name of the programme.
 * @param [in,out] execpath The buffer to be filled with the filename
 * @param [in] execlen The length of the buffer in bytes.
 * 
 * @return The status of the call.
 * @retval 0 The call failed.
 * @retval 1 The call was successful.
 */
static int
execFilename (char *argname, char *execpath, int execlen)
{
    int status = 0;                     /* The call failed. */
#ifdef _SUNOS
    pid_t pid;
    
    /* Get the PID */
    if ((pid = getpid ()) >= 0)
    {
        struct stat statbuf;
        char buf [meBUF_SIZE_MAX];
        int n;
        
        /* Create the pathname of the executable in the /proc file system and
         * read in the link. */
        sprintf (buf, "/proc/%d/path/a.out", pid);
        if ((lstat(buf, &statbuf) != 0) &&
            (S_ISLNK(statbuf.st_mode)) &&
            ((n = readlink (buf, execpath, execlen)) > 0))
        {
            execpath [n] = '\0';
            status = 1;
        }
    }
#elif (defined _LINUX)
    pid_t pid;
    
    /* Get the PID */
    if ((pid = getpid ()) >= 0)
    {
        struct stat statbuf;
        char buf [meBUF_SIZE_MAX];
        int n;
        
        /* Create the pathname of the executable in the /proc file system and
         * read in the link. */
        sprintf (buf, "/proc/%d/exe", pid);
        if ((lstat(buf, &statbuf) != 0) &&
            (S_ISLNK(statbuf.st_mode)) &&
            ((n = readlink (buf, execpath, execlen)) > 0))
        {
            execpath [n] = '\0';
            status = 1;
        }
    }
#elif (defined _HPUX)
    /* The following only works on later version of HPUX >11.00 */
#if 0
     pid_t pid;
    
    /* Get the PID */
    if ((pid = getpid ()) >= 0)
    {
        struct pst_status pst;
        
        pst.pst_pid = -1;
        if (pstat_getproc (&pst, sizeof (pst), 0, pid) > 0)
        {
            /* Get the name of the file. */
            if ((n = pstat_getpathname (execpath, execlen, &pst.pst_fid_text)) > 0)
                status = 1;
        }
    }
#endif
#endif
    /* The call failed. */
    return status;
}

void
meSetupPathsAndUser(char *progname)
{
    /* Pre-built search path, this is a path built into the executable. The
     * installation root path of the installation macros searched in the order
     * specified. Once a directory has been located then it is used as the
     * default path. */
#ifndef _SEARCH_PATH
#define _SEARCH_PATH _DEFAULT_SEARCH_PATH
#endif
    static meUByte lpath[] = _SEARCH_PATH;
    struct stat dotstat, pwdstat;
    struct passwd *pwdp;            /* Password structure entry */
    meUByte *ss, buff[meBUF_SIZE_MAX] ;
    int ii, ll, gotUserPath ;

    /* If PWD is accurate, use it instead of calling gwd.
     * This solves problems with bad ~/ as the shell will
     * store as /user/.... etc.
     */
    if(((ss = meGetenv("PWD")) != NULL) &&
       (ss[0] == DIR_CHAR) && (stat((char *)ss,&pwdstat) == 0) &&
       (stat(".", &dotstat) == 0) && (dotstat.st_ino == pwdstat.st_ino) &&
       (dotstat.st_dev == pwdstat.st_dev))
    {
        meUByte cc, *dd ;

        ll = meStrlen(ss) ;
        curdir = dd = meMalloc(ll+2) ;
        while((cc=*ss++) != '\0')
            *dd++ = cc ;
        if(dd[-1] != DIR_CHAR)
            *dd++ = DIR_CHAR ;
        *dd = 0 ;
    }
    else
        curdir = gwd(0) ;
    if(curdir == NULL)
        /* not yet initialised so mlwrite will exit */
        mlwrite(MWCURSOR|MWABORT|MWWAIT,(meUByte *)"Failed to get cwd\n") ;
    
    /* Setup the $progname make it an absolute path. Where the system
     * provides a system call to get the absolute path then we use that (i.e.
     * the /proc file system, where the system does not then we fall back to
     * looking for the executable from the program name. */
    if(((ii = execFilename (progname, evalResult, sizeof (evalResult))) == 0) &&
       ((ii = executableLookup((meUByte *) progname,evalResult)) == 0))
    {
        /* Some shells, specifically zsh, will execute from the current
         * directory and pass through the argv[0] parameter with no pathname
         * component. Attempt to fix this condition. Note that we cannot add
         * this to executableLookup() since we may not search the current
         * directory unless it is specifically on the $PATH hence the
         * additional test here. */
        if (meStrrchr(progname,DIR_CHAR) == NULL)
            ii = fileLookup((meUByte *)progname,NULL,
                            meFL_CHECKDOT|meFL_EXEC,evalResult);
    }
    if (ii != 0)
        meProgName = meStrdup(evalResult) ;
    else
    {
#ifdef _ME_FREE_ALL_MEMORY
        /* stops problems on exit */
        meProgName = meStrdup(progname) ;
#else
        meProgName = (meUByte *)progname ;
#endif
    }
    
#if MEOPT_BINFS
    /* Initialise the built-in file system. Note for speed we only check the
     * header. Scope the "exepath" locally so it is discarded once the
     * pathname is passed to the mount and we exit the braces. */
    bfsdev = bfs_mount (meProgName, BFS_CHECK_HEAD);
#endif
        
    if((meUserName == NULL) &&
       ((ss = meGetenv ("MENAME")) != NULL) && (ss[0] != '\0'))
        meUserName = meStrdup(ss) ;

    /* Get the user name and home directory from the password file. */
    pwdp = getpwuid (meUid);        /* Get the password entry */
    if (pwdp != NULL)
    {
        /* Copy out the string information we need and initialise the
         * string variables. */
        if(pwdp->pw_dir != NULL)
            fileNameSetHome((meUByte *) pwdp->pw_dir) ;
        if((pwdp->pw_name != NULL) && (meUserName == NULL))
            meUserName = meStrdup((meUByte *) pwdp->pw_name) ;
        /* Shut down the password retrieval and allow system to relinquish
         * any resource it may have cached. */
        endpwent() ;
    }

    /* If no name yet then set to default 'user'. */
    if(meUserName == NULL)
        meUserName = meStrdup((meUByte *) "user") ;

    /* get the users home directory, user path and search path */
    if((homedir == NULL) &&
       (((ss = meGetenv("HOME")) != NULL) && (ss[0] != '\0')))
        fileNameSetHome(ss) ;

    if(((ss = meGetenv ("MEUSERPATH")) != NULL) && (ss[0] != '\0'))
        meUserPath = meStrdup(ss) ;

    if(((ss = meGetenv ("MEPATH")) != NULL) && (ss[0] != '\0'))
    {
        /* explicit path set by the user, don't need to look at anything else */
        searchPath = meStrdup(ss) ;
        /* we just need to add the $user-path to the front */
        if(meUserPath != NULL)
        {
            /* check that the user path is first in the search path, if not add it */
            ll = meStrlen(meUserPath) ;
            if(meStrncmp(searchPath,meUserPath,ll) ||
               ((searchPath[ll] != '\0') && (searchPath[ll] != mePATH_CHAR)))
            {
                /* meMalloc will exit if it fails as ME has not finished initialising */
                ss = meMalloc(ll + meStrlen(searchPath) + 2) ;
                meStrcpy(ss,meUserPath) ;
                ss[ll] = mePATH_CHAR ;
                meStrcpy(ss+ll+1,searchPath) ;
                meFree(searchPath) ;
                searchPath = ss ;
            }
        }
    }
    else
    {
        /* construct the search-path */
        /* put the $user-path first */
        if((gotUserPath = (meUserPath != NULL)))
            meStrcpy(evalResult,meUserPath) ;
        else
            evalResult[0] = '\0' ;
        ll = meStrlen(evalResult) ;

        /* look for the ~/.jasspa directory */
        if(homedir != NULL)
        {
            meStrcpy(buff,homedir) ;
            meStrcat(buff,".jasspa") ;
            if(((ll = mePathAddSearchPath(ll,evalResult,buff,&gotUserPath)) > 0) && !gotUserPath)
                /* as this is the user's area, use this directory unless we find
                 * a .../<$user-name>/ directory */
                gotUserPath = -1 ;
        }

        /* Get the system path of the installed macros. Use $MEINSTPATH as the
         * MicroEmacs standard macros */
        if((((ss = meGetenv ("MEINSTALLPATH")) != NULL) && (ss[0] != '\0')) ||
           (((ss = lpath) != NULL) && (ss[0] != '\0')))
        {
            meStrcpy(buff,ss) ;
            ll = mePathAddSearchPath(ll,evalResult,buff,&gotUserPath) ;
        }

        /* also check for directories in the same location as the binary */
        if((meProgName != NULL) && ((ss=meStrrchr(meProgName,DIR_CHAR)) != NULL))
        {
            ii = (((size_t) ss) - ((size_t) meProgName)) ;
            meStrncpy(buff,meProgName,ii) ;
            buff[ii] = '\0' ;
            ll = mePathAddSearchPath(ll,evalResult,buff,&gotUserPath) ;
        }
        
#if MEOPT_BINFS
        /* also check for the built-in file system */
        ll = mePathAddSearchPath(ll,evalResult,(meUByte *) "{BFS}",&gotUserPath) ;
#endif        
        
        if(!gotUserPath && (homedir != NULL))
        {
            /* We have not found a user path so set ~/ as the user-path
             * as this is the best place for macros to write to etc. */
            meStrcpy(buff,homedir) ;
            if(ll)
            {
                ii = meStrlen(buff) ;
                buff[ii++] = mePATH_CHAR ;
                meStrcpy(buff+ii,evalResult) ;
            }
            searchPath = meStrdup(buff) ;
        }
        else if(ll > 0)
            searchPath = meStrdup(evalResult) ;
    }
    if(searchPath != NULL)
    {
        if(meUserPath == NULL)
        {
            /* no user path yet, take the first path from the search-path, this
             * should be a sensible directory to use */
            if((ss = meStrchr(searchPath,mePATH_CHAR)) != NULL)
                *ss = '\0' ;
            meUserPath = meStrdup(searchPath) ;
            if(ss != NULL)
                *ss = mePATH_CHAR ;
        }
    }
    if(meUserPath != NULL)
    {
        ll = meStrlen(meUserPath) ;
        if(meUserPath[ll-1] != DIR_CHAR)
        {
            meUserPath = meRealloc(meUserPath,ll+2) ;
            meUserPath[ll++] = DIR_CHAR ;
            meUserPath[ll] = '\0' ;
        }
    }
}

meInt
meFileGetAttributes(meUByte *fname)
{
    struct stat statbuf;
    if(stat((char *)fname, &statbuf) != 0)
        return -1 ;
    return statbuf.st_mode ;
}

void
sigAlarm(SIGNAL_PROTOTYPE)
{
    /* Trap Alarm signals under UNIX. Used to do auto saving. */
#if !((defined (_POSIX_SIGNALS)) || (defined _BSD_SIGNALS))
    signal (SIGALRM, sigAlarm);
#endif
    /* Note that we use the first timers expire time here and not the actual time.
     * sometimes the itimers fire a little bit early i.e. +-5 msec, to
     * ensure that we do not hang then we force the timer to expire.
     * There is no other way for the timer to expire (that I am aware of) */
    if (timers != NULL)
        timerCheck(timers->abstime);           /* Timer has expired */
}

int
meGidInGidList(gid_t gid)
{
    int ii=meGidSize ;
    while(--ii>=0)
        if(meGidList[ii] == gid)
            return 1 ;
    return 0 ;
}

static void
waitForEvent(int mode)
{
    static int lost=0 ;
#if MEOPT_IPIPES
    meIPipe *ipipe, *pp ;
    int ipipeEvent ;
#endif
    fd_set select_set;
    int pfd ;

    /* While no input keys and ipipe or alarm signals. */
    for(;;)
    {
        if(TTahead() ||
#ifdef _CLIPBRD
           (clipState & CLIP_RECEIVED) ||
#endif
           isTimerExpired(AUTOS_TIMER_ID) ||
#if MEOPT_CALLBACK
           isTimerExpired(CALLB_TIMER_ID) ||
#endif
           isTimerExpired(CURSOR_TIMER_ID) ||
#if MEOPT_SOCKET
           isTimerExpired(SOCKET_TIMER_ID) ||
#endif
           (sgarbf == meTRUE))
            break ;

        TTdieTest() ;
        FD_ZERO(&select_set);
        FD_SET(meStdin, &select_set);
        pfd = meStdin ;
#if MEOPT_IPIPES
        ipipeEvent = 0 ;
        ipipe = ipipes ;
        while(ipipe != NULL)
        {
            pp = ipipe->next ;
            if(ipipe->pid < 0)
            {
                ipipeRead(ipipe) ;
                ipipeEvent = 1 ;
            }
            else
            {
                FD_SET(ipipe->rfd, &select_set);
                if(ipipe->rfd > pfd)
                    pfd = ipipe->rfd ;
            }
            ipipe = pp ;
        }
        if(ipipeEvent && mode)
            return ;
#endif
        pfd++ ;
        if(select(pfd,&select_set,NULL,NULL,NULL) >= 0)
        {
            int ii;
#if MEOPT_IPIPES
            ipipe = ipipes ;
            while(ipipe != NULL)
            {
                pp = ipipe->next ;
                /* See if theres anything to read, if so read it */
                if(FD_ISSET(ipipe->rfd,&select_set) || (ipipe->pid < 0))
                {
                    /* reset the lost */
                    lost = 0 ;
                    ipipeRead(ipipe) ;
                    ipipeEvent = 1 ;
                }
                ipipe = pp ;
            }
            if(ipipeEvent && mode)
                return ;
#endif
            if(FD_ISSET(meStdin,&select_set))
            {
                /* this is a check to see if we've lost stdin,
                 * if we have there's nothing we can do other than die.
                 * This is common when the X-window is destroyed using
                 * the window manager.
                 */
#ifdef _USEPOLL
                struct pollfd pfds [1];

                /* Set up the poll structure */
                pfds[0].fd = meStdin;
                pfds[0].events = POLLIN;

                /* As there is only one event we can rely on the return status
                 * to tell us what has happened. */
                ii = poll (pfds, 1, 0);
                if ((ii < 0) ||
                    ((ii == 1) && (pfds[0].revents & (POLLERR|POLLHUP|POLLNVAL))))
                    ii = 0;             /* Failed */
                else
                    ii = 1;             /* OK */
#else
#ifdef FIONREAD
                int x;      /* holds # of pending chars */
                ii = ioctl(meStdin,FIONREAD,&x);

                /* if ioctl fails don't die! */
                ii = ((ii < 0) ? 1 : x);
#else
                ii = ioctl(meStdin, FIORDCHK,0);
#endif /* FIONREAD */
#endif /* _USEPOLL */
                if(!ii)
                {
                    /* Found this test can go wrong in some strange case,
                     * which is a real bummer as me now dies! as a fix, and I
                     * know its a bit of a bodge, only die is this happens 3
                     * times on the trot
                     */
                    if(lost++ == 2)
                    {
                        /* If there's nothing to read, yet select flags
                         * meStdin as triggered then we've lost the input -
                         * Die... Unfortunately we must be very careful on
                         * what we do, we cannot write out to the x-window
                         * cause its gone, but meDie takes care of this
                         */
                        meDie() ;
                    }
                }
                else
                    lost = 0 ;
            }
        }
        else
            /* Premeture exit - probably a signal for an alarm */
            break;
    }
}

#ifdef _ME_CONSOLE
#ifdef _TCAP

static void
TCAPgetWinSize(void)
{
#ifdef TIOCGWINSZ
    /* BSD-style and lately SunOS.  */
    struct winsize size;

    if((ioctl(meStdin,TIOCGWINSZ,&size) == -1) ||
       ((TTnewWid = size.ws_col) <= 0) ||
       ((TTnewHig = size.ws_row) <= 0))
#else
#ifdef TIOCGSIZE
    /* SunOS - style.  */
    struct ttysize size;

    if((ioctl(meStdin,TIOCGSIZE,&size) == -1) ||
       ((TTnewWid = size.ts_col) <= 0) ||
       ((TTnewHig = size.ts_lines) <= 0))
#endif
#endif
    {
        int  ii ;

        /* Get the number of columns on the screen */
        if((ii=tgetnum(tcaptab[TCAPcols].capKey)) != -1)
            TTnewWid = ii ;
        else
            TTnewWid = frameCur->width ;

	/* Get the number of lines on the screen */
        if((ii=tgetnum(tcaptab[TCAPlines].capKey)) != -1)
            TTnewHig = ii ;
        else
	{
            TTnewHig = frameCur->depth + 1 ;
	    /* return now to avoid a potential window size decrease if the
             * below scroll glitch is true */
	    return ;
	}
    }

    /* If there is a new line glitch and we have automargins then it is
     * dangerous for us to use the last line as we cause a scroll that we
     * cannot easily correct. If this is the case then reduce the number of
     * lines by 1. */
    if ((tcaptab[TCAPam].code.value != 0) &&
        (tcaptab[TCAPxenl].code.value == 0) &&
        (TTaMarginsDisabled == 0))
        TTnewHig-- ;
}

static void
sigSize(SIGNAL_PROTOTYPE)
{
#if !((defined (_POSIX_SIGNALS)) || (defined _BSD_SIGNALS))
    /* Reset the signal handler */
    signal (SIGWINCH, sigSize) ;
#endif
    TCAPgetWinSize() ;

    if((TTnewWid != frameCur->width) || (TTnewHig != frameCur->depth+1))
        alarmState |= meALARM_WINSIZE ;
}
#endif /* _TCAP */
#endif /* _ME_CONSOLE */

#ifdef _XTERM

static Font
__XTERMfontGetId(meUByte font)
{
    meUByte fontNU ;

    /* remove the underline as this is not a different font,
     * the underline is drawn on after */
    fontNU = (font & ~meFONT_UNDERLINE) ;
    if(mecm.fontTbl[fontNU] == BadName)
    {
        XFontStruct *fontQ ;
        char *weight;
        char *slant;
        char buf [meBUF_SIZE_MAX];              /* Local name buffer */

        /* Process the bold field. If the X font specifier (weight) is
         * medium or undefined and this is a bold field identifier then
         * specify a bold font */
        if(fontNU & meFONT_BOLD)
            weight = "bold";
        else if(fontNU & meFONT_LIGHT)
            weight = "light";
        else
            weight = (char *)mecm.fontPart [2];

        /* Process the italic field. If the X font specified (slant) is r
         * or undefined and this is a italic field identifier then specify
         * an italic font. */
        if(fontNU & meFONT_ITALIC)
            slant = "o";
        else
            slant = (char *)mecm.fontPart [3];

        /* Construct the new font name */
        sprintf (buf, "-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s",
                 mecm.fontPart [0],  mecm.fontPart [1],  weight,
                 slant,              mecm.fontPart [4],  mecm.fontPart [5],
                 mecm.fontPart [6],  mecm.fontPart [7],  mecm.fontPart [8],
                 mecm.fontPart [9],  mecm.fontPart [10], mecm.fontPart [11],
                 mecm.fontPart [12], mecm.fontPart [13]);

        if((fontQ=XLoadQueryFont(mecm.xdisplay,buf)) == NULL)
            mecm.fontTbl[fontNU] = mecm.fontTbl[0] ;
        else
        {
            if(((fontQ->ascent + fontQ->descent) != mecm.fdepth) ||
               (fontQ->max_bounds.width != mecm.fwidth))
                mecm.fontTbl[fontNU] = mecm.fontTbl[0] ;
            else
            {
                mecm.fontTbl[fontNU] = fontQ->fid ;
                mecm.fontFlag[fontNU] = 1 ;
            }
            XFreeFontInfo(NULL,fontQ,1) ;
        }
    }
    mecm.fontTbl[font] = mecm.fontTbl[fontNU] ;
    return mecm.fontTbl[font] ;
}

#define XTERMfontGetId(font) \
((mecm.fontTbl[font] == BadName) ? __XTERMfontGetId(font):mecm.fontTbl[font])

void
meFrameXTermSetScheme(meFrame *frame, meScheme scheme)
{
    meUInt valueMask = 0 ;
    meUByte cc ;

    cc = meStyleGetFColor(meSchemeGetStyle(scheme)) ;
    if(meFrameGetXGCFCol(frame) != cc)
    {
        meFrameSetXGCFCol(frame,cc) ;
        meFrameGetXGCValues(frame).foreground = colTable[cc] ;
        valueMask |= GCForeground ;
    }
    cc = meStyleGetBColor(meSchemeGetStyle(scheme)) ;
    if(meFrameGetXGCBCol(frame) != cc)
    {
        meFrameSetXGCBCol(frame,cc) ;
        meFrameGetXGCValues(frame).background = colTable[cc] ;
        valueMask |= GCBackground ;
    }
    if(mecm.fontName != NULL)
    {
        cc = meStyleGetFont(meSchemeGetStyle(scheme)) ;
        if(meSchemeTestNoFont(scheme))
            cc &= ~(meFONT_BOLD|meFONT_ITALIC|meFONT_UNDERLINE) ;
        if(meFrameGetXGCFont(frame) != cc)
        {
            meFrameSetXGCFont(frame,cc) ;
            meFrameSetXGCFontId(frame,XTERMfontGetId(cc)) ;
            if(meFrameGetXGCValues(frame).font != meFrameGetXGCFontId(frame))
            {
                meFrameGetXGCValues(frame).font = meFrameGetXGCFontId(frame) ;
                valueMask |= GCFont ;
            }
        }
    }
    if(valueMask)
        XChangeGC(mecm.xdisplay,meFrameGetXGC(frame),valueMask,&meFrameGetXGCValues(frame)) ;
}

/* meFrameXTermDrawSpecialChar; Draw a special character to the screen. x is the lefthand
 * edge of the character, y is the top of the character. xgc is used for the
 * colors.
 */
void
meFrameXTermDrawSpecialChar(meFrame *frame, int x, int y, meUByte cc)
{
    XPoint points[4] ;
    int ii ;
    /* Fill in the character */
    switch (cc)
    {
    case 0x01:          /* checkbox left side ([) */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x + mecm.fwidth - 1, y + mecm.fhdepth - mecm.fhwidth,
                  x + mecm.fwidth - 2, y + mecm.fhdepth - mecm.fhwidth) ;
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x + mecm.fwidth - 2, y + mecm.fhdepth - mecm.fhwidth,
                  x + mecm.fwidth - 2, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x + mecm.fwidth - 2, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth,
                  x + mecm.fwidth - 1, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        break;

    case 0x02:          /* checkbox center not selected */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth - mecm.fhwidth,
                  x + mecm.fwidth - 1, y + mecm.fhdepth - mecm.fhwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth,
                  x + mecm.fwidth - 1, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        break;

    case 0x03:          /* checkbox center not selected */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth - mecm.fhwidth,
                  x + mecm.fwidth - 1, y + mecm.fhdepth - mecm.fhwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth,
                  x + mecm.fwidth - 1, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        points[0].x = x ;
        points[0].y = y + mecm.fhdepth - mecm.fhwidth + 2 ;
        points[1].x = x ;
        points[1].y = y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth - 1 ;
        points[2].x = x + mecm.fwidth ;
        points[2].y = y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth - 1 ;
        points[3].x = x + mecm.fwidth ;
        points[3].y = y + mecm.fhdepth - mecm.fhwidth + 2 ;
        XFillPolygon(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), points, 4, Convex, CoordModeOrigin) ;
        break;

    case 0x04:          /* checkbox right side (]) */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth - mecm.fhwidth,
                  x + 1, y + mecm.fhdepth - mecm.fhwidth) ;
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x + 1, y + mecm.fhdepth - mecm.fhwidth,
                  x + 1, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x + 1, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth,
                  x, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        break;

    case 0x07:          /* Line space '.' */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x+mecm.fhwidth, y+mecm.fhdepth, x+mecm.fhwidth+1, y+mecm.fhdepth);
        break;

    case 0x08:          /* Backspace character <- */
        ii=(mecm.fhdepth+1) >> 1 ;
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x+mecm.fwidth-2, y+mecm.fhdepth, x+mecm.fhwidth, y+mecm.fhdepth);
        points[0].x = x+mecm.fhwidth ;
        points[0].y = y+ii ;
        points[1].x = x+mecm.fhwidth ;
        points[1].y = y+mecm.fdepth-ii-1;
        points[2].x = x+1 ;
        points[2].y = y+mecm.fhdepth ;
        XFillPolygon(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), points, 3, Convex, CoordModeOrigin) ;
        break;

    case 0x09:          /* Tab character -> */
        ii=(mecm.fhdepth+1) >> 1 ;
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x+1, y+mecm.fhdepth, x+mecm.fhwidth-1, y+mecm.fhdepth);
        points[0].x = x+mecm.fhwidth-1 ;
        points[0].y = y+ii ;
        points[1].x = x+mecm.fhwidth-1 ;
        points[1].y = y+mecm.fdepth-ii-1;
        points[2].x = x+mecm.fwidth-2 ;
        points[2].y = y+mecm.fhdepth ;
        XFillPolygon(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), points, 3, Convex, CoordModeOrigin) ;
        break;

    case 0x0a:          /* CR character / <-| */
        ii=(mecm.fhdepth+1) >> 1 ;
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x+mecm.fhwidth,  y+mecm.fhdepth, x+mecm.fwidth-2, y+mecm.fhdepth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x+mecm.fwidth-2, y+mecm.fhdepth, x+mecm.fwidth-2, y+ii-1);
        points[0].x = x+mecm.fhwidth ;
        points[0].y = y+ii ;
        points[1].x = x+mecm.fhwidth ;
        points[1].y = y+mecm.fdepth-ii-1;
        points[2].x = x+1 ;
        points[2].y = y+mecm.fhdepth ;
        XFillPolygon(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), points, 3, Convex, CoordModeOrigin) ;
        break;

    case 0x0b:          /* Line Drawing / Bottom right _| */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fhwidth, y);
        break;

    case 0x0c:          /* Line Drawing / Top right */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fdepth - 1);
        break;

    case 0x0d:          /* Line Drawing / Top left */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fwidth - 1, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fdepth - 1);
        break;

    case 0x0e:          /* Line Drawing / Bottom left |_ */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        break;

    case 0x0f:          /* Line Drawing / Centre cross + */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fdepth - 1);
        break;

    case 0x10:          /* Cursor Arrows / Right */
        ii = mecm.fwidth ;
        if(ii > mecm.fhdepth)
            ii = mecm.fhdepth ;
        points[0].x = x + 1 ;
        points[0].y = y + mecm.fhdepth - ii ;
        points[1].x = x + 1 ;
        points[1].y = y + mecm.fhdepth + ii ;
        points[2].x = x + ii + 1 ;
        points[2].y = y + mecm.fhdepth ;
        XFillPolygon(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), points, 3, Convex, CoordModeOrigin) ;
        break;

    case 0x11:          /* Cursor Arrows / Left */
        ii = mecm.fwidth ;
        if(ii > mecm.fhdepth)
            ii = mecm.fhdepth ;
        points[0].x = x + mecm.fwidth - 1 ;
        points[0].y = y + mecm.fhdepth + ii ;
        points[1].x = x + mecm.fwidth - 1 ;
        points[1].y = y + mecm.fhdepth - ii ;
        points[2].x = x + mecm.fwidth - 1 - ii ;
        points[2].y = y + mecm.fhdepth ;
        XFillPolygon(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), points, 3, Convex, CoordModeOrigin) ;
        break;

    case 0x12:          /* Line Drawing / Horizontal line - */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        break;

    case 0x13:          /* cross box empty ([ ]) */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth - mecm.fhwidth + 1,
                  x + mecm.fwidth - 1, y + mecm.fhdepth - mecm.fhwidth + 1);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth,
                  x + mecm.fwidth - 1, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth - mecm.fhwidth + 1,
                  x, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x + mecm.fwidth - 1, y + mecm.fhdepth - mecm.fhwidth + 1,
                  x + mecm.fwidth - 1, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        break;

    case 0x14:          /* cross box ([X]) */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth - mecm.fhwidth + 1,
                  x + mecm.fwidth - 1, y + mecm.fhdepth - mecm.fhwidth + 1);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth,
                  x + mecm.fwidth - 1, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth - mecm.fhwidth + 1,
                  x, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x + mecm.fwidth - 1, y + mecm.fhdepth - mecm.fhwidth + 1,
                  x + mecm.fwidth - 1, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth - mecm.fhwidth + 1,
                  x + mecm.fwidth - 1, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame),
                  x, y + mecm.fhdepth + mecm.fwidth - mecm.fhwidth,
                  x + mecm.fwidth - 1, y + mecm.fhdepth - mecm.fhwidth + 1);
        break;

    case 0x15:          /* Line Drawing / Left Tee |- */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fdepth - 1);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        break;

    case 0x16:          /* Line Drawing / Right Tee -| */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fdepth - 1);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fhdepth);
        break;

    case 0x17:          /* Line Drawing / Bottom Tee _|_ */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fhdepth);
        break;

    case 0x18:          /* Line Drawing / Top Tee -|- */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x, y + mecm.fhdepth, x + mecm.fwidth - 1, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y + mecm.fdepth - 1, x + mecm.fhwidth, y + mecm.fhdepth);
        break;

    case 0x19:          /* Line Drawing / Vertical Line | */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y, x + mecm.fhwidth, y + mecm.fdepth - 1);
        break;

    case 0x1a:          /* Line Drawing / Bottom right _| with resize */
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x, y + mecm.fhdepth, x + mecm.fhwidth, y + mecm.fhdepth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + mecm.fhwidth, y + mecm.fhdepth, x + mecm.fhwidth, y);

        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x, y + mecm.fdepth - 1, x + mecm.fwidth - 1, y + mecm.fdepth - mecm.fwidth);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + 2, y + mecm.fdepth - 1, x + mecm.fwidth - 1, y + mecm.fdepth - mecm.fwidth + 2);
        XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x + 4, y + mecm.fdepth - 1, x + mecm.fwidth - 1, y + mecm.fdepth - mecm.fwidth + 4);
        break ;

    case 0x1b:          /* Scroll box - vertical */
        for (ii = (y+1) & ~1; ii < y+mecm.fdepth; ii += 2)
            XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), x, ii, x + mecm.fwidth - 1, ii);
        break;

    case 0x1d:          /* Scroll box - horizontal */
        for (ii = (x+1) & ~1; ii < x+mecm.fwidth; ii += 2)
            XDrawLine(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), ii, y, ii, y + mecm.fdepth - 1);
        break;

    case 0x1e:          /* Cursor Arrows / Up */
        points [0].x = x - 1 ;
        points [0].y = y + mecm.fdepth - 1 ;
        points [1].x = mecm.fhwidth + (mecm.fwidth & 0x01) ;
        points [1].y = -mecm.fadepth ;
        points [2].x = mecm.fhwidth + 1 ;
        points [2].y = mecm.fadepth ;
        XFillPolygon(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), points, 3, Convex, CoordModePrevious) ;
        break;

    case 0x1f:          /* Cursor Arrows / Down */
        points [0].x = x - 1 ;
        points [0].y = y + 1 ;
        points [1].x = mecm.fhwidth + (mecm.fwidth & 0x01) ;
        points [1].y = mecm.fadepth ;
        points [2].x = mecm.fhwidth + 1 ;
        points [2].y = -mecm.fadepth ;
        XFillPolygon(mecm.xdisplay, meFrameGetXWindow(frame), meFrameGetXGC(frame), points, 3, Convex, CoordModePrevious) ;
        break;
    }
}

/*
 * meFrameXTermDraw
 * Repaint the screen for the specified region from the frame store.
 * We use the colour and text information from the frame store and
 * re-paint it on the canvas, thereby refreshing the screen.
 *
 * srow - Start row
 * erow - End row
 * scol - Start column
 * ecol - End column
 */
void
meFrameXTermDraw(meFrame *frame, int srow, int scol, int erow, int ecol)
{
    meFrameLine *flp;                   /* Frame store line pointer */
    meScheme  *fssp;                    /* Frame store colour pointer */
    meUByte     *fstp;                  /* Frame store text pointer */
    meScheme   schm;                    /* Current colour */
    int col;                            /* Current column position */
    int row;                            /* Current row screen position */
    int tcol;                           /* Text column start */
    int length;                         /* Length of string */

    /* Process each row in turn until we reach the end of the line */
    if (meSystemCfg & meSYSTEM_FONTFIX)
    {
        meUByte cc, *sfstp, buff[meBUF_SIZE_MAX] ;
        int spFlag ;

        for (flp = frame->store + srow; srow < erow; srow++, flp++)
        {
            length = 0;                     /* Initialise the string length */
            col = scol;                     /* Current column becomes start column */
            tcol = col;                     /* Start of the text column */
            row = rowToClient(srow) ;

            /* Get pointers aligned into the frame store */
            sfstp = flp->text ;
            fstp = sfstp + scol ;           /* Point to text block */
            fssp = flp->scheme + scol ;     /* Point to colour block */
            schm = *fssp;                   /* Get the initial scheme */
            spFlag = 0 ;
            while (col < ecol)
            {
                if (*fssp++ != schm)        /* Change in colour ?? */
                {
                    /* Output the current text item */
                    meFrameXTermSetScheme(frame,schm) ;
                    meFrameXTermDrawString(frame,colToClient(tcol),row,buff,length);
                    while(--spFlag >= 0)
                    {
                        while (((cc=sfstp[tcol]) & 0xe0) != 0)
                            tcol++ ;
                        meFrameXTermDrawSpecialChar(frame,tcol*mecm.fwidth,row-mecm.ascent,cc) ;
                        tcol++ ;
                    }
                    spFlag = 0 ;
                    tcol = col;             /* Move the text position */
                    length = 0;             /* Reset the length */
                    schm = fssp[-1];        /* Get the next colour */
                }
                if(((cc=*fstp++) & 0xe0) == 0)
                {
                    spFlag++ ;
                    cc = ' ' ;
                }
                buff[length++]=cc;          /* Set & Increment the string length */
                col++;                      /* Next colunm */
            }

            /* Output the remaining text item */
            if (length > 0)
            {
                meFrameXTermSetScheme(frame,schm) ;
                meFrameXTermDrawString(frame,colToClient(tcol),row,buff,length);
                while(--spFlag >= 0)
                {
                    while (((cc=sfstp[tcol]) & 0xe0) != 0)
                        tcol++ ;
                    meFrameXTermDrawSpecialChar(frame,tcol*mecm.fwidth,row-mecm.ascent,cc) ;
                    tcol++ ;
                }
            }
        }
    }
    else
    {
        for (flp = frame->store + srow; srow < erow; srow++, flp++)
        {
            length = 0;                     /* Initialise the string length */
            col = scol;                     /* Current column becomes start column */
            tcol = col;                     /* Start of the text column */

            /* Get pointers aligned into the frame store */
            fstp = flp->text + scol ;       /* Point to text block */
            fssp = flp->scheme + scol ;     /* Point to colour block */
            schm = *fssp;                   /* Get the initial scheme */

            while (col < ecol)
            {
                if (*fssp++ != schm)        /* Change in colour ?? */
                {
                    /* Output the current text item */
                    meFrameXTermSetScheme(frame,schm) ;
                    meFrameXTermDrawString(frame,colToClient(tcol),rowToClient(srow),fstp,length);
                    fstp += length;         /* Move the text pointer */
                    tcol = col;             /* Move the text position */
                    length = 0;             /* Reset the length */
                    schm = fssp[-1];        /* Get the next colour */
                }
                length++;                   /* Increment the string length */
                col++;                      /* Next colunm */
            }

            /* Output the remaining text item */
            if (length > 0)
            {
                meFrameXTermSetScheme(frame,schm) ;
                meFrameXTermDrawString(frame,colToClient(tcol),rowToClient(srow),fstp,length);
            }
        }
    }
}

#if MEOPT_MWFRAME

static meFrame *
meXEventGetFrame(XEvent *event)
{
    meFrameLoopBegin() ;

    if(((loopFrame->flags & meFRAME_HIDDEN) == 0) &&
       (meFrameGetXWindow(loopFrame) == event->xany.window))
            return loopFrame ;

    meFrameLoopEnd() ;

    return NULL ;
}

#else

#define meXEventGetFrame(event) frameCur

#endif

static void
meFrameGainFocus(meFrame *frame)
{
    /* have we not got the focus? */
    if(frame->flags & meFRAME_NOT_FOCUS)
    {
        frame->flags &= ~meFRAME_NOT_FOCUS ;
#if MEOPT_MWFRAME
        if(frameCur != frame)
            frameFocus = frame ;
#endif
        if((cursorState >= 0) && blinkState)
        {
            if(cursorBlink)
                TThandleBlink(2) ;
            else
                meFrameXTermShowCursor(frame) ;
            XFlush(mecm.xdisplay) ;
        }
    }
}

static void
meFrameKillFocus(meFrame *frame)
{
    /* have we got the focus to loose it? */
    if(!(frame->flags & meFRAME_NOT_FOCUS))
    {
        frame->flags |= meFRAME_NOT_FOCUS ;
#if MEOPT_MWFRAME
        if(frameFocus == frame)
            frameFocus = NULL ;
#endif
        if(cursorState >= 0)
        {
            /* because the cursor is a part of the solid cursor we must
             * remove the old one first and then redraw
             */
            if(blinkState)
                meFrameXTermHideCursor(frame) ;
            blinkState = 1 ;
            meFrameXTermShowCursor(frame) ;
            XFlush(mecm.xdisplay) ;
        }
    }
}

/* following are used for incremental Selection getting */
static meUByte *meClipBuff=NULL ;
static meUInt   meClipSize=0 ;

/* The xterm XEvent handler */
static void
meXEventHandler(void)
{
    meFrame *frame ;
    XEvent event ;

    /* Get the next event - wait if necesary */
    XNextEvent(mecm.xdisplay,&event);

    /* printf("Got event %d\n",event.type) ; */
    switch(event.type)
    {
    case MapNotify:
        /* The window has been mapped, simply save the mapped state of the
         * window. */
        if((frame = meXEventGetFrame(&event)) != NULL)
            meFrameSetXMapState(frame,meXMAP_MAP);
        break;
        
    case UnmapNotify:
        /* The window has been un-mapped. */
        if((frame = meXEventGetFrame(&event)) != NULL)
        {
            int state = meFrameGetXMapState(frame);
            
            /* Only handle the Font mapping state, when flagged with a font
             * change then the size hints of the unmapped window should be
             * changed and the window immediately re-mapped. */
            if (state == meXMAP_FONT)
            {
                /* Change the size hints of the window and re-map the window,
                 * note that we do not check the error return because there
                 * is not actually anything that we can do if the hints
                 * cannot be set. */
                XSetWMNormalHints(mecm.xdisplay,meFrameGetXWindow(frame),&sizeHints) ;
                /* Now that the hints have been changed then the window may
                 * be re-mapped to display it again. */
                XMapWindow(mecm.xdisplay,meFrameGetXWindow(frame)) ;
            }
            /* Ensure that the frame state is unmapped, we do not want to
             * apply any further font changes. */
            meFrameSetXMapState(frame,meXMAP_UNMAP) ;
        }
        break;
    
    case ConfigureNotify:
        /* window has been resized, change width and height to send to
         * draw_text and draw_graphics */
        if((frame = meXEventGetFrame(&event)) != NULL)
        {
            /* Get the width and heigth back and setup the frame->depthMax etc */
            int ww, hh, sizeSet ;

            /* Make sure that there are no other pending ConfigureNotify
             * events, if there are then find the last one */
#if 0
            /* Although advised causes a pasued startup */
            {
                XEvent nextEvent;
                
                while (XCheckTypedWindowEvent(mecm.xdisplay, 
                                              meFrameGetXWindow(frame), 
                                              ConfigureNotify,
                                              &nextEvent) != 0)
                {
                    /* Overwrite the current envent with the new one. */
                    memcpy (&event, &nextEvent, sizeof (XEvent));
                }
            }
#endif            
            sizeHints.x = event.xconfigure.x ;
            sizeHints.y = event.xconfigure.y ;
            sizeHints.height = event.xconfigure.height ;
            sizeHints.width  = event.xconfigure.width ;
            hh = event.xconfigure.height / mecm.fdepth ;
            ww = event.xconfigure.width / mecm.fwidth ;
            /*            printf("Got event2 Config %d %d\n",hh,ww) ;*/
            /*        printf("Got configure event! (%d,%d)\n",frame->width,frame->depth) ;*/
            /* Disable the window resize until BOTH width and height changes
             * have been established. Both LINUX and SUN suffer from severe
             * beating between the client and server when the resize requests
             * are split. Allow the new sizes to be computed and then inform
             * the server in one invocation. */
            disableResize = 1;            /* Disable X reconfiguration */
            sizeSet = 0 ;
            if(ww != frame->width)
            {
                meFrameChangeWidth(frame,ww); /* Change width */
                sizeSet = 1 ;
            }
            if(hh != (frame->depth+1))
            {
                meFrameChangeDepth(frame,hh); /* Change depth */
                sizeSet = 1 ;
            }
            disableResize = 0;            /* Re-enable the resize */

            /* Change the size of the window now that both sizes have been
             * established. Only perform the resize if there is something to
             * change */
            if ((hh != frame->depth+1) || (ww != frame->width))
                meFrameSetWindowSize(frame) ;
            if(sizeSet && !screenUpdateDisabledCount)
                screenUpdate(meTRUE,2-sgarbf) ;
        }
        break;
    case Expose:
        /*
         * All, or part of the window has been exposed. probably caused by
         * another window passing ontop of the MicroEmacs canvas, or we have
         * just been exposed.
         *
         * Update the canvas from the frame store, there is no need to
         * regenerate the screen, simply repaint those canval pixels which
         * have been corrupted by X over-painting another window.
         *
         * Note that we use the 'count' in the XExposeEvent. This tells us
         * if another Expose event is coming on the input queue when non-zero.
         * In this case we hold off on the paint and remember the largest
         * bounding box that contains the region to be painted. Only when it
         * reaches zero do we actually perform the paint. This allows us to
         * optimise the painting otherwise we may over paint the same region
         * multiple times. Who knows what the user is doing !!
         *
         * Note that keeping a single bounding box may mean that we update
         * too much of the screen, however in most cases multiple paints
         * are in adjacent regions.
         *
         * Jon Green 28/06/97
         */
        if((frame = meXEventGetFrame(&event)) != NULL)
        {
            static int sscol= -1;       /* History - Start column */
            static int ssrow;           /* History - Start row */
            static int secol;           /* History - End column */
            static int serow;           /* History - End row */
            int scol;                   /* Current - Start column */
            int srow;                   /* Current - Start row */
            int ecol;                   /* Current - End column */
            int erow;                   /* Current - End row */

            /* Compute the start and end column widths. For the end
             * column then a rounding operation is performed to ensure
             * that the whole character is covered */
            scol = clientToCol(event.xexpose.x);
            ecol = clientToCol(event.xexpose.x+event.xexpose.width+mecm.fwidth-1);
            if (ecol > frame->width)
                ecol = frame->width;
            srow = clientToRow(event.xexpose.y);
            erow = clientToRow(event.xexpose.y+event.xexpose.height+mecm.fdepth-1);
            if (erow > (frame->depth+1))
                erow = (frame->depth+1);

            /*
             * Determine if the paint should be held off, other paints
             * are on their way. If the paint is to be held off then
             * remeber the extents of the bounding box, otherwise
             * do the paint.
             */
            if (sscol == -1)            /* No historical bounding information */
            {
                if (event.xexpose.count == 0)   /* Last of the expose events ?? */
                    meFrameXTermDraw(frame,srow, scol, erow, ecol);
                else                    /* More to come - save bounding box */
                {
                    sscol = scol;
                    secol = ecol;
                    ssrow = srow;
                    serow = erow;
                }
            }
            /* A historical bounding box exists. */
            else
            {
                /* Update the historical bounding box infomation by using the
                 * maximum and minimum values of the existing and current
                 * Exposure boxes. */
                if (scol < sscol)
                    sscol = scol;
                if (ecol > secol)
                    secol = ecol;
                if (srow < ssrow)
                    ssrow = srow;
                if (erow > serow)
                    serow = erow;
                /* If this is the last of the Expose events then initiate
                 * a paint to redraw the screen. */
                if (event.xexpose.count == 0)
                {
                    meFrameXTermDraw(frame,ssrow, sscol, serow, secol);
                    sscol = -1;         /* Reset the history to none */
                }
            }
            if((cursorState >= 0) && blinkState)
                meFrameXTermShowCursor(frame) ;
            XFlush(mecm.xdisplay) ;
        }
        break;

#if MEOPT_MOUSE
    case MotionNotify:                  /* Mouse has moved. */
        if(meMouseCfg & meMOUSE_ENBLE)
        {
            /* Collect the position of the mouse. Require the row/column mouse
             * position. Also require the fractional bits incase we are scrolling
             *
             * Really we should re-evaluate the buttons here. However X-Windows
             * does not seemed to have aquired an incorrect state (i.e. button
             * state mis-represented). I will leave it for the time being (JDG)
             */
            meUShort cc;                /* Character code */
            meUInt arg;                 /* Decode key argument */
            mouse_X = event.xmotion.x / mecm.fwidth ;
            mouse_Y = event.xmotion.y / mecm.fdepth ;
            mouse_dX = ((event.xmotion.x - (mouse_X * mecm.fwidth)) << 8) / mecm.fwidth;
            mouse_dY = ((event.xmotion.y - (mouse_Y * mecm.fdepth)) << 8) / mecm.fdepth;
            /* Check for a mouse-move key */
            cc = (ME_SPECIAL | mouseKeyState |
                  (SKEY_mouse_move+mouseKeys[mouseButtonGetPick()])) ;
            /* Are we after all movements or mouse-move bound ?? */
            if((TTallKeys & 0x1) || (!TTallKeys && (decode_key(cc,&arg) != -1)))
                addKeyToBufferOnce(cc) ; /* Add key to keyboard buffer */
        }
        break;
#endif

    case EnterNotify:
        /* Some window managers do not send the FocusIn and FocusOut events correctly
         * To fix this we have to track down Window Enter and Leave events and check
         * for a focus change ourselves */
        if(((frame = meXEventGetFrame(&event)) != NULL) &&
           (frame->flags & meFRAME_NOT_FOCUS))
        {
            Window fwin ;
            int rtr ;
            XGetInputFocus(mecm.xdisplay,&fwin,&rtr) ;
            if(fwin == meFrameGetXWindow(frame))
                meFrameGainFocus(frame) ;
        }
        break ;

    case FocusIn:
        if(((frame = meXEventGetFrame(&event)) != NULL) &&
           (frame->flags & meFRAME_NOT_FOCUS))
            meFrameGainFocus(frame) ;
        break ;

    case LeaveNotify:
        /* Only get this event if using the -f option */
        if(((frame = meXEventGetFrame(&event)) != NULL) &&
           !(frame->flags & meFRAME_NOT_FOCUS))
        {
            Window fwin ;
            int rtr ;
            XGetInputFocus(mecm.xdisplay,&fwin,&rtr) ;
            if(fwin != meFrameGetXWindow(frame))
                meFrameKillFocus(frame) ;
        }
        break ;

    case FocusOut:
        if(((frame = meXEventGetFrame(&event)) != NULL) &&
           !(frame->flags & meFRAME_NOT_FOCUS))
            meFrameKillFocus(frame) ;
        break ;

#if MEOPT_MOUSE
    case ButtonPress:
        if((meMouseCfg & meMOUSE_ENBLE) &&
           ((frame = meXEventGetFrame(&event)) != NULL))

        {
            unsigned int   bb ;
            meUShort ss ;
            if(!(frame->flags & meFRAME_NOT_FOCUS))
            {
                /* if we haven't currently got the input focus, grab it now */
                XSetInputFocus(mecm.xdisplay,meFrameGetXWindow(frame),RevertToPointerRoot,CurrentTime) ;
                meFrameGainFocus(frame) ;
            }
            /* Collect the position of the mouse. Require the row/column mouse
             * position. Also require the fractional bits incase we are scrolling */
            mouse_X = event.xbutton.x / mecm.fwidth ;
            mouse_Y = event.xbutton.y / mecm.fdepth ;
            mouse_dX = ((event.xbutton.x - (mouse_X * mecm.fwidth)) << 8) / mecm.fwidth;
            mouse_dY = ((event.xbutton.y - (mouse_Y * mecm.fdepth)) << 8) / mecm.fdepth;
            bb = event.xbutton.button ;

            /* Add the presense of the button to the mouseState. */
            mouseButtonPick(bb) ;

            /* Generate keycode and shunt on the key queue */
            ss = event.xbutton.state ;
            mouseKeyState = ((ss & 0x01) << 8) | ((ss & 0x0C) << 7);
            ss = (ME_SPECIAL | (SKEY_mouse_pick_1+mouseKeys[bb]-1) | mouseKeyState) ;
            addKeyToBuffer(ss) ;
        }
        break ;
    case ButtonRelease:
        if(meMouseCfg & meMOUSE_ENBLE)
        {
            unsigned int   bb ;
            meUShort ss ;
            /* Collect the position of the mouse. Require the row/column mouse
             * position. Also require the fractional bits incase we are scrolling */
            mouse_X = event.xbutton.x / mecm.fwidth ;
            mouse_Y = event.xbutton.y / mecm.fdepth ;
            mouse_dX = ((event.xbutton.x - (mouse_X * mecm.fwidth)) << 8) / mecm.fwidth;
            mouse_dY = ((event.xbutton.y - (mouse_Y * mecm.fdepth)) << 8) / mecm.fdepth;
            bb = event.xbutton.button ;

            /* Remove the presense of the button from the mouseState. */
            mouseButtonDrop(bb) ;

            /* Generate keycode and shunt on the key queue */
            ss = event.xbutton.state ;
            mouseKeyState = ((ss & 0x01) << 8) | ((ss & 0x0C) << 7);
            ss = (ME_SPECIAL | (SKEY_mouse_drop_1+mouseKeys[bb]-1) | mouseKeyState) ;
            addKeyToBuffer(ss) ;
        }
        break ;
#endif

    case KeyPress:
        {
            meUShort ii, ss ;
            KeySym keySym ;
            char   keyStr[20];

            ss = event.xkey.state ;
            XLookupString(&event.xkey,keyStr,20,&keySym,NULL);
            /* printf("#1 got key %x, ss=%x \n",(unsigned int) keySym, ss) ;*/
            /* keyStr[19] = '\0' ;*/
            /* printf("got key %x, ss=%x [%s]\n",(unsigned int) keySym, ss, keyStr) ;*/
            /* printf("#2 got key %x, ss=%x \n",(unsigned int) keySym, ss) ;*/
            if(keySym > 0xff)
            {
                switch (keySym)
                {
                case XK_BackSpace:      ii = SKEY_backspace; goto special_key;
                case XK_Tab:            ii = SKEY_tab; goto special_key;
                case XK_Linefeed:       ii = SKEY_linefeed; goto special_key;
                case XK_Clear:          ii = SKEY_clear; goto special_key;
                case XK_Return:         ii = SKEY_return; goto special_key;
                    /* Pause, hold */
#if (defined _SUNOS5)
                case XK_F21:
#endif
                case XK_Pause:          ii = SKEY_pause; goto special_key;
#if (defined _SUNOS5)
                case XK_F22:
                case SunXK_Sys_Req:
#endif
                case XK_Sys_Req:        ii = SKEY_sys_req; goto special_key;
                case XK_Escape:         ii = SKEY_esc; goto special_key;
                    /* Delete, rubout */
                case XK_Delete:         ii = SKEY_delete; goto special_key;

                    /* Cursor control & motion */

                case XK_Home:           ii = SKEY_home; goto special_key;
                    /* Move left, left arrow */
                case XK_Left:           ii = SKEY_left; goto special_key;
                    /* Move up, up arrow */
                case XK_Up:             ii = SKEY_up; goto special_key;
                    /* Move right, right arrow */
                case XK_Right:          ii = SKEY_right; goto special_key;
                    /* Move down, down arrow */
                case XK_Down:           ii = SKEY_down; goto special_key;
                    /* Prior, previous */
                    /* XK_Prior*/
                case XK_Page_Up:        ii = SKEY_page_up; goto special_key;
                    /* XK_Next  */
                case XK_Page_Down:      ii = SKEY_page_down; goto special_key;
                    /* EOL */
                case XK_End:            ii = SKEY_end; goto special_key;
                    /* BOL */
                case XK_Begin:          ii = SKEY_home; goto special_key;

                    /* Misc Functions */

                    /* Select, mark */
                case XK_Select:         ii = SKEY_select; goto special_key;
                case XK_Print:          ii = SKEY_print; goto special_key;
                    /* Execute, run, do */
                case XK_Execute:        ii = SKEY_execute; goto special_key;
                    /* Insert, insert here */
                case XK_Insert:         ii = SKEY_insert; goto special_key;
                    /* Undo, oops */
                case XK_Undo:           ii = SKEY_undo; goto special_key;
                    /* redo, again */
                case XK_Redo:           ii = SKEY_redo; goto special_key;
                case XK_Menu:           ii = SKEY_menu; goto special_key;
                    /* Find, search */
                case XK_Find:           ii = SKEY_find; goto special_key;
                    /* Cancel, stop, abort, exit */
                case XK_Cancel:         ii = SKEY_cancel; goto special_key;
                    /* Help */
                case XK_Help:           ii = SKEY_help; goto special_key;
                case XK_Break:          ii = SKEY_break; goto special_key;
                    /* Mode switch - used on foreign keyboards, always ignore. */

                    /* Character set switch */
#ifdef XK_Mode_switch
                case XK_Mode_switch:
#ifdef XK_script_switch
#if XK_script_switch != XK_Mode_switch
                case XK_script_switch:
#endif
#endif
                    goto ignore_key;
#else
#ifdef XK_script_switch
                    /* Alias for mode_switch */
                case XK_script_switch:
                    goto ignore_key;
#endif
#endif
                    /* Keypad Functions, keypad numbers cleverly chosen to map to ascii */
                case XK_KP_Space:       ii = SKEY_space; goto special_key;
                case XK_KP_Tab:         ii = SKEY_tab; goto special_key;
                    /* enter */
                case XK_KP_Enter:       ii = SKEY_return; goto special_key;
                    /* PF1, KP_A, ... */
                case XK_KP_F1:          ii = SKEY_f1; goto special_key;
                case XK_KP_F2:          ii = SKEY_f2; goto special_key;
                case XK_KP_F3:          ii = SKEY_f3; goto special_key;
                case XK_KP_F4:          ii = SKEY_f4; goto special_key;

#if (defined _SUNOS5)
                case XK_F27:
#endif
                case XK_KP_Home:        ii = SKEY_kp_home; goto special_key;
                case XK_KP_Left:        ii = SKEY_kp_left; goto special_key;
                case XK_KP_Up:          ii = SKEY_kp_up; goto special_key;
                case XK_KP_Right:       ii = SKEY_kp_right; goto special_key;
                case XK_KP_Down:        ii = SKEY_kp_down; goto special_key;
                    /*  case XK_KP_Prior:       ii = SKEY_home;  goto special_key;*/
#if (defined _SUNOS5)
                case XK_F29:
#endif
                case XK_KP_Page_Up:     ii = SKEY_kp_page_up; goto special_key;
                    /* case XK_KP_Next:        ii = SKEY_end; goto special_key;*/
#if (defined _SUNOS5)
                case XK_F35:
#endif
                case XK_KP_Page_Down:   ii = SKEY_kp_page_down; goto special_key;
#if (defined _SUNOS5)
                case XK_F33:
#endif
                case XK_KP_End:         ii = SKEY_kp_end; goto special_key;
                case XK_KP_Begin:       ii = SKEY_kp_begin; goto special_key;
                case XK_KP_Insert:      ii = SKEY_kp_insert; goto special_key;
                case XK_KP_Delete:      ii = SKEY_kp_delete; goto special_key;
                    /* equals */
                case XK_KP_Equal:       ii = '='; goto done_key;
#if (defined _SUNOS5)
                case XK_F26:
#endif
                case XK_KP_Multiply:    ii = '*'; goto done_key;
                case XK_KP_Add:         ii = '+'; goto done_key;
                    /* separator, often comma */
                case XK_KP_Separator:   ii = ','; goto done_key;
#if (defined _SUNOS5)
                case XK_F24:
#endif
                case XK_KP_Subtract:    ii = '-'; goto done_key;
                case XK_KP_Decimal:     ii = '.'; goto done_key;
#if (defined _SUNOS5)
                case XK_F25:
#endif
                case XK_KP_Divide:      ii = '/'; goto done_key;

                case XK_KP_0:           ii = '0'; goto done_key;
                case XK_KP_1:           ii = '1'; goto done_key;
                case XK_KP_2:           ii = '2'; goto done_key;
                case XK_KP_3:           ii = '3'; goto done_key;
                case XK_KP_4:           ii = '4'; goto done_key;
                case XK_KP_5:           ii = '5'; goto done_key;
                case XK_KP_6:           ii = '6'; goto done_key;
                case XK_KP_7:           ii = '7'; goto done_key;
                case XK_KP_8:           ii = '8'; goto done_key;
                case XK_KP_9:           ii = '9'; goto done_key;

                    /* foreign keyboard accent keys */
#ifdef XK_dead_circumflex
                case XK_dead_circumflex: ii = '^'; goto done_key;
#endif
#ifdef XK_dead_diaeresis
                case XK_dead_diaeresis: ii = 0xa8; goto done_key;
#endif

                    /* Auxilliary Functions; note the duplicate definitions
                     * for left and right function keys; Sun keyboards and a
                     * few other manufactures have such function key groups on
                     * the left and/or right sides of the keyboard. We've not
                     * found a keyboard with more than 35 function keys total. */

                case XK_F1:             ii = SKEY_f1; goto special_key;
                case XK_F2:             ii = SKEY_f2; goto special_key;
                case XK_F3:             ii = SKEY_f3; goto special_key;
                case XK_F4:             ii = SKEY_f4; goto special_key;
                case XK_F5:             ii = SKEY_f5; goto special_key;
                case XK_F6:             ii = SKEY_f6; goto special_key;
                case XK_F7:             ii = SKEY_f7; goto special_key;
                case XK_F8:             ii = SKEY_f8; goto special_key;
                case XK_F9:             ii = SKEY_f9; goto special_key;
                case XK_F10:            ii = SKEY_f10; goto special_key;
#if (defined _SUNOS5)
                case SunXK_F36:
#endif
                case XK_F11:            ii = SKEY_f11; goto special_key;
#if (defined _SUNOS5)
                case SunXK_F37:
#endif
                case XK_F12:            ii = SKEY_f12; goto special_key;
                    /* ISO keys */
#ifdef XK_ISO_Left_Tab
                    /* Back-tab */
                case XK_ISO_Left_Tab:  ii = SKEY_tab | ME_SHIFT; goto special_key;
#endif

                    /* Modifiers */

                case XK_Caps_Lock:      ii = SKEY_caps_lock; goto special_bound;
                case XK_Num_Lock:       ii = SKEY_num_lock; goto special_bound;
#if (defined _SUNOS5)
                case XK_F23:
#endif
                case XK_Scroll_Lock:    ii = SKEY_scroll_lock; goto special_bound;
                case XK_Shift_Lock:     ii = SKEY_shift_lock; goto special_bound;

                    /* ignore these as ME has its own S-pick/S-drop key event generators */
                case XK_Shift_L:
                case XK_Shift_R:
                case XK_Control_L:
                case XK_Control_R:
                case XK_Meta_L:
                case XK_Meta_R:
                case XK_Alt_L:
                case XK_Alt_R:
#ifdef XK_ISO_Level3_Shift
                case XK_ISO_Level3_Shift:
#endif
#ifdef XK_Super_L
                    /* Windows keys */
                case XK_Super_L:
                case XK_Super_R:
#endif
                    goto ignore_key;

                default:
                    /* printf ("This is a default %d (0x%04x)\n", ii, ii);*/
                    ii = keySym & 0xff;
                    goto done_key;
                }
special_key:
                ii |= ME_SPECIAL; /*bindKeyLookUp[ss & ME_BIND_MASK];*/

                /* Only add the shift mask if it is special */
                if (ShiftMask & ss)
                    ii |= ME_SHIFT;
done_key:
                if (ControlMask & ss)
                    ii |= ME_CONTROL;
                if (Mod1Mask & ss)
                    ii |= ME_ALT;

                /* printf ("Adding ff key to buffer %d(0x%04x)\n", ii, ii);*/
                addKeyToBuffer(ii) ;
ignore_key:
                break;

                /* Keys that are tested for a binding */
special_bound:
                ii |= ME_SPECIAL;

                if (ShiftMask & ss)
                    ii |= ME_SHIFT;
                if (ControlMask & ss)
                    ii |= ME_CONTROL;
                if (Mod1Mask & ss)
                    ii |= ME_ALT;

                /* Key bound */
                /* printf ("Testing ff/bound key to buffer %d(0x%04x)\n", ii, ii);*/
                {
                    meUInt arg;         /* Decode key argument */
                    if (decode_key(ii,&arg) == -1)
                        break ;
                }
                /* printf ("Adding ff/bound key to buffer %d(0x%04x)\n", ii, ii);*/
                addKeyToBuffer(ii) ;
                break;
            }
            else
            {
                keySym &= 0xff ;
                if(ss & (ControlMask|Mod1Mask))
                {
                    keySym = toUpper(keySym) ;
                    if(((ss & ControlMask) == 0) ||
                       ((keySym < 'A') || (keySym > '_')))
                    {
                        keySym = toLower(keySym) ;
                        ii = keySym;
                        if (ss & ControlMask)
                            ii |= ME_CONTROL;
                        if (ss & Mod1Mask)
                            ii |= ME_ALT;
                    }
                    else
                        ii = keySym-'@' ;
                        if (ss & Mod1Mask)
                            ii |= ME_ALT;
                }
                else
                    ii = keySym ;
            }
            /* printf ("Adding 00 key to buffer %d(0x%04x)\n", ii, ii);*/
            addKeyToBuffer(ii) ;
            break;
        }
#ifdef _CLIPBRD
    case SelectionClear:
        if((meXEventGetFrame(&event) != NULL) &&
           (event.xselection.selection == XA_PRIMARY))
            clipState &= ~CLIP_OWNER ;
        break ;

    case SelectionRequest:
        {
            XSelectionEvent reply;

            /* printf("Got SelectionRequest %d, Target = %d %s, string %d\n",*/
            /*        event.xselectionrequest.requestor, event.xselectionrequest.target,*/
            /*        XGetAtomName(mecm.xdisplay, event.xselectionrequest.target),XA_STRING) ;*/
            reply.type = SelectionNotify;
            reply.serial = 1;
            reply.send_event = 1;
            reply.display = mecm.xdisplay;
            reply.requestor = event.xselectionrequest.requestor ;
            reply.selection = event.xselectionrequest.selection ;
            reply.target = event.xselectionrequest.target ;
            reply.property = None ;
            reply.time = event.xselectionrequest.time ;

            if(event.xselectionrequest.selection == XA_PRIMARY)
            {
                if((event.xselectionrequest.target == XA_STRING) && (klhead != NULL))
                {
                    static meUByte *data=NULL ;
                    static int dataLen=0 ;
                    meUByte *ss, *dd, cc ;
                    meKillNode *killp ;
                    int   len ;

                    len = 0 ;
                    killp = klhead->kill;
                    while(killp != NULL)
                    {
                        len += meStrlen(killp->data) ;
                        killp = killp->next;
                    }
                    if((meSystemCfg & meSYSTEM_NOEMPTYANK) && (len == 0))
                        len++ ;
                    if((dataLen <= len) &&
                       ((ss = meMalloc(len+1)) != NULL))
                    {
                        meNullFree(data) ;
                        data = ss ;
                        dataLen = len+1 ;
                    }
                    if(dataLen > len)
                    {
                        ss = data ;
                        killp = klhead->kill;
                        while(killp != NULL)
                        {
                            dd = killp->data ;
                            while((cc = *dd++))
                                *ss++ = cc ;
                            killp = killp->next ;
                        }
                        if((meSystemCfg & meSYSTEM_NOEMPTYANK) && (ss == data))
                            *ss++ = ' ' ;
                        *ss = '\0' ;
                        reply.property = event.xselectionrequest.property ;
                        XChangeProperty(mecm.xdisplay,reply.requestor,reply.property,reply.target,
                                        8,PropModeReplace,data,len);
                    }
                }
                else if(event.xselectionrequest.target == meAtoms[meATOM_TARGETS])
                {
                    reply.property = event.xselectionrequest.property ;
                    XChangeProperty(mecm.xdisplay,reply.requestor,reply.property,reply.target,
                                    32,PropModeReplace,(unsigned char *) (meAtoms+meATOM_TARGETS),2);
                }
            }
            XSendEvent(mecm.xdisplay,reply.requestor,False,0,(XEvent *) &reply) ;
            break;
        }
    case SelectionNotify:
        if((frame = meXEventGetFrame(&event)) != NULL)
        {
            unsigned long nitems, left;
            meUByte *buff, *dd ;
            Atom type ;
            int  fmt ;

            if((event.xselection.selection == XA_PRIMARY) &&
               (event.xselection.property == meAtoms[meATOM_COPY_TEXT]) &&
               (XGetWindowProperty(mecm.xdisplay,meFrameGetXWindow(frame),meAtoms[meATOM_COPY_TEXT],0,0x1fffffffL,False,AnyPropertyType,
                                   &type, &fmt, &nitems, &left, &buff) == Success))
            {
                if(type == meAtoms[meATOM_MULTIPLE])
#ifndef NDEBUG
                    printf("Currently don't support multiple\n")
#endif
                          ;
                else if((type == meAtoms[meATOM_INCR]) && (fmt == 32) && (nitems > 0))
                {
                    nitems = ((long *) buff)[0] ;
                    XFree(buff) ;
                    /* always killSave, don't want to glue 'em together */
                    killSave();
                    if((dd = killAddNode(nitems)) != NULL)
                    {
                        dd[0] = '\0' ;
                        thisflag = meCFKILL ;
                        meClipSize = nitems ;
                        meClipBuff = dd ;
                        /* Start the text transfer */
                        XDeleteProperty(mecm.xdisplay,meFrameGetXWindow(frame),meAtoms[meATOM_COPY_TEXT]);
                        XFlush(mecm.xdisplay);
                        /* don't flag that we've got the clipboard yet cos we haven't */
                        break ;
                    }
                }
                else if((type == XA_STRING) && (fmt == 8) && (nitems > 0))
                {
                    if((klhead == NULL) || (klhead->kill == NULL) ||
                       (klhead->kill->next != NULL) ||
                       meStrncmp(klhead->kill->data,buff,nitems) ||
                       (klhead->kill->data[nitems] != '\0'))
                    {
                        /* always killSave, don't want to glue 'em together */
                        killSave();
                        if((dd = killAddNode(nitems)) != NULL)
                        {
                            meStrncpy(dd,buff,nitems) ;
                            dd[nitems] = '\0' ;
                        }
                        thisflag = meCFKILL;
                    }
                }
                XFree(buff) ;

                /* Always flag that we got the event */
                clipState |= CLIP_RECEIVED ;
            }
            else
                xdndSelectionNotify (&event, meXEventGetFrame(&event));
            break ;
        }

    case PropertyNotify:
        if(((frame = meXEventGetFrame(&event)) != NULL) &&
           (meClipSize > 0) && (event.xproperty.state == PropertyNewValue) &&
           (event.xproperty.atom == meAtoms[meATOM_COPY_TEXT]))
        {
            unsigned long nitems, left;
            meUByte *buff ;
            Atom type ;
            int  fmt, ret ;

            ret = XGetWindowProperty(mecm.xdisplay,meFrameGetXWindow(frame),meAtoms[meATOM_COPY_TEXT],
                                     0L,0x1fffffffL,True,AnyPropertyType,
                                     &type, &fmt, &nitems, &left, &buff) ;

            if((ret == Success) && (type == XA_STRING) && (fmt == 8))
            {
                if(nitems == 0)
                {
                    /* got all we're going to */
                    meClipSize = 0 ;
                    clipState |= CLIP_RECEIVED ;
                }
                else
                {
                    if(nitems >= meClipSize)
                    {
                        /* flag that we have the clipboard */
                        nitems = meClipSize ;
                        clipState |= CLIP_RECEIVED ;
                    }
                    memcpy(meClipBuff,buff,nitems) ;
                    meClipSize -= nitems ;
                    meClipBuff += nitems ;
                    *meClipBuff = '\0' ;
                    XFree(buff) ;
                }
            }
        }
        break ;
#endif
    case ClientMessage:
        if(((frame = meXEventGetFrame(&event)) != NULL) &&
           (event.xclient.message_type == meAtoms[meATOM_WM_PROTOCOLS]) &&
           (event.xclient.format == 32))
        {
            if(((Atom) event.xclient.data.l[0]) == meAtoms[meATOM_WM_DELETE_WINDOW])
            {
#if MEOPT_MWFRAME
                if(meFrameDelete(frame,6) <= 0)
#endif
                {
                    /* Use the normal command to save buffers and exit
                     * if it doesn't exit then carry on as normal
                     * Must ensure we ask the user, not a macro
                     */
                    int savcle ;
                    savcle = clexec ;
                    clexec = meFALSE ;
                    exitEmacs(1,3) ;
                    clexec = savcle ;
                }
            }
            else if(((Atom) event.xclient.data.l[0]) == meAtoms[meATOM_WM_SAVE_YOURSELF])
            {
                /* printf("Got a SAVE_YOURSELF\n") ;*/
            }
        }
        else
           xdndClientMessage (&event, meXEventGetFrame(&event));
        break ;
    default:
        /* all events selected by StructureNotifyMask
         * except ConfigureNotify are thrown away here,
         * since nothing is done with them */
        break;
    } /* end switch */
}

#endif

#ifdef _ME_CONSOLE
#ifdef _TCAP

/****************************************************************************
 * TCAP FUNCTIONS                                                           *
 ****************************************************************************/

#ifdef _USG
/* Get the terminal attributes structure. There are cases when the call will
 * fail we have been launched off the desktop via an open action. Return an
 * integer status of -1 if the call failed and we filled in the values. */
static int
TCAPgetattr (meTermio p, int isX)
{
    int status;                         /* Status for attribute retrieval. */

#ifdef _TERMIOS
    if ((status = tcgetattr (0, p)) < 0)
#else
    if ((status = ioctl(0, TCGETA, p)) < 0)
#endif
    {
        /* Input flag defaults */
        p->c_iflag = BRKINT|ICRNL|IXANY;
        /* Output flag defaults */
        p->c_oflag = OPOST|ONLCR;
        /* Control modes */
        p->c_cflag = CS8|CREAD|HUPCL;
        /* Local modes */
        p->c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHOCTL|ECHOKE;

        /* Terminal special characters */
        p->c_cc [VINTR] = 'C' - '@';    /* C-c : CINTR */
        p->c_cc [VQUIT] = CQUIT;        /* FS, cntl | */
        p->c_cc [VERASE] = 'H' - '@';   /* Backspace or '#' 0x7f */
        p->c_cc [VKILL] = 'K' - '@';    /* C-k */
        p->c_cc [VEOF] = CEOF;          /* C-d */
        p->c_cc [VEOL] = 'J' - '@';     /* C-j */
        p->c_cc [VMIN] = 1;
        p->c_cc [VTIME] = 0;
#ifdef _TERMIOS
#ifdef VWERASE
        p->c_cc [VWERASE] = 'W' - '@';  /* C-w */
#endif
        p->c_cc [VLNEXT] = CLNEXT;      /* C-v */
#ifdef VDSUSP
        p->c_cc [VDSUSP] = CDSUSP;      /* C-y */
#endif
        p->c_cc [VSUSP] = CSUSP;        /* C-z */
        p->c_cc [VSTART] = CSTART;      /* C-q */
        p->c_cc [VSTOP]  = CSTOP;       /* C-s */
#endif
    }
    else if (isX != 0)
    {
        /* We are running X-Windows. We have been launched from the command
         * line as we have obtained the terminal environment. Set this up for
         * an interactive shell session. */
#ifdef _HPUX
        /* HP-UX seems to be susceptible to "su" problems when running "me&".
         *
         * Fix any base modes in-case we have been started in the background.
         * At the very minimum we need to echo characters and process
         * concanical input. */
        p->c_iflag |= ICRNL;
        p->c_lflag |= ICANON|ECHO|ECHOK|ECHOE;
#endif
        /* Correct the erase if it is defaulted */
        if (p->c_cc [VERASE] == 0x7f)
            p->c_cc [VERASE] = 'H' - '@';  /* Backspace */
    }

    /* Logging for tracing the output of the termio statement at start-up. We
     * cannot dump onto stdio because it may not exist. */
#if 0
    {
        FILE *fp;
        int ii;
#ifdef _TERMIOS
#define MECC_LEN NCCS
#else
#define MECC_LEN NCC
#endif
        if ((fp = fopen ("/tmp/termio.out", "w")) != NULL)
        {
            fprintf (fp, "status = %d\n", status);
            fprintf (fp, "c_iflag = 0x%08x %011o\n", p->c_iflag, p->c_iflag);
            fprintf (fp, "c_oflag = 0x%08x %011o\n", p->c_oflag, p->c_oflag);
            fprintf (fp, "c_cflag = 0x%08x %011o\n", p->c_cflag, p->c_cflag);
            fprintf (fp, "c_lflag = 0x%08x %011o\n", p->c_lflag, p->c_lflag);

            for (ii = 0; ii < MECC_LEN; ii++)
                fprintf (fp, "c_cc[%d] = 0x%08x %011o\n",
                         ii, p->c_cc[ii], p->c_cc[ii]);
            fclose (fp);
        }
    }
#endif

    /* Return the status of the system call. */
    return status;
}
#endif /* _USG */

int
TCAPstart(void)
{
    char *p ;
    char tcbuf[4096];                   /* Jon 2001/12/15 increase for cygwin terminal */
    char *tv_stype;
    char err_str[72];
    int  ii ;

    /* Get the attributes from the system */
#ifdef _USG
    TCAPgetattr (&otermio, 0);
#endif

    /* default to use fonts - usually supports reverse */
    meSystemCfg &= ~meSYSTEM_RGBCOLOR ;
    meSystemCfg |= (meSYSTEM_CONSOLE|meSYSTEM_FONTS) ;
    if((tv_stype = meGetenv("TERM")) == NULL)
    {
        puts("Environment variable TERM not defined");
        meExit(1);
    }

    if((tgetent(tcbuf, tv_stype)) != 1)
    {
        sprintf(err_str, "Unknown terminal type [%s]", tv_stype);
        puts(err_str);
        meExit(1);
    }
    /* Initialise the termcap strings */
    p = tcapbuf;
    for (ii = 0; ii < TCAPmax; ii++)
    {
        if (tcaptab[ii].type == TERMCAP_BOOLN)
            tcaptab[ii].code.value = tgetflag (tcaptab[ii].capKey);
        else if (tcaptab[ii].type == TERMCAP_DIGIT)
            tcaptab[ii].code.value = tgetnum (tcaptab[ii].capKey);
        else
        {
            tcaptab[ii].code.str = tgetstr(tcaptab[ii].capKey, &p);
            if ((tcaptab[ii].code.str == NULL) ||
                (tcaptab[ii].code.str[0] == '\0'))
                tcaptab[ii].code.str = NULL;
        }
    }

    /* Make sure that there was sufficient buffer space to process the strings */
    if (p >= &tcapbuf[TCAPSLEN])
    {
        sprintf(err_str, "%s termcap longer than %d characters",
                tv_stype, TCAPSLEN);
        puts(err_str);
        meExit(1);
    }

    /* Sort out the visibility flags. We must have both set or both NULL */
    if (tcaptab[TCAPcivis].code.str == NULL)
        tcaptab[TCAPcnorm].code.str = NULL;
    else if (tcaptab[TCAPcnorm].code.str == NULL)
        tcaptab[TCAPcivis].code.str = NULL;

    /* Determine if there is a mechanism to enable and disable the automatic
     * margins. If there is then disable them now */
    if (tcaptab[TCAPam].code.value == 1)
    {
        /* Try to disable the margins */
        if (tcaptab[TCAPsmam].code.str != NULL)
        {
            putpad (tcaptab[TCAPsmam].code.str);
            tcaptab[TCAPam].code.value = tgetnum (tcaptab[TCAPam].capKey);
            TTaMarginsDisabled = 1;
        }
    }
    TCAPgetWinSize() ;
    TTdepthDefault = TTnewHig ;
    TTwidthDefault = TTnewWid ;

    /* We rely on Clear and Move <x><y>, make sure that these exist */
    if ((tcaptab[TCAPclear].code.str == NULL) ||
        (tcaptab[TCAPcup].code.str == NULL))
    {
        sprintf(err_str, "%s termcap lacks %s or %s entry",
                tv_stype,tcaptab[TCAPclear].capKey, tcaptab[TCAPcup].capKey);
        puts(err_str);
        meExit(1);
    }

    /* Try and setup some of the standard keys like the cursor keys */
    {
        char buf[20] ;
        meUShort ii, sl[20] ;

        for(ii=0 ; ii<SKEY_MAX ; ii++)
        {
            if(tcapSpecKeyDefs[ii] != NULL)
                translateKeyAdd(&TTtransKey,charListToShorts(sl,tcapSpecKeyDefs[ii]),
                                TTtransKey.time,sl,ME_SPECIAL|ii) ;
        }
        for(ii=0 ; ii<SKEY_MAX ; ii++)
        {
            p = buf ;
            if((tcapSpecKeyStrs[ii] != NULL) && ((p=tgetstr(tcapSpecKeyStrs[ii], &p)) != NULL))
                translateKeyAdd(&TTtransKey,charListToShorts(sl,(meUByte *) p),
                                TTtransKey.time,sl,(ii|ME_SPECIAL)) ;
            else
                tcapSpecKeyStrs[ii] = NULL ;
        }
        /* KEY_PPAGE, sent by previous-page key */
        if((tcapSpecKeyStrs[SKEY_page_up] == NULL) && ((p=tgetstr("kP", &p)) != NULL))
        {
            tcapSpecKeyStrs[SKEY_page_up] = "kP" ;
            translateKeyAdd(&TTtransKey,charListToShorts(sl,(meUByte *) p),
                            TTtransKey.time,sl,(SKEY_page_up|ME_SPECIAL)) ;
        }
        /* KEY_NPAGE, sent by next-page key */
        if((tcapSpecKeyStrs[SKEY_page_down] == NULL) && ((p=tgetstr("kN", &p)) != NULL))
        {
            tcapSpecKeyStrs[SKEY_page_down] = "kN" ;
            translateKeyAdd(&TTtransKey,charListToShorts(sl,(meUByte *) p),
                            TTtransKey.time,sl,(SKEY_page_down|ME_SPECIAL)) ;
        }
    }
    /* Switch off the vertical window scroll bar */
    gsbarmode &= ~WMVBAR ;
    {
#ifdef _POSIX_SIGNALS
        struct sigaction sa ;
        sigemptyset(&sa.sa_mask) ;
        sa.sa_flags=0 ;
        sa.sa_handler=sigSize ;
        sigaction(SIGWINCH,&sa,NULL) ;
#else
        signal (SIGWINCH, sigSize);
#endif
    }
    return TCAPopen() ;
}

int
TCAPopen(void)
{
    if(alarmState & meALARM_PIPED)
        return meTRUE ;

#ifdef _USG
    ntermio = otermio;

/*    ntermio.c_lflag &= ~(ISIG|ICANON|ECHO);*/
    ntermio.c_lflag &= ~(ISIG|ICANON|ECHO|IEXTEN);
/*    ntermio.c_iflag &= ~(IXON|ICRNL);*/
    ntermio.c_iflag &= ~(IXON|INLCR|INPCK|ICRNL|ISTRIP);
#ifndef _TERMIOS
    /* Additional SVR4 settings for termio */
    ntermio.c_iflag &= ~(IXANY);
#endif
#ifdef TAB3
    ntermio.c_oflag &= ~(ONLCR|TAB3);
#else
    ntermio.c_oflag &= ~ONLCR ;
#endif
    ntermio.c_iflag |= IGNBRK ;
    ntermio.c_cc[VMIN] = 1 ;
    ntermio.c_cc[VTIME] = 0 ;
/*    ntermio.c_cc[VINTR] = 7 ;*/
#ifdef _TERMIOS
    tcsetattr (meStdin, TCSAFLUSH, &ntermio);
#else
    ioctl(0, TCSETA, &ntermio); /* and activate them */
#endif
#endif /* _USG */

#ifdef _BSD
    /* Set up the TTY states */
    gtty (0, &osgttyb);
    memcpy (&nsgttyb,  &osgttyb,  sizeof (struct sgttyb)) ;
#ifdef _BSD_CBREAK
    /* Use CBREAK rather than raw mode */
    nsgttyb.sg_flags &= ~(ECHO|CRMOD) ;
    nsgttyb.sg_flags |= CBREAK ;
#else
    nsgttyb.sg_flags &= ~(ECHO|CRMOD) ;
    nsgttyb.sg_flags |= RAW ;
#endif
    stty (0, &nsgttyb);

    /* Set up the line disciplines */
#ifdef _BSD_CBREAK
    ioctl (0, TIOCGETC, &otchars) ;
    memcpy (&ntchars,  &otchars,  sizeof (struct tchars)) ;
    ntchars.t_intrc  = -1 /*07*/;       /* Interrupt character ^G */
    ntchars.t_quitc  = -1 ;             /* Quit - disabled - disabled */
    ntchars.t_startc = -1 /*021*/;      /* Start character ^Q - disabled */
    ntchars.t_stopc  = -1 /*023*/;      /* Stop character ^S - disabled */
    ntchars.t_eofc   = -1 ;             /* End of file ^D - disabled */
    ntchars.t_brkc   = -1 ;             /* End of line - diabled */
    ioctl (0, TIOCSETC, &ntchars) ;
#endif

    /* Set up the local line disciplines special */
    ioctl (0, TIOCGLTC, &oltchars) ;
    memcpy (&nltchars, &oltchars, sizeof (struct ltchars)) ;
    nltchars.t_suspc  = -1 ;            /* Disable the lot */
    nltchars.t_dsuspc = -1 ;
    nltchars.t_rprntc = -1 ;
    nltchars.t_flushc = -1 ;
    nltchars.t_werasc = -1 ;
    nltchars.t_lnextc = -1 ;
    ioctl (0, TIOCSLTC, &nltchars) ;

    /* Set up the type ahead buffer. There are some inconsistancies here on
     * various systems so it is better simply to define a default output
     * buffer size. */
    setbuffer (stdout, termOutBuf, sizeof (termOutBuf));
#endif /* _BSD */

#if (defined NTTYDISC) && (defined TIOCSETD)
     /* Set the line discipline to the new Berkley standard */
     {
         int ldisc = NTTYDISC;
         ioctl (0, TIOCSETD, &ldisc);
     }
#endif /*  NTTYDISC/TIOCSETD */

    /* If automatic margins are enabled then try to disable them */
    if ((tcaptab[TCAPam].code.value) &&
        (tcaptab[TCAPrmam].code.str != NULL) &&
        (TTaMarginsDisabled == 0))
    {
        /* Automatic margins are enabled - disable */
        tputs(tcaptab[TCAPrmam].code.str,1,putchar) ;
        tcaptab[TCAPam].code.value = tgetnum(tcaptab[TCAPam].capKey);
        TTaMarginsDisabled = 1;
    }

    /* Success */
    return meTRUE ;
}

/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
int
TCAPclose(void)
{
    if(alarmState & meALARM_PIPED)
        return meTRUE ;

    mlerase(MWERASE|MWCURSOR);
    TCAPschemeReset() ;

#if MEOPT_COLOR
    /* Restore the original colors */
    if (tcaptab[TCAPoc].code.str != NULL)/* Restore colors */
        putpad(tcaptab[TCAPoc].code.str) ;
    if (tcaptab[TCAPop].code.str != NULL)/* Restore pairs */
        putpad(tcaptab[TCAPop].code.str) ;
#endif

    /* If automatic margins are disabled then try to enable them again */
    if (TTaMarginsDisabled != 0)
    {
        if (tcaptab[TCAPsmam].code.str != NULL)
        {
            putpad(tcaptab[TCAPsmam].code.str);
            tcaptab[TCAPam].code.value = tgetnum (tcaptab[TCAPam].capKey);
        }
        TTaMarginsDisabled = 0;
    }

    /* Restore the terminal modes */
#ifdef _USG
#ifdef _TERMIOS
    return tcsetattr (meStdin, TCSAFLUSH, &otermio);
#else
    return ioctl(0, TCSETA, &otermio);  /* restore terminal settings */
#endif
#endif /* _USG */
#ifdef _BSD
    /* Restore the terminal settings */
    stty (0, &osgttyb);
#ifdef _BSD_CBREAK
    ioctl (0, TIOCSETC, &otchars) ;
#endif
    ioctl (0, TIOCSLTC, &oltchars) ;
    return meTRUE;
#endif
}

void
TCAPhideCur(void)
{
    /* If we have a cursor hide capability then use it */
    if (tcaptab[TCAPcivis].code.str != NULL)
    {
        if (TTcursorVisible != 0)
        {
            putpad (tcaptab[TCAPcivis].code.str);
            TTcursorVisible = 0;
        }
    }
    else
    {
        /* Otherwise, move the cursor off screen (bottom right) */
        TCAPmove(frameCur->depth-1,frameCur->width-1) ;
    }
}

void
TCAPshowCur(void)
{
    if((frameCur->cursorRow <= frameCur->depth) && (frameCur->cursorColumn < frameCur->width))
    {
        /* Make sure that the cursor is visible */
        if ((TTcursorVisible == 0) && (tcaptab[TCAPcnorm].code.str != NULL))
        {
            putpad (tcaptab[TCAPcnorm].code.str);
            TTcursorVisible = 1;
        }
        TCAPmove(frameCur->cursorRow,frameCur->cursorColumn) ;
    }
    /* Cursor is off the screen - hide it */
    else if (tcaptab[TCAPcivis].code.str != NULL)
    {
        if (TTcursorVisible != 0)
        {
            putpad (tcaptab[TCAPcivis].code.str);
            TTcursorVisible = 0;
        }
    }
    else
    {
        /* move the cursor off screen (bottom right) */
        TCAPmove(frameCur->depth-1,frameCur->width-1) ;
    }
}

#if MEOPT_COLOR

int
TCAPaddColor(meUByte index, meUByte r, meUByte g, meUByte b)
{
    meUByte *ss ;
    int ii, jj, idif, jdif ;

    jdif = 256*256*3 ;                  /* Smallest least squares. */
    ss = tcapColors ;

    /* To find the nearest color then use a least squares method. This
     * produces a better approximation than a straight forward color
     * differencing algorithm as it takes into account the variance. */
    for(ii=0 ; ii<tcapNumColors ; ii++)
    {
        int delta;

        delta = r - *ss++;
        idif  = (delta * delta) ;
        delta = g - *ss++;
        idif += (delta * delta) ;
        delta = b - *ss++ ;
        idif += (delta * delta) ;
        if(idif < jdif)
        {
            jdif = idif ;
            jj = ii ;
        }
    }

    if(noColors <= index)
    {
        colTable = (meUInt *) meRealloc(colTable,(index+1)*sizeof(meUInt)) ;
        memset(colTable+noColors,0, (index-noColors+1)*sizeof(meUInt)) ;
        noColors = index+1 ;
    }
    colTable[index] = jj ;

    return meTRUE ;
}

#endif

static meUByte oschemeFcol=meCOLOR_INVALID ;
static meUByte oschemeBcol=meCOLOR_INVALID ;
static meUByte oschemeFont=0 ;
static meUByte oschemeFntr=0 ;

void
TCAPschemeSet(meScheme scheme)
{
    meStyle nstyle ;

    nstyle = meSchemeGetStyle(scheme) ;

    /* Termcap fonts */
#ifdef _TCAPFONT
    if(meSystemCfg & meSYSTEM_FONTS)
    {
        meUByte font ;

        font = meStyleGetFont(nstyle) ;
        if(meSchemeTestNoFont(scheme))
            font &= ~(meFONT_BOLD|meFONT_ITALIC|meFONT_UNDERLINE) ;

        if(oschemeFont != font)
        {
            if(oschemeFntr)
            {
                /* Remove the old font attributes. We cannot guarantee the
                 * state of some attributes so it is best to turn all off and
                 * start again, */
                if ((oschemeFont & meFONT_ITALIC) && (tcaptab[TCAPritm].code.str != NULL))
                    putpad (tcaptab[TCAPritm].code.str);
                if ((oschemeFont & meFONT_UNDERLINE) && (tcaptab[TCAPrmul].code.str != NULL))
                    putpad (tcaptab[TCAPrmul].code.str);
                if ((oschemeFont & (meFONT_BOLD|meFONT_REVERSE|meFONT_LIGHT)) &&
                    (tcaptab[TCAPsgr0].code.str != NULL))
                    putpad (tcaptab[TCAPsgr0].code.str);
                oschemeFcol = meCOLOR_INVALID ;
                oschemeBcol = meCOLOR_INVALID ;
                oschemeFntr = 0 ;
            }
            oschemeFont = font;
            if (font & meFONT_MASK)
            {
                oschemeFntr = 1 ;
                /* Apply the modes */
                if ((font & meFONT_BOLD) && (tcaptab[TCAPbold].code.str != NULL))
                    putpad(tcaptab[TCAPbold].code.str);
                if ((font & meFONT_ITALIC) && (tcaptab[TCAPsitm].code.str != NULL))
                    putpad (tcaptab[TCAPsitm].code.str);
                if ((font & meFONT_LIGHT) && (tcaptab[TCAPdim].code.str != NULL))
                    putpad (tcaptab[TCAPdim].code.str);
                if ((font & meFONT_REVERSE) && (tcaptab[TCAPrev].code.str != NULL))
                    putpad (tcaptab[TCAPrev].code.str);
                if ((font & meFONT_UNDERLINE) && (tcaptab[TCAPsmul].code.str != NULL))
                    putpad (tcaptab[TCAPsmul].code.str);
            }
        }
    }
#endif /* _TCAPFONT */

    /* Termcap coloring */
#if MEOPT_COLOR
    if(meSystemCfg & meSYSTEM_ANSICOLOR)
    {
        meUByte col ;

        /* Foreground color */
        col = colTable[meStyleGetFColor(nstyle)] ;
        if(oschemeFcol != col)
        {
            if (tcaptab[TCAPsetaf].code.str != NULL)
            {
                /* Have a termcap entry for color ?? */
                putpad (meTCAPParm(tcaptab[TCAPsetaf].code.str, col));
            }
            else
            {
                /* Try our ANSI color */
                FCOLSTR[3]= (col & 0x07) + 48;
                putpad (FCOLSTR);
            }
            oschemeFcol = col ;
        }

        /* Background color */
        col = colTable[meStyleGetBColor(nstyle)] ;
        if(oschemeBcol != col)
        {
            if (tcaptab[TCAPsetab].code.str != NULL)
            {
                /* Have a termcap entry for color ?? */
                putpad(meTCAPParm(tcaptab[TCAPsetab].code.str, col));
            }
            else
            {
                BCOLSTR[3]=(col & 0x07) + 48 ;
                putpad (BCOLSTR);
            }
            oschemeBcol = col ;
        }
    }
#endif /* MEOPT_COLOR */
}

void
TCAPschemeReset(void)
{
#ifdef _TCAPFONT
    if(oschemeFntr)
    {
        /* Remove the old font attributes. We cannot guarantee the state of
         * some attributes so it is best to turn all off and start again, */
        if ((oschemeFont & meFONT_ITALIC) && (tcaptab[TCAPritm].code.str != NULL))
            putpad (tcaptab[TCAPritm].code.str);
        if ((oschemeFont & meFONT_UNDERLINE) && (tcaptab[TCAPrmul].code.str != NULL))
            putpad (tcaptab[TCAPrmul].code.str);
        if ((oschemeFont & (meFONT_BOLD|meFONT_REVERSE|meFONT_LIGHT)) &&
            (tcaptab[TCAPsgr0].code.str != NULL))
            putpad (tcaptab[TCAPsgr0].code.str);
        oschemeFntr = 0 ;
    }
#endif

#if MEOPT_COLOR
    if(meSystemCfg & meSYSTEM_ANSICOLOR)
    {
        if ((oschemeFcol != meCOLOR_INVALID) || (oschemeBcol != meCOLOR_INVALID))
        {
            /* Disable the color mode */
            if (tcaptab[TCAPsgr0].code.str != NULL)
                putpad (tcaptab[TCAPsgr0].code.str);
            else
                putpad (RCOLSTR);
        }
    }
#endif
    /* Reset the fonts regardless of the mode */
    oschemeFont = 0 ;
    oschemeFcol = meCOLOR_INVALID ;
    oschemeBcol = meCOLOR_INVALID ;
}

void
TCAPmove(int row, int col)
{
    /* We really need to perform this operation without cup, hence the
     * movement to a function from a macro. In addition we should really be
     * checking if we can move with attributes enabled, if this is not the
     * case then we need to reset out modes, move and then re-apply. The API
     * would be better served with a TCAPmoveScheme() where we could set the
     * next scheme at the same time. Keep this for a rainy day !! */
    tputs(tgoto(tcaptab[TCAPcup].code.str,col,row),1,putchar);
}

#endif /* _TCAP */
#endif /* _ME_CONSOLE */

#if MEOPT_MOUSE
/*
 * TTinitMouse
 * Sort out what to do with the mouse buttons.
 */
void
TTinitMouse(void)
{
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
        meMouseCfg &= ~meMOUSE_ENBLE ;
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
        if(meMouseCfg & meMOUSE_ENBLE)
    {
#ifdef _XTERM
        static meUByte  meCurCursor=0 ;
        static Cursor meCursors[meCURSOR_COUNT]={0,0,0,0,0,0,0} ;
        static meUByte  meCursorChar[meCURSOR_COUNT]={
            0,XC_arrow,XC_xterm,XC_crosshair,XC_hand2,XC_watch,XC_pirate} ;
        meUByte cc ;
        int b1, b2, b3 ;

        if(meMouseCfg & meMOUSE_SWAPBUTTONS)
            b1 = 3, b3 = 1 ;
        else
            b1 = 1, b3 = 3 ;
        if((meMouseCfg & meMOUSE_NOBUTTONS) > 2)
            b2 = 2 ;
        else
            b2 = b3 ;
        mouseKeys[1] = b1 ;
        mouseKeys[2] = b2 ;
        mouseKeys[3] = b3 ;

        cc = (meUByte) ((meMouseCfg & meMOUSE_ICON) >> 16) ;
        if(cc >= meCURSOR_COUNT)
            cc = 0 ;
        if(cc != meCurCursor)
        {
            if(meCurCursor)
            {
                XUndefineCursor(mecm.xdisplay,meFrameGetXWindow(frameCur)) ;
                meCurCursor = 0 ;
            }
            if(cc && ((meCursors[cc] != 0) ||
                      ((meCursors[cc] = XCreateFontCursor(mecm.xdisplay,meCursorChar[cc])) != 0)))
            {
                XDefineCursor(mecm.xdisplay,meFrameGetXWindow(frameCur),meCursors[cc]) ;
                meCurCursor = cc ;
            }
        }
#endif /* _XTERM */
    }
#endif /* _ME_WINDOW */
}
#endif

#ifdef _ME_WINDOW
#ifdef _XTERM

/* XTERMsetFont; Determine the font that we shall use. Use the base
 * string provided by the user (or system) and determine if there
 * are any derivatives of the font */
static int
XTERMsetFont(char *fontName)
{
    int ii;
    XFontStruct *font ;
    
    if (fontName != NULL)
    {
        /* Load the basic font into the server, fail if we cannot find it. */
        if((font=XLoadQueryFont(mecm.xdisplay,fontName)) == NULL)
            return meFALSE ;
    }
    else
    {
        /* No font has been specified. Attempt to find a font that we know
         * might exist. We provide a few alternatives to fall back on as this
         * was an issue with Fedora. Only search for mono or character
         * upright fonts. */
        char *altFonts[] =
        {
            "fixed",
            "-*-clean-medium-r-*-*-*-130-*-*-*-*-iso8859-1",
            "-*-clean-medium-r-*-*-*-130-*-*-*-*-iso8859-*",
            "-*-fixed-medium-r-*-*-*-120-*-*-*-*-iso8859-*",
            "-*-*-medium-r-*-*-*-*-*-*-c-*-iso8859-*",
            "-*-*-medium-r-*-*-*-*-*-*-m-*-iso8859-*",
            "-*-*-regular-r-*-*-*-*-*-*-c-*-iso8859-*",
            "-*-*-regular-r-*-*-*-*-*-*-m-*-iso8859-*",
            "-*-*-medium-r-*-*-*-*-*-*-c-*-*-*",
            "-*-*-medium-r-*-*-*-*-*-*-m-*-*-*",
            "-*-*-regular-r-*-*-*-*-*-*-c-*-*-*",
            "-*-*-regular-r-*-*-*-*-*-*-m-*-*-*",
            NULL
        };
        char **p = altFonts;
        
        /* There is no font specified, iterate over the fonts to find
         * something we can load as default unstead of failing. */
        do
        {
            /* Advance the font and bail out if we cannot load anyting. */
            if ((fontName = *p++) == NULL)
                return meFALSE ;
            /* Load the font into the server. We will drop out of the loop if the  */
            font = XLoadQueryFont(mecm.xdisplay,fontName);
        }
        while (font == NULL);
    }
    
    /* Make sure that the font is legal and we do not get a divide by zero
     * error through zero width characters. */
    if ((font->ascent + font->descent == 0) ||
        (font->max_bounds.width == 0))
    {
        XUnloadFont (mecm.xdisplay, font->fid);
        return meFALSE;
    }

    /* Font is acceptable - continue to load. */
    mecm.ascent    = font->ascent ;
    mecm.descent   = font->descent ;
    mecm.fdepth    = mecm.ascent + font->descent ;
    mecm.fwidth    = font->max_bounds.width ;
    mecm.fhdepth   = mecm.fdepth >> 1 ;
    mecm.fhwidth   = mecm.fwidth >> 1 ;
    mecm.underline = mecm.fdepth - mecm.ascent - 1;

    if((mecm.fadepth = mecm.fwidth+1) > mecm.fdepth)
        mecm.fadepth = mecm.fdepth ;

    sizeHints.height_inc = mecm.fdepth ;
    sizeHints.width_inc  = mecm.fwidth ;
    sizeHints.min_height = mecm.fdepth*4 ;
    sizeHints.min_width  = mecm.fwidth*10 ;
    sizeHints.base_height = mecm.fdepth ;
    sizeHints.base_width  = mecm.fwidth ;

    /* Clean up the font table for the existing font. Unload all of the
     * previously loaded fonts */
    for (ii = 0; ii < meFONT_MAX ; ii++)
    {
        if(mecm.fontFlag[ii])
            XUnloadFont (mecm.xdisplay, mecm.fontTbl[ii]);
        mecm.fontTbl[ii] = BadName;
        mecm.fontFlag[ii] = 0 ;
    }

    /* Assign the base font */
    mecm.fontTbl[0] = font->fid ;
    mecm.fontFlag[0] = 1;
    mecm.fontId = font->fid ;

    XFreeFontInfo(NULL,font,1) ;

    if(mecm.fontName != NULL)
        free(mecm.fontName) ;

    if((meSystemCfg & meSYSTEM_FONTS) &&
       ((mecm.fontName=meStrdup((meUByte *) fontName)) != NULL))
    {
        meUByte cc, *p = mecm.fontName ;

        /* Break up the font. This must be the full X-Window font definition, we
         * do not understand the X-Windows wildcards and aliases. Lifes too short
         * to sit down and address these now !! */
        for (ii = 0 ; (ii < 14) ; ii++)
        {
            if (*p != '-')
                break;
            *p++ = '\0' ;
            mecm.fontPart[ii] = p ;
            while (((cc = *p) != '\0') && (cc != '-'))
                p++;
        }
        if ((ii != 14) || (*p != '\0'))
        {
            free(mecm.fontName) ;
            mecm.fontName = NULL ;
        }
    }
    else
        mecm.fontName = NULL ;

    return meTRUE;
}

static meFrameData *
XTERMcreateWindow(meUShort width, meUShort depth)
{
    meFrameData *frameData ;
    XClassHint *classHint ;

    if((frameData = meMalloc(sizeof(meFrameData))) == NULL)
        return NULL ;

    sizeHints.x = TTdefaultPosX ;
    sizeHints.y = TTdefaultPosY ;
    sizeHints.width  = width*mecm.fwidth ;
    sizeHints.height = depth*mecm.fdepth ;
    frameData->xmap = meXMAP_UNMAP;
    frameData->xwindow = XCreateSimpleWindow(mecm.xdisplay,RootWindow(mecm.xdisplay,xscreen),
                                             sizeHints.x,sizeHints.y,sizeHints.width,sizeHints.height,0,
                                             WhitePixel(mecm.xdisplay,xscreen),BlackPixel(mecm.xdisplay,xscreen)) ;

    /* Set the class hint correctly - Thanks to Jeremy Cowgar */
    classHint = XAllocClassHint();
    classHint->res_name = "microemacs";
    classHint->res_class = "MicroEmacs";
    XSetClassHint(mecm.xdisplay, frameData->xwindow, classHint);
    XFree(classHint);

    XSetWMNormalHints(mecm.xdisplay,frameData->xwindow,&sizeHints);

    XSetWMProtocols(mecm.xdisplay,frameData->xwindow,meAtoms,2) ;

    /* Make the window DND aware */
    xdndAware (frameData->xwindow);
    
    /* Set the iconic name of the program and icon itself */
    XSetIconName(mecm.xdisplay,frameData->xwindow,meIconName);
    meSetIconState (mecm.xdisplay,frameData->xwindow);
    
    frameData->font = 0 ;
    frameData->fontId = mecm.fontId ;
    frameData->xgcv.font = mecm.fontId ;
    frameData->xgc = XCreateGC(mecm.xdisplay,frameData->xwindow,GCFont,&frameData->xgcv) ;

    /* To get the mouse positional information then we register for
     * "PointerMotionMask" events. */
    /* olwm on sunos has a Focus bug, it does not sent the FocusIn
     * and FocusOut events properly, but as it does send EnterNotify
     * and LeaveNotify events. use this to check the Focus state
     * as well - SWP 17/8/99
     */
    XSelectInput(mecm.xdisplay,frameData->xwindow,
                 (ExposureMask|StructureNotifyMask|KeyPressMask|ButtonPressMask|
                  ButtonReleaseMask|PointerMotionMask|FocusChangeMask|
                  PropertyChangeMask|LeaveWindowMask|EnterWindowMask)) ;
    
    /* Map the window a.s.a.p. cos officially we can't draw to the window
     * until we get an Expose event. As this is not checked for by mapping now
     * there is less chance of this being a problem.
     * 
     * Jon 2006-09-17. The Window cannot be Mapped until the Input is selected
     * otherwise we do not receive any (Un)MapNotify events in Linux. If the
     * above comment is true then we can easily check the Mapping status
     * before processing any paint/expose events.
     */
    XMapWindow(mecm.xdisplay,frameData->xwindow) ;
    
    return frameData ;
}

static meFrameData *firstFrameData=NULL ;
int
meFrameXTermInit(meFrame *frame, meFrame *sibling)
{
    if(sibling == NULL)
    {
        Window fwin ;
        int rtr ;

#if MEOPT_MWFRAME
        if(firstFrameData != NULL)
        {
            /* first call, we have already created the first window
             * in XTERMstart - use that */
            frame->termData = firstFrameData ;
            firstFrameData = 0 ;
        }
        /* An external frame, a new window is required */
        else if((frame->termData = XTERMcreateWindow(frame->width,frame->depth+1)) == NULL)
            return meFALSE ;
#else
        frame->termData = firstFrameData ;
#endif

        /* see if we are the current focus */
        XGetInputFocus(mecm.xdisplay,&fwin,&rtr) ;
        if(fwin == meFrameGetXWindow(frame))
            frame->flags &= ~meFRAME_NOT_FOCUS ;
        else
            frame->flags |= meFRAME_NOT_FOCUS ;
    }
    else
        /* internal frame, just copy the window handler */
        frame->termData = sibling->termData ;
    return meTRUE ;
}

void
meFrameXTermFree(meFrame *frame, meFrame *sibling)
{
    if(sibling == NULL)
    {
        XDestroyWindow(mecm.xdisplay,meFrameGetXWindow(frame)) ;
        XFreeGC(mecm.xdisplay,meFrameGetXGC(frame)) ;
        free(frame->termData) ;
    }
}

void
meFrameXTermMakeCur(meFrame *frame)
{
    XRaiseWindow(mecm.xdisplay, meFrameGetXWindow(frame)) ;
}

/* Heres the xterm equivalent, note that its done in two stages */
int
XTERMstart(void)
{
    XrmDatabase  rdb ;
    XrmValue     retVal ;
    char        *retType ;
    char        *xdefs ;
    int          xx, yy ;
    meUInt       ww, hh  ;
    char        *ss ;

    /* Copy the Terminal I/O. We may spawn a terminal in the window later and
     * the termio structure must be initialised. The structure may be
     * uninitialised if we have been launched off the desktop via an open
     * action. */
#ifdef _USG
    TCAPgetattr (&otermio, 1);
#endif

    /* Configure X-Windows */
    XSetLocaleModifiers ("");
    {
        char *disp ;
        if((disp=meGetenv("DISPLAY")) == NULL)
            disp = ":0.0" ;
        if((mecm.xdisplay = XOpenDisplay(disp)) == NULL)
        {
            fprintf(stderr,"MicroEmacs: Failed to open DISPLAY \"%s\"\n",disp) ;
            exit(1) ;
        }
    }

    meStdin = ConnectionNumber(mecm.xdisplay);
    xscreen = DefaultScreen(mecm.xdisplay) ;
    xcmap   = DefaultColormap(mecm.xdisplay,xscreen) ;

    sizeHints.flags = PSize | PResizeInc | PMinSize | PBaseSize ;
    sizeHints.max_height = DisplayHeight(mecm.xdisplay,xscreen) ;
    sizeHints.max_width  = DisplayWidth(mecm.xdisplay,xscreen) ;

    XrmInitialize ();
    xdefs = XResourceManagerString(mecm.xdisplay);
    if (xdefs != NULL)
        rdb = XrmGetStringDatabase (xdefs);
    else
    {
        char buff[1048] ;
        meStrcpy(buff,(homedir != NULL) ? homedir:(meUByte *)"./") ;
        meStrcat(buff,".Xdefaults") ;
        rdb = XrmGetFileDatabase(buff);
    }
    if(XrmGetResource(rdb,"MicroEmacs.font","MicroEmacs.Font",&retType,&retVal) &&
       !strcmp(retType,"String"))
        ss = retVal.addr ;
    else
        ss = NULL ;

    /* Load the font into the system */
    if((XTERMsetFont(ss) == meFALSE) &&
       ((ss == NULL) || (XTERMsetFont(NULL) == meFALSE)))
        return meFALSE ;

    /* Set the default geometry, then look for an override */
    ww = 80 ;
    hh = 50 ;
    xx = 0 ;
    yy = 0 ;
    if(XrmGetResource(rdb,"MicroEmacs.geometry","MicroEmacs.Geometry",&retType,&retVal) &&
       !strcmp(retType,"String"))
    {
        int tw, th ;
        sizeHints.flags = USSize | PResizeInc | PMinSize | PBaseSize ;
        if(sscanf(retVal.addr,"%dx%d%d%d",&tw,&th,&xx,&yy) > 2)
            sizeHints.flags |= USPosition ;
        ww = (meUInt) tw ;
        hh = (meUInt) th ;
    }
    else
        sizeHints.flags = PSize | PResizeInc | PMinSize | PBaseSize ;

    if((ww*mecm.fwidth) > ((meUInt) sizeHints.max_width))
        ww = sizeHints.max_width / mecm.fwidth ;
    if((hh*mecm.fdepth) > ((meUInt) sizeHints.max_height))
        hh = sizeHints.max_height / mecm.fdepth ;
    TTdepthDefault = hh ;
    TTwidthDefault = ww ;

    if(xx < 0)
        xx = sizeHints.max_width + xx - (ww * mecm.fwidth) ;
    if(yy < 0)
        yy = sizeHints.max_height + yy - (hh * mecm.fdepth) ; ;
    TTdefaultPosX = xx ;
    TTdefaultPosY = yy ;

    /* Set up the  protocol  defaults  required. We must do this before we map
     * the window. */
    {
        static char* meAtomNames[7] = {
            "WM_DELETE_WINDOW",
            "WM_SAVE_YOURSELF",
            "WM_PROTOCOLS",
            "__COPY_TEXT",
            "INCR",
            "MULTIPLE",
            "TARGETS"
        } ;
        int ii ;
        for(ii=0 ; ii<7 ; ii++)
            meAtoms[ii] = XInternAtom(mecm.xdisplay,meAtomNames[ii], meFALSE);
        meAtoms[ii] = XA_STRING ;
    }

    /* Initialise XDND */
    xdndInitialize (mecm.xdisplay);

    if(XrmGetResource(rdb,"MicroEmacs.name","MicroEmacs.Name",&retType,&retVal) &&
       !strcmp(retType,"String") &&
       ((ss = meStrdup((meUByte *) retVal.addr)) != NULL))
        meName = ss ;

    if(XrmGetResource(rdb,"MicroEmacs.iconname","MicroEmacs.IconName",&retType,&retVal) &&
       !strcmp(retType,"String") && ((ss = meStrdup((meUByte *) retVal.addr)) != NULL))
        meIconName = ss ;

    /* Free off the resource database */
    XrmDestroyDatabase(rdb) ;

    if((firstFrameData = XTERMcreateWindow(TTwidthDefault,TTdepthDefault)) == NULL)
        return meFALSE ;

#ifdef _ME_CONSOLE
#ifdef _TCAP
    /* When the xterm version of ME has to die using meDie (using killing it
     * etc) it cannot write to the X-Window as it may have gone. Therefore the
     * die messages "*** Emergency..." are written to the terminal. This is
     * done by setting the meSystem variable to a value which make it use
     * TCAP. But TCAP has not been initialise and mlwrite will use TCAPmove so
     * we must initialise the TCAPcup (cursor move string) to something to
     * stop it core-dumping.
     */
    tcaptab[TCAPcup].code.str = "" ;
#endif /* _TCAP */
#endif /* _ME_CONSOLE */

    return meTRUE ;
}

/*
 * TThideCur - hide the cursor
 */
void
meFrameXTermHideCursor(meFrame *frame)
{
    if((frame->cursorRow <= frame->depth) && (frame->cursorColumn < frame->width))
    {
        meFrameLine *flp;                     /* Frame store line pointer */
        meUByte     *cc ;                     /* Current cchar  */
        meScheme   schm;                    /* Current colour */

        flp  = frame->store + frame->cursorRow ;
        cc   = flp->text+frame->cursorColumn ;           /* Get char under cursor */
        schm = flp->scheme[frame->cursorColumn] ;        /* Get scheme under cursor */

        meFrameXTermSetScheme(frame,schm) ;
        if ((meSystemCfg & meSYSTEM_FONTFIX) && !((*cc) & 0xe0))
        {
            static char ss[1]={' '} ;
            meFrameXTermDrawString(frame,colToClient(frame->cursorColumn),rowToClient(frame->cursorRow),ss,1);
            meFrameXTermDrawSpecialChar(frame,colToClient(frame->cursorColumn),rowToClientTop(frame->cursorRow),*cc) ;
        }
        else
            meFrameXTermDrawString(frame,colToClient(frame->cursorColumn),rowToClient(frame->cursorRow),cc,1);
    }
}

/*
 * TTshowCur - show the cursor
 */
void
meFrameXTermShowCursor(meFrame *frame)
{
    if((frame->cursorRow <= frame->depth) && (frame->cursorColumn < frame->width))
    {
        meFrameLine *flp;                     /* Frame store line pointer */
        meUByte     *cc ;                     /* Current cchar  */
        meScheme   schm;                    /* Current colour */

        flp  = frame->store + frame->cursorRow ;
        cc   = flp->text+frame->cursorColumn ;           /* Get char under cursor */
        schm = flp->scheme[frame->cursorColumn] ;        /* Get scheme under cursor */

        if(!(frame->flags & meFRAME_NOT_FOCUS))
        {
            meUInt valueMask=0 ;
            meUByte ff ;
            ff = meStyleGetBColor(meSchemeGetStyle(schm)) ;
            if(meFrameGetXGCFCol(frame) != ff)
            {
                meFrameSetXGCFCol(frame,ff) ;
                meFrameGetXGCValues(frame).foreground = colTable[ff] ;
                valueMask |= GCForeground ;
            }
            if(meFrameGetXGCBCol(frame) != cursorColor)
            {
                meFrameSetXGCBCol(frame,cursorColor) ;
                meFrameGetXGCValues(frame).background = colTable[cursorColor] ;
                valueMask |= GCBackground ;
            }
            if(mecm.fontName != NULL)
            {
                ff = meStyleGetFont(meSchemeGetStyle(schm)) ;
                if(meSchemeTestNoFont(schm))
                    ff &= ~(meFONT_BOLD|meFONT_ITALIC|meFONT_UNDERLINE) ;
                if(meFrameGetXGCFont(frame) != ff)
                {
                    meFrameSetXGCFont(frame,ff) ;
                    meFrameSetXGCFontId(frame,XTERMfontGetId(ff)) ;
                    if(meFrameGetXGCValues(frame).font != meFrameGetXGCFontId(frame))
                    {
                        meFrameGetXGCValues(frame).font = meFrameGetXGCFontId(frame) ;
                        valueMask |= GCFont ;
                    }
                }
            }
            if(valueMask)
                XChangeGC(mecm.xdisplay,meFrameGetXGC(frame),valueMask,&meFrameGetXGCValues(frame)) ;
            if ((meSystemCfg & meSYSTEM_FONTFIX) && !((*cc) & 0xe0))
            {
                static char ss[1]={' '} ;
                meFrameXTermDrawString(frame,colToClient(frame->cursorColumn),rowToClient(frame->cursorRow),ss,1);
                meFrameXTermDrawSpecialChar(frame,colToClient(frame->cursorColumn),rowToClientTop(frame->cursorRow),*cc) ;
            }
            else
                meFrameXTermDrawString(frame,colToClient(frame->cursorColumn),rowToClient(frame->cursorRow),cc,1);
        }
        else
        {
            if(meFrameGetXGCFCol(frame) != cursorColor)
            {
                meFrameSetXGCFCol(frame,cursorColor) ;
                meFrameGetXGCValues(frame).foreground = colTable[cursorColor] ;
                XChangeGC(mecm.xdisplay,meFrameGetXGC(frame),GCForeground,&meFrameGetXGCValues(frame)) ;
            }
            XDrawRectangle(mecm.xdisplay,meFrameGetXWindow(frame),meFrameGetXGC(frame),
                           colToClient(frame->cursorColumn),
                           rowToClientTop(frame->cursorRow),mecm.fwidth-1,mecm.fdepth-1) ;
        }
    }
}

int
changeFont(int f, int n)
{
    meUByte        buff[meBUF_SIZE_MAX] ;            /* Input buffer */

    if(meSystemCfg & meSYSTEM_CONSOLE)
        /* change-font not supported on termcap */
        return notAvailable(f,n) ;

    /* Get the name of the font. If it is specified as default then
     * do not collect the remaining arguments */
    if(meGetString((meUByte *)"Font Name", 0, 0, buff, meBUF_SIZE_MAX) == meABORT)
        return meFALSE ;

    /* Change the font */
    if(XTERMsetFont ((char *)buff) == meFALSE)
        return meFALSE ;

    /* Set up the arguments for a resize operation. Because the
     * font has changed then we need to define the new window
     * base size, minimum size and increment size.
     */
    meFrameLoopBegin() ;

    meFrameLoopContinue(loopFrame->flags & meFRAME_HIDDEN) ;

    meFrameSetWindowSize(loopFrame) ;

    /* Make the current font invalid and force a complete redraw */
    meFrameSetXGCFont(loopFrame,0) ;
    meFrameSetXGCFontId(loopFrame,mecm.fontId) ;
    meFrameGetXGCValues(loopFrame).font = mecm.fontId ;
    XChangeGC(mecm.xdisplay,meFrameGetXGC(loopFrame),GCFont,&meFrameGetXGCValues(loopFrame)) ;

    /* In order for us to change the hints for a font change then the
     * X-Window has to be un-mapped. Therefore on a font change unmap the
     * window, the hints size is then changed when we receive the MapNotify
     * event and the X-Window may be re-mapped. Unmap the window ready to
     * change the size hints for the font. Note that the hint values are the
     * same irrespective of the number of windows. */
    if (meFrameGetXMapState(loopFrame) == meXMAP_MAP)
    {
        /* Tell the Unmap event that this is a font change mapping and then
         * unmap the window. */
        meFrameSetXMapState(loopFrame, meXMAP_FONT);
        XUnmapWindow (mecm.xdisplay, meFrameGetXWindow(loopFrame));
    }
    
    meFrameLoopEnd() ;

    sgarbf = meTRUE;
    
    return meTRUE ;
}

int
XTERMaddColor(meColor index, meUByte r, meUByte g, meUByte b)
{
    XColor col ;

    if(noColors <= index)
    {
        colTable = meRealloc(colTable,(index+1)*sizeof(meUInt)) ;
        memset(colTable+noColors,0,(index-noColors+1)*sizeof(meUInt)) ;
        noColors = index+1 ;
    }
    col.red   = ((meUShort) r) << 8 ;
    col.green = ((meUShort) g) << 8 ;
    col.blue  = ((meUShort) b) << 8 ;
    if(!XAllocColor(mecm.xdisplay,xcmap,&col))
    {
        static int noCells=-1 ;
        static meUByte *cells=NULL ;
        meUByte *ss ;
        int ii, diff;
        int bDiff= 256*256*3 ;          /* Smallest least squares. */

        if(noCells < 0)
        {
            /* printf("Warning: Failed to allocate colors, looking up closest\n") ;*/
            if(((noCells = DisplayCells(mecm.xdisplay,xscreen)) > 0) &&
               ((cells = meMalloc(noCells*3*sizeof(meUByte))) != NULL))
            {
                ii = noCells ;
                ss = cells ;
                for(ii=0 ; ii<noCells ; ii++)
                {
                    col.pixel = ii ;
                    XQueryColor(mecm.xdisplay,xcmap,&col) ;
                    *ss++ = (col.red   >> 8) ;
                    *ss++ = (col.green >> 8) ;
                    *ss++ = (col.blue  >> 8) ;
                }
            }
        }
        col.pixel = BlackPixel(mecm.xdisplay,xscreen) ;
        /* To find the nearest color then use a least squares method. This
         * produces a better approximation than a straight forward color
         * differencing algorithm as it takes into account the variance. */
        if(cells != NULL)
        {
            ii = noCells ;
            ss = cells ;
            for(ii=0 ; ii<noCells ; ii++)
            {
                int delta;

                delta = ((int) *ss++) - ((int) r) ;
                diff  = (delta * delta) ;
                delta = ((int) *ss++) - ((int) g) ;
                diff += (delta * delta) ;
                delta = ((int) *ss++) - ((int) b) ;
                diff += (delta * delta) ;
                if(diff < bDiff)
                {
                    bDiff = diff ;
                    col.pixel = ii ;
                }
            }
        }
        XQueryColor(mecm.xdisplay,xcmap,&col) ;
        if(!XAllocColor(mecm.xdisplay,xcmap,&col))
            printf("Warning: Failed to allocate closest color\n") ;

    }
    colTable[index] = col.pixel ;

    /* the default colors are created before the first frame is so check there is a frame */
    if(frameCur != NULL)
    {
        meFrameLoopBegin() ;

        meFrameLoopContinue(loopFrame->flags & meFRAME_HIDDEN) ;

        if(meFrameGetXGCFCol(loopFrame) == index)
            meFrameSetXGCFCol(loopFrame,meCOLOR_INVALID) ;
        if(meFrameGetXGCBCol(loopFrame) == index)
            meFrameSetXGCBCol(loopFrame,meCOLOR_INVALID) ;

        meFrameLoopEnd() ;
    }
    return meTRUE ;
}

void
XTERMsetBgcol(void)
{
    meFrameLoopBegin() ;

    meFrameLoopContinue(loopFrame->flags & meFRAME_HIDDEN) ;

    /* change the background */
    XSetWindowBackground(mecm.xdisplay,meFrameGetXWindow(loopFrame),colTable[meStyleGetBColor(meSchemeGetStyle(globScheme))]);

    meFrameLoopEnd() ;
}

/*
 * meFrameSetWindowSize
 *
 * Resize the width & depth of the frame window. If the resize has been
 * disabled (within the configureNotify event) then do not modify the window
 * size - this operation is performed elsewhere.
 */
void
meFrameSetWindowSize(meFrame *frame)
{
    if(
#ifdef _ME_CONSOLE
       !(meSystemCfg & meSYSTEM_CONSOLE) &&
#endif /* _ME_CONSOLE */
       (disableResize == 0))
    {
        sizeHints.height = mecm.fdepth*(frame->depth+1) ;
        sizeHints.width  = mecm.fwidth*frame->width ;
        /*printf("meFrameSetWindowSize::" __FILE__ ":%d: XResizeWindow\n", __LINE__);*/
        XResizeWindow(mecm.xdisplay,meFrameGetXWindow(frame),sizeHints.width,sizeHints.height) ;
    }
}

#if MEOPT_EXTENDED
void
meFrameRepositionWindow(meFrame *frame, int resize)
{
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
    {
        int xx, yy, wbs, tbs ;
        unsigned int ww, hh ;
        XWindowAttributes xwa;
        Status status;
        
        /* Getting the real window border and title-bar size is a little
         * tricky - guess for now. Also there is a little confusion and
         * disagreement on what an XMoveWindow to x,y should do, move the WM
         * frame to x,y or the window to x,y (frame to x-fx,y-fy). Judging by
         * the ICCCM the window should be moved to x,y (i.e. same as if there
         * is no WM) so this assumes this behaviour.
         * 
         * Use the XGetWindowAttributes to retrive the window offset position,
         * this gives us the information that we need for the z and y offset
         * relative to the parent.
         * 
         * Jon - 2009-09-26. This section of code used to break Linux because the 
         * the sizes were being made up and the wxa attributes returned are 
         * actually the inner window which meant that a spurious size is returned.
         * We cannot find out the X-Window boarder and top bar so simply
         * use the values that are supplied.
         */
        status = XGetWindowAttributes (mecm.xdisplay,
                                       meFrameGetXWindow(frame),
                                       &xwa);
        /* if (status == 0)*/
        /* { */
        /* printf ("Status = %d\n", status);*/
        /* printf ("meWindowSize[%s (%d,%d or %d,%d)]\n", s, x, y, x/mecm.fwidth, y/mecm.fdepth);*/
        /* printf ("  Location of window %d, %d. Char width %d %d\n", xwa.x, xwa.y, mecm.fwidth, mecm.fdepth);*/
        /* printf ("  Width and height of window %d, %d [%d,%d]\n", xwa.width, xwa.height, xwa.width/mecm.fwidth, xwa.height/mecm.fdepth);*/
        /* printf ("  Border width of window %d\n", xwa.border_width);*/
        /* }*/
        /* printf ("Sizehints x,y = %d,%d\n", sizeHints.x, sizeHints.y);*/
        /* printf ("Sizehints width,height = %d,%d\n", sizeHints.width, sizeHints.height);*/
        
        /* Get the border information */
        if (status == 0)
        {
            /* Assign the current border width. */
            wbs = xwa.border_width; /* Border width */
            tbs = xwa.border_width; /* Title bar - we don't know! */
        }
        else
        {
            /* Assume some sensible defaults in the absence of anything else */
            wbs = 3;
            tbs = 3;
        }
        
        /* Get the size information */
        /* TODO: The sizeHints is horribly overloaded and needs to be moved into meFrame */
        xx = sizeHints.x ;
        yy = sizeHints.y ;
        ww = sizeHints.width ;
        hh = sizeHints.height ;
        
        if((xx + ww) > sizeHints.max_width)
            xx = sizeHints.max_width - ww;
        if(xx < 0)
            xx = 0 ;
        
        if((yy + hh) > sizeHints.max_height)
            yy = sizeHints.max_height - hh;
        if(yy < 0)
            yy = 0 ;
        
        if(resize && (((xx + ww + wbs + wbs) > sizeHints.max_width) || 
                      ((yy + hh + tbs + wbs) > sizeHints.max_height)))
        {
            sizeHints.x = xx ;
            sizeHints.y = yy ;
            if((xx + ww + wbs + wbs) > sizeHints.max_width)
                ww = sizeHints.max_width - xx - wbs - wbs;
            if((yy + hh + tbs + wbs) > sizeHints.max_height)
                hh = sizeHints.max_height - yy - wbs - tbs;
            /* printf("meFrameRepositionWindow::" __FILE__ ":%d: XMoveResizeWindow\n", __LINE__);*/
            XMoveResizeWindow(mecm.xdisplay,meFrameGetXWindow(frame),
                              sizeHints.x,sizeHints.y,ww,hh) ;
        }
        else if((xx != sizeHints.x) || (yy != sizeHints.y))
        {
            sizeHints.x = xx ;
            sizeHints.y = yy ;
            /* printf("meFrameRepositionWindow::" __FILE__ ":%d: XMoveWindow\n", __LINE__);*/
            XMoveWindow(mecm.xdisplay,meFrameGetXWindow(frame),
                        sizeHints.x-wbs,sizeHints.y-tbs) ;
        }
    }
#endif /* _ME_WINDOW */
}
#endif

/* meFrameSetWindowTitle
 *
 * Put the name of the buffer into the window frame
 */
void
meFrameSetWindowTitle(meFrame *frame, meUByte *str)
{
#ifdef _ME_CONSOLE
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
    {
        char buf[meBUF_SIZE_MAX], *ss ;

#if MEOPT_EXTENDED
        if(frameTitle != NULL)
            meStrcpy(buf,frameTitle) ;
        else
#endif
            meStrcpy(buf,meName);
        if(str != NULL)
        {
            meStrcat(buf,": ") ;
            meStrcat(buf,str) ;
        }
        ss = buf ;
        XStoreName(mecm.xdisplay,meFrameGetXWindow(frame),ss);
    }
}

#ifdef _CLIPBRD
void
TTsetClipboard(void)
{
    if(!(meSystemCfg & (meSYSTEM_CONSOLE|meSYSTEM_NOCLIPBRD)) &&
       !(clipState & (CLIP_OWNER|CLIP_RECEIVING|CLIP_DISABLED)) && (kbdmode != mePLAY))
    {
        XSetSelectionOwner(mecm.xdisplay,XA_PRIMARY,meFrameGetXWindow(frameCur),CurrentTime) ;
        if(XGetSelectionOwner(mecm.xdisplay,XA_PRIMARY) == meFrameGetXWindow(frameCur))
            clipState |= CLIP_OWNER ;
    }
}

void
TTgetClipboard(void)
{
    if(!(meSystemCfg & (meSYSTEM_CONSOLE|meSYSTEM_NOCLIPBRD)) &&
       !(clipState & (CLIP_OWNER|CLIP_DISABLED)) && (kbdmode != mePLAY))
    {
        /* Add the CLIP_RECEIVING flag. This is really important if increment
         * copy texts are being used. If they are and this flag is set after receiving
         * the initial size, the killSave then take ownership of the block and so we never
         * get the copy text. Take ownership at the end */
        clipState &= ~CLIP_RECEIVED ;
        clipState |= CLIP_RECEIVING ;
        /* Request for the current Primary string owner to send a
         * SelectionNotify event to us giving the current string
         */
        XConvertSelection(mecm.xdisplay,XA_PRIMARY,XA_STRING,meAtoms[meATOM_COPY_TEXT],
                          meFrameGetXWindow(frameCur),CurrentTime) ;
        /* Must do a flush to ensure the request has gone */
        XFlush(mecm.xdisplay) ;
        /* Wait for the returned value, alarmState bit will be set */
        while(!TTahead() && !(clipState & CLIP_RECEIVED))
            waitForEvent(0) ;
        clipState &= ~(CLIP_RECEIVING|CLIP_RECEIVED) ;
        /* reset the increment clip size to zero (just incase there was an interuption) */
        meClipSize=0 ;
        TTsetClipboard() ;
    }
}
#endif

#endif /* _XTERM */

#ifdef _ME_CONSOLE
/* Special TTstart when both console and window are enabled, Its purpose is to
 * work out where to call XTERMstart or TCAPstart
 */
int
TTstart(void)
{
    if((meSystemCfg & meSYSTEM_CONSOLE) || (meGetenv("DISPLAY") == NULL))
        return TCAPstart() ;
    return XTERMstart() ;
}
#endif /* _ME_CONSOLE */
#endif /* _ME_WINDOW */

void
TTwaitForChar(void)
{
#if MEOPT_MOUSE
    /* If no keys left then if theres currently no mouse timer and
     * theres a button press (No mouse-time key) then check for a
     * time-mouse-? key, if found add a timer start a mouse checking
     */
    if(!isTimerSet(MOUSE_TIMER_ID) && (mouseState != 0))
    {
        meUShort mc ;
        meUInt arg ;
        mc = ME_SPECIAL | mouseKeyState | (SKEY_mouse_time+mouseKeys[mouseButtonGetPick()]) ;
        /* mouse-time bound ?? */
        if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
        {
            /* Start a timer and move to timed state 1 */
            /* Start a new timer to clock in at 'delay' intervals */
            /* printf("Setting mouse timer for delay %d\n",delayTime) ;*/
            timerSet(MOUSE_TIMER_ID,-1,delayTime);
        }
    }
#endif
#if MEOPT_CALLBACK
    /* IDLE TIME: Check the idle time events */
    if(kbdmode == meIDLE)
        /* Check the idle event */
        doIdlePickEvent() ;
#endif
    for(;;)
    {
#ifdef _ME_WINDOW
#ifdef _XTERM
#if (defined _LINUX_BASE) || (defined _CYGWIN)
#ifdef _ME_CONSOLE
        if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
        {
            /* Make a call to XSync(). This ensures that the X-Server has
             * caught up with the client (us !!). On operations such as
             * scrolling etc. then the X-Server tends to lag behind giving
             * a very 'rubbery' feel to the scroll bars. This reduces that
             * effect, allowing the server to process all of our outstanding
             * events (typically mecm.xdisplay requests). */
            XSync (mecm.xdisplay, meFALSE);
        }
#endif
#endif /* _XTERM */
#endif /* _ME_WINDOW */

        /* Must only do one at a time else if you get A key while autosaving
         * the key can be proccessed by TTahead and then you sit and wait
         * in TCAPwait
         */
        handleTimerExpired() ;
        if(TTahead())
            break ;
        if(sgarbf == meTRUE)
        {
            update(meFALSE) ;
            mlerase(MWCLEXEC) ;
        }
#if MEOPT_MWFRAME
        /* if the user has changed the window focus using the OS
         * but ME can swap to this frame because there is an active frame
         * then give a warning */
        if((frameFocus != NULL) && (frameFocus != frameCur))
        {
            meUByte scheme=(globScheme/meSCHEME_STYLES) ;
            meFrame *fc=frameCur ;
            frameCur = frameFocus ;
            pokeScreen(0x10,frameCur->depth,(frameCur->width >> 1)-5,&scheme,
                       (meUByte *) "[NOT FOCUS]") ;
            frameCur = fc ;
        }
#endif
        waitForEvent(0) ;
    }
}

void
TTsleep(int  msec, int  intable, meVarList *waitVarList)
{
    meUByte *ss ;
    int sgarbfOld ;

    if(intable && ((kbdmode == mePLAY) || (clexec == meTRUE)))
        return ;

    if(msec >= 0)
        /* Don't actually need the abs time as this will remain the next alarm */
        timerSet(SLEEP_TIMER_ID,-1,msec);
    else if(waitVarList != NULL)
        timerKill(SLEEP_TIMER_ID) ;             /* Kill off the timer */
    else
        return ;

    sgarbfOld = sgarbf ;
    sgarbf = meFALSE ;
    do
    {
        handleTimerExpired() ;
        if(intable && TTahead())
            break ;
        if((waitVarList != NULL) &&
           (((ss=getUsrLclCmdVar((meUByte *)"wait",waitVarList)) == errorm) || !meAtoi(ss)))
            break ;
        waitForEvent(1) ;
    } while(!isTimerExpired(SLEEP_TIMER_ID)) ;
    timerKill(SLEEP_TIMER_ID) ;             /* Kill off the timer */
    sgarbf = sgarbfOld ;
}

#if MEOPT_TYPEAH
int
TTahead(void)
{
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
    {
#ifdef _XTERM
        int ii ;
        while((ii=XPending(mecm.xdisplay)))
        {
            do
                meXEventHandler() ;
            while(--ii > 0) ;
        }
        /* don't process the timers if we have a key waiting!
         * This is because the timers can generate a lot of timer
         * keys, filling up the input buffer - these are not wanted.
         * By not processing, we leave them there until we need them.
         */
        if(TTnoKeys)
            return TTnoKeys ;

#if MEOPT_MOUSE
        /* If an alarm hCheck the mouse */
        if(isTimerExpired(MOUSE_TIMER_ID))
        {
            meUShort mc ;
            meUInt arg ;

            timerClearExpired(MOUSE_TIMER_ID) ;
            mc = ME_SPECIAL | mouseKeyState |
                      (SKEY_mouse_time+mouseKeys[mouseButtonGetPick()]) ;
            /* mouse-time bound ?? */
            if((!TTallKeys && (decode_key(mc,&arg) != -1)) || (TTallKeys & 0x2))
            {
                /* Timer has expired and timer still bound. Report the key. */
                /* Push the generated keycode into the buffer */
                addKeyToBufferOnce(mc) ;
                /* Set the new timer and state */
                /* Start a new timer to clock in at 'repeat' intervals */
                /* printf("Setting mouse timer for repeat %d\n",repeatTime) ;*/
                timerSet(MOUSE_TIMER_ID,-1,repeatTime);
            }
        }
#endif /* MEOPT_MOUSE */
#endif /* _XTERM */
    }
#ifdef _ME_CONSOLE
    else
#endif /* _ME_CONSOLE */
#endif /* _ME_WINDOW */
#ifdef _ME_CONSOLE
    {
#ifdef _USEPOLL
        struct pollfd pfds [1];

        /* Set up the poll structure */
        pfds[0].fd = meStdin;
        pfds[0].events = POLLIN;
#endif
        /* Pasting in termcap results in lots of keys all at once rather than
         * lose them, keep them there until there enough room in the input key
         * buffer to store them. */
        while(TTnoKeys != KEYBUFSIZ)
        {
            meUByte cc ;
#ifdef _USEPOLL
            int status;                 /* Status of the input check */

            status = poll (pfds, 1, 0);
            if ((status != 1) || ((pfds[0].revents & (POLLIN|POLLPRI)) == 0))
                break;                  /* No data pending */
#else
#ifdef FIONREAD
            int status;                 /* Status of the input check */

            if (ioctl(meStdin,FIONREAD,&status) < 0)
                break;                  /* ioctl failed */
            if (status <= 0)
                break;                  /* No data pending */
#else
            if (ioctl(meStdin, FIORDCHK,0) <= 0)
                break;                  /* No data pending */
#endif /* FIONREAD */
#endif /* _USEPOLL */
            /* There is some data present. Read it */
            if(read(meStdin,&cc,1) > 0)
            {
                /* C-Space is returned as "\x00" (or ^@) or nul. This is a
                 * horrible character to translate so we do it here before we
                 * enter the system. There is not an easy way to add this
                 * translation. */
                if (cc == '\0')
                    addKeyToBuffer (ME_CONTROL|' ');
                else
                    addKeyToBuffer(cc) ;
            }
        }

        if(alarmState & meALARM_WINSIZE)
        {
            frameChangeWidth(meTRUE,TTnewWid-frameCur->width);     /* Change width */
            frameChangeDepth(meTRUE,TTnewHig-(frameCur->depth+1)); /* Change depth */
            alarmState &= ~meALARM_WINSIZE ;
        }
        if(TTnoKeys)
            return TTnoKeys ;
    }
#endif /* _ME_CONSOLE */
#if MEOPT_CALLBACK
    if(isTimerExpired(IDLE_TIMER_ID))
    {
        register int index;
        meUInt arg;           /* Decode key argument */
        if((index=decode_key(ME_SPECIAL|SKEY_idle_time,&arg)) != -1)
        {
            /* Execute the idle-time key */
            execFuncHidden(ME_SPECIAL|SKEY_idle_time,index,arg) ;

            /* Now set the timer for the next */
            timerSet(IDLE_TIMER_ID,-1,idleTime);
        }
        else if(decode_key(ME_SPECIAL|SKEY_idle_drop,&arg) != -1)
            meTimerState[IDLE_TIMER_ID] = IDLE_STATE_DROP ;
        else
            meTimerState[IDLE_TIMER_ID] = 0 ;
    }
#endif
    return TTnoKeys ;
}
#endif /* MEOPT_TYPEAH */

#if MEOPT_CLIENTSERVER

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>

int meCSSock=-1 ;
int meCCSSock=-1 ;
void
TTopenClientServer (void)
{
    /* If the server has not been created then create it now */
    if(meCSSock == -1)
    {
        struct sockaddr_un cssa ;
        meIPipe *ipipe ;
        meBuffer *bp ;
        meMode sglobMode ;
        int ii ;

        if((meCSSock=socket(AF_UNIX,SOCK_STREAM, 0)) < 0)
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            return ;
        }
        ii = sprintf(cssa.sun_path,"/tmp/mesrv%d",(int) meUid);
        ii += sizeof(cssa.sun_family);
        cssa.sun_family = AF_UNIX;

        if(!meTestExist(cssa.sun_path) &&
           (connect(meCSSock,(struct sockaddr *)&cssa,ii) >= 0))
        {
            /* theres another me acting as a server, quit! */
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            close(meCSSock) ;
            meCSSock = -1 ;
            return ;
        }
        meUnlink(cssa.sun_path) ;
        if((bind(meCSSock,(struct sockaddr *)&cssa,ii) < 0) ||
           (listen(meCSSock,20) < 0))
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            close(meCSSock) ;
            meCSSock = -1 ;
            return ;
        }
        /* Change socket permissions so only the user can send commands */
        meFileSetAttributes((char *) cssa.sun_path,0700);
        /* Create the ipipe buffer */
        meModeCopy(sglobMode,globMode) ;
        meModeSet(globMode,MDPIPE) ;
        meModeSet(globMode,MDLOCK) ;
        meModeSet(globMode,MDHIDE) ;
        meModeClear(globMode,MDWRAP) ;
        meModeClear(globMode,MDUNDO) ;
        if(((bp=bfind(BserverN,BFND_CREAT)) == NULL) ||
           ((ipipe = meMalloc(sizeof(meIPipe))) == NULL))
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
            close(meCSSock) ;
            meCSSock = -1 ;
            return ;
        }
        meModeCopy(globMode,sglobMode) ;
        bp->intFlag |= BIFNODEL ;
        ipipe->next = ipipes ;
        ipipe->pid = 0 ;
        ipipe->rfd = meCSSock ;
        ipipe->outWfd = meCSSock ;
        ipipes = ipipe ;
        noIpipes++ ;
        ipipe->bp = bp ;
        /* setup the buffer */
        {
            meUByte buff[meBUF_SIZE_MAX] ;
            sprintf((char *)buff,"Client Server: /tmp/mesrv%d\n\n",(int) meUid) ;
            addLineToEob(bp,buff) ;              /* Add string */
            bp->dotLine = meLineGetPrev(bp->baseLine) ;
            bp->dotOffset = 0 ;
            bp->dotLineNo = bp->lineCount-1 ;
            meAnchorSet(bp,'I',bp->dotLine,bp->dotOffset,1) ;
        }
        /* Set up the window dimensions - default to having auto wrap */
        ipipe->flag = 0 ;
        ipipe->strRow = 0 ;
        ipipe->strCol = 0 ;
        ipipe->noRows = 0 ;
        ipipe->noCols = 0 ;
        ipipe->curRow = 0 ;
        ipipe->curRow = bp->dotLineNo ;
        /* get a popup window for the command output */
        ipipeSetSize(frameCur->windowCur,bp) ;
    }
}

void
TTkillClientServer(void)
{
    /* Close & delete the client file */
    if(meCSSock != -1)
    {
        meIPipe *ipipe ;
        char fname[meBUF_SIZE_MAX] ;

        /* get the ipipe node */
        ipipe = ipipes ;
        while(ipipe != NULL)
        {
            if(ipipe->pid == 0)
                break ;
            ipipe = ipipe->next ;
        }
        if(ipipe != NULL)
        {
            ipipe->bp->intFlag |= BIFBLOW ;
            zotbuf(ipipe->bp,1) ;
        }
        else
            close(meCSSock) ;
        sprintf(fname,"/tmp/mesrv%d",(int) meUid) ;
        meUnlink(fname) ;
        meSystemCfg &= ~meSYSTEM_CLNTSRVR ;
        meCSSock = -1 ;
    }
    if(meCCSSock != -1)
    {
        close(meCSSock) ;
        meCCSSock = -1 ;
    }
}

int
TTconnectClientServer (void)
{
    /* If the server has not been connected then create it now */
    if(meCCSSock == -1)
    {
        struct sockaddr_un cssa ;
        int ii ;

        if((meCCSSock=socket(AF_UNIX,SOCK_STREAM, 0)) < 0)
            return 0 ;

        ii = sizeof(cssa.sun_family);
        ii += sprintf(cssa.sun_path,"/tmp/mesrv%d",(int) meUid);
        cssa.sun_family = AF_UNIX;

        if(connect(meCCSSock,(struct sockaddr *)&cssa,ii) < 0)
        {
            close(meCCSSock) ;
            meCCSSock = -1 ;
            return 0 ;
        }
    }
    return 1 ;
}
void
TTsendClientServer(meUByte *line)
{
    if(meCCSSock >= 0)
    {
        int ii, ll=meStrlen(line) ;
        while(ll)
        {
            if((ii=write(meCCSSock,line,ll)) < 0)
                return ;
            ll -= ii ;
            line += ii ;
        }
    }
}
#endif /* _CLIENTSERVER */

/**************************************************************************
* MISCELEANEOUS FUNCTIONS                                                 *
**************************************************************************/
#ifdef _NOSTRDUP
/* strdup; String duplication is not supported on some earlier platforms
 * provide one if it does not exits. Typically absent on earlier BSD4.2
 * platforms. */
char *
strdup (const char *s)
{
    char *p;

    if ((p = (char *) meMalloc ((meStrlen (s)) + 1)) != NULL)
        meStrcpy (p, s);
    return (p);
}

#endif

#ifdef _NOPUTENV
int
putenv (const char *string)
{
    char **p;                           /* Temp environment ptr */
    int len;                            /* Length of the string */
    int ii;                             /* Loop counter */

    /* Test the string for a valid value */
    if ((string == NULL) || (*string == '\0'))
        return -1;

    /* If the local environment space has not been set up then do so now */
    if (meEnviron == NULL)
    {
        int jj;

        /* Count the number of entries */
        for (ii = 0, p = environ; *p != NULL; p++)
            ii++;

        /* Allocate space */
        if ((meEnviron = meMalloc (sizeof (char *) * (ii + 1))) == NULL)
            return -1;

        /* Initialise to NULL */
        jj = ii;
        do
            meEnviron[jj] = NULL;
        while (--jj >= 0);

        /* Copy across the environment */
        for (jj = 0; jj < ii; jj++)
        {
            if ((meEnviron[jj] = meStrdup(environ[jj])) == NULL)
            {
                while (--jj > 0)
                    meFree (meEnviron[jj]);
                meFree (meEnviron);
                return -1;
            }
        }
    }

    /* Now try and find an existing entry with the same name */
    for (len = 0; string[len] != '\0'; len++)
        if (string [len] == '=')
            break;

    /* Test for valid putenv value */
    if (string[len] == '\0')
        return -1;                      /* Invalid string */

    /* Try and locate the environment value in the existing environment */
    for (ii = 0, p = meEnviron; (*p != NULL); ii++, p++)
    {
        if ((meStrnicmp (*p, string, len) == 0) && ((*p)[len] == '='))
        {
            /* Same entry. push the new value */
            meFree (*p);
            *p = string;
            return (0);
        }
    }

    /* The variable value does not exist in the environment, make a new entry.
     * First re-size the environment. */
    if ((p = meRealloc (meEnviron, (ii+2) * sizeof (char *))) == NULL)
        return -1;

    /* Inherit the new environment and add the new variable. */
    meEnviron = p;
    meEnviron [ii] = string;
    meEnviron [ii+1] = NULL;
    return 0;
}

char *
megetenv (const char *string)
{
    if (meEnviron != NULL)
    {
        char **p;
        int len;

        /* Test the string for a valid value */
        if ((string == NULL) || (*string == '\0'))
            return NULL;

        /* Get the length of the string for the search */
        len = strlen (string);
        if ((p = meEnviron) != NULL)
        {
            while (*p != NULL)
            {
                if((meStrnicmp (*p, string, len) == 0) && ((*p)[len] == '='))
                    return (&((*p)[len+1]));
                p++;
            }
        }
        /* Not found */
        return NULL;
    }
    return (getenv(string));
}
#endif

/***************************************************************************
 * XDND Functions                                                          *
 *                                                                         *
 * XDND defines the X-Windows drag 'n' drop extensions and is based        *
 * heavily on the work done by Paul Sheer especially files xdnd.h and      *
 * xdnd.c which have been picked over and reduced to the minimum to        *
 * support the xdnd drop protocol.                                         *
 *                                                                         *
 * The coupling to unixterm.c has been minimised as much as possible to    *
 * reduce the complexity of the code and allow the xdnd code to be         *
 * conditionally included.                                                 *
 ***************************************************************************/

#if (defined _DRAGNDROP) && (defined _XTERM)

/* you can set this to either 2 (which support 0 and 1 as well) or 3 */
/*#define XDND_VERSION 2*/
#define XDND_VERSION 3

/* XdndEnter */
#define XDND_THREE 3
#define XDND_ENTER_SOURCE_WIN(e)	((e)->xclient.data.l[0])
#define XDND_ENTER_THREE_TYPES(e)	(((e)->xclient.data.l[1] & 0x1UL) == 0)
#define XDND_ENTER_THREE_TYPES_SET(e,b)	(e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x1UL) | (((b) == 0) ? 0 : 0x1UL)
#define XDND_ENTER_VERSION(e)		((e)->xclient.data.l[1] >> 24)
#define XDND_ENTER_VERSION_SET(e,v)	(e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~(0xFF << 24)) | ((v) << 24)
#define XDND_ENTER_TYPE(e,i)		((e)->xclient.data.l[2 + i])	/* i => (0, 1, 2) */

/* XdndPosition */
#define XDND_POSITION_SOURCE_WIN(e)	((e)->xclient.data.l[0])
#define XDND_POSITION_ROOT_X(e)		((e)->xclient.data.l[2] >> 16)
#define XDND_POSITION_ROOT_Y(e)		((e)->xclient.data.l[2] & 0xFFFFUL)
#define XDND_POSITION_ROOT_SET(e,x,y)	(e)->xclient.data.l[2]  = ((x) << 16) | ((y) & 0xFFFFUL)
#define XDND_POSITION_TIME(e)		((e)->xclient.data.l[3])
#define XDND_POSITION_ACTION(e)		((e)->xclient.data.l[4])

/* XdndStatus */
#define XDND_STATUS_TARGET_WIN(e)	((e)->xclient.data.l[0])
#define XDND_STATUS_WILL_ACCEPT(e)	((e)->xclient.data.l[1] & 0x1L)
#define XDND_STATUS_WILL_ACCEPT_SET(e,b) (e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x1UL) | (((b) == 0) ? 0 : 0x1UL)
#define XDND_STATUS_WANT_POSITION(e)	((e)->xclient.data.l[1] & 0x2UL)
#define XDND_STATUS_WANT_POSITION_SET(e,b) (e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x2UL) | (((b) == 0) ? 0 : 0x2UL)
#define XDND_STATUS_RECT_X(e)		((e)->xclient.data.l[2] >> 16)
#define XDND_STATUS_RECT_Y(e)		((e)->xclient.data.l[2] & 0xFFFFL)
#define XDND_STATUS_RECT_WIDTH(e)	((e)->xclient.data.l[3] >> 16)
#define XDND_STATUS_RECT_HEIGHT(e)	((e)->xclient.data.l[3] & 0xFFFFL)
#define XDND_STATUS_RECT_SET(e,x,y,w,h)	{(e)->xclient.data.l[2] = ((x) << 16) | ((y) & 0xFFFFUL); (e)->xclient.data.l[3] = ((w) << 16) | ((h) & 0xFFFFUL); }
#define XDND_STATUS_ACTION(e)		((e)->xclient.data.l[4])

/* XdndLeave */
#define XDND_LEAVE_SOURCE_WIN(e)	((e)->xclient.data.l[0])

/* XdndDrop */
#define XDND_DROP_SOURCE_WIN(e)		((e)->xclient.data.l[0])
#define XDND_DROP_TIME(e)		((e)->xclient.data.l[2])

/* XdndFinished */
#define XDND_FINISHED_TARGET_WIN(e)	((e)->xclient.data.l[0])

/* Debugging */
/*#define DEBUGOUT(x) (printf x, fflush (stdout))*/
/*#define XWARNING(x) (printf x, fflush (stdout))*/
#define DEBUGOUT(x)
#define XWARNING(x)

typedef struct _DndClass DndClass;
struct _DndClass
{
    /**< The X-Windows display. */
    Display *display;

    /**< The atom identities. */
    Atom XdndAware;
    Atom XdndSelection;
    Atom XdndEnter;
    Atom XdndLeave;
    Atom XdndPosition;
    Atom XdndDrop;
    Atom XdndFinished;
    Atom XdndStatus;
    Atom XdndActionCopy;
    /* Atom XdndActionMove;*/
    /* Atom XdndActionLink;*/
    /* Atom XdndActionAsk;*/
    /* Atom XdndActionPrivate;*/
    /* Atom XdndTypeList;*/
    /* Atom XdndActionList;*/
    /* Atom XdndActionDescription;*/

    Atom Xdnd_NON_PROTOCOL_ATOM;
     Atom Xdnd_text_uri_list;

    /**< The version of DND we are using. */
    Atom version;

    /**< The XDND processing step. */
#define XDND_DROP_STAGE_IDLE		0
#define XDND_DRAG_STAGE_DRAGGING	1
#define XDND_DRAG_STAGE_ENTERED		2
#define XDND_DROP_STAGE_CONVERTING	3
#define XDND_DROP_STAGE_ENTERED		4
    int stage;

    /**< The window of the dragging source */
    Window dragger_window;

    /**< The dragger display */
    Display *dragger_display;

    /**< The window where the drop may occur. */
    Window dropper_window;

    /**< The XDND version of the dragging window */
    Atom dragging_version;

    /**< The Drop mouse position. */
    int drop_x;
    int drop_y;
};

/* The XDND class information used for intermediate state processing. */
static DndClass dnd;

/**
 * Get the position relative to the window.
 *
 * @param w     The window we are computing.
 * @param x_rel Relative X-coordinate.
 * @param y_rel Relative y-coordinate.
 */
static void
xdndWindowOffset (Window w, int *xp, int *yp)
{
    Display *d;
    Window root, parent;
    int x, y;

    DEBUGOUT(("%s:%d: xdndWindowOffset\n", __FILE__, __LINE__));

    d = dnd.dragger_display;
    x = dnd.drop_x;
    y = dnd.drop_y;
    if (d == NULL || w == 0)
        return;
    /* Iterate over the tree until we get to the root window. */
    do
    {
        Window *children;
        unsigned int nchildren, width, height, border, depth;
        int rel_x, rel_y;

        rel_x = rel_y = 0;
        if (!XQueryTree(d, w, &root, &parent, &children, &nchildren))
            return;
        if (children)
            XFree(children);
        /* Retrive the geometry information about the window */
        if (!XGetGeometry (d, w, &root, &rel_x, &rel_y,
                           &width, &height, &border, &depth))
            return;
        DEBUGOUT(("%s:%d: %d %d\n", __FILE__, __LINE__, rel_x, rel_y));
        x -= rel_x;
        y -= rel_y;
        w = parent;
    }
    while (parent != root);
    *xp = x;
    *yp = y;
    DEBUGOUT(("%s:%d: xdndWindowOffset (%d,%d)\n",
              __FILE__, __LINE__, x, y));
}

/**
 * Reset the DND structure.
 *
 * @param dnd The drag'n'drop class to be initialised.
 */
static void
xdndReset (DndClass *dnd)
{
    dnd->stage = XDND_DROP_STAGE_IDLE;
    dnd->dragging_version = 0;
    dnd->dropper_window = 0;
    dnd->dragger_window = 0;
    dnd->dragger_display = NULL;
}

/**
 * Initialise the xdnd component. Saves the X-display value, initialises
 * structures and installs the X-Atoms required by DND.
 *
 * @param display The X-Windows display.
 */
static void
xdndInitialize (Display *display)
{
    DEBUGOUT(("%s:%d: xdndInitialize\n", __FILE__, __LINE__));

    memset (&dnd, 0, sizeof (dnd));

    /* Initialise the display */
    dnd.display = display;
    dnd.version = XDND_VERSION;

    /* Register the atoms. */
    dnd.XdndAware = XInternAtom (dnd.display, "XdndAware", False);
    dnd.XdndSelection = XInternAtom (dnd.display, "XdndSelection", False);
    dnd.XdndEnter = XInternAtom (dnd.display, "XdndEnter", False);
    dnd.XdndLeave = XInternAtom (dnd.display, "XdndLeave", False);
    dnd.XdndPosition = XInternAtom (dnd.display, "XdndPosition", False);
    dnd.XdndDrop = XInternAtom (dnd.display, "XdndDrop", False);
    dnd.XdndFinished = XInternAtom (dnd.display, "XdndFinished", False);
    dnd.XdndStatus = XInternAtom (dnd.display, "XdndStatus", False);
    dnd.XdndActionCopy = XInternAtom (dnd.display, "XdndActionCopy", False);
    /* dnd.XdndActionMove = XInternAtom (dnd.display, "XdndActionMove", False);*/
    /* dnd.XdndActionLink = XInternAtom (dnd.display, "XdndActionLink", False);*/
    /* dnd.XdndActionAsk = XInternAtom (dnd.display, "XdndActionAsk", False);*/
    /* dnd.XdndActionPrivate = XInternAtom (dnd.display, "XdndActionPrivate", False);*/
    /* dnd.XdndTypeList = XInternAtom (dnd.display, "XdndTypeList", False);*/
    /* dnd.XdndActionList = XInternAtom (dnd.display, "XdndActionList", False);*/
    /* dnd.XdndActionDescription = XInternAtom (dnd.display, "XdndActionDescription", False);*/

    dnd.Xdnd_NON_PROTOCOL_ATOM = XInternAtom (dnd.display, "meSelectionProperty", False);
    dnd.Xdnd_text_uri_list = XInternAtom (dnd.display, "text/uri-list", False);

    /* Initialise the state */
    xdndReset (&dnd);
}

/**
 * Make the X-Window DND aware.
 *
 * @param window The X-Window.
 */
static void
xdndAware (Window window)
{
    DEBUGOUT(("%s:%d: xdndAware\n", __FILE__, __LINE__));

    /* Signal the window as XDND aware. */
    XChangeProperty (dnd.display, window, dnd.XdndAware, XA_ATOM, 32,
                     PropModeReplace, (unsigned char *) &dnd.version, 1);
}

/**
 * Send a finsihed message to the calling window.
 *
 * @param  window  The desination window.
 * @param  from    The originating window.
 */
static void
xdndSendFinished (Window window, Window from)
{
    XEvent xevent;

    DEBUGOUT(("%s:%d: xdndSendFinished\n", __FILE__, __LINE__));

    memset (&xevent, 0, sizeof (xevent));

    xevent.xany.type = ClientMessage;
    xevent.xany.display = dnd.display;
    xevent.xclient.window = window;
    xevent.xclient.message_type = dnd.XdndFinished;
    xevent.xclient.format = 32;

    XDND_FINISHED_TARGET_WIN (&xevent) = from;

    XSendEvent (dnd.display, window, 0, 0, &xevent);
}

/**
 * Handle a client message from the X-server.
 *
 * @param xevent The X-event.
 * @param frame  The frame associated with the event..
 *
 * @return The processing state of the event.
 * @retval 0 The event was not processed.
 * @retval 1 The event was processed.
 */
static int
xdndClientMessage (XEvent *xevent, meFrame *frame)
{
    int processed = 0;

    DEBUGOUT(("%s:%d: got ClientMessage\n", __FILE__, __LINE__));
    if (frame == NULL)
        DEBUGOUT(("%s:%d: NULL Frame \n", __FILE__, __LINE__));
    else if (xevent->xclient.message_type == dnd.XdndEnter)
    {
        DEBUGOUT(("%s:%d: message_type is XdndEnter\n", __FILE__, __LINE__));

        /* Start the DND transaction again */
        xdndReset (&dnd);
        dnd.dragger_window = XDND_ENTER_SOURCE_WIN (xevent);
        dnd.dropper_window = xevent->xany.window;
        /* TODO: Ckeck the types */
        dnd.dragging_version = XDND_ENTER_VERSION (xevent);
        dnd.stage = XDND_DROP_STAGE_ENTERED;
        processed = 1;
    }
    else if (xevent->xclient.message_type == dnd.XdndLeave)
    {
        DEBUGOUT(("%s:%d: message_type is XdndLeave\n", __FILE__, __LINE__));
        if ((dnd.dragger_window == XDND_LEAVE_SOURCE_WIN (xevent)) &&
            (dnd.stage == XDND_DROP_STAGE_ENTERED))
        {
            DEBUGOUT(("%s:%d: leaving\n", __FILE__, __LINE__));

            /* Left the window. Reset the dnd transaction. */
            dnd.stage = XDND_DROP_STAGE_IDLE;
            processed = 1;
        }
        else
            DEBUGOUT(("%s:%d: wrong stage or from wrong window\n",
                      __FILE__, __LINE__));
    }
    else if (xevent->xclient.message_type == dnd.XdndPosition)
    {
        DEBUGOUT(("%s:%d: message_type is XdndPosition\n",
                  __FILE__, __LINE__));
        if ((dnd.dragger_window == XDND_POSITION_SOURCE_WIN (xevent)) &&
            (dnd.stage == XDND_DROP_STAGE_ENTERED))
        {
            XEvent revent;              /* Return event. */
            Display *d;
            Window w;

            d = xevent->xany.display;
            w = xevent->xany.window;

            /* Save the position information relative to the window. */
            dnd.drop_x = (xevent->xclient.data.l[2] >> 16) & 0xfff;
            dnd.drop_y = (xevent->xclient.data.l[2]) & 0xffff;
            dnd.dragger_display = d;
            DEBUGOUT(("%s:%d: positioning (%d,%d)\n",
                      __FILE__, __LINE__, dnd.drop_x, dnd.drop_y));

            /* Acknowledge that we will accept the event. */
            memset (&revent, 0, sizeof (revent));
            revent.xany.type = ClientMessage;
            revent.xany.display = dnd.display;
            revent.xclient.message_type = dnd.XdndStatus;
            revent.xclient.window = xevent->xclient.data.l[0];
            revent.xclient.format = 32;

            XDND_STATUS_TARGET_WIN (&revent) = w;
            XDND_STATUS_WILL_ACCEPT_SET (&revent, 1);
            XDND_STATUS_WANT_POSITION_SET (&revent, 0);
            if (dnd.dragging_version >= 2)
                XDND_STATUS_ACTION (&revent) = dnd.XdndActionCopy;

            /* Send the event to the client window */
            XSendEvent (d, revent.xclient.window, 0, 0, &revent);

            processed = 1;
        }
        else
            DEBUGOUT(("%s:%d: wrong stage or from wrong window\n",
                      __FILE__, __LINE__));
    }
    else if (xevent->xclient.message_type == dnd.XdndDrop)
    {
        DEBUGOUT(("%s:%d:  message_type is XdndDrop\n", __FILE__, __LINE__));
        if ((dnd.dragger_window == XDND_DROP_SOURCE_WIN (xevent)) &&
            (dnd.stage == XDND_DROP_STAGE_ENTERED))
        {
            /* Convert to a selection event. */
            XConvertSelection (xevent->xany.display, dnd.XdndSelection,
                               dnd.Xdnd_text_uri_list, dnd.Xdnd_NON_PROTOCOL_ATOM,
                               xevent->xany.window, /*xevent->xclient.data.l[2]*/
                               CurrentTime);
            dnd.stage = XDND_DROP_STAGE_CONVERTING;
            processed = 1;
        }
        else
            DEBUGOUT(("%s:%d: wrong stage or from wrong window\n",
                      __FILE__, __LINE__));
    }
    else
        DEBUGOUT(("%s:%d: Not a xdnd message %d\n",
                  __FILE__, __LINE__, xevent->xclient.message_type));

    return processed;
}

/**
 * Push the file onto the Drag and drop list.
 *
 * @param frame The microemacs frame.
 * @param mouse x position.
 * @param mouse y position.
 * @param filename The filename to be pushed.
 */
static void
xdndPushFile (meFrame *frame, int x, int y, char *filename)
{
    struct s_DragAndDrop *dadp;         /* Drag and drop pointer */
    int len;

    len = meStrlen (filename);

    /* Get the drag and drop buffer and add to the list */
    if ((dadp = (struct s_DragAndDrop *) meMalloc (sizeof(struct s_DragAndDrop)
                                                   + len)) == NULL)
        return;

    /* See if this is a local file object expressed as a URL. */
    if ((len >= 7) &&
        ((filename[0] == 'f') || (filename[0] == 'F')) &&
        ((filename[1] == 'i') || (filename[1] == 'I')) &&
        ((filename[2] == 'l') || (filename[2] == 'L')) &&
        ((filename[3] == 'e') || (filename[3] == 'E')) &&
        (filename[4] == ':') &&
        (filename[5] == '/') &&
        (filename[6] == '/'))
    {
        char *p, *q;                    /* Working loop pointers */

        /* This is a local file URL. Remove the special URL characters of %xx
         * where x is a hexadecimal character. */
        p = &filename[7];
        q = (char *) &dadp->fname[0];
        while ((*q = *p++) != '\0')
        {
            if ((*q == '%') && isXDigit (p[0]) && isXDigit (p[1]))
            {
                *q++ = (hexToNum (p[0]) << 4) | hexToNum (p[1]);
                p += 2;
            }
            else
                q++;
        }
    }
    else
    {
        /* Simply copy the name. This may be a http: or ftp: type file. */
        meStrcpy (&dadp->fname[0], filename);
    }

    /* Add the dad structure to the list of pending files awaiting
     * processing. */
    dadp->mouse_x = x;
    dadp->mouse_y = y;
    dadp->frame = frame;
    dadp->next = dadHead;
    dadHead = dadp;
    DEBUGOUT(("%s:%d: xdndPushFile \"%s\"\n", __FILE__, __LINE__, dadp->fname));
}

/**
 * Selection event occured.
 *
 * @param xevent The X-event.
 * @param frame  The frame associated with the event.
 *
 * @return The processing state of the event.
 * @retval 0 The event was not processed.
 * @retval 1 The event was processed.
 */
static int
xdndSelectionNotify (XEvent *xevent, meFrame *frame)
{
    int processed = 0;

    if (xevent->type == SelectionNotify)
    {
	DEBUGOUT(("%s:%d: got SelectionNotify\n", __FILE__, __LINE__));
	if ((xevent->xselection.property == dnd.Xdnd_NON_PROTOCOL_ATOM) &&
            (dnd.stage == XDND_DROP_STAGE_CONVERTING) && (frame != NULL))
        {
            long read;                  /* Number of bytes read */
            unsigned long remaining;    /* Number of remaining calls */
            char *buf = NULL;           /* Expanded buffer */
            char *start;                /* Start of the filename */
            char *end;                  /* End of the filename */

            DEBUGOUT(("%s:%d: property is Xdnd_NON_PROTOCOL_ATOM"
                      " - getting selection\n", __FILE__, __LINE__));

            read = 0;
            do
            {
                unsigned char *s = NULL;
                Atom actual;
                int format;
                unsigned long count;

                /* Retrive the file information from the property window */
                if (XGetWindowProperty (dnd.display,
                                        xevent->xany.window,
                                        xevent->xselection.property,
                                        read, 65536, True,
                                        dnd.Xdnd_text_uri_list,
                                        &actual, &format, &count, &remaining, &s) != Success)
                {
                    if (s != NULL)
                        XFree (s);
                    DEBUGOUT(("%s:%d: ERROR: count=%d remaining=%d read=%d format=%d\n",
                              __FILE__, __LINE__, count, remaining, read, format));
                    return 1;
                }
                DEBUGOUT(("%s:%d: count=%d remaining=%d read=%d format=%d\n",
                          __FILE__, __LINE__, count, remaining, read, format));

                /* Build the buffer by concatinating the data extracted from
                 * the call into a linear buffer space. */
                if ((count > 0) && (format == 8))
                {
                    int oread = read;
                    char *p;
                    read += count;

                    if (buf == NULL)
                    {
                        p = malloc (read+1);
                        if (p != NULL)
                        {
                            buf = p;
                            memcpy (buf, s, read);
                            buf[read] = '\0';
                        }
                    }
                    else if ((p = realloc (buf, read+1)) != NULL)
                    {
                        buf = p;
                        memcpy (&buf[oread], s, count);
                        buf[read] = '\0';
                    }
                }

                /* Free off the X data */
                if (s != NULL)
                    XFree (s);
            } while (remaining != 0);

            /* Extract all of the files from the composed buffer. */
            if (buf != NULL)
            {
                int x, y;               /* Position of the cursor. */

                /* XDND provides us with an absolute screen coordinate, this
                 * needs to be changed relative to the window. */
                x = dnd.drop_x;
                y = dnd.drop_y;
                xdndWindowOffset (xevent->xany.window, &x, &y);

                /* Unpack the buffer into files. */
                start = buf;
                end = start;
                for (;;)
                {
                    if ((*end == '\r') || (*end == '\n'))
                    {
                        *end = '\0';
                        if (start != end)
                        {
                            /* Push the file ready for processing */
                            xdndPushFile (frame, x, y, start);
                            DEBUGOUT(("%s:%d: FILE: (%d,%d) %s\n",
                                      __FILE__, __LINE__, x, y, start));
                        }

                        start = end+1;
                        while ((*start == '\r') || (*start == '\n'))
                            start++;
                        if (*start == '\0')
                            break;
                        end = start;
                    }
                    else if (*end == '\0')
                    {
                        /* Sometimes the filename is not \r\n terminated,
                         * push what has been received. */
                        if (*start != '\0') 
                        {
                            /* Push the file ready for processing */
                            xdndPushFile (frame, x, y, start);
                            DEBUGOUT(("%s:%d: FILE: (%d,%d) %s\n",
                                      __FILE__, __LINE__, x, y, start));
                        }
                        /* Finished */
                        break;
                    }
                    else
                        end++;
                }
                free (buf);
            }

	    /* Tell the client that we have finished. */
            if (dnd.dragging_version >= 2)
		xdndSendFinished (dnd.dragger_window, dnd.dropper_window);

            /* Clean up the transaction. */
            xdndReset (&dnd);
	    dnd.stage = XDND_DROP_STAGE_IDLE;
            processed = 1;

            /* Flush the input queue, send an abort to kill any command off.
             * The drag and drop list is processed once we return to a base
             * state. */
            if (dadHead != NULL)
                addKeyToBuffer(breakc);  /* Break character (ctrl-G) */
        }
        else
        {
	    DEBUGOUT(("%s:%d: property is not Xdnd_NON_PROTOCOL_ATOM"
                      " - ignoring (%d/0x%08x)\n", __FILE__, __LINE__,
                      xevent->xselection.property, xevent->xselection.property));
	}
    }
    return processed;
}

#endif

#ifdef _XTERM
/***************************************************************************
 * Setup the ME icon for X-Windows                                         *
 ***************************************************************************/

#ifdef _XPM
/* Use a colour icon */
#include <X11/xpm.h>        /* Xpm support for colored icon          */

static void
meSetIconState (Display *display, Window window)
{
    static char *iconBytes[] = {
#ifdef _NANOEMACS
        "48 48 7 1",
        " 	c None",
        ".	c #000000",
        "+	c #FF0000",
        "@	c #C0C0C0",
        "#	c #000000",
        "$	c #FFFF00",
        "%	c #0000FF",
        ".##############################################.",
        ".%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%.",
        ".%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%.",
        " .%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%# ",
        " .%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%. ",
        "  .%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%.  ",
        "  .@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.  ",
        "   .########################################.   ",
        "    .#######################################.   ",
        "    .###################$$$$$##############.    ",
        "     .##########$$$$##$$$$$$$$$############.    ",
        "     .##########$$$$##$$$$$$$$$###########.     ",
        "      .#########$$$$$$$$$$$$$$$$##########.     ",
        "      .#########$$$$$$$###$$$$$$$########.      ",
        "       .########$$$$$$#####$$$$$$########.      ",
        "       .########$$$$$$######$$$$$#######.       ",
        "        .#######$$$$$#######$$$$$#######.       ",
        "        .#######$$$$$#######$$$$$######.        ",
        "         .######$$$$$#######$$$$$######.        ",
        "         .######$$$$$#######$$$$$#####.         ",
        "          .#####$$$$$#######$$$$$#####.         ",
        "          .#####$$$$$#######$$$$$####.          ",
        "           .####$$$$$#######$$$$$####.          ",
        "           .####$$$$$#######$$$$$###.           ",
        "            .###$$$$$#######$$$$$###.           ",
        "            .###$$$$$#######$$$$$##.            ",
        "             .##$$$$$#######$$$$$#.             ",
        "             ..#$$$$$#######$$$$$#.             ",
        "              .#$$$$$#######$$$$$.              ",
        "              ..$$$$$#######$$$$$.              ",
        "               .################.               ",
        "               ..###############.               ",
        "                .##############.                ",
        "                ..#############.                ",
        "                 .############.                 ",
        "                 .@@@@@@@@@@@@.                 ",
        "                  .++++++++++.                  ",
        "                  .++++++++++.                  ",
        "                   .++++++++.                   ",
        "                   .++++++++.                   ",
        "                    .++++++.                    ",
        "                    .++++++.                    ",
        "                     .++++.                     ",
        "                     .++++.                     ",
        "                      .++.                      ",
        "                      .++.                      ",
        "                       ..                       ",
        "                       ..                       "
#else
        "48 48 6 1",
        " 	c None",
        ".	c #C0C0C0",
        "+	c #000000",
        "@	c #0000FF",
        "#	c #FFFF00",
        "$	c #FF0000",
        "................................................",
        "................................................",
        "+@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+",
        "+@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+",
        "++@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@++",
        " +@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@+ ",
        "  +..........................................+  ",
        "  +..........................................+  ",
        "   ++++++++++++++++++++++++++++++++++++++++++   ",
        "   ++++++++++++++++++++++++++++++++++++++++++   ",
        "    +++++++++++++++++++++++++++++++++++++++++   ",
        "    +++++++++++++++##########+++++++++++++++    ",
        "     ++++++++++++##############+++++++++++++    ",
        "     +++++++++++#################++++++++++     ",
        "      +++++++++###################+++++++++     ",
        "      ++++++++########++++########++++++++      ",
        "       +++++++#######++++++########+++++++      ",
        "        +++++#######++++++++#######++++++       ",
        "        ++++#######++++++++++######++++++       ",
        "        ++++######++++++++++++######++++        ",
        "         +++######++++++++++++######+++         ",
        "         +++########################+++         ",
        "          ++########################++          ",
        "           +########################++          ",
        "           +########################++          ",
        "           +########################+           ",
        "           +######+++++++++++++++++++           ",
        "           +######++++++++++++++++++            ",
        "            +######+++++++++++++++++            ",
        "            +#######+++++++++++++++             ",
        "             +#######++++++++####++             ",
        "             +########++++++######++            ",
        "              +#####################+           ",
        "               ++##################+            ",
        "                ++################+             ",
        "                ++++###########+++              ",
        "                +++++++++++++++++               ",
        "                 +++++++++++++++                ",
        "                 +............+                 ",
        "                  +$$$$$$$$$$+                  ",
        "                   +$$$$$$$$$+                  ",
        "                   +$$$$$$$$+                   ",
        "                    +$$$$$$$+                   ",
        "                    +$$$$$$+                    ",
        "                     +$$$$$+                    ",
        "                     +$$$$+                     ",
        "                      +$$$+                     ",
        "                       +++                      "
#endif
    };
    
    Pixmap iconPixmap ;
    Pixmap iconMask ;
    XWMHints wmHints ;
    
    /* Create the pixmap */
    XpmCreatePixmapFromData (display,
                             window,
                             iconBytes,
                             &iconPixmap,
                             &iconMask,
                             NULL);
    
    wmHints.flags = (InputHint|StateHint) ;
    wmHints.input = True ;
    wmHints.initial_state = NormalState ;
    if(iconPixmap != 0)
    {
        wmHints.flags |= IconPixmapHint|IconMaskHint ;
        wmHints.icon_pixmap = iconPixmap ;
        wmHints.icon_mask = iconMask;
    }
    XSetWMHints(display, window, &wmHints) ;
}
#else
/* Use a monochrome icon */
static void
meSetIconState (Display *display, Window window)
{
#define iconWidth  48
#define iconHeight 48
    static meUByte iconBits[] = {
#ifdef _NANOEMACS
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xfc, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xfc, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xfc, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xfc, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xf8, 0xff, 0xff, 0xff, 0xff, 0x3f,
        0x08, 0x00, 0x00, 0x00, 0x00, 0x20,
        0x10, 0x00, 0x00, 0x00, 0x00, 0x10,
        0xe0, 0xff, 0xff, 0xff, 0xff, 0x1f,
        0xe0, 0xff, 0xff, 0xff, 0xff, 0x1f,
        0xc0, 0xff, 0xff, 0xc0, 0xff, 0x0f,
        0xc0, 0xff, 0x20, 0x00, 0xff, 0x07,
        0xc0, 0xff, 0x00, 0x00, 0xfe, 0x07,
        0x80, 0xff, 0x00, 0x00, 0xfe, 0x03,
        0x00, 0xff, 0x80, 0x07, 0xfe, 0x03,
        0x00, 0xff, 0xc0, 0x0f, 0xfe, 0x03,
        0x00, 0xfe, 0xc0, 0x0f, 0xfe, 0x01,
        0x00, 0xfe, 0xe0, 0x0f, 0xfe, 0x00,
        0x00, 0xfe, 0xe0, 0x0f, 0xfe, 0x00,
        0x00, 0xfc, 0xe0, 0x0f, 0x7e, 0x00,
        0x00, 0xf8, 0xe0, 0x0f, 0x7e, 0x00,
        0x00, 0xf8, 0xe0, 0x0f, 0x3e, 0x00,
        0x00, 0xf0, 0xe0, 0x0f, 0x3e, 0x00,
        0x00, 0xf0, 0xe0, 0x0f, 0x1e, 0x00,
        0x00, 0xf0, 0xe0, 0x0f, 0x1e, 0x00,
        0x00, 0xe0, 0xe0, 0x0f, 0x0e, 0x00,
        0x00, 0xc0, 0xe0, 0x0f, 0x0e, 0x00,
        0x00, 0xc0, 0xe0, 0x0f, 0x0e, 0x00,
        0x00, 0x80, 0xe0, 0x0f, 0x06, 0x00,
        0x00, 0x80, 0xe0, 0x0f, 0x02, 0x00,
        0x00, 0x80, 0xff, 0xff, 0x03, 0x00,
        0x00, 0x00, 0xff, 0xff, 0x01, 0x00,
        0x00, 0x00, 0xfe, 0xff, 0x01, 0x00,
        0x00, 0x00, 0xfe, 0xff, 0x01, 0x00,
        0x00, 0x00, 0xfc, 0xff, 0x01, 0x00,
        0x00, 0x00, 0x04, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x04, 0x40, 0x00, 0x00,
        0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00,
        0x00, 0x00, 0xf0, 0x3f, 0x00, 0x00,
        0x00, 0x00, 0xf0, 0x3f, 0x00, 0x00,
        0x00, 0x00, 0xe0, 0x1f, 0x00, 0x00,
        0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00,
        0x00, 0x00, 0xe0, 0x07, 0x00, 0x00,
        0x00, 0x00, 0xc0, 0x07, 0x00, 0x00,
        0x00, 0x00, 0x40, 0x04, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x04, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
#else
        0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a,
        0x54, 0x55, 0x55, 0x55, 0x55, 0x15,
        0x54, 0x55, 0x55, 0x55, 0x55, 0x0d,
        0xf8, 0xff, 0xff, 0xff, 0xff, 0x0f,
        0xf8, 0xff, 0xff, 0xff, 0xff, 0x07,
        0xf0, 0xff, 0xff, 0xff, 0xff, 0x07,
        0xf0, 0xff, 0x07, 0xf0, 0xff, 0x03,
        0xe0, 0xff, 0x01, 0xc0, 0xff, 0x03,
        0xe0, 0xff, 0x00, 0x80, 0xff, 0x01,
        0xc0, 0x7f, 0x00, 0x00, 0xff, 0x01,
        0xc0, 0x3f, 0xe0, 0x01, 0xfe, 0x00,
        0x80, 0x1f, 0xf8, 0x07, 0xfe, 0x00,
        0x80, 0x0f, 0xfc, 0x0f, 0x7c, 0x00,
        0x00, 0x0f, 0xfc, 0x1f, 0x7c, 0x00,
        0x00, 0x0f, 0xfc, 0x1f, 0x38, 0x00,
        0x00, 0x0e, 0xfc, 0x1f, 0x18, 0x00,
        0x00, 0x0e, 0x00, 0x00, 0x18, 0x00,
        0x00, 0x0c, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x0c, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x08, 0xfc, 0xff, 0x07, 0x00,
        0x00, 0x08, 0xfc, 0xff, 0x07, 0x00,
        0x00, 0x08, 0xfc, 0xff, 0x03, 0x00,
        0x00, 0x10, 0xf8, 0xff, 0x01, 0x00,
        0x00, 0x10, 0xf0, 0x8f, 0x03, 0x00,
        0x00, 0x20, 0xe0, 0x07, 0x07, 0x00,
        0x00, 0x40, 0x00, 0x00, 0x04, 0x00,
        0x00, 0x40, 0x00, 0x00, 0x04, 0x00,
        0x00, 0x80, 0x00, 0x00, 0x02, 0x00,
        0x00, 0x00, 0x07, 0xe0, 0x01, 0x00,
        0x00, 0x00, 0xff, 0x3f, 0x00, 0x00,
        0x00, 0x00, 0xaa, 0x2a, 0x00, 0x00,
        0x00, 0x00, 0x54, 0x15, 0x00, 0x00,
        0x00, 0x00, 0x54, 0x15, 0x00, 0x00,
        0x00, 0x00, 0xa8, 0x0a, 0x00, 0x00,
        0x00, 0x00, 0xa8, 0x0a, 0x00, 0x00,
        0x00, 0x00, 0x50, 0x05, 0x00, 0x00,
        0x00, 0x00, 0x50, 0x05, 0x00, 0x00,
        0x00, 0x00, 0xa0, 0x02, 0x00, 0x00,
        0x00, 0x00, 0xa0, 0x02, 0x00, 0x00,
        0x00, 0x00, 0x40, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x40, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif
    };
    Pixmap iconPixmap ;
    XWMHints wmHints ;
    
    /* Create the monocrome bitmap from the static image */
    iconPixmap = XCreatePixmapFromBitmapData (display, window, 
                                              (char *)iconBits, iconWidth,iconHeight,
                                              WhitePixel(mecm.xdisplay,xscreen),
                                              BlackPixel(mecm.xdisplay,xscreen),1);
    wmHints.flags = (InputHint|StateHint) ;
    wmHints.input = True ;
    wmHints.initial_state = NormalState ;
    if(iconPixmap != 0)
    {
        wmHints.flags |= IconPixmapHint ;
        wmHints.icon_pixmap = iconPixmap ;
    }
    XSetWMHints(display, window, &wmHints) ;
}
#endif /* _XPM */
#endif /* _XTERM */    
