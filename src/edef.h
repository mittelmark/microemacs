/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * edef.h - Global variable definitions.
 *
 * Originally written by Dave G. Conroy for MicroEmacs
 * Copyright (C) 1988-2009 JASSPA (www.jasspa.com)
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
 * Created:     For MicroEMACS 3.2
 * Synopsis:    Global variable definitions.
 * Authors:     Dave G. Conroy, Steve Wilhite, George Jones, Daniel Lawrence,
 *          Jon Green & Steven Phillips
 */

extern  meDirList  curDirList ;
#if MEOPT_OSD
extern  struct osdDIALOG  *osdDialogHd; /* Root of the on screen displays */
extern  struct osdDISPLAY *osdDisplayHd;/* OSD dialog display list      */
#endif
extern  meRegister *meRegHead;          /* The register Head            */
extern  meRegister *meRegCurr;          /* The current register set     */
extern  meSelection selhilight;         /* Selection hilight            */
extern  meSchemeSet  *hilBlock;         /* Hilighting colour changes    */
extern  meColor    noColors ;           /* No defined colours           */
extern  meInt      styleTableSize;      /* Size of the colour table     */
extern  meStyle   *styleTable;          /* Highlighting colour table    */
extern  meStyle    defaultScheme[] ;    /* Default colour scheme        */
#if MEOPT_ABBREV
extern  meAbbrev  *globalAbbrevFile ;   /* Global Abreviation file      */
#endif
#if MEOPT_POSITION
extern  mePosition *position ;          /* Position stack head          */
#endif
extern  meUShort   hilBlockS ;          /* Hilight - HilBlock array siz */
extern  meInt      cursorState ;        /* Current state of cursor      */
extern  meUByte   *meProgName ;           /* the program name (argv[0])   */
extern  meUByte  **ModeLineStr ;        /* modeline line format         */
extern  meUByte    orgModeLineStr[] ;   /* original modeline line format*/
extern  meUByte   *modeLineStr ;        /* current modeline format      */
extern  meUByte    modeLineFlags ;      /* current modeline flags       */
extern  meUInt     fileSizePrompt;      /* size at which to start prompt*/
extern  meInt      keptVersions;        /* No. of kept backup versions  */
#if MEOPT_EXTENDED
extern  meInt      nextFrameId;         /* frame-id of the next create  */
extern  meInt      nextWindowId;        /* window-id of the next create */
extern  meShort    pauseTime;           /* Fence matching delay length  */
#else
#define pauseTime 2000
#endif
extern  meInt      autoTime;            /* auto save time in seconds    */
#if MEOPT_MOUSE
extern  meUInt     delayTime;           /* mouse-time delay time        */
extern  meUInt     repeatTime;          /* mouse-time repeat time       */
#endif
#if MEOPT_CALLBACK
extern  meUInt     idleTime;            /* idle-time delay time         */
#endif
#if MEOPT_WORDPRO
extern  meUByte    fillbullet[];        /* Fill bullet character class  */
extern  meUByte    fillbulletlen;       /* Fill lookahead limit         */
extern  meUShort   fillcol;             /* Fill column                  */
extern  meUByte    filleos[];           /* Fill E-O-S character class   */
extern  meUByte    filleoslen;          /* Fill E-O-S ' ' insert len    */
extern  meUByte    fillignore[];        /* Fill Ignore character class  */
extern  meUByte    fillmode;            /* Justify mode                 */
#endif
extern  meUByte    indentWidth;             /* Virtual Tab size             */
extern  meUByte    tabWidth;            /* Real TAB size                */
extern  meUByte   *homedir;             /* Home directory               */
extern  meUByte   *searchPath;          /* emf search path              */
extern  meUByte   *curdir;              /* current working directory    */
extern  meUByte   *curFuncName ;        /* Current macro command name   */
extern  meUByte   *execstr;             /* pointer to string to execute */
extern  int        execlevel;           /* execution IF level           */
extern  int        bufHistNo;           /* inc buff hist numbering      */
extern  int        lastCommand ;        /* The last user executed key   */
extern  int        lastIndex ;          /* The last user executed comm  */
extern  int        thisCommand ;        /* The cur. user executed key   */
extern  int        thisIndex ;          /* The cur. user executed comm  */
extern  meUByte    hexdigits[];
extern  meUShort   keyTableSize;        /* The current size of the key table */
extern  meBind     keytab[];            /* key bind to functions table  */
extern  meUByte    quietMode ;          /* quiet mode (0=bell)          */
extern  meUByte    scrollFlag ;         /* horiz/vert scrolling method  */
extern  meUByte    sgarbf;              /* State of screen unknown      */
extern  meUByte    clexec;              /* command line execution flag  */
extern  meUByte    mcStore;             /* storing text to macro flag   */
extern  meUByte    cmdstatus;           /* last command status          */
extern  meUByte    kbdmode;             /* current keyboard macro mode  */
#if MEOPT_DEBUGM
extern  meUByte    macbug;              /* macro debuging flag          */
#endif
extern  meUByte    thisflag;            /* Flags, this command          */
extern  meUByte    lastflag;            /* Flags, last command          */
extern  meUByte    lastReplace;         /* set to non-zero if last was a replace */

