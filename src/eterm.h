/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * eterm.h - Terminal interface definitions.
 *
 * Copyright (C) 1994-2009 JASSPA (www.jasspa.com)
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
 * Created:     1994
 * Synopsis:    Terminal interface definitions.
 * Authors:     Steven Phillips & Jon Green
 * Notes:
 *     every system must provide the following variables / constants
 *
 *        frameCur->depthMax    - maximum number of rows
 *        frameCur->depth    - current number of rows
 *        frameCur->widthMax    - maximum number of columns
 *        frameCur->width    - current number of columns
 *        TTpause   - # times thru update to pause
 *
 *     every system must provide the following functions
 *
 *        TTstart   - Creates the terminal (called once at start)
 *        TTend     - Destroyes the terminal (called once at end)
 *        TTopen    - opens the terminal called after any shell call.
 *        TTclose   - closes the terminal called before any shell call.
 *        TTwaitForChar - gets a character
 *        TTflush   - flushes the output terminal
 *        TTmove    - most the cursor to a given position
 *        TThideCur - hide the cursor
 *        TTshowCur - show the cursor
 *        TTNbell   - Do a noisy beep
 *
 *     The following must be provided if MEOPT_TYPEAH=meTRUE
 *
 *        TTahead   - any characters waiting ?
 *
 *     TTbreakTest(n) - Test for a break from the keyboard. The numeric argument
 *     'n' is set to 1 if the break test may include 'display update' messages,
 *     or 0 if it is only allowed to collect user input. Note that not all platforms
 *     support this argument.
 */
#ifndef __ETERM_H
#define __ETERM_H

/**************************************************************************
* Common variables, constants and defines                                 *
**************************************************************************/

/* Standard key input definitions, found in termio.c */
typedef struct meTRANSKEY {
    int      count ;
    int      time ;
    meUShort key ;
    meUShort map ;
    struct meTRANSKEY *child ;
} meTRANSKEY ;

#define KEYBUFSIZ      128              /* Length of keyboard buffer    */
#define TTBREAKCNT     100              /* Initial ttbreakCnt value     */
#define TTSPECCHARS    32               /* End of the special chars     */

extern meTRANSKEY TTtransKey ;          /* A translated key             */
extern meUShort   TTwidthDefault ;      /* Default no. of cols per frame*/
extern meUShort   TTdepthDefault ;      /* Default no. of rows per frame*/
extern meUShort   TTkeyBuf[KEYBUFSIZ] ; /* Key beuffer/pending keys     */
extern meUByte ttSpeChars [TTSPECCHARS];/* Special characters           */
extern meUByte    TTnextKeyIdx ;        /* Circular buffer index        */
extern meUByte    TTlastKeyIdx ;        /* Key buffer - last index.     */
extern meUByte    TTnoKeys ;            /* Number of keys in buffer     */
extern meUByte    TTbreakFlag ;         /* Break outstanding on input   */
extern meUByte    TTbreakCnt ;          /* Number breaks outsanding     */
extern meUByte    TTallKeys;            /* Report all keys              */

extern meUShort   TTgetc(void) ;
extern void       TThandleBlink(int initFlag) ;
extern void       TTmove(int r, int c) ;
extern void       TTwaitForChar(void) ;
extern void       TTsleep(int msec, int intable, meVarList *waitVarList) ;
#if MEOPT_TYPEAH
extern int        TTahead(void) ;
#endif
#define TTinflush()   (TTahead(),TTlastKeyIdx=TTnextKeyIdx,TTnoKeys=0)
extern void       addKeyToBuffer(meUShort cc) ;
extern void       addKeyToBufferOnce(meUShort cc) ;
#if MEOPT_MOUSE
extern void       TTallKeysFlush(void) ;
#else
#define TTallKeysFlush() /* Nothing */
#endif
#if MEOPT_CALLBACK
extern void       doIdlePickEvent(void) ;
#endif
extern void       setAlarm(meInt absTime, meInt offTime) ;

extern void       meSetupPathsAndUser(char *progname) ;

#ifdef _UNIX

