/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * estruct.h - Structures and their defines.
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
 * Created:     For MicroEMACS 3.7
 * Synopsis:    Structures and their defines.
 * Authors:     Dave G. Conroy, Steve Wilhite, George Jones, Daniel Lawrence,
 *          Jon Green, Steven Phillips.
 */

/* internal constants */
#ifdef _NANOEMACS
#define meBIND_MAX            192               /* max # of globally bound keys */
#else
#define meBIND_MAX            384               /* max # of globally bound keys */
#endif
#define meLINE_BLOCK_SIZE     12                /* line block chunk size        */
#define meWINDOW_MAX          64                /* # of windows MAXIMUM         */
#define meSBUF_SIZE_MAX       128               /* # of bytes, string buffers   */
#define meBUF_SIZE_MAX        1024              /* size of various inputs       */
#define meTOKENBUF_SIZE_MAX   meBUF_SIZE_MAX+4  /* meBUF_SIZE_MAX + an overrun safe area*/
#define meMLDISP_SIZE_MAX     meBUF_SIZE_MAX+20 /* meBUF_SIZE_MAX + completion label*/
#define meTIME_STAMP_SIZE_MAX 40                /* Max len of a time stamp str. */
#define meMACRO_DEPTH_MAX     20                /* maximum depth of recursion   */

#define meCHAR_BELL           0x07              /* the bell character           */
#define meCHAR_TAB            0x09              /* the tab character            */
#define meCHAR_NL             0x0a              /* the \n char ^J, not ^M       */
#define meCHAR_LEADER         0x1c              /* the special char leader flag */
#define meCHAR_TRAIL_NULL     0x01              /* the trail byte of a '\0'     */
#define meCHAR_TRAIL_SPECIAL  0x02              /* the trail byte of a \s??     */
#define meCHAR_TRAIL_HOTKEY   0x03              /* trail of an osd hotkey (\H)  */
#define meCHAR_TRAIL_HILSTART 0x04              /* trail of osd start hilight (\{) */
#define meCHAR_TRAIL_HILSTOP  0x05              /* trail of osd stop hilight (\}) */
#define meCHAR_TRAIL_LEADER   meCHAR_LEADER     /* trail of a leader char (must be same char) */

#define ME_SHIFT       0x0100                   /* special key shift            */
#define ME_CONTROL     0x0200                   /* special key conrtol          */
#define ME_ALT         0x0400                   /* special key alt              */
#define ME_SPECIAL     0x0800                   /* special key (function keys)  */
#define ME_PREFIX_NUM  16                       /* the number of prefixes       */
#define ME_PREFIX_BIT  12                       /* the number of prefixes       */
#define ME_PREFIX_MASK 0xf000                   /* the prefix bit mask          */
#define ME_PREFIX1     0x1000                   /* prefix 1 flag, or'ed in      */
#define ME_PREFIX2     0x2000                   /* prefix 2 flag, or'ed in      */
#define ME_PREFIX3     0x3000                   /* prefix 3 flag, or'ed in      */
#define ME_PREFIX4     0x4000                   /* prefix 4 flag, or'ed in      */
#define ME_INVALID_KEY 0x0000                   /* an invalid key number        */

/* Status defines */
#define meABORT  -1                             /* Death, ^G, abort, etc.       */
#define meFALSE   0                             /* False, no, bad, etc.         */
#define meTRUE    1                             /* True, yes, good, etc.        */

/* Keyboard states */
#define meSTOP    0                             /* keyboard macro not in use    */
#define mePLAY    1                             /*                playing       */
#define meRECORD  2                             /*                recording     */
#define meIDLE    3                             /* keyboard not in use/idle     */

/* Search etc. directions */
#define meFORWARD 0                             /* forward direction            */
#define meREVERSE 1                             /* backwards direction          */

/* Number of different history types & Maximum history size */
#define meHISTORY_COUNT 5
#define meHISTORY_SIZE 20

/* Last command states */
#define meCFCPCN  0x0001                        /* Last command was C-P, C-N    */
#define meCFKILL  0x0002                        /* Last command was a kill      */
#define meCFYANK  0x0004                        /* Last command was yank        */
#define meCFRYANK 0x0008                        /* Last command was reyank      */
#define meCFUNDO  0x0010                        /* Last command was undo        */

/* Path separator character used in pathname lists i.e. env variables */
#ifdef _UNIX
#define mePATH_CHAR ':'
#else
#define mePATH_CHAR ';'
#endif

/* Standard function prototypes used in various static tables */
typedef int (*meIFuncII)(int,int);
typedef int (*meIFuncSS)(const meUByte *, const meUByte *);
typedef int (*meIFuncSSI)(const meUByte *, const meUByte *, size_t);

/* Handling of special pointer types */
#define mePtrOffset(p,x)  ((meUByte *)(p) + x)
#ifdef _64BIT
#define meIntFromPtr(n)   ((meInt)(intptr_t)(n))
#define mePtrFromInt(n)   ((meUByte *)(intptr_t)(n))
#define mePtrFromStr(n)   ((void *) atoll((char *) n))
#else
#define meIntFromPtr(n)   ((meInt)(n))
#define mePtrFromInt(n)   ((meUByte *)(n))
#define mePtrFromStr(n)   ((void *) atoi((char *) n))
#endif
//#define mePtrFromInt(n)   ((meUByte *)(n))

/* meStyle contains color and font information coded into an meInt the
 * following #defines and macros are used to manipulate them.
 */
typedef meUByte   meColor ;
#define meCOLOR_FDEFAULT 0
#define meCOLOR_BDEFAULT 1
#define meCOLOR_INVALID  0xff
#define meFColorCheck(x)   (((x)>=noColors)       ? meCOLOR_FDEFAULT : (x))
#define meBColorCheck(x)   (((x)>=noColors)       ? meCOLOR_BDEFAULT : (x))


typedef meUInt  meStyle ;
#define meSTYLE_NDEFAULT  0x00000100
#define meSTYLE_RDEFAULT  0x00080001

#define meSTYLE_FCOLOR    0x000000ff
#define meSTYLE_BCOLOR    0x0000ff00
#define meSTYLE_FONT      0x001f0000
#define meSTYLE_BOLD      0x00010000
#define meSTYLE_ITALIC    0x00020000
#define meSTYLE_LIGHT     0x00040000
#define meSTYLE_REVERSE   0x00080000
#define meSTYLE_UNDERLINE 0x00100000
#define meSTYLE_CMPBCOLOR (meSTYLE_BCOLOR|meSTYLE_REVERSE)
#define meSTYLE_INVALID   0xffffffff

#define meFONT_COUNT      5
#define meFONT_BOLD       0x01
#define meFONT_ITALIC     0x02
#define meFONT_LIGHT      0x04
#define meFONT_REVERSE    0x08
#define meFONT_UNDERLINE  0x10
#define meFONT_MAX        0x20
#define meFONT_MASK       0x1f

#define meStyleSet(ss,fc,bc,ff) ((ss)=((fc)|((bc)<<8)|((ff)<<16)))
#define meStyleGetFColor(ss)    ((ss)&meSTYLE_FCOLOR)
#define meStyleGetBColor(ss)    (((ss)&meSTYLE_BCOLOR) >> 8)
#define meStyleGetFont(ss)      (((ss)&meSTYLE_FONT) >> 16)
#define meStyleCmp(s1,s2)       ((s1) != (s2))
#define meStyleCmpBColor(s1,s2) ((s1 & meSTYLE_CMPBCOLOR) != ((s2) & meSTYLE_CMPBCOLOR))

/* An meScheme is simply an index into the meStyle table.
 * Each scheme created by add-color-scheme consists of 8 meSTYLEs, as there
 * are upto 256 schemes there can be 8*256 styles hence its a meUShort.
 * note that users enter the base scheme number, i.e. 0, 1, 2, ...
 * which is then multiplied by 8 and stored, i.e. 0, 8, 16, ...
 */
