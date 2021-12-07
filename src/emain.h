/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * emain.h - Encapsulate all platform definitions.
 *
 * Copyright (C) 1997-2009 JASSPA (www.jasspa.com)
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
 * Created:     Thu Nov 27 1997
 * Synopsis:    Encapsulate all platform definitions.
 * Authors:     Steve Phillips & Jon Green
 * Description:
 *      Encapsulate all of the platform definitions into a sigle file.
 *
 * Notes:
 *      Macro definitions defined within this file obey the
 *      following rules:-
 * 
 *       _XXXX - Underscore names are tested using
 *               "#if (defined _XXXX)", and are disabled by
 *               using "#undef _XXXX" following the definition
 *               to remove or disable.
 *
 *       XXXX  - Non-underscore names are tested using 
 *               "#if XXXXX". A zero disables the option.
 *               Non-zero (typically 1) enables the option.
 */

#ifndef __EMAIN_H__
#define __EMAIN_H__

/* the following 3 defines are used for debugging and memory leak checking,
 * _ME_WIN32_FULL_DEBUG - Enables Windows debugging
 * _ME_FREE_ALL_MEMORY  - Frees all memory before exiting
 * _ME_USE_STD_MALLOC   - Use standard C malloc commands directly
 */
/*#define _ME_WIN32_FULL_DEBUG*/
/*#define _ME_FREE_ALL_MEMORY*/
/*#define _ME_USE_STD_MALLOC*/

/* These next define is platform specific, but as all supported 
 * platforms use these and all future ones should I've put them here
 * for now. NOTE APRAM has been removed, it became unused & redundant
 */
#define _STDARG 1       /* Use <stdarg.h> in defintions                 */

/* One may think that this should be platform dependant, BUT time and
 * experience has shown that it is best to always use a '/' in the body
 * of MicroEmacs (simplifies everything) and convert to or from the
 * system DIR char if required. No system yet needs this but
 * as some need to check and convert to a '/' they define a CONVDIR_CHAR
 */
#define DIR_CHAR  '/'   /* directory divide char, ie /bin/ */

/**************************************************************************
* UNIX : IRIX                                                             *
**************************************************************************/
#if (defined _IRIX5) || (defined _IRIX6)
#define meSYSTEM_NAME  "irix"           /* Identity name of the system   */
#define _IRIX          1                /* This is an irix box           */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _SOCKET        1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _DRAGNDROP     1                /* Drag and drop supported.      */
#define _DEFAULT_SEARCH_PATH "/opt/jasspa:/usr/share/jasspa:/usr/local/jasspa"
#endif

/**************************************************************************
* UNIX : HPUX                                                             *
**************************************************************************/
#if (defined _HPUX9) || (defined _HPUX10) || (defined _HPUX11)
#define meSYSTEM_NAME "hpux"            /* Identity name of the system   */
#define _HPUX          1                /* This is a hpux box            */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#if (defined _HPUX10) || (defined _HPUX11)
#define _TERMIOS       1                /* Use termios, not termio       */
#endif
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _SOCKET        1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _DRAGNDROP     1                /* Drag and drop supported.      */
#define _BINFS         1                /* Use the built-in File system  */
#define _meDEF_SYS_ERRLIST              /* errno.h not def sys_errlist   */
#define _DEFAULT_SEARCH_PATH "/opt/jasspa:/usr/share/jasspa:/usr/local/jasspa"
#endif

/**************************************************************************
* UNIX : UNIXWARE                                                         *
**************************************************************************/
#ifdef _UNIXWR1 
#define meSYSTEM_NAME  "unixwr1"        /* Identity name of the system   */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _SOCKET        1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _DRAGNDROP     1                /* Drag and drop supported.      */
#define _DEFAULT_SEARCH_PATH "/opt/jasspa:/usr/share/jasspa:/usr/local/jasspa"
#endif