/* Signal definitions */
#ifdef _POSIX_SIGNALS
#define meSigHold()    ((++meSigLevel == 0) ? sigprocmask(SIG_BLOCK, &meSigHoldMask, NULL):0)
#define meSigRelease() ((--meSigLevel  < 0) ? sigprocmask(SIG_UNBLOCK, &meSigHoldMask, NULL):0)
/* The prototype of a signal in POSIX */
#define SIGNAL_PROTOTYPE  int sig
#else
/* BSD Signals */
#ifdef _BSD_SIGNALS
#define meSigHold()    ((++meSigLevel == 0) ? sigsetmask(meSigHoldMask):0)
#define meSigRelease() ((--meSigLevel  < 0) ? sigsetmask(0):0)
/* The prototype of a signal call in BSD */
#define SIGNAL_PROTOTYPE  int sig, int code, struct sigcontext *context

/* Remove any signal definition */
#ifdef signal
#undef signal
#endif /* signal */

/* Redefined for the safe BSD signal functions. We should be able to use a
 * regular signal() however is is easier to just assume all signals are broken
 * and use the safe ones !! We hope that the compiler will optimise structure
 * re-use when we have a series of these, but we are not too worried either
 * way !! */
#ifdef _BSD_42
#define signal(x,y)        \
do {                       \
    struct sigvec sa;      \
    sa.sv_mask = 0;        \
    sa.sv_onstack = 0;     \
    sa.sv_handler = y;     \
    sigvec (x, &sa, NULL); \
} while (0)
#endif /* _BSD_42 */
#ifdef _BSD_43
#define signal(x,y)        \
do {                       \
    struct sigvec sa ;     \
    sa.sv_mask = 0 ;       \
    sa.sv_flags = 0 ;      \
    sa.sv_handler = y ;    \
    sigvec (x, &sa, NULL); \
} while (0)
#endif /* _BSD_43 */
#else /* ! (_BSD_SIGNALS || _POSIX_SIGNALS) */
/* No hold mechanism */
#define meSigHold()    /* */
#define meSigRelease() /* */
/* The prototype of any other signal */
#define SIGNAL_PROTOTYPE  int sig
#endif /* _BSD_SIGNALS */
#endif /* _POSIX_SIGNALS */

/* Timer definitions */
extern void sigAlarm(SIGNAL_PROTOTYPE) ;
#define	_SINGLE_TIMER 1		/* Only one itimer available  */
#define	TIMER_MIN     10	/* Only one itimer available  */

/* Additional UNIX externals */
extern char  *CM, *CL ;

#ifndef _CYGWIN
/* Following are functions used by termcap & an xterm */
extern	int   tputs(char *, int, int (*)(int)) ;
extern	char *tgoto(char *, int, int ) ;
#endif

#define TTNbell()      (putchar(meCHAR_BELL),fflush(stdout))
#define TTdieTest()    if(alarmState & meALARM_DIE) meDie()
#define TTbreakTest(x) ((--TTbreakCnt == 0) &&                         \
                       (((alarmState & meALARM_DIE) && meDie()) ||     \
                        (TTbreakCnt=TTBREAKCNT,TTahead(),TTbreakFlag)))
#if MEOPT_MOUSE
extern void TTinitMouse(void);
#endif

#ifdef _ME_CONSOLE
#ifdef _TCAP

/* Following are termcap function */
extern int TCAPstart(void) ;
extern int TCAPopen(void) ;
extern int TCAPclose(void) ;
extern void TCAPmove(int row, int col);
#define TCAPputc(c)    putchar(c)
#define TCAPflush()    fflush(stdout)
extern void TCAPhideCur(void) ;
extern void TCAPshowCur(void) ;
extern void TCAPhandleBlink(void) ;

#if MEOPT_COLOR
extern int  TCAPaddColor(meColor index, meUByte r, meUByte g, meUByte b) ;
extern void TCAPschemeSet(meScheme scheme) ;
extern void TCAPschemeReset(void) ;
#endif

#endif /* _TCAP */