typedef meUShort meScheme ;
#define meSCHEME_NORMAL   0                     /* Normal style */
#define meSCHEME_RNORMAL  1                     /* Reverse normal style */
#define meSCHEME_CURRENT  2                     /* Current foreground colour */        
#define meSCHEME_RCURRENT 3                     /* Reverse current foreground colour */        
#define meSCHEME_SELECT   4                     /* Selected foreground colour */        
#define meSCHEME_RSELECT  5                     /* Reverse celected foreground colour */        
#define meSCHEME_CURSEL   6                     /* Current selected foreground color */
#define meSCHEME_RCURSEL  7                     /* Reverse current selected foreground color */
#define meSCHEME_STYLES   8                     /* Number of styles in a scheme */
#define meSCHEME_NDEFAULT 0
#define meSCHEME_RDEFAULT meSCHEME_STYLES
#define meSCHEME_INVALID   0xffff

#define meSCHEME_STYLE         0x0fff
#define meSCHEME_NOFONT        0x1000

#define meSchemeCheck(x)                 (((x)>=styleTableSize) ? meSCHEME_NDEFAULT:(x))
#define meSchemeGetStyle(x)              (styleTable[(x) & meSCHEME_STYLE])
#define meSchemeTestStyleHasFont(x)      (styleTable[(x) & meSCHEME_STYLE] & (meSTYLE_UNDERLINE|meSTYLE_ITALIC|meSTYLE_BOLD))
#define meSchemeTestNoFont(x)            ((x) & meSCHEME_NOFONT)
#define meSchemeSetNoFont(x)             ((x) | meSCHEME_NOFONT)
/*
 * All text is kept in circularly linked lists of "meLine" structures. These
 * begin at the header line (which is the blank line beyond the end of the
 * buffer). This line is pointed to by the "meBuffer". Each line contains a the
 * number of bytes in the line (the "used" size), the size of the text array,
 * and the text. The end of line is not stored as a byte; its implied. Future
 * additions will include update hints, and a list of marks into the line.
 */
/* due to packing sizeof(meLine) can waste precious bytes, avoid this by
 * calculating the real size ourselves, see asserts in meInit which check
 * these values are correct */
#if MEOPT_EXTENDED
typedef meUShort meLineFlag ;                   /* In extended vers use 16 bits */
#else
typedef meUByte meLineFlag ;
#endif

typedef struct meLine
{
    struct meLine     *next ;                   /* Link to the next line        */
    struct meLine     *prev ;                   /* Link to the previous line    */
    meUShort           length ;                 /* Used size                    */
    meLineFlag         flag ;                   /* Line is marked if true       */
    meUByte            unused ;                 /* length + unused = max size   */
    meUByte            text[1] ;                /* A bunch of characters.       */
} meLine ;

#define meMEMORY_BOUND          8               /* the size memory is rounded up to */
#define meLINE_SIZE             ((size_t) (&(((meLine *) 0)->text[1])))
#define meLineMallocSize(ll)    ((meLINE_SIZE + (meMEMORY_BOUND-1) + (ll)) & ~(meMEMORY_BOUND-1))

#define meLINE_CHANGED          0x0001
#define meLINE_ANCHOR_AMARK     0x0002
#define meLINE_ANCHOR_NARROW    0x0004
#define meLINE_ANCHOR_OTHER     0x0008
#define meLINE_ANCHOR           (meLINE_ANCHOR_AMARK|meLINE_ANCHOR_NARROW|meLINE_ANCHOR_OTHER)
#define meLINE_NOEOL            0x0010          /* Save line with no \n or \0   */
#if MEOPT_EXTENDED
#define meLINE_MARKUP           0x0020
#define meLINE_PROTECT          0x0040
#define meLINE_SET_MASK         0x0ff0
#define meLINE_SCHEME_MASK      0xf000
#define meLINE_SCHEME_MAX       0x10
#define meLINE_SCHEME_SHIFT     12
#define meLineIsSchemeSet(lp)   ((lp->flag & meLINE_SCHEME_MASK) != 0)
#define meLineGetSchemeIndex(lp) ((lp->flag & meLINE_SCHEME_MASK) >> meLINE_SCHEME_SHIFT)
#endif

#define meLineGetNext(lp)       ((lp)->next)
#define meLineGetPrev(lp)       ((lp)->prev)
#define meLineGetFlag(lp)       ((lp)->flag)
#define meLineGetChar(lp, n)    ((lp)->text[(n)])
#define meLineSetChar(lp, n, c) ((lp)->text[(n)]=(c))
#define meLineGetText(lp)       ((lp)->text)
#define meLineGetLength(lp)     ((lp)->length)
#define meLineGetMaxLength(lp)  (((int) (lp)->length) + ((int) (lp)->unused))

/*
 * There is a window structure allocated for every active display window. The
 * windows are kept in a big list, one per frame, in top to bottom screen order, with the
 * listhead at "frame->windowList". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands to guide
 * redisplay; although this is a bit of a compromise in terms of decoupling,
 * the full blown redisplay is just too expensive to run for every input
 * character.
 * 
 * The physical screen extents of the window are contained in frameColumn, frameRow,
 * width depth. These define the physical extremities of the window, within
 * those boundaries lie the message line, scroll bar and text area. 
 * The separate window extents appear to duplicate some of the existing values,
 * (which they do), however they do serve a purpose in that they decouple the 
 * window tesselation and layout (i.e. where a window is positioned on the
 * screen) from having to have any knowledge about the exact format of the 
 * contents of the window, thereby reducing maintenance of the tesselation 
 * code which is a bit of a mind bender at times.
 * 
 * Note that the "updateFlags" are at the top. We access these alot on the display
 * side. Positioned here ensures that they are available immediatly without
 * any indirect offset - just give that compiler a hand. 
 */

/*
 * meWindow size limits, note that the minimum width must allow for the
 * narrow -> wide scroll-bar change as we don't want to resize all the windows
 * if the user makes this change */
#define meWINDOW_WIDTH_MIN  5           /* Minimum width of a window    */
#define meWINDOW_DEPTH_MIN  2           /* Minimum width of a window    */
/*
 * meWindow structure updateFlag values.
 * Define states to the display drivers to update the screen. 
 */
#define WFFORCE       0x0001            /* Window needs forced reframe  */
#define WFRESIZE      0x0002            /* Reset the width of lines.    */
#define WFMOVEL       0x0004            /* Movement from line to line   */
#define WFMOVEC       0x0008            /* Movement from col to col     */
#define WFMAIN        0x0010            /* Update window main area      */
#define WFMODE        0x0020            /* Update window mode line      */
#define WFDOT         0x0040            /* Redraw windows dot line      */
#define WFREDRAW      0x0080            /* Redraw main window area      */
#define WFSBSPLIT     0x0100            /* Scroll split has changed     */
#define WFSBUP        0x0200            /* Scroll up arrow change       */
#define WFSBUSHAFT    0x0400            /* Scroll bar up shaft change   */
#define WFSBBOX       0x0800            /* Scroll bar box change        */
#define WFSBDSHAFT    0x1000            /* Scroll bar down shaft change */
#define WFSBDOWN      0x2000            /* Scroll bar down arrow change */
#define WFSBML        0x4000            /* Scroll bar mode line changed */
#define WFSELHIL      0x8000            /* Selection highlight changed  */
#define WFSELDRAWN    0x10000           /* Editing within a line        */
#define WFLOOKBK      0x20000           /* Check the hilight look-back  */
/* Composite mode to force an update of the scroll box */
#define WFSBOX        (WFSBUSHAFT|WFSBBOX|WFSBDSHAFT)    
/* Composite mode to force update of the scroll bar */
#define WFSBAR        (WFSBSPLIT|WFSBUP|WFSBDOWN|WFSBML|WFSBOX)

/* A garbage update of the screen adds following modes */
#define WFUPGAR       (WFMODE|WFRESIZE|WFSBAR)

/*
 * meWindow structure flags values.
 */