extern  meUInt  meSystemCfg;            /* ME system config variable    */
#define meSYSTEM_CONSOLE    0x000001    /* This is a console version    */
#define meSYSTEM_RGBCOLOR   0x000002    /* System has definable rgb color */
#define meSYSTEM_ANSICOLOR  0x000004    /* Ansi standard color (8 colors) */
#define meSYSTEM_XANSICOLOR 0x000008    /* Extended Ansi colors (16)    */
#define meSYSTEM_FONTS      0x000010    /* Use termcap fonts (bold etc) */
#define meSYSTEM_OSDCURSOR  0x000020    /* Show cursor in osd dialogs   */
#define meSYSTEM_UNIXSYSTEM 0x000080    /* This is a unix sys           */
#define meSYSTEM_MSSYSTEM   0x000100    /* This is a dos or win32 sys   */
#define meSYSTEM_DRIVES     0x000200    /* This system has drives (C:\) */
#define meSYSTEM_DOSFNAMES  0x000400    /* Dos 8.3 file name restriction*/
#define meSYSTEM_IPIPES     0x000800    /* The system supports ipipes   */
#define meSYSTEM_TABINDANY  0x001000    /* Tab key indents from any pos */
#define meSYSTEM_ALTMENU    0x002000    /* Enable Alt as menu hot key   */
#define meSYSTEM_ALTPRFX1   0x004000    /* Enable Alt as prefix1        */
#define meSYSTEM_KEEPUNDO   0x008000    /* Keep undo history after save */
#define meSYSTEM_FONTFIX    0x010000    /* Enables ANSI 0-31 rendering  */
#define meSYSTEM_CLNTSRVR   0x020000    /* Enables the client server    */
#define meSYSTEM_CTCHASPC   0x040000    /* Enables win32 catch A-space  */
#define meSYSTEM_SHOWWHITE  0x080000    /* Display TAB, CR & space chars*/
#define meSYSTEM_HIDEBCKUP  0x100000    /* Hide backup files            */
#define meSYSTEM_TABINDFST  0x200000    /* Tab key indents first col pos*/
#define meSYSTEM_NOEMPTYANK 0x400000    /* Don't allow empty yank (ext) */
#define meSYSTEM_NOCLIPBRD  0x800000    /* Don't use the sys clip-board */
#define meSYSTEM_PIPEDMODE  0x1000000   /* -p or -P piped mode          */
#define meSYSTEM_NOALTSBUF  0x10000000  /* No Alternative screen buffer */

#ifdef _UNIX
#if MEOPT_CLIENTSERVER
#define meSYSTEM_MASK (meSYSTEM_ANSICOLOR|meSYSTEM_FONTS|meSYSTEM_OSDCURSOR|meSYSTEM_DOSFNAMES|meSYSTEM_IPIPES|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1|meSYSTEM_KEEPUNDO|meSYSTEM_FONTFIX|meSYSTEM_CLNTSRVR|meSYSTEM_SHOWWHITE|meSYSTEM_HIDEBCKUP|meSYSTEM_TABINDFST|meSYSTEM_NOEMPTYANK|meSYSTEM_NOCLIPBRD)
#else
#define meSYSTEM_MASK (meSYSTEM_ANSICOLOR|meSYSTEM_FONTS|meSYSTEM_OSDCURSOR|meSYSTEM_DOSFNAMES|meSYSTEM_IPIPES|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1|meSYSTEM_KEEPUNDO|meSYSTEM_FONTFIX|meSYSTEM_SHOWWHITE|meSYSTEM_HIDEBCKUP|meSYSTEM_TABINDFST|meSYSTEM_NOEMPTYANK|meSYSTEM_NOCLIPBRD)
#endif
#endif
#ifdef _DOS
#define meSYSTEM_MASK (meSYSTEM_OSDCURSOR|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1|meSYSTEM_KEEPUNDO|meSYSTEM_SHOWWHITE|meSYSTEM_HIDEBCKUP|meSYSTEM_TABINDFST|meSYSTEM_NOEMPTYANK)
#endif
#ifdef _WIN32
extern  meUInt meSYSTEM_MASK;           /* ME system mask - dependant on win32 flavour */
#endif

/* the $mouse variables always exist so the C variables must */
#define meMOUSE_NOBUTTONS   0x0000f     /* # mouse buttons              */
#define meMOUSE_ENBLE       0x00010     /* mouse is enabled             */
#define meMOUSE_SWAPBUTTONS 0x00020     /* swap mouse buttons           */
#define meMOUSE_ICON        0xf0000     /* Mask to set the mouse pointer icon */
extern  meUInt meMouseCfg;              /* ME mouse config variable     */
extern  meUByte mouse_pos;              /* mouse virtual position       */
extern  meShort mouse_X;                /* mouse X pos at last event    */
extern  meShort mouse_Y;                /* mouse X pos at last event    */
#if MEOPT_MOUSE
extern  meShort mouse_dX;               /* mouse delta X last event pos */
extern  meShort mouse_dY;               /* mouse delta X last event pos */
#endif

#if MEOPT_UNDO
extern  meUInt undoContFlag;            /* continuation of an undo      */
#endif