#ifndef _ME_WINDOW
/* If no window then just use the console ones */
#define TTstart    TCAPstart
#define TTopen     TCAPopen
#define TTclose    TCAPclose
#define TTend      TCAPclose
#define TTflush    TCAPflush
#define TThideCur  TCAPhideCur
#define TTshowCur  TCAPshowCur
#define TTaddColor TCAPaddColor
#define TTsetBgcol()
#define meFrameTermInit(f,s) meTRUE
#define meFrameTermFree(f,s)
#define meFrameTermMakeCur(f)

#endif /* !_ME_WINDOW */

#endif /* _ME_CONSOLE */

#ifdef _ME_WINDOW
/* Display information */

typedef struct
{
    Window    xwindow ;                 /* the x window */
    GC        xgc ;                     /* the x draw style */
    XGCValues xgcv;                     /* X colour values */
    meUByte   fcol;                     /* Foreground color */
    meUByte   bcol;                     /* Background color */
    meUByte   font;                     /* Font style */
    Font      fontId;                   /* Font X id */
    int       xmap;                     /* Frame is mapped */
} meFrameData ;

#define meFrameGetXWindow(ff)     (((meFrameData *) ff->termData)->xwindow)
#define meFrameGetXGC(ff)         (((meFrameData *) ff->termData)->xgc)
#define meFrameGetXGCValues(ff)   (((meFrameData *) ff->termData)->xgcv)
#define meFrameGetXGCFCol(ff)     (((meFrameData *) ff->termData)->fcol)
#define meFrameGetXGCBCol(ff)     (((meFrameData *) ff->termData)->bcol)
#define meFrameGetXGCFont(ff)     (((meFrameData *) ff->termData)->font)
#define meFrameGetXGCFontId(ff)   (((meFrameData *) ff->termData)->fontId)
#define meFrameSetXGCFCol(ff,v)   (((meFrameData *) ff->termData)->fcol = (v))
#define meFrameSetXGCBCol(ff,v)   (((meFrameData *) ff->termData)->bcol = (v))
#define meFrameSetXGCFont(ff,v)   (((meFrameData *) ff->termData)->font = (v))
#define meFrameSetXGCFontId(ff,v) (((meFrameData *) ff->termData)->fontId = (v))
/* Mapped window state */
#define meXMAP_FONT      -1             /* Unmapped and requires font change */
#define meXMAP_UNMAP      0             /* Unmapped, no font change required */
#define meXMAP_MAP        1             /* Window is mapped */
#define meFrameSetXMapState(ff,v)   (((meFrameData *) ff->termData)->xmap = (v))
#define meFrameGetXMapState(ff)     (((meFrameData *) ff->termData)->xmap)

typedef struct
{
    Display  *xdisplay ;                /* the x display */
    Font      fontId;                   /* Font X id */
    int       fwidth ;                  /* Font width in pixels */
    int       fdepth ;                  /* Font depth in pixels */
    int       fhwidth ;                 /* Font half width in pixels */
    int       fhdepth ;                 /* Font half depth in pixels */
    int       fadepth ;                 /* Font up-arrow depth in pixels */
    int       ascent ;                  /* Font ascent */
    int       descent ;                 /* Font descent */
    int       underline ;               /* The underline position */
    meUByte  *fontName;                 /* The current Font name */
    Font      fontTbl[meFONT_MAX];      /* table of font X ids for diff styles */
    meUByte  *fontPart[meFONT_MAX];     /* pointers to parts in fontname */
    meUByte   fontFlag[meFONT_MAX];     /* Font loaded ? */
} meCellMetrics;                        /* The character cell metrics */

extern meCellMetrics mecm ;

/* Set of macros to interchange pixel and character spaces coordinates */
#define colToClient(x)    (mecm.fwidth*(x))                /* Convert column char => pixel */
#define rowToClient(y)    ((mecm.fdepth*(y))+mecm.ascent)  /* Convert row char => pixel (for text drawing) */
#define rowToClientTop(y) (mecm.fdepth*(y))                /* Convert row char => pixel (top of row) */
#define clientToRow(y)    ((y)/mecm.fdepth)                /* Convert row pixel => char */
#define clientToCol(x)    ((x)/mecm.fwidth)                /* Convert column pixel => char */