#define meWINDOW_LOCK_WIDTH   0x0001    /* Lock the width of the window */
#define meWINDOW_LOCK_DEPTH   0x0002    /* Lock the depth of the window */
#define meWINDOW_LOCK_BUFFER  0x0004    /* Lock the buffer shown        */
#define meWINDOW_NO_CMP       0x0008    /* Exclude from compare-windows */
#define meWINDOW_NO_NEXT      0x0010    /* Exclude from next/prev-window*/
#define meWINDOW_NO_OTHER_DEL 0x0020    /* Exclude from del-other-window*/
#define meWINDOW_NO_DELETE    0x0040    /* Cannot delete window         */
#define meWINDOW_NO_SPLIT     0x0080    /* Cannot split window          */
#define meWINDOW_NOT_ALONE    0x0100    /* Cannot exist alone           */

/*
 * meWindow structure vertScrollBarMode values.
 * Modes define the sroll bar state.
 */
#define WMVWIDE 0x01                    /* PUBLIC:  Wide vert divider 2 chars    */
#define WMUP    0x02                    /* PUBLIC:  Has upper end cap            */
#define WMDOWN  0x04                    /* PUBLIC:  Has lower end cap            */
#define WMBOTTM 0x08                    /* PUBLIC:  Has a mode line character    */
#define WMSCROL 0x10                    /* PUBLIC:  Has a box on scrolling shaft */
#define WMRVBOX 0x20                    /* PUBLIC:  Reverse video on box         */
#define WMHWIDE 0x40                    /* PUBLIC:  Wide horz divider 2 chars    */
#define WMSPLIT 0x80                    /* PUBLIC:  Has a splitter               */
#define WMVBAR  0x100                   /* PUBLIC:  Window has a vertical bar    */
#define WMHORIZ 0x200                   /* PRIVATE: This is a horizontal bar     */
#define WMWIDE  0x400                   /* PRIVATE: Wide scroll-bar (internal)   */
  
/* The user interface mask for the scroll mode */
#define WMUSER  (WMVWIDE|WMUP|WMDOWN|WMBOTTM|WMSCROL|WMRVBOX|WMHWIDE|WMSPLIT|WMVBAR)
/*
 * Define the window character array. This is a fixed array of 
 * characters which define the window components
 */
/* Mode line */
#define WCMLCWSEP     0                  /* Mode line current window sep  (=)*/
#define WCMLIWSEP     1                  /* Mode line inactive window sep (-)*/
#define WCMLROOT      2                  /* Mode line root indicatior     (#)*/
#define WCMLBCHNG     3                  /* Mode line buffer changed char (*)*/
#define WCMLBVIEW     4                  /* Mode line buffer view char    (%)*/
/* Scroll Bar */
#define WCVSBSPLIT    5                  /* Buffer split character #1        */
#define WCVSBUP       6                  /* Scroll bar up character          */
#define WCVSBUSHAFT   7                  /* Scroll bar up shaft character    */
#define WCVSBBOX      8                  /* Scroll bar box character         */
#define WCVSBDSHAFT   9                  /* Scroll bar down shaft character  */
#define WCVSBDOWN    10                  /* Scroll bar down character        */
#define WCVSBML      11                  /* Scroll bar/mode line point       */
#define WCVSBSPLIT1  12                  /* Buffer split character #1        */
#define WCVSBSPLIT2  13                  /* Buffer split character #2        */
#define WCVSBUP1     14                  /* Wide scroll bar up char #1       */
#define WCVSBUP2     15                  /* Wide scroll bar up char #2       */
#define WCVSBUSHAFT1 16                  /* Wide scroll bar up shaft char #1 */
#define WCVSBUSHAFT2 17                  /* Wide scroll bar up shaft char #2 */
#define WCVSBBOX1    18                  /* Wide scroll bar box char #1      */
#define WCVSBBOX2    19                  /* Wide scroll bar box char #2      */
#define WCVSBDSHAFT1 20                  /* Wide scroll bar dn shaft char #1 */
#define WCVSBDSHAFT2 21                  /* Wide scroll bar dn shaft char #2 */
#define WCVSBDOWN1   22                  /* Wide scroll bar down char #1     */
#define WCVSBDOWN2   23                  /* Wide scroll bar down char #2     */
#define WCVSBML1     24                  /* Wide scroll bar/mode point #1    */
#define WCVSBML2     25                  /* Wide scroll bar/mode point #2    */
#define WCHSBSPLIT   26                  /* Buffer split character #1        */
#define WCHSBUP      27                  /* Scroll bar up character          */
#define WCHSBUSHAFT  28                  /* Scroll bar up shaft character    */
#define WCHSBBOX     29                  /* Scroll bar box character         */
#define WCHSBDSHAFT  30                  /* Scroll bar down shaft character  */
#define WCHSBDOWN    31                  /* Scroll bar down character        */
#define WCHSBML      32                  /* Scroll bar/mode line point       */
#define WCHSBSPLIT1  33                  /* Buffer split character #1        */
#define WCHSBSPLIT2  34                  /* Buffer split character #2        */
#define WCHSBUP1     35                  /* Wide scroll bar up char #1       */
#define WCHSBUP2     36                  /* Wide scroll bar up char #2       */
#define WCHSBUSHAFT1 37                  /* Wide scroll bar up shaft char #1 */
#define WCHSBUSHAFT2 38                  /* Wide scroll bar up shaft char #2 */
#define WCHSBBOX1    39                  /* Wide scroll bar box char #1      */
#define WCHSBBOX2    40                  /* Wide scroll bar box char #2      */
#define WCHSBDSHAFT1 41                  /* Wide scroll bar dn shaft char #1 */
#define WCHSBDSHAFT2 42                  /* Wide scroll bar dn shaft char #2 */
#define WCHSBDOWN1   43                  /* Wide scroll bar down char #1     */
#define WCHSBDOWN2   44                  /* Wide scroll bar down char #2     */
#define WCHSBML1     45                  /* Wide scroll bar/mode point #1    */
#define WCHSBML2     46                  /* Wide scroll bar/mode point #2    */
#define WCOSDTTLBR   47                  /* Osd title bar char               */
#define WCOSDTTLKLL  48                  /* Osd title bar kill char          */
#define WCOSDRESIZE  49                  /* Osd resize char                  */
#define WCOSDBTTOPN  50                  /* Osd button start char e.g. '['   */
#define WCOSDDBTTOPN 51                  /* Osd default button start e.g. '>'*/
#define WCOSDBTTCLS  52                  /* Osd button close char e.g. ']'   */
#define WCOSDDBTTCLS 53                  /* Osd default button close e.g. '<'*/
#define WCDISPTAB    54                  /* Display tab char                 */
#define WCDISPCR     55                  /* Display new-line char            */
#define WCDISPSPACE  56                  /* Display space char               */
#define WCDISPTXTLFT 57                  /* Display text off to left         */
#define WCDISPTXTRIG 58                  /* Display text off to right        */
#define WCDISPSPLTLN 59                  /* Display split line char (ipipe)  */
#define WCLEN        60                  /* Number of characters             */

/* The box chars BC=Box Chars, N=North, E=East, S=South, W=West,
 * So BCNS = Box Char which has lines from North & South to centre, i.e. 
 *           the vertical line |
 *    BCNESW = the + etc.
 */
#define BCNS         0
#define BCES         1
#define BCSW         2
#define BCNE         3
#define BCNW         4
#define BCESW        5
#define BCNES        6
#define BCNESW       7
#define BCNSW        8
#define BCNEW        9
#define BCEW        10
#define BCLEN       11                          /* Number of characters         */