/**************************************************************************
* UNIX : SunOS Sparc + Intel                                              *
**************************************************************************/
/* _SUNOS5 == Sparc Solaris; _SUNOS_X86 == i86 Solaris */
#if (defined _SUNOS5) || (defined _SUNOS_X86)
#define meSYSTEM_NAME  "sunos"          /* Identity name of the system   */
#define _SUNOS         1                /* This is a sunos box           */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _SOCKET        1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _DRAGNDROP     1                /* Drag and drop supported.      */
#define _BINFS         1                /* Use the built-in File system  */
#define _meDEF_SYS_ERRLIST              /* errno.h not def sys_errlist   */

/* Search path for CSW Sun build. */
#ifdef _CSW
#define _DEFAULT_SEARCH_PATH "/opt/csw/share/jasspa"
#else
#define _DEFAULT_SEARCH_PATH "/opt/jasspa:/usr/share/jasspa:/usr/local/jasspa"
#endif
#endif

/**************************************************************************
* UNIX : FreeBSD based OSs                                                *
**************************************************************************/
#ifdef _DARWIN
#define meSYSTEM_NAME  "darwin"         /* Identity name of the system   */
#define _FREEBSD_BASE  1                /* Uses FreeBSD as base          */
#endif

#ifdef _FREEBSD
#define meSYSTEM_NAME  "freebsd"        /* Identity name of the system   */
#define _FREEBSD_BASE  1                /* Uses FreeBSD as base          */
#endif

#ifdef _OPENBSD
#define meSYSTEM_NAME  "openbsd"        /* Identity name of the system   */
#define _FREEBSD_BASE  1                /* Uses FreeBSD as base          */
#endif

#ifdef _OSF1
#define meSYSTEM_NAME  "osf1"           /* Identity name of the system   */
#define _FREEBSD_BASE  1                /* Uses FreeBSD as base          */
#endif

#ifdef _FREEBSD_BASE
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _SOCKET        1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _DRAGNDROP     1                /* Drag and drop supported.      */
#define _BINFS         1                /* Use the built-in File system  */
#define _DEFAULT_SEARCH_PATH "/opt/jasspa:/usr/share/jasspa:/usr/local/jasspa:/usr/local/share/jasspa"
#endif

/**************************************************************************
* UNIX : AIX                                                              *
**************************************************************************/
#ifdef _AIX
#define meSYSTEM_NAME  "aix"            /* Identity name of the system   */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _SOCKET        1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _DRAGNDROP     1                /* Drag and drop supported.      */
#ifdef _AIX4
#define _meDEF_SYS_ERRLIST              /* errno.h doesnt def sys_errlist*/
#endif
#define _DEFAULT_SEARCH_PATH "/opt/jasspa:/usr/share/jasspa:/usr/local/jasspa"
#endif

/**************************************************************************
* UNIX : Linux based OSs                                                  *
**************************************************************************/
#ifdef _LINUX
#define meSYSTEM_NAME  "linux"          /* Identity name of the system   */
#define _LINUX_BASE    1                /* Uses Linux as base            */
#endif

#ifdef _ZAURUS
#define meSYSTEM_NAME  "zaurus"         /* Identity name of the system   */
#define _LINUX_BASE    1                /* Uses Linux as base            */
#endif

#ifdef _LINUX_BASE
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _XTERM         1                /* Use Xlib                      */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _CLIENTSERVER  1                /* Client server support         */
#define _SOCKET        1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _DRAGNDROP     1                /* Drag and drop supported.      */
#define _BINFS         1                /* Use the built-in File system  */
#define _DEFAULT_SEARCH_PATH "/usr/share/jasspa:/usr/local/jasspa"
#endif