extern int  meFrameXTermInit(meFrame *frame, meFrame *sibling) ;
extern void meFrameXTermFree(meFrame *frame, meFrame *sibling) ;
extern void meFrameXTermMakeCur(meFrame *frame) ;
extern void meFrameXTermHideCursor(meFrame *frame) ;
extern void meFrameXTermShowCursor(meFrame *frame) ;
extern void meFrameXTermSetScheme(meFrame *frame,meScheme scheme) ;
extern void meFrameXTermDraw(meFrame *frame, int srow, int scol, int erow, int ecol) ;
extern void meFrameXTermDrawSpecialChar(meFrame *frame, int x, int y, meUByte cc) ;
#define     meFrameXTermDrawString(frame,col,row,str,len)                            \
do {                                                                               \
    XDrawImageString(mecm.xdisplay,meFrameGetXWindow(frame),                       \
                     meFrameGetXGC(frame),(col),(row),(char *)(str),(len));        \
    if(meFrameGetXGCFont(frame) & meFONT_UNDERLINE)                                \
        XDrawLine(mecm.xdisplay,meFrameGetXWindow(frame),                          \
                  meFrameGetXGC(frame),(col),(row)+mecm.underline,                 \
                  (col)+colToClient(len)-1,(row)+mecm.underline);                  \
} while(0)
#define XTERMstringDraw(col,row,str,len) meFrameXTermDrawString(frameCur,col,row,str,len)                                           \                                          \

extern int  XTERMstart(void);
extern void XTERMmove(int r, int c) ;
extern void XTERMclear(void) ;
extern int  XTERMaddColor(meColor index, meUByte r, meUByte g, meUByte b) ;
extern void XTERMsetBgcol(void) ;

/* Some extra function, only available to xterms */
extern void meFrameSetWindowTitle(meFrame *frame, meUByte *str) ;
extern void meFrameSetWindowSize(meFrame *frame) ;
#if MEOPT_EXTENDED
extern void meFrameRepositionWindow(meFrame *frame, int resize) ;
#endif

#ifdef _CLIPBRD
extern void TTgetClipboard(void);
extern void TTsetClipboard(void);
extern void TTsetPrimary(void);
#endif

#ifdef _ME_CONSOLE

/* If both console and window must test which one should be used */
extern int  TTstart(void) ;
#define TTend()               ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPclose():0)
#define TTopen()              ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPopen():0)
#define TTclose()             ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPclose():0)
#define TTflush()             ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPflush():(XFlush(mecm.xdisplay),1))
#define TThideCur()           ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPhideCur():meFrameXTermHideCursor(frameCur))
#define TTshowCur()           ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPshowCur():meFrameXTermShowCursor(frameCur))
#define TTaddColor(i,r,g,b)   ((meSystemCfg & meSYSTEM_CONSOLE) ? TCAPaddColor(i,r,g,b):XTERMaddColor(i,r,g,b))
#define TTsetBgcol()          ((meSystemCfg & meSYSTEM_CONSOLE) ? 0:(XTERMsetBgcol(),0))
#define meFrameTermInit(f,s)  ((meSystemCfg & meSYSTEM_CONSOLE) ? meTRUE:meFrameXTermInit(f,s))
#define meFrameTermFree(f,s)  ((meSystemCfg & meSYSTEM_CONSOLE) ? 0:(meFrameXTermFree(f,s),0))
#define meFrameTermMakeCur(f) ((meSystemCfg & meSYSTEM_CONSOLE) ? 0:(meFrameXTermMakeCur(f),0))

#else /* _ME_CONSOLE */

/* If no console then just use the window ones */
#define TTstart()             XTERMstart()
#define TTend()     
#define TTopen()    
#define TTclose()   
#define TTflush()             XFlush(mecm.xdisplay)
#define TThideCur()           meFrameXTermHideCursor(frameCur)
#define TTshowCur()           meFrameXTermShowCursor(frameCur)
#define TTaddColor(i,r,g,b)   XTERMaddColor(i,r,g,b)
#define TTsetBgcol()          XTERMsetBgcol()
#define meFrameTermInit(f,s)  meFrameXTermInit(f,s)
#define meFrameTermFree(f,s)  meFrameXTermFree(f,s)
#define meFrameTermMakeCur(f) meFrameXTermMakeCur(f)