typedef struct  meWindow {
    meUInt             updateFlags;             /* Window update flags.         */
    struct  meWindow  *next;                    /* Next window                  */
    struct  meWindow  *prev;                    /* Previous window              */
    struct  meWindow  *videoNext;               /* meVideo thread of windows    */
    struct  meVideo   *video;                   /* Virtual video block          */
    struct  meBuffer  *buffer;                  /* Buffer displayed in window   */
    struct  meBuffer  *bufferLast;              /* Last Buffer displayed        */
    meLine            *dotLine;                 /* Line containing "."          */
    meLine            *markLine;                /* Line containing "mark"       */
    meLine            *modeLine;                /* window's mode-line buffer    */
    meLine            *dotCharOffset;           /* Current lines char offsets   */
    meInt              vertScroll;              /* windows top line number      */
    meInt              dotLineNo;               /* current line number          */
    meInt              markLineNo;              /* current mark line number     */
#if MEOPT_EXTENDED
    meInt              id;                      /* $window-id                   */
#endif
    meShort            windowRecenter;          /* If NZ, forcing row.          */
    meUShort           dotOffset;               /* Byte offset for "."          */
    meUShort           markOffset;              /* Byte offset for "mark"       */
    meUShort           frameRow;                /* Window starting row          */
    meUShort           frameColumn;             /* Window starting column       */
    meUShort           width;                   /* Window number text columns   */
    meUShort           depth;                   /* Window number text rows      */
    meUShort           textDepth;               /* # of rows of text in window  */
    meUShort           textWidth;               /* Video number of columns      */
    meInt              horzScroll;              /* cur horizontal scroll column */
    meInt              horzScrollRest;          /* the horizontal scroll column */
    meUShort           marginWidth;             /* The margin for the window    */
#if MEOPT_EXTENDED
    meUShort           flags;                   /* $window-flags                */
#endif
#if MEOPT_SCROLL
    meUShort           vertScrollBarPos[WCVSBML-WCVSBSPLIT+1];
                                                /* Vert Scroll bar positions*/
    meUShort           vertScrollBarMode;       /* Operating mode of window     */
#endif
} meWindow ;

#define ME_IO_NONE          0           /* No file operation. */
#define ME_IO_FILE          1           /* Regular file system file handle */
#define ME_IO_SOCKET        2           /* A socket transfer. */
#define ME_IO_BINFS         3           /* A Built-in file system operation. */
#define ME_IO_PIPE          4           /* A pipe operation. */

/* Define a file pointer type. */
typedef struct meIo {
    meUInt type;                        /* Currently operation in progress */

#ifdef _WIN32
    HANDLE    rp;                       /* File read pointer, all func. */
    HANDLE    wp;                       /* File write pointer, all func.*/
#else
    FILE     *rp;                       /* File read pointer, all func. */
    FILE     *wp;                       /* File write pointer, all func.*/
#endif
#ifdef MEOPT_BINFS
    bfsfile_t binfs;                    /* The built-in file system handle */
#endif
} meIo;

/* SWP 2002-01-09 - Windows file timestamps use 2 longs, a dwHigh and a dwLow
 *                  to count the 100 nanosecs since 1601-01-01 (why???)
 *                  So create an meFiletime type to handle this which is
 *                  then used everywhere else.
 */
#ifdef _WIN32

typedef FILETIME meFiletime ;

/* initialize to a recognizable duff value */
#define meFiletimeInit(t1)            ((t1).dwHighDateTime = (t1).dwLowDateTime = -1)

/* returns true if file time is the initialize value */
#define meFiletimeIsInit(t1)          (((t1).dwHighDateTime == -1) && ((t1).dwLowDateTime = -1))

/* return meTRUE if t1 == t2 */
#define meFiletimeIsSame(t1,t2)                 \
(((t1).dwHighDateTime == (t2).dwHighDateTime) && ((t1).dwLowDateTime == (t2).dwLowDateTime))

/* return meTRUE if t1 is newer than t2 */
#define meFiletimeIsModified(t1,t2)             \
(((t1).dwHighDateTime == (t2).dwHighDateTime) ? \
 ((t1).dwLowDateTime > (t2).dwLowDateTime):((t1).dwHighDateTime > (t2).dwHighDateTime))

/* return a single int value - Dangerously ingoring the higher bits! */
#define meFiletimeToInt(t1)                     \
((meFiletimeIsInit(t1)) ? -1:                   \
 ((((t1).dwLowDateTime  >> 24) & 0x000000ff) | (((t1).dwHighDateTime << 8) & 0x7fffff00)))

#else
#ifdef _UNIX
typedef time_t meFiletime ;
#else
typedef meInt meFiletime ;
#endif

/* initialize to a recognizable duff value */
#define meFiletimeInit(t1)            ((t1) = -1)

/* returns true if file time is the initialize value */
#define meFiletimeIsInit(t1)          ((t1) == -1)

/* return meTRUE if t1 == t2 */
#define meFiletimeIsSame(t1,t2)       ((t1) == (t2))

/* return meTRUE if t1 is newer than t2 */
#define meFiletimeIsModified(t1,t2)   ((t1) > (t2))

/* return a single in value */
#define meFiletimeToInt(t1)           (t1)

#endif

/* meStat Contains the file node information */
typedef struct {
    meFiletime         stmtime;                 /* modification time of file    */
    meUInt             stsizeHigh ;             /* File's Size (higher 4bytes)  */
    meUInt             stsizeLow ;              /* File's Size (lower 4bytes)   */
#ifdef _UNIX
    uid_t              stuid ;                  /* File's User id               */
    gid_t              stgid ;                  /* File's Group id              */
    dev_t              stdev ;                  /* Files device ID no.          */
    ino_t              stino ;                  /* Files Inode number           */
#endif
    meUShort           stmode ;                 /* file mode flags              */
} meStat ;

/*
 * meAbbrev
 * 
 * structure to store info on an abbreviation file
 */
typedef struct meAbbrev {
    struct meAbbrev   *next ;                   /* Pointer to the next abrev    */
    meLine             hlp ;                    /* head line                    */
    meUByte            loaded ;                 /* modification time of file    */
    meUByte            fname[1] ;               /* Users abrev file name        */
} meAbbrev ;

/* structure to hold user variables and their definitions */
typedef struct meVariable
{
    struct meVariable *next ;                   /* Next pointer, MUST BE FIRST as with meVarList */
    meUByte           *value ;                  /* value (string)               */
    meUByte            name[1] ;                /* name of user variable        */
} meVariable;

typedef struct meVarList
{
    struct meVariable *head ;
    int                count ;
} meVarList ;

/* structure for the name binding table */

typedef struct meCommand {
    struct meCommand  *anext ;                  /* alphabetically next command  */
#if MEOPT_CMDHASH
    struct meCommand  *hnext ;                  /* next command in hash table   */
#endif
#if MEOPT_EXTENDED
    meVarList          varList ;                /* command variables list       */
#endif
    meUByte           *name ;                   /* name of command              */
    int                id ;                     /* command id number            */
    meIFuncII          func ;                   /* function name is bound to    */
} meCommand ;

typedef struct meMacro {
    meCommand         *anext ;                  /* alphabetically next command  */
#if MEOPT_CMDHASH
    meCommand         *hnext ;                  /* next command in hash table   */
#endif
#if MEOPT_EXTENDED
    meVarList          varList ;                /* command variables list       */
#endif
    meUByte           *name ;                   /* name of macro                */
    int                id ;                     /* command id number            */
    meLine            *hlp ;                    /* Head line of macro           */
#if MEOPT_EXTENDED
    meUByte           *fname ;                  /* file name for file-macros    */
#endif
#if MEOPT_CALLBACK
    meInt              callback ;               /* callback time for macro      */
#endif
} meMacro ;

#define meMACRO_HIDE  meLINE_ANCHOR_AMARK       /* Hide the function            */
#define meMACRO_EXEC  meLINE_ANCHOR_NARROW      /* Buffer is being executed     */
#define meMACRO_FILE  meLINE_ANCHOR_OTHER       /* macro file define            */


/* An alphabetic mark is as follows. Alphabetic marks are implemented as a
 * linked list of amark structures, with the head of the list being pointed to
 * by anchorList in the buffer structure.
 */
typedef struct meAnchor {
    struct meAnchor   *next ;                   /* pointer to next anchor       */
    meLine            *line ;                   /* pointer to anchor line       */
    meInt              name ;                   /* anchor name                  */
    meUShort           offs ;                   /* anchor line offset           */
} meAnchor;