/**************************************************************************
* UNIX : Cyqwin on Windows                                                *
**************************************************************************/
#ifdef _CYGWIN
#define meSYSTEM_NAME  "cygwin"         /* Identity name of the system   */
#define _UNIX          1                /* This is a UNIX system         */
#define _USG           1                /* UNIX system V                 */
#define _DIRENT        1                /* Use <dirent.h> for directory  */
#define _TERMIOS       1                /* Use termios, not termio       */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#undef  _WIN32                          /* This is not win32             */
#define _POSIX_SIGNALS 1                /* use POSIX signals             */
#define _USEPOLL       1                /* use poll() for stdin polling  */
#define _XTERM         1                /* Use Xlib                      */
#define _CLIENTSERVER  1                /* Client server support         */
#define _SOCKET        1                /* Supports url reading          */
#define _DRAGNDROP     1                /* Drag and drop supported.      */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _BINFS         1                /* Use the built-in File system  */
#define _DEFAULT_SEARCH_PATH "/usr/share/jasspa:/usr/local/jasspa:/usr/local/share/jasspa"
/* Under cygwin sys_errlist is defined slightly differently - redefine   */
#define sys_errlist _sys_errlist        /* sys_errlist specially defined */
#endif /* _CYGWIN */

/**************************************************************************
* UNIX : OpenStep NexT                                                    *
*                                                                         *
* Tested on Openstep 4.2 / NeXT Motorola 040                              *
**************************************************************************/
#ifdef _NEXT
#define meSYSTEM_NAME  "openstep"       /* Identity name of the system   */
#define _UNIX          1                /* This is a UNIX system         */
#define _BSD_43        1                /* This is a BSD 4.3 system      */
#define _BSD_CBREAK    1                /* Use CBREAK or RAW settings    */
#define _TCAP          1                /* Use TERMCAP                   */
#define _TCAPFONT      1                /* Use TERMCAP fonts to color    */
#define _SOCKET        1                /* Supports url reading          */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _NOSTRDUP      1                /* No strdup                     */
#define _NOPUTENV      1                /* No putenv support             */

/* NeXT provides us with libc.h this includes all of the API definitions for
 * the standard 'C' library calls. Nice touch !! */
#include <libc.h>
#define _DEFAULT_SEARCH_PATH "/usr/local/jasspa"
#endif /* _NEXT */

/**************************************************************************
* Microsoft : DOS                                                         *
**************************************************************************/
#ifdef _DOS
#define meSYSTEM_NAME  "dos"            /* Identity name of the system   */
#define _CTRLZ         1                /* ^Z at end of files (MSDOS)    */
#define _CTRLM         1                /* ^M at end of lines (MSDOS)    */
#define _SINGLE_CASE   1                /* Files only have single case   */
#define _MOUSE         1                /* Mouse supported               */
#define _DRV_CHAR     ':'               /* drive divide letter, C:\dos   */
#define _CONVDIR_CHAR '\\'              /* Filename convert '\\' => '/'  */
#endif /* _DOS */

/**************************************************************************
* MinGW : GNU GCC on Windows                                              *
**************************************************************************/
#ifdef _MINGW
#define _WIN32         1                /* Use win32                     */
#define _BINFS         1                /* Use the built-in File system  */
#endif

/**************************************************************************
* Microsoft : 32s/'95/'98/NT                                              *
**************************************************************************/
#ifdef _WIN32s
#define _WIN32         1                /* Use win32                     */
#endif
#ifdef _WIN32
#define meSYSTEM_NAME  "win32"          /* Identity name of the system   */
#ifndef WIN32
#define WIN32          1                /* Standard win32 definition     */
#endif
#define _CTRLZ         1                /* ^Z at end of files (MSDOS)    */
#define _CTRLM         1                /* ^M at end of lines (MSDOS)    */
#if (_MSC_VER != 900)
/* MSVC Not version 2 - win32s */
#define _IPIPES        1                /* platform supports Inc. pipes  */
#define _CLIENTSERVER  1                /* Client server support         */
#endif
/* The next option is commented out as the win32*.mak file define it when required */
/*#define _SOCKET     1*/                /* Supports url reading          */
#define _MOUSE         1                /* Mouse supported               */
#define _CLIPBRD       1                /* Inter window clip board supp  */
#define _WINDOW        1                /* Windowed, resizing & title    */
#define _INSENSE_CASE  1                /* File names case insensitive   */
#define _BINFS         1                /* Use the built-in File system  */
#define _DRAGNDROP     1                /* Drag and drop supported.      */
#define _DRV_CHAR     ':'               /* drive divide letter, C:\dos   */
#define _CONVDIR_CHAR '\\'              /* Filename convert '\\' => '/'  */
#ifdef _ME_WINDOW                       /* windowing mode?               */
#define _MULTI_WINDOW  1                /* can support multiple window frames    */
#endif
#ifndef _WIN32_FULL_INC
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>                    /* Standard windows API          */
#ifdef _DRAGNDROP
#include <shellapi.h>                   /* Drag and drop API             */
#endif /* _DRAGNDROP */
#endif /* _WIN32 */