#endif /* _ME_CONSOLE */
#endif /* _ME_WINDOW */

#if MEOPT_CLIENTSERVER
extern void TTopenClientServer(void) ;
extern void TTkillClientServer(void) ;
extern int  TTconnectClientServer(void) ;
extern void TTsendClientServer(meUByte *) ;
#endif

#endif /* _UNIX */


#ifdef _WIN32

#define meHWndNull NULL
#define	_MULTI_TIMER 1		     /* Multiple timers can be set at once */
#ifdef _ME_CONSOLE
/* strange but true, when running as a console app, you can't assign ids
 * so effectively when a timer pops you have to work out which one has
 * if any. Bummer is you can kill them either! Thank you Bill */
#define	_MULTI_NOID_TIMER 1	     /* Multi timer but no ids */
#endif
#define	TIMER_MIN    1		     /* The minimum timer allowed */
/* executable extension list, in reverse order of dos priority - Must have the end NULL
 * included 4dos's btm files for completeness */
#define noExecExtensions 4
extern meUByte *execExtensions[noExecExtensions] ;

extern meUInt *colTable ;
extern HWND baseHwnd ;               /* This is the base hidden window handle */

extern int TTstart(void) ;
extern int TTopen(void) ;
#define TTclose()

#ifdef _ME_CONSOLE
extern BOOL ConsolePaint(void) ;
extern void ConsoleDrawString(meUByte *s, WORD wAttribute, int x, int y, int len) ;
#define TTcolorSet(f,b) ((f) | ((b) << 4))
#define TTschemeSet(scheme) \
TTcolorSet(colTable[meStyleGetFColor(meSchemeGetStyle(scheme))], \
           colTable[meStyleGetBColor(meSchemeGetStyle(scheme))])


extern int TTend(void) ;

#ifdef _ME_WINDOW

#define TTflush()   ((meSystemCfg & meSYSTEM_CONSOLE) ? ConsolePaint () : UpdateWindow (meFrameGetWinHandle(frameCur)))

#else

#define TTflush()   ConsolePaint ()

#endif /* _ME_WINDOW */

#else

#define TTend()
/*
 * TTflush()
 * Because of the delayed nature of the windows display perform the
 * flush by processing any outstanding PAINT messages ONLY. This
 * will update the display leaving the remaining messages on the
 * input queue.
 */
#define TTflush()   UpdateWindow (meFrameGetWinHandle(frameCur))
#endif

#ifdef _ME_WINDOW

extern RECT ttRect ;                 /* Area of screen to update */
extern int fdepth, fwidth ;
extern int ascent ;

typedef struct
{
    HWND     hwnd;                      /* This is the window handle */
    RECT     canvas;                    /* Screen extents */
    int      maximized;                 /* Maximised flags */
    int      paintAll;                  /* Screen paint all flag */
    int      paintDepth;                /* Screen paint depth */
    meShort *paintStartCol;             /* Screen paint start col */
    meShort *paintEndCol;               /* Screen paint end col */
} meFrameData ;

#define meFrameGetWinHandle(ff)         (((meFrameData *) (ff)->termData)->hwnd)
#define meFrameGetWinCanvas(ff)         (((meFrameData *) (ff)->termData)->canvas)
#define meFrameGetWinMaximized(ff)      (((meFrameData *) (ff)->termData)->maximized)
#define meFrameGetWinPaintAll(ff)       (((meFrameData *) (ff)->termData)->paintAll)
#define meFrameGetWinPaintDepth(ff)     (((meFrameData *) (ff)->termData)->paintDepth)
#define meFrameGetWinPaintStartCol(ff)  (((meFrameData *) (ff)->termData)->paintStartCol)
#define meFrameGetWinPaintEndCol(ff)    (((meFrameData *) (ff)->termData)->paintEndCol)

extern void TTputs(int row, int col, int len) ;