#define meANCHOR_TYPE_MASK     0xff000000
#define meANCHOR_EXEC_BUFFER   0x01000001
#define meANCHOR_ABS_LINE      0x01000002
#define meANCHOR_FILL_DOT      0x01000003
#define meANCHOR_NARROW        0x02000000
#define meANCHOR_POSITION_DOT  0x03000000
#define meANCHOR_POSITION_MARK 0x04000000
#define meANCHOR_EXSTRPOS      0x000000fe

#define meAnchorGetNext(aa)     ((aa)->next)
#define meAnchorGetName(aa)     ((aa)->name)
#define meAnchorGetType(aa)     (meAnchorGetName(aa) & meANCHOR_TYPE_MASK)
#define meAnchorGetLine(aa)     ((aa)->line)
#define meAnchorGetOffset(aa)   ((aa)->offset)
#define meAnchorGetLineFlag(aa) \
((meAnchorGetType(aa) == 0) ? meLINE_ANCHOR_AMARK : \
 (meAnchorGetType(aa) == meANCHOR_NARROW) ? meLINE_ANCHOR_NARROW : meLINE_ANCHOR_OTHER)

/* A position, stores the current window, buffer, line etc which can
 * be restore later, used by push-position and pop-position */
typedef struct mePosition {
    struct mePosition *next;                   /* pointer to previous position (stack)      */
    meWindow          *window;                 /* Current window                            */
    struct meBuffer   *buffer;                 /* windows buffer                            */
    meInt              anchor;                 /* Anchor name                               */
    meInt              vertScroll;             /* windows top line number                   */
    meInt              dotLineNo;              /* current line number                       */
    meInt              markLineNo;             /* current mark line number                  */
    meUShort           winMinRow;              /* Which window - store the co-ordinate      */
    meUShort           winMinCol;              /* so we can restore to the best matching    */
    meUShort           winMaxRow;              /* window on a goto - this greatly improves  */
    meUShort           winMaxCol;              /* its use.                                  */
    meInt              horzScroll;             /* cur horizontal scroll column              */
    meInt              horzScrollRest;         /* the horizontal scroll column              */
    meUShort           dotOffset;              /* Byte offset for "."                       */
    meUShort           markOffset;             /* Byte offset for "mark"       */
    meUShort           flags;                  /* Whats stored bit mask                     */
    meUShort           name;                   /* position name, (letter associated with it)*/
} mePosition;
#define mePOS_WINDOW    0x001
#define mePOS_WINXSCRL  0x002
#define mePOS_WINXCSCRL 0x004
#define mePOS_WINYSCRL  0x008
#define mePOS_BUFFER    0x010
#define mePOS_LINEMRK   0x020
#define mePOS_LINENO    0x040
#define mePOS_LINEOFF   0x080
#define mePOS_MLINEMRK  0x100
#define mePOS_MLINENO   0x200
#define mePOS_MLINEOFF  0x400
#define mePOS_DEFAULT   \
(mePOS_WINDOW|mePOS_WINXSCRL|mePOS_WINXCSCRL|mePOS_WINYSCRL|mePOS_BUFFER|mePOS_LINEMRK)

#if MEOPT_NARROW

typedef struct meNarrow {
    struct meNarrow   *next ;                   /* pointer to next narrow in list            */
    struct meNarrow   *prev ;                   /* pointer to previous narrow in list        */
    meLine            *slp ;                    /* pointer to narrow start line              */
    meLine            *elp ;                    /* pointer to narrow end line                */
    meLine            *markupLine ;             /* pointer to the markup line                */
    meInt              noLines ;                /* Number of lines narrowed out              */
    meInt              sln ;                    /* Narrows start line number                 */
    meInt              name ;                   /* anchor name                               */
    meInt              markupCmd ;              /* Command called to create markup line text */
    meUByte            scheme ;                 /* Line scheme used                          */
    meUByte            expanded ;               /* Flag whether it is temporarily expanded   */
} meNarrow ;

/* the following is encoded in the narrow name */
#define meNARROW_TYPE_OUT       0x00100000
#define meNARROW_TYPE_TO_BOTTOM 0x00200000
#define meNARROW_TYPE_TO_TOP    0x00300000
#define meNARROW_TYPE_MASK      0x00f00000
#define meNARROW_NUMBER_MASK    0x000fffff
#define meNarrowNameGetMarkupArg(name) (((name) & meNARROW_TYPE_MASK) >> 20)

#endif

/*
 * Text is kept in buffers. A buffer header, described below, exists for every
 * buffer in the system. The buffers are kept in a big list, so that commands
 * that search for a buffer by name can find the buffer header. There is a
 * safe store for the dot and mark in the header, but this is only valid if
 * the buffer is not being displayed (that is, if "windowCount" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with a pointer to
 * the header line in "baseLine".
 * Buffers may be "Inactive" which means the files accosiated with them
 * have not been read in yet. These get read in at "use buffer" time.
 */
typedef struct  meBuffer {
    struct  meBuffer  *next ;                   /* Link to next struct meBuffer */
#if MEOPT_ABBREV
    meAbbrev          *abbrevFile ;             /* Abreviation file             */
#endif
    meAnchor          *anchorList ;             /* pointer to the mark list     */
#if MEOPT_LOCALBIND
    struct meBind     *bindList ;               /* pointer to local bindings    */
#endif
#if MEOPT_NARROW
    meNarrow          *narrow ;                 /* pointer to narrow structures */
#endif
#if MEOPT_EXTENDED
    meVarList          varList ;                /* User local buffer variables  */
#endif
    meStat             stats ;                  /* file stats - inc. mod time   */
    meLine            *dotLine;                 /* Link to "." meLine structure */
    meLine            *markLine;                /* The same as the above two,   */
    meLine            *baseLine;                /* Link to the header meLine    */
    meUByte           *fileName ;               /* File name                    */
    meUByte           *name ;                   /* Buffer name                  */
#if MEOPT_EXTENDED
    meUByte           *modeLineStr ;            /* buffer mode-line format      */
#endif
#if MEOPT_CRYPT
    meUByte           *cryptKey;                /* current encrypted key        */
#endif
#if MEOPT_UNDO
    struct meUndoNode *undoHead ;               /* First undo node              */
    struct meUndoNode *undoTail ;               /* Last undo node               */
    meUInt             undoContFlag ;           /* Was the last undo this com'd?*/ 
#endif
    meInt              autoTime;                /* auto-save time for file      */
    meInt              vertScroll ;             /* Windows top line number      */
    meInt              dotLineNo;               /* current line number          */
    meInt              markLineNo;              /* current mark line number     */
    meInt              lineCount;               /* end line number              */
#if MEOPT_FILEHOOK
    meInt              fhook;                   /* file hook function           */
    meInt              bhook;                   /* b buffer hook function       */
    meInt              dhook;                   /* d buffer hook function       */
    meInt              ehook;                   /* e buffer hook function       */
#endif
#if MEOPT_EXTENDED
    meInt              inputFunc;               /* input handle function        */
#endif
#if MEOPT_IPIPES
    meInt              ipipeFunc;               /* ipipe input handle function  */
#endif
    meInt              histNo;                  /* Buff switch hist no.         */
    meMode             mode;                    /* editor mode of this buffer   */
    meUShort           dotOffset;               /* Offset of "." in above meLine*/
    meUShort           markOffset;              /* but for the "mark"           */
#if MEOPT_LOCALBIND
    meUShort           bindCount;               /* but for the "mark"           */
#endif
#if MEOPT_WORDPRO
    meUShort           fillcol;                 /* Current fill column          */
#endif
#if MEOPT_COLOR
    meScheme           scheme;                  /* Current scheme index         */
#endif
#if MEOPT_HILIGHT
    meScheme           lscheme[meLINE_SCHEME_MAX]; /* line scheme index         */
    meUByte            lschemeNext ;            /* Next line scheme index       */
#endif
    meUByte            fileFlag;                /* file type & attrib flags     */
    meUByte            intFlag;                 /* internal buffer flags        */
    meUByte            windowCount;             /* Count of windows on buffer   */
#if MEOPT_EXTENDED
    meUByte            isWordMask ;             /* isWord lookup table bit mask */
#endif
    meUByte            modeLineFlags ;          /* buffer mode-line flags       */
#if MEOPT_HILIGHT
    meUByte            hilight ;                /* hilight number               */
    meUByte            indent ;                 /* indent number                */
#endif
#if MEOPT_WORDPRO
    meUByte            fillmode;                /* Justification mode           */
#endif
    meUByte            tabWidth;                /* Virtual tab size             */
    meUByte            indentWidth;             /* Real tab size                */
} meBuffer ;