/* File Modes */
#if (defined _DOS) || (defined _WIN32)
extern  meUShort meUmask ;              /* file creation umask          */
#endif
#ifdef _UNIX
extern  meUShort  meUmask ;             /* file creation umask          */
extern  mode_t    meXUmask ;            /* directory creation umask     */
extern  uid_t     meUid ;               /* current user id              */
extern  gid_t     meGid ;               /* current group id             */
extern  int       meGidSize ;           /* number of groups the user belongs to   */
extern  gid_t    *meGidList ;           /* list of group ids the user belongs to  */
#endif /* _UNIX */

/* Signals */
#ifdef _UNIX
#ifdef _POSIX_SIGNALS
extern  sigset_t  meSigHoldMask ;       /* signals held when spawning and reading */
#endif /* _POSIX_SIGNALS */
#ifdef _BSD_SIGNALS
extern int meSigHoldMask;               /* signals held when spawning and reading */
#endif /* _BSD_SIGNALS */
extern int meSigLevel ;                 /* signal level */
#endif /* _UNIX */

/* Environment Variables */
#if (defined _UNIX) && (defined _NOPUTENV)
extern char    **meEnviron;             /* Our own environment */
#endif /* (defined _UNIX) && (defined _NOPUTENV) */

#if MEOPT_IPIPES
/* Incremental pipe variables */
extern meIPipe  *ipipes ;               /* list of all the current pipes*/
extern int       noIpipes ;             /* count of all the cur pipes   */
#endif

#define meALARM_DIE          0x01
#define meALARM_WINSIZE      0x02
#define meALARM_VARIABLE     0x04
#define meALARM_INITIALIZED  0x08
#define meALARM_PIPED        0x10
#define meALARM_PIPED_QUIET  0x20
#define meALARM_PIPE_COMMAND 0x40

extern  meUByte   alarmState;           /* Auto-save alarm time         */
extern  meInt     startTime;            /* me start time. used as offset*/
extern  meInt     meGetKeyFirst;        /* initial command, set by respawn() */
extern  meLine   *lpStore;              /* line off which to store macro*/
extern  meBuffer *lpStoreBp ;           /* help is stored in a buffer   */
extern  meUShort  thiskey;              /* the current key              */
extern  meUShort  prefixc[];            /* prefix chars           */
extern  meUShort  reptc;                /* current universal repeat char*/
extern  meUShort  breakc;               /* current abort-command char*/
#ifdef _CLIPBRD
/* When implementing the clipboard it is best if the clipboard is only
 * attempted to be got or set once per command. This is because of other
 * window interaction.
 *
 * If the command is a macro the the following nasty things can happen if this
 * is not so
 *
 * 1) The 1st yank in a macro may return a different value to the 2nd yank, if
 *    the user defines another region in a different window.
 *
 * 2) If the macro is looping round cutting and pasting then the user will now
 *    be able to reliably cut 'n' paste in another window.
 *
 * 3) A macro could cut a region and the paste a different one!
 *
 * Nasty. Try and ensure these don't happen.
 */

#define CLIP_OWNER      0x01            /* We are the owners of clipbrd */
#define CLIP_STALE      0x02            /* Clipboard is stale           */
#define CLIP_IGNORE_DC  0x04            /* Ignore the next DESTROYCLIP  */
#define CLIP_RECEIVING  0x08            /* Currently receiving clipboard*/
#define CLIP_RECEIVED   0x10            /* Clipboard has been obtained  */
#define CLIP_DISABLED   0x20            /* Currently disabled Start&end */
extern  meUByte   clipState;            /* clipboard status flag        */
#endif
extern  meUInt    cursorBlink;          /* cursor-blink blink time      */
extern  int       blinkState;           /* cursor blink state           */
#if MEOPT_COLOR
extern  meColor   cursorColor;          /* cursor-color scheme          */
extern  meScheme  mlScheme;             /* Message line color scheme    */
extern  meScheme  mdLnScheme;           /* Mode line color scheme       */
extern  meScheme  sbarScheme;           /* Scroll bar color scheme      */
extern  meScheme  globScheme;           /* Global color scheme          */
extern  meScheme  trncScheme;           /* truncate color scheme        */
#if MEOPT_OSD
extern  meScheme  osdScheme;            /* Menu line color scheme       */
#endif
#endif

extern  int       gsbarmode;            /* global scroll bar mode       */
extern  meUByte   boxChars [];          /* Array of box characters      */
extern  meUByte   windowChars [];       /* Array of window characters   */
extern  meUByte   displayTab;           /* tab \t display character */
extern  meUByte   displayNewLine;       /* new-line \n display character */
extern  meUByte   displaySpace;         /* space ' ' display character */

extern  meUByte  *envars[];             /* list of recognized env. vars */
extern  meUByte  *derNames[];           /* name list of directives      */
extern  meUByte   derTypes[];           /* type list of directives      */
extern  meUByte  *funcNames[];          /* name list of user funcs      */
extern  meUByte   funcTypes[];          /* type list of user funcs      */
extern  meKillNode *kbufp;              /* current kill buffer chunk pointer */
extern  meKillNode *kbufh;              /* kill buffer header pointer   */
extern  meUByte   lkbdptr[];            /* Holds last keybd macro data  */
extern  int       lkbdlen;              /* Holds length of last macro   */
extern  meUByte  *kbdptr;               /* start of current keyboard buf*/
extern  int       kbdlen;               /* len of current macro         */
extern  int       kbdoff;               /* current offset of macro      */
extern  int       kbdrep;               /* number of repetitions        */
extern  meUByte   emptym[];             /* empty literal                */
extern  meUByte   errorm[];             /* error literal                */
extern  meUByte   abortm[];             /* abort literal                */
extern  meUByte   truem[];              /* true literal         */
extern  meUByte   falsem[];             /* false litereal               */

