/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * efunc.h - Command name defintions.
 *
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
 * Created:     Unknown
 * Synopsis:    Command name defintions.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     Includes efunc.def to create a list of built in MicroEmacs commands
 *     within a structure which stores command variables and flags on how
 *     the command effects things like the display and region etc.
 *     A hash table is also created & initialized which is used for rapid
 *     command-name -> function lookups.
 * 
 * Notes:
 *     Adding a new command requires:
 *       * Adding external declaration to eextern.h
 *       * adding new entry to the table given in efunc.def giving the
 *         command name (1st column), function (3rd) & ebind label (4th)
 *       * Determining the command flag (2nd column)
 *       * Correcting the table list pointers (5th Column)
 *       * Adding the command to the built in hash table (see below & 6th)
 *     To check this has been done correctly set KEY_TEST to 1 in exec.c
 *     and execute the !test macro directive.
 * 
 *     The generated list of commands MUST be alphabetically ordered.
 */

#define DEFFUNC(s,t,f,r,n,h)  r,

enum
{
#include        "efunc.def"
    CK_MAX
};

#undef  DEFFUNC

extern int     cmdTableSize;            /* The current size of the command table */
extern meCommand   __cmdArray[];            /* Array of internal commands   */
extern meCommand  *__cmdTable[];            /* initial command table        */
extern meCommand **cmdTable;                /* command table                */
extern meCommand  *cmdHead;                 /* command alpha list head      */
extern meUByte commandFlag[] ;          /* selection hilight cmd flags  */
#if MEOPT_CMDHASH
#define cmdHashSize 511
extern meCommand  *cmdHash[cmdHashSize];    /* command hash lookup table    */
#endif

#define comIgnore     0xff              /* Ignore this command wrt state & hilighting */
#define comSelStart   0x01              /* Start hilighting - remove fix and make active */
#define comSelStop    0x02              /* If active but not fixed then stop, make inactive */
#define comSelKill    0x04              /* Selection kill - remove active and fix */
#define comSelSetDot  0x08              /* Set a new dot position */
#define comSelSetMark 0x10              /* Set a new mark position */
#define comSelSetFix  0x20              /* Fix the selection region */
#define comSelDelFix  0x40              /* Stop hilighting if fixed, does not remove the fix */
#define comSelIgnFail 0x80              /* Don't stop even on a fail */

#define isComIgnore(ix)     (commandFlag[(ix)] == comIgnore)
#define isComSelStart(ix)   (commandFlag[(ix)] & comSelStart)
#define isComSelStop(ix)    (commandFlag[(ix)] & comSelStop)
#define isComSelKill(ix)    (commandFlag[(ix)] & comSelKill)
#define isComSelSetDot(ix)  (commandFlag[(ix)] & comSelSetDot)
#define isComSelSetMark(ix) (commandFlag[(ix)] & comSelSetMark)
#define isComSelSetFix(ix)  (commandFlag[(ix)] & comSelSetFix)
#define isComSelDelFix(ix)  (commandFlag[(ix)] & comSelDelFix)
#define isComSelIgnFail(ix) (commandFlag[(ix)] & comSelIgnFail)

#define getCommandFunc(i)    ((i<CK_MAX) ? cmdTable[i]->func:NULL)
#define getCommandName(i)    (cmdTable[i]->name)
#define getMacro(i)          ((meMacro *) cmdTable[i])
#define getMacroLine(i)      (((meMacro *) cmdTable[i])->hlp)

/*---   If we are in the main program then set up the Name binding table. */

#ifdef  maindef

#define CMDTABLEINITSIZE (((CK_MAX>>5)+1)<<5)

#define DEFFUNC(s,t,f,r,n,h)  t,

meUByte commandFlag[] = 
{
#include        "efunc.def"
};
#undef  DEFFUNC

#if MEOPT_EXTENDED
#if MEOPT_CMDHASH
#define DEFFUNC(s,t,f,r,n,h)          {(meCommand *) n, (meCommand *) h, { (meVariable *) 0, 0}, (meUByte *)s, r, f} ,
#else
#define DEFFUNC(s,t,f,r,n,h)          {(meCommand *) n, { (meVariable *) 0, 0}, (meUByte *)s, r, f} ,
#endif
#else
#if MEOPT_CMDHASH
#define DEFFUNC(s,t,f,r,n,h)          {(meCommand *) n, (meCommand *) h, (meUByte *)s, r, f} ,
#else
#define DEFFUNC(s,t,f,r,n,h)          {(meCommand *) n, (meUByte *)s, r, f} ,
#endif
#endif