/*************************************************************************
 * X-Windows Setup - Definitions required for X-Lib support              *
 *************************************************************************/
#ifndef _ME_WINDOW              /* console only mode?                    */
#undef _XTERM                   /* Do not want X-Windows                 */
#endif
#ifdef _XTERM
#define _MOUSE          1       /* Mouse supported on XTERM              */
#define _CLIPBRD        1       /* Inter window clip board supp          */
#define _WINDOW         1       /* Window, need resizing & title code    */
#define _MULTI_WINDOW   1       /* can support multiple window frames    */
#include <X11/Intrinsic.h>      /* Intrinsics Definitions                */
#include <X11/StringDefs.h>     /* Standard Name-String definitions      */
#include <X11/keysym.h>         /* Keyboard symbols                      */
#include <X11/Xutil.h>
#endif /* _XTERM */

#ifndef _ME_CONSOLE             /* window only mode?                     */
#undef _TCAP                    /* Do not want Termcap                   */
#endif

#ifdef _BINFS
#define MEOPT_BINFS     1       /* enable the built in file system       */
#endif

#ifndef _NANOEMACS
/*************************************************************************
 * MicroEmacs Configuration options - all feature version                *
 *************************************************************************/
#define MEOPT_TYPEAH    1       /* type ahead causes update to be skipped*/
#define MEOPT_DEBUGM    1       /* enable macro debugging $debug         */
#define MEOPT_TIMSTMP   1       /* Enable time stamping of files on write*/
#define MEOPT_COLOR     1       /* color commands and windows            */
#define MEOPT_HILIGHT   1       /* color hilighting                      */
#define MEOPT_ISEARCH   1       /* Incremental searches like ITS EMACS   */
#define MEOPT_WORDPRO   1       /* Advanced word processing features     */
#define MEOPT_FENCE     1       /* fench matching in CMODE               */
#define MEOPT_CRYPT     1       /* file encryption enabled?              */
#define MEOPT_MAGIC     1       /* include regular expression matching?  */
#define MEOPT_ABBREV    1       /* Abbreviated files                     */
#define MEOPT_FILENEXT  1       /* include the file next buffer stuff    */
#define MEOPT_RCS       1       /* do rcs check ins and outs             */
#define MEOPT_LOCALBIND 1       /* do local key binding (buffer)         */
#define MEOPT_PRINT     1       /* do printing of buffers and regions    */
#define MEOPT_SPELL     1       /* do spelling of words and buffers      */
#define MEOPT_UNDO      1       /* undo capability                       */
#define MEOPT_NARROW    1       /* enable narrowing functionality        */
#define MEOPT_REGISTRY  1       /* enable registry functionality         */
#define MEOPT_OSD       1       /* enable OSD functionality              */
#define MEOPT_DIRLIST   1       /* enable Directory tree and listing     */
#define MEOPT_FILEHOOK  1       /* enable File hook assigning            */
#define MEOPT_TAGS      1       /* enable TAGS support                   */
#define MEOPT_CMDHASH   1       /* use a hash table for command lookup   */
#define MEOPT_POSITION  1       /* enable set & goto position            */
#define MEOPT_EXTENDED  1       /* enable miscellaneous extended features*/
#define MEOPT_CALLBACK  1       /* enable macro callbacks                */
#define MEOPT_SPAWN     1       /* enable spawning                       */
#define MEOPT_SCROLL    1       /* enable scroll bars                    */
#define MEOPT_HSPLIT    1       /* enable vertical window                */ 
#define MEOPT_FRAME     1       /* enable multiple frames                */
#if MEOPT_FRAME && (defined _MULTI_WINDOW)
#define MEOPT_MWFRAME   1       /* enable multiple window frames         */
#else
/* DO NOT CHANGE THIS VALUE */
#define MEOPT_MWFRAME   0
#endif