/* global buffer names */
extern meUByte    BvariablesN[];
extern meUByte    BbindingsN[];
extern meUByte    BtranskeyN[];
extern meUByte    BcompleteN[];
extern meUByte    BcommandsN[];
extern meUByte    BicommandN[];
extern meUByte    BregistryN[];
extern meUByte    BbuffersN[];
extern meUByte    BcommandN[];
extern meUByte    BolhelpN[];
extern meUByte    BserverN[];
extern meUByte    BaboutN[];
extern meUByte    BstdinN[];
extern meUByte    BmainN[];
extern meUByte    BhelpN[];

#if MEOPT_EXTENDED
extern meVarList  usrVarList ;          /* user variables list          */
extern meUByte   *fileIgnore ;
extern meUByte   *frameTitle ;          /* String used in frame title   */
#endif

extern meUByte   *meUserName ;
extern meUByte   *meUserPath ;

#if MEOPT_FILENEXT
extern meUByte   *flNextFileTemp ;
extern meUByte   *flNextLineTemp ;

extern meUByte    noNextLine ;
extern meUByte  **nextName ;
extern meUByte   *nextLineCnt ;
extern meUByte ***nextLineStr ;
#endif

#if MEOPT_RCS
extern meUByte   *rcsFile ;
extern meUByte   *rcsCoStr ;
extern meUByte   *rcsCoUStr ;
extern meUByte   *rcsCiStr ;
extern meUByte   *rcsCiFStr ;
extern meUByte   *rcsUeStr ;
#endif

/* history variables used in meGetStringFromUser */
extern  meUByte   numStrHist ;          /* curent no. of hist. strings    */
extern  meUByte   numBuffHist ;         /* curent no. of hist. buff names */
extern  meUByte   numCommHist ;         /* curent no. of hist. comm names */
extern  meUByte   numFileHist ;         /* curent no. of hist. file names */
extern  meUByte   numSrchHist ;         /* curent no. of hist. search strs*/
extern  meUByte **strHist ;             /* string history list            */
extern  meUByte **buffHist ;            /* etc.                           */
extern  meUByte **commHist ;
extern  meUByte **fileHist ;
extern  meUByte **srchHist ;

#if MEOPT_HILIGHT
extern meUByte     noHilights ;
extern meHilight **hilights ;
extern meUByte     noIndents ;
extern meHilight **indents ;
#endif

#if MEOPT_LOCALBIND
extern meUByte    useMlBinds;           /* flag to use ml bindings      */
extern meUShort   mlNoBinds;            /* no. of message line bindings */
extern meBind    *mlBinds;              /* pointer to ml local bindings */
#endif

#if MEOPT_FRAME
extern meFrame *frameList ;
#if MEOPT_MWFRAME
extern meFrame *frameFocus ;
#endif
#endif
extern meFrame *frameCur ;

extern meUByte *disLineBuff ;           /* interal display buffer array */
extern int      disLineSize ;           /* interal display buffer size  */

/* uninitialized global external declarations */
extern meUByte    resultStr[meBUF_SIZE_MAX] ;   /* $result variable             */
extern meUByte    evalResult[meTOKENBUF_SIZE_MAX] ;/* Result string from functions */
extern int        curgoal;              /* Goal for C-P, C-N            */
extern meBuffer  *bheadp;               /* Head of list of buffers      */
#if MEOPT_ABBREV
extern meAbbrev  *aheadp;               /* Head of list of abrev files  */
#endif
extern struct meKill *klhead;           /* Head of klist                */

/* the character lookup tables */
extern meUByte    charMaskTbl1[] ;
extern meUByte    charMaskTbl2[] ;
extern meUByte    charCaseTbl[] ;
extern meUByte    charLatinUserTbl[] ;
extern meUByte    charUserLatinTbl[] ;
extern meUByte    charMaskFlags[] ;
extern meUByte   *charKeyboardMap ;
extern meUByte    isWordMask ;

/* the following are global variables but not defined in this file */
extern int        screenUpdateDisabledCount ;
extern meUByte    meCopyright[] ;

/* fileio file pointer */
#if 0
#ifdef _WIN32
extern HANDLE     ffrp;
extern HANDLE     ffwp;
#else
extern FILE      *ffrp;
extern FILE      *ffwp;
#endif
#endif
extern meIo       meio;                 /* The current I/O operation    */

#if MEOPT_OSD
extern int        osdCol ;              /* The osd current column */
extern int        osdRow ;              /* The osd current row */
#endif

#define MLSTATUS_KEEP    0x01
#define MLSTATUS_RESTORE 0x02
#define MLSTATUS_POSML   0x04
#define MLSTATUS_POSOSD  0x08
#define MLSTATUS_CLEAR   0x10
#define MLSTATUS_NINPUT  0x20
#define MLSTATUS_OSDPOS  0x40

#ifdef _DRAGNDROP
extern struct s_DragAndDrop *dadHead;   /* Drag and drop list */
#endif