meCommand __cmdArray[CK_MAX] = 
{
#include        "efunc.def"
};
#undef  DEFFUNC

int cmdTableSize = CK_MAX ;

#define DEFFUNC(s,t,f,r,n,h)          &__cmdArray[r],

meCommand *__cmdTable[CMDTABLEINITSIZE] = 
{
#include        "efunc.def"
    NULL
};
#undef  DEFFUNC

/* initialize the command name lookup hash table.
 * This is horrid but greatly improves macro language performance without
 * increasing start-up time.
 */
meCommand **cmdTable = __cmdTable ;
meCommand  *cmdHead = &__cmdArray[0] ;

#if MEOPT_CMDHASH
meCommand  *cmdHash[cmdHashSize] = 
{
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    &__cmdArray[CK_SWNDV],      NULL,                       NULL,                       NULL,                       &__cmdArray[CK_GOBOP],
    NULL,                       &__cmdArray[CK_DELFRAME],   &__cmdArray[CK_DELWND],     NULL,                       NULL,
    &__cmdArray[CK_FNDFIL],     NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       &__cmdArray[CK_BAKSRCH],    NULL,                       &__cmdArray[CK_SAVREGY],    NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       &__cmdArray[CK_INSMAC],     NULL,                       &__cmdArray[CK_UNVARG],     NULL,
    NULL,                       NULL,                       NULL,                       &__cmdArray[CK_SVDICTS],    NULL,
    &__cmdArray[CK_SAVSBUF],    NULL,                       &__cmdArray[CK_EXENCMD],    &__cmdArray[CK_INSTAB],     NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       &__cmdArray[CK_DELWBAK],
    &__cmdArray[CK_REDREGY],    &__cmdArray[CK_HIWRD],      &__cmdArray[CK_GOFNC],      NULL,                       NULL,
    NULL,                       NULL,                       NULL,                       &__cmdArray[CK_SUSPEND],    &__cmdArray[CK_WDWWDTH],
    NULL,                       &__cmdArray[CK_BUFMOD],     &__cmdArray[CK_INSFLNM],    NULL,                       NULL,
    &__cmdArray[CK_BAKCHR],     NULL,                       NULL,                       &__cmdArray[CK_YANK],       NULL,
    &__cmdArray[CK_BUFPOS],     NULL,                       NULL,                       &__cmdArray[CK_ONLYWND],    &__cmdArray[CK_REDFILE],
    &__cmdArray[CK_PRTBUF],     &__cmdArray[CK_SWNDH],      &__cmdArray[CK_CRTCLBK],    &__cmdArray[CK_APPBUF],     NULL,
    NULL,                       &__cmdArray[CK_IPIPWRT],    NULL,                       &__cmdArray[CK_GOTOPOS],    &__cmdArray[CK_SETMRK],
    NULL,                       &__cmdArray[CK_SAVBUF],     &__cmdArray[CK_ABTCMD],     NULL,                       NULL,
    &__cmdArray[CK_FNDTAG],     NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       &__cmdArray[CK_DELBLK],     NULL,                       NULL,                       NULL,
    NULL,                       NULL,                       &__cmdArray[CK_LOWWRD],     &__cmdArray[CK_SSWM],       NULL,
    &__cmdArray[CK_WRPWRD],     &__cmdArray[CK_ENDMAC],     &__cmdArray[CK_SHOWCUR],    NULL,                       NULL,
    &__cmdArray[CK_DELBAK],     NULL,                       NULL,                       &__cmdArray[CK_TRNLINE],    &__cmdArray[CK_STRREP],
    NULL,                       NULL,                       NULL,                       NULL,                       &__cmdArray[CK_DEFFMAC],
    NULL,                       NULL,                       &__cmdArray[CK_CHGFONT],    NULL,                       &__cmdArray[CK_NEWLIN],
    NULL,                       NULL,                       &__cmdArray[CK_FILEOP],     NULL,                       NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    &__cmdArray[CK_UPDATE],     NULL,                       NULL,                       NULL,                       NULL,
    &__cmdArray[CK_WDWDPTH],    &__cmdArray[CK_DELSBUF],    &__cmdArray[CK_UNDO],       NULL,                       NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    &__cmdArray[CK_GOBOL],      &__cmdArray[CK_DELFOR],     NULL,                       &__cmdArray[CK_TRNSKEY],    &__cmdArray[CK_NAMBUF],
    NULL,                       NULL,                       NULL,                       NULL,                       &__cmdArray[CK_ABOUT],
    &__cmdArray[CK_LCLBIND],    &__cmdArray[CK_REYANK],     NULL,                       NULL,                       NULL,
    &__cmdArray[CK_DOTAB],      &__cmdArray[CK_GOEOP],      &__cmdArray[CK_BAKLIN],     &__cmdArray[CK_BISRCH],     &__cmdArray[CK_SETCRY],
    NULL,                       &__cmdArray[CK_BAKWND],     &__cmdArray[CK_WRTMSG],     NULL,                       NULL,
    &__cmdArray[CK_PKSCRN],     &__cmdArray[CK_GOEOL],      NULL,                       NULL,                       NULL,
    &__cmdArray[CK_DELTAB],     &__cmdArray[CK_OSDBIND],    NULL,                       NULL,                       &__cmdArray[CK_WRTBUF],
    NULL,                       NULL,                       &__cmdArray[CK_LCLUNBD],    NULL,                       &__cmdArray[CK_DELDICT],
    NULL,                       &__cmdArray[CK_BINDKEY],    &__cmdArray[CK_FILTBUF],    NULL,                       NULL,
    NULL,                       NULL,                       &__cmdArray[CK_OSD],        NULL,                       NULL,
    &__cmdArray[CK_FORLIN],     NULL,                       NULL,                       &__cmdArray[CK_HILIGHT],    &__cmdArray[CK_SORTLNS],
    NULL,                       NULL,                       &__cmdArray[CK_CPYCLIP],    &__cmdArray[CK_KILRECT],    NULL,
    &__cmdArray[CK_KILREG],     NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       NULL,                       &__cmdArray[CK_SETPOS],     NULL,                       NULL,
    &__cmdArray[CK_DEFHELP],    NULL,                       NULL,                       NULL,                       &__cmdArray[CK_FLHOOK],
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    &__cmdArray[CK_PRTCOL],     &__cmdArray[CK_UNSET],      &__cmdArray[CK_SCLPRV],     NULL,                       NULL,
    &__cmdArray[CK_BAKWRD],     NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       NULL,                       &__cmdArray[CK_BUFABBREV],  NULL,                       NULL,
    NULL,                       &__cmdArray[CK_FORSRCH],    NULL,                       NULL,                       NULL,
    NULL,                       &__cmdArray[CK_EXECMD],     NULL,                       NULL,                       NULL,
    &__cmdArray[CK_ADDCOLSCHM], NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       NULL,                       &__cmdArray[CK_MOVUWND],    NULL,                       &__cmdArray[CK_GOBOF],
    NULL,                       &__cmdArray[CK_USEBUF],     &__cmdArray[CK_GLOBMOD],    NULL,                       NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       NULL,                       NULL,                       &__cmdArray[CK_MOVLWND],    &__cmdArray[CK_YANKRECT],
    &__cmdArray[CK_RCASHI],     NULL,                       &__cmdArray[CK_NRRWBUF],    NULL,                       NULL,
    &__cmdArray[CK_DESCVAR],    &__cmdArray[CK_MCRQURY],    NULL,                       NULL,                       &__cmdArray[CK_NBUFMOD],
    NULL,                       &__cmdArray[CK_SFNDBUF],    NULL,                       NULL,                       NULL,
    &__cmdArray[CK_KILEOL],     &__cmdArray[CK_SETVAR],     NULL,                       NULL,                       NULL,
    &__cmdArray[CK_FORCHR],     NULL,                       NULL,                       NULL,                       &__cmdArray[CK_HELPCOM],
    NULL,                       &__cmdArray[CK_ADDRULE],    NULL,                       NULL,                       &__cmdArray[CK_CTOMOUSE],
    &__cmdArray[CK_SPWCLI],     NULL,                       NULL,                       NULL,                       &__cmdArray[CK_DIRTREE],
    &__cmdArray[CK_PREFIX],     NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       &__cmdArray[CK_SCLNXT],     NULL,                       NULL,                       &__cmdArray[CK_SRCHBUF],
    NULL,                       &__cmdArray[CK_SHOWSEL],    NULL,                       NULL,                       NULL,
    &__cmdArray[CK_HUNBAK],     &__cmdArray[CK_SWPMRK],     NULL,                       NULL,                       NULL,
    NULL,                       &__cmdArray[CK_IPIPKLL],    &__cmdArray[CK_CMPBUF],     NULL,                       &__cmdArray[CK_PPPWND],
    &__cmdArray[CK_GOAMRK],     &__cmdArray[CK_ADDDICT],    NULL,                       NULL,                       &__cmdArray[CK_HUNFOR],
    NULL,                       NULL,                       NULL,                       NULL,                       &__cmdArray[CK_DELREGY],
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       NULL,                       &__cmdArray[CK_GOEOF],      NULL,                       NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       &__cmdArray[CK_MOVDWND],    NULL,                       NULL,                       NULL,
    &__cmdArray[CK_EXEMAC],     &__cmdArray[CK_LSTBUF],     NULL,                       NULL,                       &__cmdArray[CK_SPLLWRD],
    NULL,                       NULL,                       &__cmdArray[CK_CMDWAIT],    &__cmdArray[CK_SYSTEM],     &__cmdArray[CK_NXTWND],
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       NULL,                       &__cmdArray[CK_QUOTE],      NULL,                       NULL,
    NULL,                       &__cmdArray[CK_QEXIT],      NULL,                       NULL,                       NULL,
    NULL,                       &__cmdArray[CK_INSSPC],     NULL,                       &__cmdArray[CK_PRTRGN],     NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    NULL,                       NULL,                       &__cmdArray[CK_LSTCOM],     &__cmdArray[CK_DELFWRD],    NULL,
    NULL,                       &__cmdArray[CK_PIPCMD],     &__cmdArray[CK_QREP],       &__cmdArray[CK_HELP],       NULL,
    NULL,                       NULL,                       &__cmdArray[CK_MRKREGY],    &__cmdArray[CK_RCASLOW],    NULL,
    &__cmdArray[CK_EXESTR],     NULL,                       NULL,                       NULL,                       NULL,
    &__cmdArray[CK_CPYREG],     &__cmdArray[CK_ADDCOL],     &__cmdArray[CK_NAMMAC],     NULL,                       NULL,
    &__cmdArray[CK_OPNLIN],     NULL,                       NULL,                       NULL,                       &__cmdArray[CK_DELBUF],
    NULL,                       NULL,                       &__cmdArray[CK_MOVRWND],    NULL,                       NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    &__cmdArray[CK_INSSTR],     NULL,                       &__cmdArray[CK_EXABBREV],   NULL,                       &__cmdArray[CK_EXEBUF],
    NULL,                       NULL,                       &__cmdArray[CK_DEFMAC],     &__cmdArray[CK_MLBIND],     &__cmdArray[CK_FRMDPTH],
    &__cmdArray[CK_ADNXTLN],    NULL,                       NULL,                       NULL,                       &__cmdArray[CK_CHGFIL],
    NULL,                       NULL,                       NULL,                       &__cmdArray[CK_VIWFIL],     &__cmdArray[CK_CRTFRAME],
    NULL,                       NULL,                       &__cmdArray[CK_RESIZEALL],  NULL,                       NULL,
    &__cmdArray[CK_VOIDFUNC],   NULL,                       NULL,                       &__cmdArray[CK_FORWRD],     NULL,
    NULL,                       NULL,                       NULL,                       NULL,                       NULL,
    &__cmdArray[CK_APROPS],     NULL,                       NULL,                       &__cmdArray[CK_SETAMRK],    NULL,
    &__cmdArray[CK_EXIT],       &__cmdArray[CK_STCHRMK],    NULL,                       &__cmdArray[CK_DESCKEY],    NULL,
    &__cmdArray[CK_IPIPCMD],    NULL,                       &__cmdArray[CK_FISRCH],     &__cmdArray[CK_OSDUNBD],    &__cmdArray[CK_SFNDFIL],
    NULL,                       &__cmdArray[CK_INSFIL],     NULL,                       NULL,                       NULL,
    NULL
} ;
#endif

#endif