#ifdef _MOUSE
#define MEOPT_MOUSE     1       /* do mouse pointer stuff                */
#else
/* DO NOT CHANGE THIS VALUE */
#define MEOPT_MOUSE     0
#endif
#if MEOPT_SPAWN && (defined _IPIPES)
#define MEOPT_IPIPES    1       /* use incremental pipes                 */
#else
/* DO NOT CHANGE THIS VALUE */
#define MEOPT_IPIPES    0
#endif
#if MEOPT_IPIPES && (defined _CLIENTSERVER)
#define MEOPT_CLIENTSERVER 1    /* enable the client server              */
#else
/* DO NOT CHANGE THIS VALUE */
#define MEOPT_CLIENTSERVER 0
#endif
#ifdef _SOCKET
#define MEOPT_SOCKET    1       /* Supports sockets - can read urls      */
#else
/* DO NOT CHANGE THIS VALUE */
#define MEOPT_SOCKET    0
#endif

/* the name used on about etc           */
#define ME_FULLNAME  ME_MICROEMACS_FULLNAME
#define ME_SHORTNAME ME_MICROEMACS_SHORTNAME

#else
/*************************************************************************
 * NanoEmacs Configuration options - Cut down version                    *
 *************************************************************************/
#define MEOPT_TYPEAH    1       /* type ahead causes update to be skipped*/
#define MEOPT_DEBUGM    0       /* enable macro debugging $debug         */
#define MEOPT_TIMSTMP   0       /* Enable time stamping of files on write*/
#define MEOPT_COLOR     1       /* color commands and windows            */
#define MEOPT_HILIGHT   0       /* color hilighting                      */
#define MEOPT_ISEARCH   1       /* Incremental searches like ITS EMACS   */
#define MEOPT_WORDPRO   0       /* Advanced word processing features     */
#define MEOPT_FENCE     0       /* fench matching in CMODE               */
#define MEOPT_CRYPT     0       /* file encryption enabled?              */
#define MEOPT_MAGIC     1       /* include regular expression matching?  */
#define MEOPT_ABBREV    0       /* Abbreviated files                     */
#define MEOPT_FILENEXT  0       /* include the file next buffer stuff    */
#define MEOPT_RCS       0       /* do rcs check ins and outs             */
#define MEOPT_LOCALBIND 0       /* do local key binding (buffer)         */
#define MEOPT_PRINT     0       /* do printing of buffers and regions    */
#define MEOPT_SPELL     0       /* do spelling of words and buffers      */
#define MEOPT_UNDO      1       /* undo capability                       */
#define MEOPT_NARROW    0       /* enable narrowing functionality        */
#define MEOPT_REGISTRY  0       /* enable registry functionality         */
#define MEOPT_OSD       0       /* enable OSD functionality              */
#define MEOPT_DIRLIST   0       /* enable Directory tree and listing     */
#define MEOPT_FILEHOOK  0       /* enable File hook assigning            */
#define MEOPT_TAGS      0       /* enable TAGS support                   */
#define MEOPT_CMDHASH   0       /* use a hash table for command lookup   */
#define MEOPT_POSITION  0       /* enable set & goto position            */
#define MEOPT_EXTENDED  0       /* enable miscellaneous extended features*/
#define MEOPT_CALLBACK  0       /* enable macro callbacks                */
#define MEOPT_SPAWN     0       /* enable spawning                       */
#define MEOPT_SCROLL    0       /* enable scroll bars                    */
#define MEOPT_HSPLIT    0       /* enable vertical window                */ 
#define MEOPT_FRAME     0       /* enable multiple frames                */
#if MEOPT_FRAME && (defined _MULTI_WINDOW)
#define MEOPT_MWFRAME   0       /* enable multiple window frames         */
#else
/* DO NOT CHANGE THIS VALUE */
#define MEOPT_MWFRAME   0
#endif