#define meBFFLAG_DIR     0x01
#define meBFFLAG_FTP     0x02
#define meBFFLAG_HTTP    0x04
#define meBFFLAG_BINARY  0x08
#define meBFFLAG_LTDIFF  0x10


#define BIFBLOW    0x01                         /* Buffer is to be blown away (bclear)  */
#define BIFLOAD    0x02                         /* Used on a reload to check tim        */
#define BIFLOCK    0x04                         /* Used in ipipe to flag a lock         */
#define BIFNACT    0x08                         /* The buffer is inactive               */
#define BIFFILE    0x10                         /* The buffer is a file - used at creation only */
#define BIFNODEL   0x20                         /* The buffer cannot be deleted         */
#define BIFNOHOOK  0x40                         /* Don't apply any context file hook    */


/*
 * The starting position of a region, and the size of the region in
 * characters, is kept in a region structure.  Used by the region commands.
 */
typedef struct  {
    meInt              size;                    /* Length in characters.        */
    meInt              lineNo;                  /* Origin meLine number.        */
    meLine            *line;                    /* Origin meLine address.       */
    meUShort           offset;                  /* Origin meLine offset.        */
} meRegion ;


/* structure for the table of initial key bindings */

typedef struct meBind {
    meUShort           code ;                   /* Key code                     */
    meUShort           index ;
    meUInt             arg ;
} meBind;


/* The editor holds deleted text chunks in the meKillNode buffer. The
   kill buffer is logically a stream of ascii characters, however
   due to its unpredicatable size, it gets implemented as a linked
   list of chunks. (The d_ prefix is for "deleted" text, as k_
   was taken up by the keycode structure */

typedef struct meKillNode {
    struct meKillNode *next;                    /* link to next chunk, NULL if last */
    meUByte            data[1] ;                /* First byte of the data (nil trm) */
} meKillNode;

/*
 * meKillNode chunks are pointed at by a linked list, which could be very long indeed,
 * saving all the text deleted per session. In practice the number of deleted
 * chunks is kept to a predefined number. A new (ie most recent) delete is
 * inserted into the head of the chain.
 *
 * The structure after a couple of deletes is therefore:
 *
 *      --------------                  ------------------
 *      | klhead     |  ----------->    | meKillNode     |  ---------> ....
 *      |            |  kill            |                |  next
 *      --------------                  ------------------
 *              |                       most recent delete
 *              | next
 *              V
 *      ----------------                ------------------
 *      | klhead->next |  ----------->  | meKillNode     |  ---------> ....
 *      |              |                |                |  next
 *      ----------------                ------------------
 *              |                       second most recent delete
 *              |
 *              V
 *      ------------------              ------------------
 *      | klhead->next   | -----------> | meKillNode     |  ---------> ....
 *      |       ->next   |              |                |  next
 *      ------------------              ------------------
 *              |                       first delete in session
 *              |
 *              V
 *            N U L L
 *
 * The final thing to know about klist structures is that the kl_type field
 * can take on one of two possible values, if set to meCFKILL, then it contains
 * killed text, if not (ie it is zero) it contains deleted text. The important
 * difference between killed and deleted text is that killed text goes in
 * large chunks, whereas deleted text happens in single character throws. It
 * is therefore possible in single character deletions, to look to see what
 * type of text is stored in a given kill buffer and so know whether to
 * create a new buffer for the soon-to-be-deleted text.
 */

#define meKILL_MAX      15                      /* number of kills held in kill list       */

typedef struct meKill {
    meKillNode        *kill ;                   /* pointer to kill chunk                   */
    struct meKill     *next ;                   /* link to next list element, NULL if last */
} meKill;


/** list of recognized user functions */

#define FUN_ARG1        0x01
#define FUN_ARG2        0x02
#define FUN_ARG3        0x04
#if MEOPT_EXTENDED
#define FUN_SETVAR      0x08
#define FUN_GETVAR      0x10
#endif
#define FUN_MONAMIC     FUN_ARG1
#define FUN_DYNAMIC     (FUN_ARG1|FUN_ARG2)
#define FUN_TRINAMIC    (FUN_ARG1|FUN_ARG2|FUN_ARG3)

#if MEOPT_HILIGHT

/* hilight init flags */
#define HFCASE     0x01
#define HFLOOKBLN  0x02
#define HFTOKEWS   0x04   /* treat a token end as a white space */
#define HFLOOKBSCH 0x08
/* hilight init internal flags */
#define HFRPLCDIFF 0x80

#define meHilightGetFlags(root)           ((root)->ignore)
#define meHilightGetTruncScheme(root)     ((root)->type)
#define meHilightGetLookBackLines(root)   ((int) ((size_t) ((root)->close)))
#define meHilightSetLookBackLines(root,v) ((root)->close = (meUByte *) (v))
#define meHilightGetLookBackScheme(root)  ((root)->tknSttOff)
#define meHilightGetColumnHilight(root)   ((meHilight *) ((root)->rclose))
#define meHilightSetColumnHilight(root,v) ((root)->rclose = (meUByte *) (v))
#define meHilightGetFromColumn(root)      meIntFromPtr((node)->close)
#define meHilightSetFromColumn(root,v)    ((node)->close = mePtrFromInt(v))
#define meHilightGetToColumn(root)        meIntFromPtr((node)->rtoken)
#define meHilightSetToColumn(root,v)      ((node)->rtoken = mePtrFromInt(v))

/* hilight token flags */
#define HLTOKEN    0x0001
#define HLENDLINE  0x0002
#define HLBRACKET  0x0004
#define HLCONTIN   0x0008
#define HLBRINEOL  0x0010
#define HLSTTLINE  0x0020
#define HLREPLACE  0x0040
#define HLBRANCH   0x0080
#define HLSOL      0x0100
#define HLVALID    0x0200
#define HLCOLUMN   0x0400
#define HLREPTOEOL 0x0800
#define HLONELNBT  0x0800
#define HLSNGLLNBT 0x1000

/* hilight token internal flags */
#define HLASOL     0x2000
#define HLASTTLINE 0x4000
#define HLRPLCDIFF 0x8000

/* indent init flags */
#define HICASE     0x01
#define HILOOKBSCH 0x02   /* this indent has a lookback indent scheme */
#define HICMODE    0x04   /* this indent uses the built in cmode      */
#define HIGFBELL   0x08   /* Ring bell if gotoFence fails             */
/* internal indent flags */
#define HIGOTCONT  0x80

#define meHICMODE_SIZE 8
#define meIndentGetFlags(ind)                meHilightGetFlags(ind)
#define meIndentGetLookBackLines(ind)        meHilightGetLookBackLines(ind)
#define meIndentSetLookBackLines(ind,v)      meHilightSetLookBackLines(ind,v)
#define meIndentGetLookBackScheme(ind)       meHilightGetLookBackScheme(ind)
#define meIndentGetStatementIndent(ind)      meIndentGetIndent(ind->token[0],frameCur->bufferCur->indentWidth)
#define meIndentGetBraceIndent(ind)          meIndentGetIndent(ind->token[1],frameCur->bufferCur->indentWidth)
#define meIndentGetBraceStatementIndent(ind) meIndentGetIndent(ind->token[2],frameCur->bufferCur->indentWidth)
#define meIndentGetContinueIndent(ind)       meIndentGetIndent(ind->token[3],frameCur->bufferCur->indentWidth)
#define meIndentGetContinueMax(ind)          meIndentGetIndent(ind->token[4],frameCur->bufferCur->indentWidth)
#define meIndentGetSwitchIndent(ind)         meIndentGetIndent(ind->token[5],frameCur->bufferCur->indentWidth)
#define meIndentGetCaseIndent(ind)           meIndentGetIndent(ind->token[6],frameCur->bufferCur->indentWidth)
#define meIndentGetCommentMargin(ind)        meIndentGetIndent(ind->token[7],frameCur->bufferCur->indentWidth)
#define meIndentGetCommentContinue(ind)      ((ind)->rtoken)