/**************************************************************************
* Constant declarations for the external definitions above. These are     *
* only processed in main.c                                                *
**************************************************************************/
#ifdef  maindef

/* for MAIN.C */

/* initialized global definitions */
#ifdef _INSENSE_CASE
meDirList curDirList={0,0,NULL,NULL,0,NULL} ;
#else
meDirList curDirList={1,0,NULL,NULL,0,NULL,0} ;
#endif
#if MEOPT_OSD
struct osdDIALOG  *osdDialogHd = NULL;  /* Root of the on screen displays */
struct osdDISPLAY *osdDisplayHd = NULL; /* Menu display list */
#endif
meRegister *meRegHead=NULL ;            /* The register Head            */
meRegister *meRegCurr=NULL ;            /* The current register set     */
meSelection selhilight={1,0} ;          /* Selection hilight            */
meUShort  hilBlockS=20 ;                /* Hilight - HilBlock array siz */
meStyle  *styleTable = NULL;            /* Highlighting colour table    */
meStyle defaultScheme[meSCHEME_STYLES*2] = /* Default colour scheme     */
{
    meSTYLE_NDEFAULT,meSTYLE_RDEFAULT,            /* Normal style */
    meSTYLE_NDEFAULT,meSTYLE_RDEFAULT,            /* Current style */
    meSTYLE_NDEFAULT,meSTYLE_RDEFAULT,            /* Region style */
    meSTYLE_NDEFAULT,meSTYLE_RDEFAULT,            /* Current region style */
    meSTYLE_RDEFAULT,meSTYLE_NDEFAULT,            /* Normal style */
    meSTYLE_RDEFAULT,meSTYLE_NDEFAULT,            /* Current style */
    meSTYLE_RDEFAULT,meSTYLE_NDEFAULT,            /* Region style */
    meSTYLE_RDEFAULT,meSTYLE_NDEFAULT,            /* Current region style */
};
#if MEOPT_ABBREV
meAbbrev *globalAbbrevFile = NULL ;     /* Global Abreviation file      */
#endif
#if MEOPT_POSITION
mePosition *position=NULL ;             /* Position stack head          */
#endif
meColor   noColors=0 ;                  /* No defined colours           */
meInt     styleTableSize=2 ;            /* Size of the style table      */
meSchemeSet *hilBlock;                  /* Hilighting style change      */
meInt     cursorState=0 ;               /* Current state of cursor      */
meUByte  *meProgName=NULL ;               /* the program name (argv[0])   */
meUByte   orgModeLineStr[]="%s%r%u " ME_SHORTNAME " (%e) - %l %b (%f) ";
meUByte  *modeLineStr=orgModeLineStr;   /* current modeline format      */
meInt     autoTime=300 ;                /* auto save time in seconds    */
#if MEOPT_WORDPRO
meUByte   fillbullet[16]="*)].-";       /* Fill bullet character class  */
meUByte   fillbulletlen = 15;           /* Fill lookahead limit         */
meUShort  fillcol = 78;                 /* Current fill column          */
meUByte   filleos[16]=".!?";            /* Fill E-O-S character class   */
meUByte   filleoslen=1;                 /* Fill E-O-S ' ' insert len    */
meUByte   fillignore[16]=">_@";         /* Fill Ignore character class  */
meUByte   fillmode='B';                 /* Justification mode           */
#endif
#if MEOPT_EXTENDED
meUInt    fileSizePrompt=1024;          /* size at which to start prompt*/
meInt     keptVersions=0 ;              /* No. of kept backup versions  */
meInt     nextFrameId=0 ;               /* frame-id of the next create  */
meInt     nextWindowId=0 ;              /* window-id of the next create */
meShort   pauseTime = 2000;             /* Fence matching sleep length  */
#endif
meUByte   indentWidth = 4;              /* Virtual Tab size             */
meUByte   tabWidth = 8;                 /* Real TAB size                */
meUByte  *searchPath=NULL;              /* emf search path              */
meUByte  *homedir=NULL;                 /* Home directory               */
meUByte  *curdir=NULL;                  /* current working directory    */
meUByte  *execstr = NULL;               /* pointer to string to execute */
#if MEOPT_MOUSE
meUInt    delayTime = 500;              /* mouse-time delay time 500ms  */
meUInt    repeatTime = 25;              /* mouse-time repeat time 25ms  */
#endif
#if MEOPT_CALLBACK
meUInt    idleTime = 1000;              /* idle-time delay time 1sec    */
#endif
int       execlevel = 0;                /* execution IF level           */
int       bufHistNo = 1;                /* inc buff hist numbering      */
int       lastCommand = 0 ;             /* The last user executed key   */
int       lastIndex = -1 ;              /* The last user executed comm  */
int       thisCommand = 0 ;             /* The cur. user executed key   */
int       thisIndex = -1 ;              /* The cur. user executed comm  */