#ifdef _MOUSE
#define MEOPT_MOUSE     0       /* do mouse pointer stuff                */
#else
/* DO NOT CHANGE THIS VALUE */
#define MEOPT_MOUSE     0
#endif
#if MEOPT_SPAWN && (defined _IPIPES)
#define MEOPT_IPIPES    0       /* use incremental pipes                 */
#else
/* DO NOT CHANGE THIS VALUE */
#define MEOPT_IPIPES    0
#endif
#if MEOPT_IPIPES && (defined _CLIENTSERVER)
#define MEOPT_CLIENTSERVER 0    /* enable the client server              */
#else
/* DO NOT CHANGE THIS VALUE */
#define MEOPT_CLIENTSERVER 0
#endif
#ifdef _SOCKET
#define MEOPT_SOCKET    0       /* Supports sockets - can read urls      */
#else
/* DO NOT CHANGE THIS VALUE */
#define MEOPT_SOCKET    0
#endif

/* the name used on about etc */
#define ME_FULLNAME  ME_NANOEMACS_FULLNAME
#define ME_SHORTNAME ME_NANOEMACS_SHORTNAME

#endif /* _NANOEMACS */

/*************************************************************************
 * ALL: Standard microemacs includes                                     *
 *************************************************************************/
#include <stdio.h>                      /* Always need this              */
#include <stdlib.h>                     /* Usually need this             */
#include <string.h>                     /* Usually need this             */
#include <errno.h>                      /* Need errno and sys_errlist    */

#ifdef _ME_WIN32_FULL_DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

/**************************************************************************
* USG[UNIX] : Include definitions. Assumed (near) POSIX compliance        *
**************************************************************************/
#if (defined _USG)
#include <signal.h>                     /* Required for signal(2)        */
#include <sys/stat.h>                   /* Required for stat(2) types    */
#include <unistd.h>
#endif

/**************************************************************************
* BSD[UNIX] : Include definitions (Support for BSD 4.2/3 only)            *
**************************************************************************/
#if (defined _BSD_43) || (defined _BSD42)
/* Additional definitions for BSD4.2 and BSD4.3 */
#define _BSD                1           /* This is version of BSD        */
#ifndef _POSIX_SIGNALS
#define _BSD_SIGNALS        1           /* Use BSD Signals               */
#endif  /* _POSIX_SIGNALS */
#endif  /* (defined _BSD_43) || (defined _BSD42) */

#ifdef _BSD
#include <signal.h>                     /* Required for signal(2)        */
#include <sys/stat.h>                   /* Required for stat(2) types    */
#include <sys/fcntl.h>                  /* Required for access(2) types  */
#include <sys/types.h>                  /* Required for seclect(2) types */
#include <unistd.h>
#endif

#ifdef _meDEF_SYS_ERRLIST
extern const char *sys_errlist[];
#endif

/* Standard Types */
typedef   signed char  meByte ;
typedef unsigned char  meUByte ;
typedef   signed short meShort ;
typedef unsigned short meUShort ;
typedef   signed int   meInt ;
typedef unsigned int   meUInt ;

/* Fix any default search path */
#ifndef _DEFAULT_SEARCH_PATH
#define _DEFAULT_SEARCH_PATH ""
#endif

#if MEOPT_BINFS
#include "bfs.h"        /* Binary file system definitions. */
#endif
#include "emode.h"      /* Mode enum, type & var defs    */
#include "estruct.h"    /* Type structure definitions    */
#include "edef.h"       /* Global variable definitions   */
#include "eterm.h"      /* platform terminal definitions */
#include "eextrn.h"     /* External function defintions  */

#endif /* __EMAIN_H__ */