typedef struct meHilight {
    struct meHilight **list ;
    meUByte           *table ; 
    meUByte           *close ;
    meUByte           *rtoken ;
    meUByte           *rclose ;
    meUShort           type ; 
    meScheme           scheme ;
    meUByte            tknSttOff ; 
    meUByte            tknEndOff ; 
    meUByte            clsSttOff ; 
    meUByte            clsEndOff ; 
    meUByte            tknSttTst ; 
    meUByte            tknEndTst ; 
    meUByte            ordSize ; 
    meUByte            listSize ; 
    meUByte            ignore ;
    meUByte            token[1] ;
} meHilight ;

#define TableLower (meUByte)(' ')
#define TableUpper (meUByte)('z')
#define TableSize  ((TableUpper - TableLower) + 3)
#define InTable(cc) (((meUByte)(cc) >= TableLower) &&  \
                     ((meUByte)(cc) <= TableUpper))
#define GetTableSlot(cc)                             \
(((meUByte)(cc) < TableLower) ? 0:                     \
 (((meUByte)(cc) > TableUpper) ? TableSize-2:          \
  ((meUByte)(cc)-TableLower+1)))
#define hilListSize(r) (r->ordSize + r->listSize)

#endif

/*
 * meSchemeSet
 * Hilighting screen structure. The structure contains blocks of style
 * information used to render the screen. Each block describes the style
 * (front and back color + font) and the column on which the style ends.
 */
typedef struct {
    meInt              column;                  /* change column */
    meScheme           scheme;                  /* style index */
} meSchemeSet;    

/*
 * Selection Highlighting 
 * The selection highlighting buffer retains information about the 
 * selection highlighting for the current buffer. The current restriction
 * is that only one buffer may have selection hilighting enabled
 */

typedef struct meSelection {
    meUShort           uFlags;                  /* Hilighting user flags     */
    meUShort           flags;                   /* Hilighting flags          */
    struct meBuffer   *bp;                      /* Selected hilight buffer   */
    meInt              dotLineNo;               /* Dot line number           */
    meInt              markLineNo;              /* Mark line number          */
    meInt              dotOffset;               /* Current line offset       */
    meInt              markOffset;              /* Current mark offset       */
    int                sline;                   /* Start line                */
    int                soff;                    /* Start offset              */
    int                eline;                   /* End line number           */
    int                eoff;                    /* End offset                */
} meSelection;

#define SELHIL_ACTIVE    0x0001                 /* Buffer has been edited    */
#define SELHIL_FIXED     0x0002                 /* Buffer has been edited    */
#define SELHIL_KEEP      0x0004                 /* Buffer has been edited    */
#define SELHIL_CHANGED   0x0008                 /* Hilighting been changed   */
#define SELHIL_SAME      0x0010                 /* Highlighting is same point*/

#define SELHILU_ACTIVE   0x0001                 /* Buffer has been edited    */
#define SELHILU_KEEP     0x0002                 /* Buffer has been edited    */

#define VFMESSL 0x0001                          /* Message line */
#define VFMENUL 0x0002                          /* Menu line changed flag */
#define VFMODEL 0x0004                          /* Mode line */
#define VFMAINL 0x0008                          /* Main line */
#define VFCURRL 0x0010                          /* Current line */
#define VFCHNGD 0x0020                          /* Changed flag */
#define VFSHBEG 0x0100                          /* Start of the selection hilight */
#define VFSHEND 0x0200                          /* End of the selection hilight */
#define VFSHALL 0x0400                          /* Whole line is selection hilighted */

#define VFSHMSK 0x0700                          /* Mask of the flags  */
#define VFTPMSK 0x003f                          /* Mask of the line type */

typedef struct  meVideoLine
{
    meWindow          *wind ;
    meLine            *line ;
    meUShort           endp ;
    meUShort           flag ;                   /* Flags */
    meScheme           eolScheme ;              /* the EOL scheme */
#if MEOPT_HILIGHT
    meUByte            hilno ;                  /* hilight-no */
    meHilight         *bracket ;
#endif
} meVideoLine;


/* Virtual Video Structure.
 * Provides the meVideoLine structure management for horizontal split
 * Windows. A single meVideo exists for each meVideoLine structure.
 */
typedef struct meVideo
{
    meVideoLine       *lineArray;               /* Pointer to the video block */
    meWindow          *window;                  /* Windows attached to video block */
    struct meVideo    *next;                    /* Next video item */
} meVideo;

/* Frame Store
 * The frame store holds the physical screen structure. This comprises the
 * text information and the colour information of each character on the
 * screen.
 *
 * The frame store is required primarily on windowing platforms such as
 * MS-Windows and X-Windows and allows the canvas to be regenerated 
 * without recomputing the contents of the window when the windowing
 * sub-system requires the window (or parts of the window) to be refreshed.
 * 
 * Hence rather than EMACS generating new lines every time we render from
 * the Frame buffer which contains a representation of the screen. (Color +
 * ASCII text information).
 * 
 * Under MS-Windows we receive a WM_PAINT message. This message may be received
 * when we are in focus or not (e.g. a window placed over ours and moved
 * will cause a WM_PAINT messaage for the area of the screen which has
 * become invalid).
 */

typedef struct
{
    meUByte           *text ;                   /* Text held on the line. */
    meScheme          *scheme ;                 /* index to the Style (fore + back + font) of each cell */
} meFrameLine;                                  /* Line of screen text */


#define meFRAME_HIDDEN     0x01
#define meFRAME_NOT_FOCUS  0x02

typedef struct meFrame
{
    struct meFrame    *next ;
    meVideo            video ;                  /* Virtual video - see display.c   */
    meFrameLine       *store ;
    meWindow          *windowList ;             /* Head of list of windows         */
    meWindow          *windowCur ;              /* Current window                  */
    meBuffer          *bufferCur ;              /* Current buffer                  */
    meLine            *mlLine ;                 /* message line.                   */
    meUByte           *mlLineStore ;            /* stored message line.            */
#ifdef _WINDOW
    meUByte           *titleBufferName ;        /* Name of buffer used when last updated the title bar */
#endif
    void              *termData ;               /* generic term data pointer       */
    int                cursorRow ;              /* Location of the cursor in frame */
    int                cursorColumn ;           /* Location of the cursor in frame */
    int                mainRow ;                /* Main Windows current row.       */
    int                mainColumn ;             /* Main Windows current column     */
    int                mlColumn ;               /* ml current column */
    int                mlColumnStore ;          /* ml Store column */
    int                pokeColumnMin ;          /* Minimum column extent of poke   */
    int                pokeColumnMax ;          /* Maximum column extent of poke   */
    int                pokeRowMin ;             /* Minimum row extent of poke      */
    int                pokeRowMax ;             /* Maximum row extent of poke      */
#if MEOPT_EXTENDED
    meInt              id ;                     /* $frame-id                       */
#endif
#if MEOPT_OSD
    meLine            *menuLine ;               /* Menu-poke line                  */
    meUShort           menuDepth ;              /* Terminal starting row           */
#endif
    meShort            windowCount ;            /* Current number of windows       */
    meUShort           width ;                  /* Number of columns               */
    meUShort           depth ;                  /* Number of rows                  */
    meUShort           widthMax ;               /* Maximum number of columns       */
    meUShort           depthMax ;               /* Maximum number of rows          */
    meUByte            mainId ;
    meUByte            flags ;
    meUByte            pokeFlag ;               /* Boolean meTRUE/meFALSE flag. meTRUE
                                                 * when a poke operation has been
                                                 * performed. */
    meUByte            mlStatus ;               /* ml status
                                                 * 0=not using it,
                                                 * 1=using it.
                                                 * 2=using it & its been broken so
                                                 * next time mlerease is used, it will
                                                 * restore */
} meFrame;