/* init the rect to invalid big areas, they are LONGS so this is okay */
#define TTinitArea()        (ttRect.left=0xfffff,ttRect.right=-1,ttRect.top=0xfffff,ttRect.bottom=-1)

#define TTsetArea(r,c,h,w)                                                   \
(ttRect.left=(c),ttRect.right=(c)+(w),ttRect.top=(r),ttRect.bottom=(r)+(h))

#define TTaddArea(r,c,h,w)                                                   \
do {                                                                         \
    if(ttRect.left > (c))                                                    \
        ttRect.left = (c) ;                                                  \
    if(ttRect.right < (c)+(w))                                               \
        ttRect.right = (c)+(w) ;                                             \
    if(ttRect.top > (r))                                                     \
        ttRect.top = (r) ;                                                   \
    if(ttRect.bottom < (r)+(h))                                              \
        ttRect.bottom = (r)+(h) ;                                            \
} while(0)

extern void TTapplyArea (void) ;

extern void meFrameTermFree(meFrame *frame, meFrame *sibling) ;
extern void meFrameTermMakeCur(meFrame *frame) ;
#else

#define meFrameTermFree(frame,sibling)
#define meFrameTermMakeCur(frame)

#endif /* _ME_WINDOW */

extern int  meFrameTermInit(meFrame *frame, meFrame *sibling) ;
extern void meFrameSetWindowTitle(meFrame *frame, meUByte *str) ;
extern void meFrameSetWindowSize(meFrame *frame) ;
#if MEOPT_EXTENDED
extern void meFrameRepositionWindow(meFrame *frame, int resize) ;
#endif
extern void meFrameShowCursor(meFrame *frame) ;
extern void meFrameHideCursor(meFrame *frame) ;

#define TTshowCur() meFrameShowCursor(frameCur)
#define TThideCur() meFrameHideCursor(frameCur)
#define TTNbell()   MessageBeep(0xffffffff)

#ifdef _CLIPBRD
extern void TTgetClipboard(void);
extern void TTsetClipboard(void);
extern void TTsetPrimary(void);
#endif

#if MEOPT_CLIENTSERVER
extern int  ttServerToRead;
extern void TTopenClientServer(void) ;
extern void TTkillClientServer(void) ;
extern int  TTcheckClientServer(void) ;
extern int  TTconnectClientServer(void) ;
extern void TTsendClientServer(meUByte *) ;
#endif

extern int TTaheadFlush(void);
#define TTdieTest()
#define TTbreakTest(x) ((TTbreakCnt-- ==  0) ? (TTbreakCnt=TTBREAKCNT,((x==0)?TTahead():TTaheadFlush()),TTbreakFlag):(TTbreakFlag))

extern int  TTaddColor(meColor index, meUByte r, meUByte g, meUByte b);
extern void TTsetBgcol(void);
extern void TTcolour (int fg, int bg);

extern int  WinMouseMode (int buttonMask, int highlight, int cursorShape);
extern int  WinLaunchProgram (meUByte *cmd, int flags, meUByte *inFile, meUByte *outFile,
#if MEOPT_IPIPES
                              meIPipe *ipipe,
#endif
                              meInt *sysRet);

#endif /* _WIN32 */

#ifdef _DOS

#define	_CONST_TIMER 1		     /* Constantly checked timer */
#define	TIMER_MIN    0		     /* The minimum timer allowed */
/* executable extension list, in reverse order of dos priority - Must have the end NULL
 * included 4dos's btm files for completeness */
#define noExecExtensions 4
extern meUByte *execExtensions[noExecExtensions] ;

extern meUByte *colTable ;

extern int  TTstart(void) ;
#define TTend()   TTclose()
extern int  TTopen(void) ;
extern int  TTclose(void) ;
#define meFrameTermInit(f,s) meTRUE
#define meFrameTermFree(f,s)
#define meFrameTermMakeCur(f)

#define TTflush()
extern void TThideCur(void) ;
extern void TTshowCur(void) ;
extern int  bdos(int func, unsigned dx, unsigned al);
#define TTNbell()   bdos(6,meCHAR_BELL, 0);