#if 0
#ifdef _WIN32
HANDLE    ffrp;                         /* File read pointer, all func. */
HANDLE    ffwp;                         /* File write pointer, all func.*/
#else
FILE     *ffrp;                         /* File read pointer, all func. */
FILE     *ffwp;                         /* File write pointer, all func.*/
#endif
#endif
meIo      meio;                         /* The current I/O operation    */
meUShort  thiskey ;                     /* the current key              */
meUByte   hexdigits[] = "0123456789ABCDEF";
meUInt    cursorBlink = 0;              /* cursor-blink blink time      */
int       blinkState=1;                 /* cursor blink state           */
meColor   cursorColor=meCOLOR_FDEFAULT; /* cursor color                 */
#if MEOPT_OSD
meScheme  osdScheme =meSCHEME_NDEFAULT; /* Menu line color scheme       */
#endif
meScheme  mlScheme  =meSCHEME_NDEFAULT; /* Message line color scheme    */
meScheme  mdLnScheme=meSCHEME_RDEFAULT; /* Mode line color scheme       */
meScheme  sbarScheme=meSCHEME_RDEFAULT; /* Scroll bar color scheme      */
meScheme  globScheme=meSCHEME_NDEFAULT; /* Global color scheme          */
meScheme  trncScheme=meSCHEME_NDEFAULT; /* Truncate color scheme        */
int       gsbarmode = (WMUP |           /* Has upper end cap            */
                       WMDOWN |         /* Has lower end cap            */
                       WMBOTTM |        /* Has a mode line character    */
                       WMSCROL |        /* Has a box on scrolling shaft */
                       WMRVBOX          /* Reverse video on box         */
#if MEOPT_MOUSE                         /* only enable scroll bar by    */
                                        /* default if mouse is supported*/
                       | WMSPLIT        /* Has a splitter               */
                       | WMVBAR         /* Window has a vertical bar    */
#endif
                      );                /* global scroll bar mode       */
meUByte   boxChars[BCLEN+1] =           /* Set of box characters        */
"|+++++++++-";
meUByte   windowChars[WCLEN+1] =        /* Set of window characters     */
{
    '=',                                /* Mode line current sep */
    '-',                                /* Mode libe inactive sep */
    '#',                                /* Root indicator */
    '*',                                /* Edit indicator */
    '%',                                /* View indicator */
    /* Single scroll bar */
    '=',                                /* Buffer split */
    '^',                                /* Scroll bar up */
    ' ',                                /* Scroll bar up shaft */
    ' ',                                /* Scroll box */
    ' ',                                /* Scroll bar down shaft */
    'v',                                /* Scroll bar down */
    '*',                                /* Scroll bar/mode line point */
    /* Double scroll bar */
    '=','=',                            /* Buffer split */
    '^','^',                            /* Scroll bar up */
    ' ',' ',                            /* Scroll bar up shaft */
    ' ',' ',                            /* Scroll box */
    ' ',' ',                            /* Scroll bar down shaft */
    'v','v',                            /* Scroll bar down */
    '*','*',                            /* Scroll bar/mode line point */
    /* Single horizontal scroll bar */
    '|',                                /* Buffer split */
    '<',                                /* Scroll bar left */
    ' ',                                /* Scroll bar left shaft */
    ' ',                                /* Scroll box */
    ' ',                                /* Scroll bar right shaft */
    '>',                                /* Scroll bar right */
    '*',                                /* Scroll bar/mode line point */
    /* Double horizontal scroll bar */
    '|','|',                            /* Buffer split */
    '<','<',                            /* Scroll bar left */
    ' ',' ',                            /* Scroll bar left shaft */
    ' ',' ',                            /* Scroll box */
    ' ',' ',                            /* Scroll bar right shaft */
    '>','>',                            /* Scroll bar right */
    '*','*',                            /* Scroll bar/mode line point */
    ' ',                                /* OSD title bar char */
    'x',                                /* OSD Title bar kill char */
    '*',                                /* OSD resize char */
    ' ',                                /* OSD button start char e.g. '['   */
    '>',                                /* OSD default button start char e.g. '>'   */
    ' ',                                /* OSD button close char e.g. ']'   */
    '<',                                /* OSD default button close char e.g. '<'   */
    /* Display characters */
    '>',                                /* tab \t display character     */
    '\\',                               /* new-line \n display character*/
    '.',                                /* space ' ' display character  */
    '$',                                /* text off screen to the left  */
    '$',                                /* text off screen to the right */
    '\\',                               /* split line char (ipipe)      */
    0
} ;
meUByte   displayTab=' ';               /* tab \t display character     */
meUByte   displayNewLine=' ';           /* new-line \n display character*/
meUByte   displaySpace=' ';             /* space ' ' display character  */
meInt     startTime;                    /* me start time used as offset */
meInt	  meGetKeyFirst=-1;             /* Push input key               */
meUByte   thisflag;                     /* Flags, this command          */
meUByte   lastflag;                     /* Flags, last command          */
meUByte   alarmState=0;                 /* Unix auto-save alarm time    */
meUByte   quietMode = 1 ;               /* quiet mode (0=bell)          */
meUByte   scrollFlag = 1 ;              /* horiz/vert scrolling method  */
meUByte   sgarbf = meTRUE;              /* meTRUE if screen is garbage  */
meUByte   clexec = meFALSE;             /* command line execution flag  */
meUByte   mcStore = meFALSE;            /* storing text to macro flag   */
#if MEOPT_DEBUGM
meUByte   macbug = 0 ;                  /* macro debuging flag          */
#endif
meUByte   cmdstatus = meTRUE;           /* last command status          */
meUByte   kbdmode=meSTOP;               /* current keyboard macro mode  */
meUByte   lastReplace=0;                /* set to non-zero if last was a replace */
meUByte   modeLineFlags=                /* current modeline flags       */
(WFMODE|WFRESIZE|WFMOVEL) ;
meUInt    meSystemCfg=                  /* ME system config variable    */
#ifdef _DOS
(meSYSTEM_CONSOLE|meSYSTEM_ANSICOLOR|meSYSTEM_XANSICOLOR|meSYSTEM_OSDCURSOR|meSYSTEM_MSSYSTEM|meSYSTEM_DRIVES|meSYSTEM_DOSFNAMES|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1) ;
#endif
#ifdef _WIN32
(meSYSTEM_RGBCOLOR|meSYSTEM_FONTS|meSYSTEM_OSDCURSOR|meSYSTEM_MSSYSTEM|meSYSTEM_DRIVES|meSYSTEM_DOSFNAMES|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1|meSYSTEM_CTCHASPC
#if MEOPT_IPIPES
|meSYSTEM_IPIPES
#endif
) ;
#endif
#ifdef _UNIX
(meSYSTEM_RGBCOLOR|meSYSTEM_FONTS|meSYSTEM_OSDCURSOR|meSYSTEM_UNIXSYSTEM|meSYSTEM_IPIPES|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1) ;
#endif
#ifdef _WIN32
meUInt    meSYSTEM_MASK=                /* ME system mask - dependant on win32 flavour */
(meSYSTEM_FONTS|meSYSTEM_OSDCURSOR|meSYSTEM_TABINDANY|meSYSTEM_ALTMENU|meSYSTEM_ALTPRFX1|meSYSTEM_KEEPUNDO|meSYSTEM_FONTFIX|meSYSTEM_CTCHASPC|meSYSTEM_SHOWWHITE|meSYSTEM_HIDEBCKUP|meSYSTEM_TABINDFST|meSYSTEM_NOEMPTYANK|meSYSTEM_NOCLIPBRD
#if !defined (_WIN32s)
|meSYSTEM_CLNTSRVR
#endif
) ;
#endif