#if MEOPT_OSD
#define meFrameGetMenuDepth(ff) ((ff)->menuDepth)
#else
#define meFrameGetMenuDepth(ff) (0)
#endif

#if MEOPT_IPIPES
/* The following is structure required for unix ipipes */

#define meIPIPE_OVERWRITE   0x01
#define meIPIPE_NEXT_CHAR   0x02
#define meIPIPE_CHILD_EXIT  0x04
#define meIPIPE_RAW         0x40        /* No messing with pipe data ! */
#define meIPIPE_BUFIPIPE    0x80        /* Forced ipipe buffer */

typedef struct meIPipe {
    meBuffer          *bp ;
    struct meIPipe    *next ;
    int                pid ;
    meInt              exitCode ;
#ifdef _WIN32
    HANDLE             rfd ;
    HANDLE             outWfd ;
    HANDLE             process ;
    DWORD              processId ;
    HWND               childWnd ;
    /* wait thread variables */
    HANDLE             childActive ;
    HANDLE             threadContinue ;
    HANDLE             thread ;
    DWORD              threadId ;
    meUByte            nextChar ;
#else
    int                rfd ;
    int                outWfd ;
#endif
    meShort            noRows ;
    meShort            noCols ;
    meShort            strRow ;
    meShort            strCol ;
    meShort            curRow ;
    meShort            flag ;
} meIPipe ;

#endif

#if MEOPT_UNDO

typedef int meUndoCoord[2] ;

typedef struct meUndoNode {
    struct meUndoNode *next ;
    union {
        meInt          dotp ;
        meInt         *lineSort ;
        meUndoCoord   *pos ;
    } udata ;
    meInt              count ;
    meUShort           doto ;
    meUByte            type ;
    meUByte            str[1] ;
} meUndoNode ;

#if MEOPT_NARROW
/* The following structure is used for narrow undos, the next and type members
 * must be in the same position as in the main meUndoNode, see asserts in
 * main.c */
typedef struct meUndoNarrow {
    struct meUndoNode *next ;
    union {
        meInt          dotp ;
        void          *dummy ;          /* Maintain structure alignment with ptr union */
    } udata ;
    meInt              count ;
    meScheme           scheme ;
    meUByte            type ;
    meUByte            markupFlag ;
    meInt              name ;
    meInt              markupCmd ;
    meUByte            str[1] ;
} meUndoNarrow ;
#endif

#define meUNDO_DELETE        0x01
#define meUNDO_INSERT        0x02
#define meUNDO_FORWARD       0x04
#define meUNDO_SINGLE        0x08
#define meUNDO_REPLACE       0x10
#define meUNDO_SPECIAL       0x40
#define meUNDO_CONTINUE      0x80

/* when special bit is set the following flags are used for 0x3f bits */
#define meUNDO_SET_EDIT      0x20
#define meUNDO_UNSET_EDIT    0x01
#define meUNDO_NARROW        0x10
#define meUNDO_NARROW_ADD    0x01
#define meUNDO_LINE_SORT     0x08
#define meUNDO_SPECIAL_MASK  0x7C


#define meUndoIsReplace(uu)  (((uu)->type & (meUNDO_SPECIAL|meUNDO_REPLACE)) == meUNDO_REPLACE)
#define meUndoIsSetEdit(uu)  (((uu)->type & meUNDO_SPECIAL_MASK) == (meUNDO_SPECIAL|meUNDO_SET_EDIT))
#define meUndoIsNarrow(uu)   (((uu)->type & meUNDO_SPECIAL_MASK) == (meUNDO_SPECIAL|meUNDO_NARROW))
#define meUndoIsLineSort(uu) (((uu)->type & meUNDO_SPECIAL_MASK) == (meUNDO_SPECIAL|meUNDO_LINE_SORT))
#endif

/* The variable register (#0 - #9) uses a linked structure
 * This is so they can easily be pushed and poped
 */
#define meREGISTER_MAX 10
typedef struct meRegister {
    struct meRegister *prev ;
#if MEOPT_EXTENDED
    meVarList         *varList ;
#endif
    meUByte           *commandName ;
    meUByte           *execstr ;
    int                f ;
    int                n ;
    meUByte            force ;
    meUByte            depth ;
    meUByte            reg[meREGISTER_MAX][meBUF_SIZE_MAX] ;
} meRegister ;


/* Note that the first part of meDirList structure must remain
 * the same as meNamesList as it is used for $file-names variable
 */
typedef struct {
    int                exact ;
    int                size ;
    meUByte          **list ;
    meUByte           *mask ;
    int                curr ;
    meUByte           *path ;
    meFiletime         stmtime ;              /* modification time of dir */
}  meDirList ;

typedef struct {
    int                exact ;
    int                size ;
    meUByte          **list ;
    meUByte           *mask ;
    int                curr ;
} meNamesList ; 

#if MEOPT_REGISTRY
/*
 * REGISTRY
 * ========
 * Definitions for the registry API. The structures remain private
 * within the registry source file, providing a API interface for
 * maintainability.
 */

/*
 * REG_HANDLE
 * Generates a registry ASCII handle which is overlayed into a
 * 32-bit integer. The handle is the lookup from the macro 
 * interface.
 */
#define REG_HANDLE(a,b,c,d) ((((meUInt)(a))<<24)|(((meUInt)(b))<<16)| \
                             (((meUInt)(c))<<8)|((meUInt)(d)))

/* Registry Open types - NOTE any changes to these must be reflected in
 * the variable meRegModeList defined in registry.c */
#define meREGMODE_INTERNAL   0x0001             /* Internal registry - hidden */
#define meREGMODE_HIDDEN     0x0002             /* Node is hidden */
#define meREGMODE_INVISIBLE  0x0004             /* Node is hidden */
#define meREGMODE_FROOT      0x0008             /* File root */
#define meREGMODE_CHANGE     0x0010             /* Tree has changed */
#define meREGMODE_BACKUP     0x0020             /* Perform a backup of the file */
#define meREGMODE_AUTO       0x0040             /* Automatic save */
#define meREGMODE_DISCARD    0x0080             /* Discardable entry (memory only) */
#define meREGMODE_CRYPT      0x0100             /* crypt the registry file */
#define meREGMODE_MERGE      0x0200             /* Merge a loaded registry */
#define meREGMODE_RELOAD     0x0400             /* Reload existing registry */
#define meREGMODE_CREATE     0x0800             /* Create if does not exist */
#define meREGMODE_QUERY      0x1000             /* Query the current node */
#define meREGMODE_GETMODE    0x2000             /* Return modes set in $result */
#define meREGMODE_STORE_MASK 0x01ff             /* Bits actually worth storing */
/*
 * meRegNode
 * Data structure to hold a hierarchy node
 */
typedef struct meRegNode
{
    meUByte           *value;                   /* The value of the node */
    struct meRegNode  *parent;                  /* Pointer to the parent */
    struct meRegNode  *child;                   /* Pointer to the child node */
    struct meRegNode  *next;                    /* Pointer to the sibling node */
    meUShort           mode;                    /* Mode flag */
    meUByte            name[1];                 /* The name of the node */
} meRegNode;

#endif

#ifdef _DRAGNDROP
/* Retain a list of drag and drop files with thier raw screen position and
 * filename for subsequent processing. This is a single linked list of
 * filenames and screen positions. */
struct s_DragAndDrop
{
    struct s_DragAndDrop *next;         /* Next drag and drop event */
#ifdef _WIN32
    /* In MS-Windows then the recieving window is raised and has focus so we
     * do not need to send the frame as this is always the current frame
     * (where we have multiple frames) */
    POINT mousePos;                     /* Position of the mouse */
#else
    /* In UNIX then the receiving frame does not gain focus automatically
     * hence we need to keep the frame information where we are running
     * multiple frames. */
    meUShort mouse_x;
    meUShort mouse_y;
    meFrame *frame;                     /* Eventing Frame */
#endif
    meUByte fname[1];                   /* Filename */
};
#endif