#define TTdieTest()
#define TTbreakTest(x) ((TTbreakCnt-- ==  0) && (TTbreakCnt=TTBREAKCNT,TTahead(),TTbreakFlag))

extern meUByte Cattr ;

extern int TTaddColor(meColor index, meUByte r, meUByte g, meUByte b);
#define TTsetBgcol()
#define TTcolorSet(f,b)     ((f) | ((b) << 4))
#define TTschemeSet(scheme) \
TTcolorSet(colTable[meStyleGetFColor(meSchemeGetStyle(scheme))], \
           colTable[meStyleGetBColor(meSchemeGetStyle(scheme))])

#endif /* _DOS */

#if MEOPT_MOUSE
void TTinitMouse(void) ;
#define meCURSOR_DEFAULT   0
#define meCURSOR_ARROW     1
#define meCURSOR_IBEAM     2
#define meCURSOR_CROSSHAIR 3
#define meCURSOR_GRAB      4
#define meCURSOR_WAIT      5
#define meCURSOR_STOP      6
#define meCURSOR_COUNT     7
#else
#define TTinitMouse()
#endif

/* timer definitions */

enum
{
    AUTOS_TIMER_ID=0,          /* Auto save timer handle         */
    SLEEP_TIMER_ID,            /* Sleep timer                    */
    CURSOR_TIMER_ID,           /* Cursor blink timer handle      */
#if MEOPT_CALLBACK
    CALLB_TIMER_ID,            /* Macro callback timer           */
    IDLE_TIMER_ID,             /* Idle timer handle              */
#endif
#if MEOPT_MOUSE
    MOUSE_TIMER_ID,            /* Mouse timer handle             */
#endif
#if MEOPT_SOCKET
    SOCKET_TIMER_ID,           /* socket connection time-out     */
#endif
    NUM_TIMERS                 /* Number of timers               */
} ;

#define TIMER_SET      0x01             /* Timer is active                */
#define TIMER_EXPIRED  0x02             /* Timer has expired              */

/*
 * Timer block structure.
 */
typedef struct TIMERBLOCK
{
    struct TIMERBLOCK *next;            /* Next block to be scheduled */
    meUInt   abstime;                   /* Absolute time */
    meUByte  id;                        /* Identity of timer */
} TIMERBLOCK;

/*
 * isTimerActive
 * Returns a boolean state (0==meFALSE) depending on if the timer
 * is outstanding or not.
 */
#define isTimerActive(id)  (meTimerState[id] & (TIMER_SET|TIMER_EXPIRED))
#define isTimerSet(id)     (meTimerState[id] & TIMER_SET)
#define isTimerExpired(id) (meTimerState[id] & TIMER_EXPIRED)

#define timerClearExpired(id) (meTimerState[(id)] = meTimerState[(id)] & ~TIMER_EXPIRED)

extern meUByte     meTimerState[] ;/* State of the timers, set or expired */
extern meInt       meTimerTime[] ; /* Absolute time of the timers */
extern TIMERBLOCK *timers ;        /* Head of timer list             */

/*
 * timerSet
 * Start a timer, reset or continue the timer. In all eventualities
 * the timer is re-started and will run.
 *
 * id     - handle of the timer.
 * tim    - absolute time of alarm time.
 *          if (tim == 0) Absolute time is unknown but value
 *              is needed in meTimerTime.
 *          if (tim < 0)  Absolute time is unknown and value
                is not needed.
 * offset - offset from the current time to alarm.
 */
extern void timerSet (int id, meInt tim, meInt offset) ;
extern int  _timerKill (int id) ;
#define timerKill(id) (isTimerSet(id)?_timerKill(id):timerClearExpired(id))
extern void handleTimerExpired(void) ;
extern void adjustStartTime(meInt tim) ;

#ifdef _MULTI_NOID_TIMER
extern void timerAlarm(int id) ;
#endif

#if (defined _CONST_TIMER) || (defined _SINGLE_TIMER)
extern void timerCheck(meInt tim) ;
#endif

/* flag used when idle-drop needs to be called */
#define IDLE_STATE_DROP         0x04  /* Idle timer is running */

#endif /* __ETERM_H */