#if MEOPT_MOUSE
meUInt    meMouseCfg=(3|meMOUSE_ENBLE); /* ME mouse config variable     */
meShort   mouse_dX = 0;                 /* mouse delta X last event pos */
meShort   mouse_dY = 0;                 /* mouse delta X last event pos */
#else
meUInt    meMouseCfg=3;                 /* ME mouse config variable - mouse not enabled */
#endif
meUByte   mouse_pos=0xff;               /* mouse virtual position       */
meShort   mouse_X = 0;                  /* mouse X pos at last event    */
meShort   mouse_Y = 0;                  /* mouse X pos at last event    */

#if MEOPT_UNDO
meUInt    undoContFlag=0;               /* continuation of an undo      */
#endif

/* File Modes */
#if (defined _DOS) || (defined _WIN32)
meUShort  meUmask=0x020 ;               /* file creation umask          */
#endif
#ifdef _UNIX
meUShort  meUmask ;                     /* file creation umask          */
mode_t    meXUmask ;                    /* directory creation umask     */
uid_t     meUid ;                       /* current user id              */
gid_t     meGid ;                       /* current group id             */
int       meGidSize ;                   /* number of groups the user belongs to   */
gid_t    *meGidList=NULL ;              /* list of group ids the user belongs to  */
#endif /* _UNIX */

/* Signals */
#ifdef _UNIX
#ifdef _POSIX_SIGNALS
sigset_t  meSigHoldMask ;               /* signals help when spawning and reading */
#endif /* _POSIX_SIGNALS */
#ifdef _BSD_SIGNALS
int       meSigHoldMask = 0;            /* signals held when spawning and reading */
#endif /* _BSD_SIGNALS */
int       meSigLevel = 0;               /* signal level */
#endif /* _UNIX */

/* Environment Variables */
#if (defined _UNIX) && (defined _NOPUTENV)
char    **meEnviron = NULL;             /* Our own environment */
#endif /* (defined _UNIX) && (defined _NOPUTENV) */

#if MEOPT_IPIPES
meIPipe  *ipipes=NULL ;                 /* list of all the current pipes*/
int       noIpipes=0 ;                  /* count of all the cur pipes   */
#endif

#if MEOPT_OSD
int       osdCol=0 ;                    /* The osd current column */
int       osdRow=0 ;                    /* The osd current row */
#endif
meLine     *lpStore = NULL;             /* line off which to store macro*/
meBuffer   *lpStoreBp = NULL;           /* help is stored in a buffer   */
meUShort  prefixc[ME_PREFIX_NUM+1]=
{ ME_INVALID_KEY,                       /* unused 0th value             */
  ME_SPECIAL|SKEY_esc,                  /* prefix1 = Escape             */
  'X'-'@',                              /* prefix2 = ^X                 */
  'H'-'@',                              /* prefix3 = ^H                 */
  ME_INVALID_KEY, ME_INVALID_KEY,       /* rest unused                  */
  ME_INVALID_KEY, ME_INVALID_KEY,
  ME_INVALID_KEY, ME_INVALID_KEY,
  ME_INVALID_KEY, ME_INVALID_KEY,
  ME_INVALID_KEY, ME_INVALID_KEY,
  ME_INVALID_KEY, ME_INVALID_KEY,
  ME_INVALID_KEY
} ;
meUShort  reptc    = 'U'-'@';           /* current universal repeat char*/
meUShort  breakc   = 'G'-'@';           /* current abort-command char*/

meKill    *klhead  = NULL;              /* klist header pointer            */
meUByte   lkbdptr[meBUF_SIZE_MAX];      /* Holds last keybd macro data     */
int       lkbdlen=0;                    /* Holds length of last macro      */
meUByte  *kbdptr=NULL;                  /* start of current keyboard buf   */
int       kbdlen=0;                     /* len of current macro            */
int       kbdoff=0;                     /* current offset of macro         */
int       kbdrep=0;                     /* number of repetitions           */
meUByte   emptym[]  = "";               /* empty literal                   */
meUByte   errorm[]  = "ERROR";          /* error literal                   */
meUByte   abortm[]  = "ABORT";          /* abort literal                   */
meUByte   truem[]   = "1";              /* true literal            */
meUByte   falsem[]  = "0";              /* false litereal                  */

/* global buffer names */
meUByte   BvariablesN[] = "*variables*" ;
meUByte   BbindingsN[] = "*bindings*" ;
meUByte   BcompleteN[] = "*complete*" ;
meUByte   BcommandsN[] = "*commands*" ;
meUByte   BcommandN[] = "*command*" ;
meUByte   BbuffersN[] = "*buffers*" ;
meUByte   BmainN[] = "*scratch*" ;
meUByte   BaboutN[] = "*about*" ;
#if MEOPT_EXTENDED
meUByte   BtranskeyN[] = "*translate-key*" ;
meUByte   BolhelpN[] = "*on-line help*" ;
meUByte   BstdinN[] = "*stdin*" ;
meUByte   BhelpN[] = "*help*" ;
#endif
#if MEOPT_IPIPES
meUByte   BicommandN[] = "*icommand*" ;
#endif
#if MEOPT_REGISTRY
meUByte   BregistryN[] = "*registry*" ;
#endif
#if MEOPT_CLIENTSERVER
meUByte   BserverN[] = "*server*" ;
#endif

#if MEOPT_EXTENDED
meVarList usrVarList={NULL,0} ;         /* user variables list             */
meUByte  *fileIgnore=NULL ;
meUByte  *frameTitle=NULL ;             /* String used in frame title   */
#endif

meUByte  *meUserName=NULL ;
meUByte  *meUserPath=NULL ;

#if MEOPT_FILENEXT
meUByte  *flNextFileTemp=NULL ;
meUByte  *flNextLineTemp=NULL ;

meUByte    noNextLine=0 ;
meUByte  **nextName=NULL ;
meUByte   *nextLineCnt=NULL ;
meUByte ***nextLineStr=NULL ;
#endif

#if MEOPT_RCS
meUByte  *rcsFile=NULL ;
meUByte  *rcsCoStr=NULL ;
meUByte  *rcsCoUStr=NULL ;
meUByte  *rcsCiStr=NULL ;
meUByte  *rcsCiFStr=NULL ;
meUByte  *rcsUeStr=NULL ;
#endif

/* history variables used in meGetStringFromUser */
meUByte   numStrHist = 0 ;              /* curent no. of hist. strings    */
meUByte   numBuffHist = 0 ;             /* curent no. of hist. buff names */
meUByte   numCommHist = 0 ;             /* curent no. of hist. comm names */
meUByte   numFileHist = 0 ;             /* curent no. of hist. file names */
meUByte   numSrchHist = 0 ;             /* curent no. of hist. search strs*/
meUByte **strHist ;                     /* string history list            */
meUByte **buffHist ;                    /* etc.                           */
meUByte **commHist ;
meUByte **fileHist ;
meUByte **srchHist ;

#if MEOPT_HILIGHT
meUByte     noHilights=0 ;
meHilight **hilights=NULL ;
meUByte     noIndents=0 ;
meHilight **indents=NULL ;
#endif

#if MEOPT_LOCALBIND
meUByte   useMlBinds=0;                 /* flag to use ml bindings      */
meUShort  mlNoBinds=0;                  /* no. of message line bindings */
meBind   *mlBinds=NULL;                 /* pointer to ml local bindings */
#endif

#if MEOPT_FRAME
meFrame *frameList=NULL ;
#if MEOPT_MWFRAME
meFrame *frameFocus=NULL ;
#endif
#endif
meFrame *frameCur=NULL ;

meUByte   *disLineBuff=NULL ;           /* interal display buffer array */
int        disLineSize=512 ;            /* interal display buffer size  */

int       curgoal;                      /* Goal for C-P, C-N            */
meBuffer   *bheadp;                     /* Head of list of buffers      */
#if MEOPT_ABBREV
meAbbrev *aheadp=NULL;                  /* Head of list of abrev files  */
#endif
meUShort  keyTableSize;                 /* The current size of the key table */
meUByte   resultStr[meBUF_SIZE_MAX];    /* Result string from commands  */

#ifdef _CLIPBRD
meUByte   clipState=CLIP_DISABLED;      /* clipboard status flag        */
#endif

#ifdef _DRAGNDROP
struct s_DragAndDrop *dadHead = NULL;   /* Drag and drop list           */
#endif

#if MEOPT_BINFS
bfs_t bfsdev = NULL;                    /* Built-in File system mount point. */
#endif
#endif
