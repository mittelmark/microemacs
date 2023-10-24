/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * main.c - Main entry point.
 *
 * Originally written by Dave G. Conroy for MicroEmacs
 * Copyright (C) 1987 by Daniel M. Lawrence
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
 * Synopsis:    Main entry point.
 * Authors:     Dave G. Conroy, Steve Wilhite, George Jones,
 *              Daniel M. Lawrence,
 *              Jon Green & Steven Phillips (JASSPA)
 */

#define	__MAINC			/* Define program name */

#define	INC_MODE_DEF

#include "evers.h"
#include "emain.h"

#define	maindef		/* Make main defintions - cannot define this at the top
                         * because all the main defs are needed to init edef's vars */

#include "efunc.h"	/* function declarations and name table	*/
#include "eskeys.h"     /* Special key names - Must include before edef.h & ebind.h */
#include "edef.h"	/* The main global variables		*/
#include "ebind.h"	/* default bindings			*/
#include "evar.h"	/* env. vars and user funcs		*/
#include "esearch.h"	/* defines PTBEG PTEND			*/

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#ifdef _UNIX
#include <fcntl.h>
#include <sys/wait.h>
#endif
#endif
#include <assert.h>

static char meHelpPage[]=
"Usage     : " ME_SHORTNAME " [options] [files]\n\n"
"Where options are:-\n"
"  @<file> : Setup using <file>[.emf], default is " ME_SHORTNAME ".emf\n"
#if MEOPT_EXTENDED
"  -       : Read input from stdin into *stdin* buffer\n"
#endif
#ifdef _NANOEMACS
"  -a      : Enable auto-saving\n"
"  -B      : Enable back-up file creation\n"
#else
"  -a      : Disable auto-saving\n"
"  -B      : Disable back-up file creation\n"
#endif
"  -b      : Load next file as a binary file\n"
#if MEOPT_EXTENDED
"  -c[name]: Continue session (default session is $user-name)\n"
"  -f      : Don't process following arguments, set .about.arg# vars.\n"
#endif
"  -h      : For this help page\n"
#ifdef _DOS
"  -i      : Insert the current screen into the *scratch* buffer\n"
#endif
"  -k[key] : Load next file as a crypted file optionally giving the <key>\n"
"  -l <n> or +<n>\n"
"          : Go to line <n> in the next given file\n"
"  -l <n>:<c> or +<n>:<c>\n"
"          : Go to line <n> column <c> in the next given file\n"
#if MEOPT_CLIENTSERVER
"  -m <msg>: Post message <msg> to " ME_FULLNAME " server\n"
#endif
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
#ifdef _WIN32
"  -n      : For no MS window, use console instead\n"
#endif
#ifdef _XTERM
"  -n      : For no X window, use console instead\n"
#endif
#endif /* _ME_CONSOLE */
#endif /* _ME_WINDOW */
#if MEOPT_CLIENTSERVER
"  -o      : One " ME_FULLNAME ", use " ME_SHORTNAME " server if available\n"
#endif
#if MEOPT_EXTENDED
"  -P      : Piped with debugging (see -p)\n"
"  -p      : Piped mode, no user interactivity\n"
#endif
"  -R      : Reverse the default color scheme\n"
"  -r      : Read-only, all buffers will be in view mode\n"
"  -s <s>  : Search for string <s> in the next given file\n"
"  -u <n>  : Set user name to <n> (sets $user-name)\n"
"  -V      : Display Version info and exit\n"
"  -v <v=s>: Set variable <v> to string <s>\n"
#ifdef _UNIX
"  -x      : Don't catch signals\n"
#endif
"  -y      : Load next file as a reduced binary file\n"
"\n"
;

/*
 * Initialize the data structures used by the display code. The edge vectors
 * used to access the screens are set up. The operating system's terminal I/O
 * channel is set up. All the other things get initialized at compile time.
 * The original window has "WFCHG" set, so that it will get completely
 * redrawn on the first call to "update".
 */

/*
 * Initialize all of the buffers and windows. The buffer name is passed down
 * as an argument, because the main routine may have been told to read in a
 * file by default, and we want the buffer name to be right.
 */
static void
meInit(meUByte *bname)
{
    meBuffer *bp;

    if (TTstart() == meFALSE)             /* Started ?? */
        meExit(1) ;

    /* add 2 to hilBlockS to allow for a double trunc-scheme change
     * Note: ME is not yet 'initialised' so any meMalloc failure will
     * lead to exit in mlwrite so we don't need to check */
    styleTable = meMalloc(2*meSCHEME_STYLES*sizeof(meStyle)) ;
    hilBlock = meMalloc((hilBlockS+2)*sizeof(meSchemeSet)) ;
    disLineBuff = meMalloc((disLineSize+32)*sizeof(meUByte)) ;

    memcpy(styleTable,defaultScheme,2*meSCHEME_STYLES*sizeof(meStyle));
    /* Set the fore and back colours */
    TTaddColor(meCOLOR_FDEFAULT,255,255,255) ;
    TTaddColor(meCOLOR_BDEFAULT,0,0,0) ;

    /* Create the frame - make sure that we do not access the screen during
     * the re-size */
    screenUpdateDisabledCount++;
    if((frameCur=meFrameInit(NULL)) == NULL)
        meExit(1) ;
#if MEOPT_FRAME
    frameList = frameCur ;
#endif
    screenUpdateDisabledCount--;

    if(((bp = bfind(bname,BFND_CREAT)) == NULL) ||
       (meFrameInitWindow(frameCur,bp) <= 0))
        meExit(1);
    alarmState |= meALARM_INITIALIZED ;
}

/*
 * insertChar
 *
 * Insert n * 'c' chars into the current buffer
 * Copes with insert and undo modes correctly
 */
int
insertChar(register int c, register int n)
{
    /*---    If we are  in overwrite mode,  not at eol,  and next char is
       not a tab or we are at a tab stop, delete a char forword */

    if(meModeTest(frameCur->bufferCur->mode,MDOVER))
    {
        int index, ii=n ;
        for(index=0,n=0 ; (ii>0) && (frameCur->windowCur->dotOffset < frameCur->windowCur->dotLine->length) ; ii--)
        {
            if((meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset) != meCHAR_TAB) ||
               (at_tab_pos(getccol()+index+1,frameCur->bufferCur->tabWidth) == 0))
            {
                lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                meLineSetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset++,c) ;
                index = 0 ;
            }
            else
            {
                index++ ;
                n++ ;
            }
        }
        n += ii ;
    }
    /*---    Do the appropriate insertion */
    if(n > 0)
    {
#if MEOPT_UNDO
        if(n == 1)
        {
            if(lineInsertChar(1,c) <= 0)
                return meFALSE ;
            meUndoAddInsChar() ;
        }
        else
        {
            if(lineInsertChar(n, c) <= 0)
                return meFALSE ;
            meUndoAddInsChars(n) ;
        }
#else
        if(lineInsertChar(n, c) <= 0)
            return meFALSE ;
#endif
    }
    return meTRUE ;
}

/*
 * This is the general command execution routine. It handles the fake binding
 * of all the keys to "self-insert". It also clears out the "thisflag" word,
 * and arranges to move it to the "lastflag", so that the next command can
 * look at it. Return the status of command.
 */
int
execute(register int c, register int f, register int n)
{
    register int index;
    meUInt arg ;
#if MEOPT_EXTENDED
    int ii ;
#endif
    
#if MEOPT_CALLBACK
    {
        /* If any of the modifiers have changed since the last key we executed
         * then execute any commands bound to the M-pick or M-drop pseudo keys,
         * e.g. the S-pick key will be generated between the following 2 keys:
         *      right S-right.
         * These keys only get 'pressed' if a command is bound to them. The
         * commands are executed in a hidden way, so they are not recorded in
         * kbd macros or in the $recent-keys. This could led to a macro failing
         * but alternatives have similar if not worse problems.
         */
        static meUShort lastcc=0 ;
        meUInt arg ;
        meUShort mask, kk, ccx=lastcc ^ c ;

        lastcc = c ;
        for(mask=ME_SHIFT ; mask != ME_SPECIAL ; mask<<=1)
        {
            if(ccx & mask)
            {
                if(c & mask)
                    kk = ME_SPECIAL|SKEY_pick|mask ;
                else
                    kk = ME_SPECIAL|SKEY_drop|mask ;
                if((index=decode_key(kk,&arg)) != -1)
                    execFuncHidden(kk,index,arg) ;
            }
        }
    }
#endif

#if MEOPT_UNDO
    if(kbdmode != mePLAY)
        undoContFlag++ ;
#endif
    lastCommand = thisCommand ;
    lastIndex = thisIndex ;
    thisCommand = c ;

    if(((index = decode_key((meUShort) c,&arg)) >= 0) && (arg != 0))
    {
        f = 1 ;
        n *= (int) (arg + 0x80000000) ;
    }
    thisIndex = index ;
    meRegCurr->f = f ;
    meRegCurr->n = n ;
    /* SWP - 27/2/99 - to enable input commands to not have to jump through
     * so many hoops I've changed the input support so that macros can fail
     * in such a way as to indicate that they have not handled this input,
     * this allows them to pick and choose.
     * The method chosen is to check the command variable status if the macro
     * aborted, if .status is set to "0" then its not handled the input
     */
#if MEOPT_EXTENDED
    if((ii=frameCur->bufferCur->inputFunc) >= 0)
    {
        meUByte *ss, ff ;
        /* set a force value for the execution as the macro is allowed to
         * fail and we don't want to kick in the macro debugging */
        ff = meRegHead->force ;
        meRegHead->force = 1 ;
        if(((cmdstatus = (execFunc(ii,f,n) > 0))) ||
           ((ss=getUsrLclCmdVar((meUByte *)"status",&(cmdTable[ii]->varList))) == errorm) || meAtoi(ss))
        {
            meRegHead->force = ff ;
            return cmdstatus ;
        }
        meRegHead->force = ff ;
    }
#endif
    if(index >= 0)
        return (cmdstatus = (execFunc(index,f,n) > 0)) ;
    if(selhilight.flags)
    {
        selhilight.bp = NULL;
        selhilight.flags = 0;
    }
    if(c < 0x20 || c > 0xff)    /* If not an insertable char */
    {
        meUByte outseq[40];	/* output buffer for keystroke sequence */
        meGetStringFromKey((meUShort) c,outseq);  /* change to something printable */
        lastflag = 0;                             /* Fake last flags.     */
        cmdstatus = 0 ;
#if MEOPT_MOUSE
        /* don't complain about mouse_move* or mouse_time* as these are
         * special keys that are only added if they are bound and due
         * to their frequence they can be in the input buffer before they're
         * unbound leading to this error */
        c &= ~(ME_SHIFT|ME_CONTROL|ME_ALT) ;
        if(((c >= (ME_SPECIAL|SKEY_mouse_move)) && (c <= (ME_SPECIAL|SKEY_mouse_move_3))) ||
           ((c >= (ME_SPECIAL|SKEY_mouse_time)) && (c <= (ME_SPECIAL|SKEY_mouse_time_3))) )
            return meTRUE ;
#endif
        return mlwrite(MWABORT,(meUByte *)"[Key not bound \"%s\"]", outseq); /* complain */
    }

    if (n <= 0)            /* Fenceposts.          */
    {
        lastflag = 0;
        return (cmdstatus = ((n<0) ? meFALSE : meTRUE));
    }
    thisflag = 0;          /* For the future.      */

    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return (cmdstatus = meFALSE) ;

#if MEOPT_WORDPRO
    /* If a space was  typed, fill column is  defined, the argument is non-
     * negative, wrap mode is enabled, and we are now past fill column, and
     * we are not read-only, perform word wrap. */

    if(meModeTest(frameCur->bufferCur->mode,MDWRAP) &&
       ((c == ' ') || (meStrchr(filleos,c) != NULL)) &&
       (frameCur->bufferCur->fillcol > 0) && (n >= 0) &&
       (getccol() > frameCur->bufferCur->fillcol) &&
       !meModeTest(frameCur->bufferCur->mode,MDVIEW))
        wrapWord(meFALSE, 1);
#endif

    /* insert the required number of chars */
    if(insertChar(c,n) <= 0)
        return (cmdstatus = meFALSE) ;

#if MEOPT_HILIGHT
    if(frameCur->bufferCur->indent && (meHilightGetFlags(indents[frameCur->bufferCur->indent]) & HIGFBELL))
    {
        meHilight *indent=indents[frameCur->bufferCur->indent] ;
        if((c == '}') || (c == '#'))
        {
            ii = frameCur->windowCur->dotOffset ;
            frameCur->windowCur->dotOffset = 0 ;
            if(gotoFrstNonWhite() == c)
            {
                frameCur->windowCur->dotOffset = ii ;
                doCindent(indent,&ii) ;
            }
            else
                frameCur->windowCur->dotOffset = ii ;
        }
        else if((meIndentGetCommentMargin(indent) >= 0) &&
                ((c == '*') || (c == '/')) && (frameCur->windowCur->dotOffset > 2) &&
                (meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset-2) == '/') &&
                (meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset-3) != '/'))
        {
            ii = frameCur->windowCur->dotOffset - 2 ;
            if((gotoFrstNonWhite() == 0) &&
               (frameCur->windowCur->dotOffset=0,(gotoFrstNonWhite() != 0)) &&
               (frameCur->windowCur->dotOffset < ii))
            {
                frameCur->windowCur->dotOffset = ii ;
                if(ii > meIndentGetCommentMargin(indent))
                    ii = (((int)(frameCur->bufferCur->indentWidth)-1) -
                          ((ii-1)%(int)(frameCur->bufferCur->indentWidth))) ;
                else
                    ii = meIndentGetCommentMargin(indent) - ii ;
                lineInsertChar(ii,' ') ;
#if MEOPT_UNDO
                meUndoAddInsChars(ii) ;
#endif
            }
            else
                frameCur->windowCur->dotOffset = ii ;
            frameCur->windowCur->dotOffset += 2 ;
        }
    }
#endif
#if MEOPT_FENCE
    /* check for fence matching */
    if(meModeTest(frameCur->bufferCur->mode,MDFENCE) && ((c == '}') || (c == ')') || (c == ']')))
    {
        frameCur->windowCur->dotOffset-- ;
        /* flag for delay move and only bell in cmode */
        gotoFence(meTRUE,(frameCur->bufferCur->indent && (meHilightGetFlags(indents[frameCur->bufferCur->indent]) & HIGFBELL)) ? 3:2) ;
        frameCur->windowCur->dotOffset++ ;
    }
#endif
    lastflag = thisflag;
    return (cmdstatus = meTRUE) ;
}

/*
** Me info
** Creates a *info* buffer and populates it with info on the current state of things.
*/

static void
addModesList(meBuffer *bp, register meUByte *buf, meUByte *name, meMode mode,
             int res)
{
    register int ii, nlen, len, ll ;

    meStrcpy(buf,name) ;
    len = nlen = meStrlen(name) ;
    for (ii = 0; ii < MDNUMMODES; ii++)	/* add in the mode flags */
        if((meModeTest(mode,ii) != 0) == res)
        {
            ll = meStrlen(modeName[ii]);
            if(len+ll >= 78)
            {
                buf[len] = '\0' ;
                addLineToEob(bp,buf) ;
                len = nlen ;
                memset(buf,' ',len) ;
            }
            buf[len++] = ' ' ;
            meStrcpy(buf+len, modeName[ii]);
            len += ll ;
        }
    buf[len] = '\0' ;
    addLineToEob(bp,buf) ;
}

static void
addModesLists(meBuffer *bp, register meUByte *buf, meMode mode)
{
    addModesList(bp,buf,(meUByte *)"  Modes on  :",mode,1) ;
    addModesList(bp,buf,(meUByte *)"  Modes off :",mode,0) ;
    addLineToEob(bp,(meUByte *)"") ;
}

meUByte meCopyright[]="Copyright (C) 1988-" meCENTURY meYEAR " " ME_COMPANY_NAME " (" ME_COMPANY_SITE ")" ;
int
meAbout(int f, int n)
{
    meWindow *wp ;
    meBuffer *bp, *tbp ;
    meInt   numchars ;		/* # of chars in file */
    meInt   numlines ;		/* # of lines in file */
    meInt   predchars ;		/* # chars preceding point */
    meInt   predlines ;		/* # lines preceding point */
    meUByte buf[meBUF_SIZE_MAX] ;
    int     ii ;

    if((wp = meWindowPopup(BaboutN,BFND_CREAT|BFND_CLEAR|WPOP_USESTR,NULL)) == NULL)
        return meFALSE ;
    bp = wp->buffer ;

     /* definitions in evers.h */
    addLineToEob(bp,(meUByte *)ME_FULLNAME " " meVERSION " - Date " meCENTURY meDATE " - " meSYSTEM_NAME "\n\nGlobal Status:") ;
    tbp = bheadp ;
    ii = 0 ;
    while(tbp != NULL)
    {
        ii++ ;
        tbp = tbp->next ;
    }
    sprintf((char *)buf,"  # buffers : %d", ii) ;
    addLineToEob(bp,buf) ;
    addModesLists(bp,buf,globMode) ;
    sprintf((char *)buf,"Current Buffer Status:\n  Buffer    : %s", frameCur->bufferCur->name) ;
    addLineToEob(bp,buf) ;
    sprintf((char *)buf,"  File name : %s",
            (frameCur->bufferCur->fileName == NULL) ? (meUByte *)"":frameCur->bufferCur->fileName) ;
    addLineToEob(bp,buf) ;

    getBufferInfo(&numlines,&predlines,&numchars,&predchars) ;
    sprintf((char *)buf,"  Lines     : Total %6d, Current %6d\n  Characters: Total %6d, Current %6d",
            numlines,predlines,numchars,predchars) ;
    addLineToEob(bp,buf) ;

    addModesLists(bp,buf,frameCur->bufferCur->mode) ;
    addLineToEob(bp,meCopyright) ;
    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotOffset = 0 ;
    bp->dotLineNo = 0 ;
    meModeClear(bp->mode,MDEDIT);     /* don't flag this as a change */
    meModeSet(bp->mode,MDVIEW);       /* put this buffer view mode */
    resetBufferWindows(bp) ;
    return meTRUE ;
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out. Normally bound to "C-X C-C".
 */
int
exitEmacs(int f, int n)
{
    int s, ec=0 ;
    char buff[128] ;

#if MEOPT_EXTENDED
    /* Set the exit code */
    if(n & 0x04)
    {
        if(meGetString((meUByte *)"Exit code", 0, 0, (meUByte *) buff,128) <= 0)
            return meFALSE ;
        ec = meAtoi(buff) ;
    }
    else
#endif
        if(n & 0x20)
            ec = 1 ;

    /* (Un)conditionally save buffers, dictionary and registry files */
    if((n & 0x02) &&
       ((saveSomeBuffers(f,(n & 0x01)) <= 0)
#if MEOPT_SPELL
        || (dictionarySave(f,2|(n & 0x01)) <= 0)
#endif
#if MEOPT_REGISTRY
        || (saveRegistry(f,2|(n & 0x01)) <= meFALSE)
#endif
       ))
        return meFALSE ;

    s = 1 ;
    if(n & 0x01)
    {
        if(anyChangedBuffer())
            strcpy(buff,"Modified buffer") ;
        else
            buff[0] = '\0' ;
#if MEOPT_SPELL
        if(anyChangedDictionary())
        {
            strcat(buff,(buff[0] == '\0') ? "Modified":",") ;
            strcat(buff," dictionary") ;
        }
#endif
#if MEOPT_REGISTRY
        if(anyChangedRegistry())
        {
            strcat(buff,(buff[0] == '\0') ? "Modified":",") ;
            strcat(buff," registry") ;
        }
#endif
#if MEOPT_IPIPES
        if(anyActiveIpipe())
        {
            strcat(buff,(buff[0] == '\0') ? "A":" and a") ;
            strcat(buff,"ctive process") ;
        }
#endif
        if(buff[0] != '\0')
        {
            /* somethings changed - check the user is happy */
            strcat(buff," exists, leave anyway") ;
            s = mlyesno((meUByte *)buff) ;
        }
    }

    if(s > 0)
    {
        meBuffer *bp, *nbp ;

#ifdef _CLIPBRD
        /* don't mess with the system clipboard from this point - could be a problem for a shut-down macro */
        clipState |= CLIP_DISABLED ;
#endif
        /* If bit 0x08 is set then create autosaves for any changed buffers,
         * otherwise remove any autosaves */
        bp = bheadp ;
        while (bp != NULL)
        {
            if(bufferNeedSaving(bp))
            {
                if (n & 0x08)
                    autowriteout(bp) ;
                else
                    autowriteremove(bp) ;
            }
            bp = bp->next;            /* on to the next buffer */
        }
#if MEOPT_CALLBACK
        {
            /* call the shut-down command if its bound */
            meUInt arg ;
            int index ;
            if((index = decode_key(ME_SPECIAL|SKEY_shut_down,&arg)) >= 0)
                execFunc(index,1,n) ;
        }
#endif
#if MEOPT_CLIENTSERVER
        /* the client-server is a psuedo ipipe but it needs
         * to be stopped properly, therefore stop it first as
         * the following ipipeRemove only does half a job and
         * TTkillClientServer will be very confused if called after
         */
        TTkillClientServer() ;
#endif
        /* go round removing all the active ipipes,
         * execute any dhooks set on active buffers
         * and remove all auto-write files
         */
#if MEOPT_IPIPES
        while(ipipes != NULL)
            ipipeRemove(ipipes) ;
#endif
#if MEOPT_FILEHOOK
        /* execute ehooks for all current buffers */
        meFrameLoopBegin() ;
        if(loopFrame->bufferCur->ehook >= 0)
            execBufferFunc(loopFrame->bufferCur,loopFrame->bufferCur->ehook,0,1) ;
        meFrameLoopEnd() ;
#endif
        /* For all buffers remove the autosave and execute the dhook */
        bp = bheadp ;
        while(bp != NULL)
        {
            nbp = bp->next ;
#if MEOPT_FILEHOOK
            if(!meModeTest(bp->mode,MDNACT) && (bp->dhook >= 0))
                execBufferFunc(bp,bp->dhook,0,1) ;     /* Execute the delete hook */
#endif
            bp = nbp ;
        }
#if MEOPT_SOCKET
        ffFileOp(NULL,NULL,meRWFLAG_NOCONSOLE|meRWFLAG_FTPCLOSE,-1) ;
#endif
        TTend();

#ifdef _ME_CONSOLE
#ifdef _TCAP
        if(!(alarmState & (meALARM_PIPED|meALARM_DIE))
#ifdef _ME_WINDOW
           && (meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
           )
            TCAPputc('\n');
#endif /* _TCAP */
#endif /* _ME_CONSOLE */

#ifdef _ME_FREE_ALL_MEMORY
        {
            /* free of everything we can */
            extern void freeToken(meHilight * root) ;
            extern void meFrameFree(meFrame *frame) ;
            extern void dirFreeMemory(void) ;
            extern void printFreeMemory(void) ;
            extern void osdFreeMemory(void) ;
            extern void regFreeMemory(void) ;
            extern void srchFreeMemory(void) ;
            extern void TTfreeTranslateKey(void) ;
            extern meUInt *colTable ;
            mePosition   *pos ;
            meBuffer     *bc, *bn ;
            meFrame      *fc, *fn ;
            meMacro      *mac ;
            meVariable   *nuv, *cuv ;
            meKill       *thiskl ;
            meKillNode   *next, *kill ;
            meAbbrev     *abrev ;
            int           ii, jj ;

            /* remove all but a simple *scratch* */
            bc = bheadp ;
            while(bc != NULL)
            {
                bn = bc->next ;
                bc->intFlag |= BIFBLOW ;
                zotbuf(bc,1) ;
                bc = bn ;
            }
            addFileHook(meTRUE,0) ;

            dictionaryDelete(1,6) ;
            spellRuleAdd(1,0) ;
            dirFreeMemory() ;
            printFreeMemory() ;
            osdFreeMemory() ;
            regFreeMemory() ;
            srchFreeMemory() ;
            TTfreeTranslateKey() ;

            meNullFree(mlBinds) ;
            meFree(hilBlock) ;
#ifdef _XTERM
            meNullFree(colTable) ;
#endif
            meNullFree(disLineBuff) ;
            meNullFree(searchPath) ;
            meNullFree(homedir) ;
            meNullFree(curdir) ;

            for(ii=0 ; ii<numStrHist ; ii++)
                free(strHist[ii]) ;
            for(ii=0 ; ii<numBuffHist ; ii++)
                free(buffHist[ii]) ;
            for(ii=0 ; ii<numCommHist ; ii++)
                free(commHist[ii]) ;
            for(ii=0 ; ii<numFileHist ; ii++)
                free(fileHist[ii]) ;
            for(ii=0 ; ii<numSrchHist ; ii++)
                free(srchHist[ii]) ;
            free(strHist) ;

            while(klhead != NULL)
            {
                thiskl = klhead ;
                klhead = klhead->next ;
                kill = thiskl->kill ;
                while(kill != NULL)
                {
                    next = kill->next;
                    free(kill);
                    kill = next;
                }
                free(thiskl) ;
            }
            while((pos=position) != NULL)
            {
                position = pos->next ;
                free(pos) ;
            }
            for(ii=0 ; ii<CK_MAX ; ii++)
            {
                cuv = cmdTable[ii]->varList.head ;
                while(cuv != NULL)
                {
                    nuv = cuv->next ;
                    meNullFree(cuv->value) ;
                    free(cuv) ;
                    cuv = nuv ;
                }
            }
            for( ; ii<cmdTableSize ; ii++)
            {
                mac = getMacro(ii) ;
                meFree(mac->name) ;
                meNullFree(mac->fname) ;
                meLineLoopFree(mac->hlp,1) ;
                cuv = mac->varList.head ;
                while(cuv != NULL)
                {
                    nuv = cuv->next ;
                    meNullFree(cuv->value) ;
                    free(cuv) ;
                    cuv = nuv ;
                }
                free(mac) ;
            }
            if(cmdTable != __cmdTable)
                free(cmdTable) ;
            cuv = usrVarList.head ;
            while(cuv != NULL)
            {
                nuv = cuv->next ;
                meNullFree(cuv->value) ;
                free(cuv) ;
                cuv = nuv ;
            }
            meFree(meRegHead) ;
            meFree(meProgName) ;
            if(modeLineStr != orgModeLineStr)
                meNullFree(modeLineStr) ;
            meNullFree(flNextFileTemp) ;
            meNullFree(flNextLineTemp) ;

            meNullFree(meUserName) ;
            meNullFree(meUserPath) ;

            meNullFree(rcsFile) ;
            meNullFree(rcsCiStr) ;
            meNullFree(rcsCiFStr) ;
            meNullFree(rcsCoStr) ;
            meNullFree(rcsCoUStr) ;
            meNullFree(rcsUeStr) ;

            {
                extern meNamesList buffNames ;
                extern meDirList   fileNames ;
                extern meNamesList commNames ;
                extern meNamesList modeNames ;
                extern meNamesList varbNames ;

                meNullFree(curDirList.path) ;
                freeFileList(curDirList.size,curDirList.list) ;
                meNullFree(fileNames.mask) ;
                meNullFree(fileNames.path) ;
                freeFileList(fileNames.size,fileNames.list) ;
                meNullFree(modeNames.mask) ;
                meNullFree(commNames.list) ;
                meNullFree(commNames.mask) ;
                meNullFree(buffNames.list) ;
                meNullFree(buffNames.mask) ;
                if(varbNames.list != NULL)
                    freeFileList(varbNames.size,varbNames.list) ;
                meNullFree(varbNames.mask) ;
            }

            if(noNextLine > 0)
            {
                for(ii=0 ; ii<noNextLine ; ii++)
                {
                    meFree(nextName[ii]) ;
                    for(jj=0 ; jj<nextLineCnt[ii] ; jj++)
                        meFree(nextLineStr[ii][jj]) ;
                    meNullFree(nextLineStr[ii]) ;
                }
                meFree(nextLineCnt) ;
                meFree(nextLineStr) ;
                meFree(nextName) ;
            }
#if MEOPT_HILIGHT
            if(noHilights > 0)
            {
                for(ii=0 ; ii < noHilights ; ii++)
                {
                    if(hilights[ii] != NULL)
                    {
                        hilights[ii]->close = NULL ;
                        hilights[ii]->rtoken = NULL ;
                        freeToken(hilights[ii]) ;
                    }
                }
                meFree(hilights) ;
            }
            if(noIndents > 0)
            {
                for(ii=0 ; ii < noIndents ; ii++)
                {
                    if(indents[ii] != NULL)
                    {
                        indents[ii]->close = NULL ;
                        indents[ii]->rtoken = NULL ;
                        freeToken(indents[ii]) ;
                    }
                }
                meFree(indents) ;
            }
#endif
            meNullFree (fileIgnore) ;

            while(aheadp != NULL)
            {
                abrev = aheadp ;
                aheadp = abrev->next ;
                meLineLoopFree(&(abrev->hlp),0) ;
                meFree(abrev) ;
            }
            meFree(styleTable) ;

            fc = frameList ;
            while(fc != NULL)
            {
                fn = fc->next ;
                meFrameFree(fc) ;
                fc = fn ;
            }
            /* as we have deleted all frames the *scratch* is not show, so we
             * don't need a replacement so ztbuf does the lot! */
            bheadp->intFlag |= BIFBLOW ;
            zotbuf(bheadp,1) ;
        }
#endif
#ifdef _ME_WIN32_FULL_DEBUG
        {
            _CrtMemState ss ;

            _CrtMemCheckpoint( &ss );
            _CrtMemDumpStatistics(&ss) ;
            _CrtCheckMemory() ;
            _CrtDumpMemoryLeaks() ;
        }
#endif
        meExit(ec);
    }
    mlerase(MWCLEXEC);

    return s ;
}

#if MEOPT_EXTENDED
/*
 * Better quit command, query saves buffers and then queries the exit
 * if modified buffers still exist
 */
int
saveExitEmacs(int f, int n)
{
    return exitEmacs(1,3) ;
}
#endif

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit emacs, otherwise simply exit.
 */
int
quickExit(int f, int n)
{
    if(n == 0)
        n = 0x10 ;
    else if(n == 2)
        n = 0x08 ;
    else
        n = 0x02 ;
    return exitEmacs(1,n) ;
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
int
ctrlg(int f, int n)
{
    /* stop recording - execution will stop at this point so there's no
     * point continuing, this provides an easy get out if the user forgets
     * the macro end record key */
    if(kbdmode == meRECORD)
    {
        kbdmode = meSTOP ;
        frameAddModeToWindows(WFMODE) ;  /* update ALL mode lines */
    }
    if(n)
        mlwrite(MWABORT,(meUByte *)"[Aborted]");
    return meABORT ;
}

/*
** function to report a commands not available (i.e. compiled out!) */
int
notAvailable(int f, int n)
{
    return mlwrite(MWABORT,(meUByte *)"[Command not available]");
}

/*
** function to report a no mark set error */
int
noMarkSet(void)
{
    return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"No mark set in this window");
}

/* tell the user that this command is illegal while we are in
   VIEW (read-only) mode                */
int
rdonly(void)
{
    return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Command illegal in VIEW mode]");
}

/* void function, does nothing but return false if an argument of zero past,
 * else return true. Used to get rid of unwanted unbounded-key bells
 */
int
voidFunc(int f, int n)
{
    return ((n) ? meTRUE:meFALSE) ;
}

int
prefixFunc(int f, int n) /* dummy prefix function */
{
    return meTRUE ;
}
int
uniArgument(int f, int n)         /* dummy function for binding to universal-argument */
{
    return meTRUE ;
}

#ifdef _UNIX
/* sigchild; Handle a SIGCHILD action. Note for the older types of signal it
 * is not necessary to reset the signal handler because they "reappear" as
 * soon as the signal handler is reset - hence we do it at the bottom */
static void
sigchild(SIGNAL_PROTOTYPE)
{
    meWAIT_STATUS status ;              /* Status return from waitpid() */

#if MEOPT_IPIPES
    if(noIpipes != 0)
    {
        meIPipe *ipipe ;

        ipipe = ipipes ;
        while(ipipe != NULL)
        {
            if((ipipe->pid > 0) &&
               (meWaitpid(ipipe->pid,&status,WNOHANG) == ipipe->pid))
            {
                if(WIFEXITED(status))
                {
                    ipipe->pid = -4 ;
                    ipipe->exitCode = WEXITSTATUS(status) ;
                }
                else if(WIFSIGNALED(status))
                    ipipe->pid = -3 ;
#ifdef WCOREDUMP
                else if(WCOREDUMP(status))
                    ipipe->pid = -2 ;
#endif
            }
            ipipe = ipipe->next ;
        }
    }
#endif
    /* clear up any zoombie children if we are not running a piped command */
    if((alarmState & meALARM_PIPE_COMMAND) == 0)
        meWaitpid(-1,&status,WNOHANG) ;

    /* Reload the signal handler. Note that the child is a special case where
     * the signal is reset at the end of the handler as opposed to the head of
     * the handler. For the POSIX or BSD signals this is not required. */
#if !((defined _POSIX_SIGNALS) || (defined _BSD_SIGNALS))
    signal (SIGCHLD, sigchild);
#endif
}
#endif

#if (defined _UNIX) || (defined _WIN32)
int
meDie(void)
{
    /* To get here we have received a signal and am about to die! Ensure that
     * we are in a DIE state to prevent any output on the display which is
     * being torn down. Our main purpose is to preserve the session
     * information and history. */
    alarmState = meALARM_DIE ;

    return exitEmacs(1,0x28) ;
}
#endif

void
autoSaveHandler(void)
{
    struct meTimeval tp ;
    register meBuffer *bp ;
    register meInt tim, next=0x7fffffff ;

    gettimeofday(&tp,NULL) ;
    tim = ((tp.tv_sec-startTime)*1000) + (tp.tv_usec/1000) ;
    bp  = bheadp;
    while (bp != NULL)
    {
        if(bp->autoTime >= 0)
        {
            if(bp->autoTime <= tim)
                autowriteout(bp) ;
            else if(bp->autoTime < next)
                next = bp->autoTime ;
        }
        bp = bp->next ;
    }
    if(next != 0x7fffffff)
        timerSet(AUTOS_TIMER_ID,next,next-tim) ;
    else
        timerClearExpired(AUTOS_TIMER_ID) ;
    if(tim & 0x40000000)
        adjustStartTime(tp.tv_sec-startTime) ;
}

#ifdef _UNIX
static void
sigeat(SIGNAL_PROTOTYPE)
{
    /* Reload the signal handler. For the POSIX or BSD signals this is not
     * required. */
#if !((defined _POSIX_SIGNALS) || (defined _BSD_SIGNALS))
    signal (sig, sigeat);
#endif
    /*
     * Trap nasty signals under UNIX. Write files and quit.
     * Can't just do that cos god knows what me is doing (broken links etc).
     * So can only flag request here and hope its seen.
     */
    alarmState |= meALARM_DIE ;
}
#endif    /* _UNIX */

int commandDepth=0 ;
void
doOneKey(void)
{
    register int    c;
    register int    f;
    register int    n;
    register int    mflag;
    int     basec;              /* c stripped of meta character   */

    update(meFALSE);                          /* Fix up the screen    */

    /*
     * If we are not playing or recording a macro. This is the ONLY place
     * where we may get an idle key back from the input stream. Set the kdb
     * mode to IDLE to indicate that we will accept an idle key.
     *
     * kdbmode is probably not the best variable to use. However it is global
     * and it is not currntly being used - hence it is available for abuse !!
     */
    if (kbdmode == meSTOP)
        kbdmode = meIDLE;             /* In an idle state  */

    c = meGetKeyFromUser(meFALSE, 1, meGETKEY_COMMAND);     /* Get a key */

    if (frameCur->mlStatus & MLSTATUS_CLEAR)
        mlerase(MWCLEXEC) ;

    f = meFALSE;
    n = 1;

    /* do ME_PREFIX1-# processing if needed */
    basec = c & ~ME_PREFIX1 ;        /* strip meta char off if there */
    if((c & ME_PREFIX1) && (((basec >= '0') && (basec <= '9')) || (basec == '`') || (basec == '-')))
    {
        f = meTRUE;
        if(basec == '-')
        {
            mflag = -1 ;
            n = 0;
        }
        else if(basec == '`')
        {
            mflag = 1 ;
            n = 99999999 ;
        }
        else
        {
            mflag = 1 ;
            n = basec - '0' ;
        }
        while((c=meGetKeyFromUser(meTRUE,(n * mflag),meGETKEY_COMMAND)) >= '0')
        {
            if(c <= '9')
                n = n * 10 + (c - '0');
            else if(c == '`')
                n = 99999999 ;
            else
                break ;
        }
        n *= mflag;    /* figure in the sign */
    }

    /* do ^U repeat argument processing */
    if(c == reptc)
    {                           /* ^U, start argument   */
        f = meTRUE;               /* In case not set */
        mflag = 1;              /* current minus flag */
        for(;;c = meGetKeyFromUser(f,n,meGETKEY_COMMAND))
        {
            switch(c)
            {
            case 21 :                   /* \U (^U) */
                n *= 4;
                continue;    /* Loop for more */
            case '-':
                /* If '-', and argument is +ve, make arg -ve */
                mflag = -1;
                if(n < 0)
                    break;
                n *= -1;
                continue;    /* Loop for more */
            }
            /* Not ^U or first '-' .. drop out */
            break;
        }
        /* handle "^U n" to give "n" (GNU Emacs does this) */
        if ((c >= '0' && c <= '9'))
        {
            for(n=c-'0' ;;)
            {
                /*
                 * get the next key, if a digit, update the
                 * count note that we do not handle "-" here
                 */
                c = meGetKeyFromUser(meTRUE,(mflag*n),meGETKEY_COMMAND);
                if(c >= '0' && c <= '9')
                    n = n * 10 + (c - '0');
                else
                    break;
            }
            n *= mflag;    /* figure in the sign */
        }
    }

    if(f == meTRUE)        /* Zap count from cmd line ? */
        mlerase(MWCLEXEC);
    commandDepth++ ;
    execute(c, f, n) ;   /* Do it. */
    commandDepth-- ;
}

void
mesetup(int argc, char *argv[])
{
    meBuffer *bp, *mainbp ;
    int     carg,rarg;                  /* current arg to scan */
    int     noFiles=0 ;
    meUByte  *file=NULL ;
#ifdef _UNIX
    int     sigcatch=1 ;                /* Dont catch signals */
#endif
#ifdef _DOS
    int     dumpScreen=0 ;
#endif
#if MEOPT_CLIENTSERVER
    meUByte  *clientMessage=NULL ;
    int     userClientServer=0 ;
#endif
    startTime = (meInt) time(NULL) ;
    
    /* asserts to check that the defines are consistent */
#if MEOPT_NARROW
    /* more info is required to undo a narrow than can be held in the main
     * structure so an alternative definition is used, but elements within it
     * must remain the same */
    assert(&(((meUndoNode *) NULL)->next) == &(((meUndoNarrow *) NULL)->next)) ;
    assert(&(((meUndoNode *) NULL)->count) == &(((meUndoNarrow *) NULL)->count)) ;
    assert(&(((meUndoNode *) NULL)->type) == &(((meUndoNarrow *) NULL)->type)) ;
#endif
#ifdef _UNIX
    /* Get the usr id and group id for mode-line and file permissions */
    meXUmask = umask(0);
    umask(meXUmask);
    meXUmask = (meXUmask & (S_IROTH|S_IWOTH|S_IXOTH|S_IRGRP|S_IWGRP|S_IXGRP|S_IRUSR|S_IWUSR|S_IXUSR)) ^
              (S_IROTH|S_IWOTH|S_IXOTH|S_IRGRP|S_IWGRP|S_IXGRP|S_IRUSR|S_IWUSR|S_IXUSR) ; /* 00666 */
    meUmask = meXUmask & (S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP|S_IRUSR|S_IWUSR) ;
    meUid = getuid();
    meGid = getgid();
    /* get a list of groups the user belongs to */
    meGidSize = getgroups(0,NULL) ;
    if(meGidSize > 1)
    {
        meGidList = meMalloc(meGidSize*sizeof(gid_t)) ;
        meGidSize = getgroups(meGidSize,meGidList) ;
    }
    else
        meGidSize = 0 ;
    
    /* Set the required alarms first so we always have them */
    /* setup alarm process for timers */
#ifdef _POSIX_SIGNALS
    {
        struct sigaction sa ;

        sigemptyset(&meSigHoldMask) ;
        sigaddset(&meSigHoldMask,SIGCHLD);
        sigaddset(&meSigHoldMask,SIGALRM);
        meSigRelease() ;

        sigemptyset(&sa.sa_mask) ;
        sa.sa_flags=0;
        sa.sa_handler=sigAlarm ;
        sigaction(SIGALRM,&sa,NULL) ;
        /* sigset(SIGALRM,sigAlarm);*/
        /* setup child process exit capture for ipipe */
        sa.sa_handler=sigchild ;
        sigaction(SIGCHLD,&sa,NULL) ;
        /* sigset(SIGCHLD,sigchild);*/
    }
#else /* _POSIX_SIGNALS */
#ifdef _BSD_SIGNALS
    /* Set up the hold mask */
    meSigHoldMask = sigmask (SIGCHLD) | sigmask (SIGALRM);
#endif /* _BSD_SIGNALS */
    signal (SIGCHLD, sigchild);
    signal (SIGALRM, sigAlarm);
#endif /* _POSIX_SIGNALS */
#endif /* _UNIX */
    count_key_table() ;

    /* Init the registers - Make the head registers point back to themselves so that
     * accessing #p? gets #g? and not a core-dump */
    if((meRegHead = meMalloc(sizeof(meRegister))) == NULL)
        exit(1) ;
    meRegHead->prev = meRegHead ;
    meRegHead->commandName = NULL ;
    meRegHead->execstr = NULL ;
#if MEOPT_EXTENDED
    meRegHead->varList = NULL ;
#endif
    meRegHead->force = 0 ;
    meRegHead->depth = 0 ;
    meRegCurr = meRegHead ;
    /* Initialise the head as this is dumped in list-variables */
    carg = meREGISTER_MAX ;
    while(--carg >= 0)
        meRegHead->reg[carg][0] = '\0' ;

    /* initialize the editor and process the command line arguments */
    initHistory() ;                     /* allocate history space */
    
    /* scan through the command line and get all global options */
    carg = rarg = 1 ;
    for( ; carg < argc; ++carg)
    {
        /* if its a switch, process it */
        if (argv[carg][0] == '-')
        {
            switch (argv[carg][1])
            {
            case 'a':
                meModeToggle(globMode,MDAUTOSV) ;
                break ;

            case 'B':
                meModeToggle(globMode,MDBACKUP) ;
                break ;

#if MEOPT_EXTENDED
            case 0:      /* -  get input from stdin */
                noFiles = 1 ;
#endif
            case 'b':    /* -b bin flag */
            case 'k':    /* -k crypt flag */
            case 'y':    /* -y rbin flag */
                /* don't want these options yet as they apply to the
                 * next file on the command line
                 */
                argv[rarg++] = argv[carg] ;
                break ;
#if MEOPT_EXTENDED
            case 'c':
                SetUsrLclCmdVar((meUByte *) "session-name",(meUByte *) (argv[carg] + 2),&usrVarList) ;
                break ;
            case 'f':
                {
                    meUByte vname[16] ;
                    int nn=0 ;
                    while(++carg < argc)
                    {
                        sprintf((char *) vname,"arg%d",nn++) ;
                        SetUsrLclCmdVar(vname,(meUByte *) argv[carg],&(__cmdArray[CK_ABOUT].varList)) ;
                    }
                    sprintf((char *) vname,"%d",nn) ;
                    SetUsrLclCmdVar((meUByte *) "argc",vname,&(__cmdArray[CK_ABOUT].varList)) ;
                }
                break ;
#endif
            case 'h':
                mePrintMessage(meHelpPage) ;
                meExit(0) ;
#ifdef _DOS
            case 'i':
                dumpScreen = 1 ;
                break ;
#endif
            case 'l':    /* -l for initial goto line */
                /* don't want these options yet as they apply to the
                 * next file on the command line
                 */
                argv[rarg++] = argv[carg] ;
                if (argv[carg][2] == 0)
                {
                    if (carg == argc-1)
                    {
missing_arg:
                        sprintf((char *)evalResult,"%s Error: Argument expected with option %s\nOption -h gives further help\n",
                                argv[0],argv[carg]);
                        mePrintMessage(evalResult) ;
                        meExit(1);
                    }
                    argv[rarg++] = argv[++carg] ;
                }
                break ;

#if MEOPT_CLIENTSERVER
            case 'm':    /* -m send a client server message */
                userClientServer |= 1 ;
                if(argv[carg][2] == '\0')
                {
                    if (carg == argc-1)
                        goto missing_arg ;
                    clientMessage = (meUByte *)argv[++carg] ;
                }
                else
                    clientMessage = (meUByte *)argv[carg]+2 ;
                break;
#endif
#ifdef _ME_CONSOLE
            case 'n':
#ifdef _ME_WINDOW
                meSystemCfg |= meSYSTEM_CONSOLE ;
#endif /* _ME_WINDOW */
                break ;
#endif /* _ME_CONSOLE */
#if MEOPT_CLIENTSERVER
            case 'o':
                userClientServer |= 2 ;
                break ;
#endif
#if MEOPT_EXTENDED
            case 'p':
                alarmState |= meALARM_PIPED_QUIET;
            case 'P':
                alarmState |= meALARM_PIPED;
                meSystemCfg |= meSYSTEM_PIPEDMODE;
                break ;
#endif
            case 'r':
                meModeToggle(globMode,MDVIEW) ;
                break ;

            /* Reverse the video, simply exchange the forward and reverse
             * schemes with each other. Note that this affects the internal
             * default scheme only and is over-ridden by any other schemes
             * that are installed. */
            case 'R':
                {
                    int ii;
                    for (ii = 0; ii < meSCHEME_STYLES*2; ii++)
                    {
                        if (defaultScheme[ii] == meSTYLE_NDEFAULT)
                            defaultScheme[ii] = meSTYLE_RDEFAULT;
                        else
                            defaultScheme[ii] = meSTYLE_NDEFAULT;
                    }
                    /* Fix the cursor color */
                    cursorColor = (meColor) meStyleGetFColor(defaultScheme[meSCHEME_NORMAL]);
                }
                break;

            case 's':    /* -s for initial search string */
                argv[rarg++] = argv[carg] ;
                if(argv[carg][2] == '\0')
                {
                    if (carg == argc-1)
                        goto missing_arg ;
                    argv[rarg++] = argv[++carg] ;
                }
                break;

            case 'u':    /* -u for setting the user name */
                {
                    char *un ;
                    if (argv[carg][2] == 0)
                    {
                        if (carg == argc-1)
                            goto missing_arg ;
                        un = argv[++carg] ;
                    }
                    else
                        un = argv[carg] + 2 ;
                    meStrrep(&meUserName,(meUByte *) un) ;
                    break;
                }

             case 'V':
                sprintf((char *)evalResult,"%s %s - Date %s%s - %s\n",
                        ME_FULLNAME, meVERSION, meCENTURY, meDATE, meSYSTEM_NAME) ;
                mePrintMessage(evalResult) ;
                meExit(0) ;

            case 'v':
                {
                    char *ss, *tt, cc ;
                    if (argv[carg][2] == 0)
                    {
                        if (carg == argc-1)
                            goto missing_arg ;
                        ss = argv[++carg] ;
                    }
                    else
                        ss = argv[carg] + 2 ;
                    if((tt = strchr(ss,'=')) == NULL)
                    {
                        sprintf((char *)evalResult,"%s Error: Mal-formed -v option\n",argv[0]) ;
                        mePrintMessage(evalResult) ;
                        meExit(1) ;
                    }

                    if(((cc=getMacroTypeS(ss)) != TKREG) && (cc != TKVAR) &&
                       (cc != TKCVR) && (cc != TKENV))
                    {
                        *tt = '\0' ;
                        sprintf((char *)evalResult,"%s Error: Cannot set variable [%s] from the command-line\n",argv[0],ss) ;
                        mePrintMessage(evalResult) ;
                        meExit(1) ;
                    }
                    if((tt = strchr(ss,'=')) != NULL)
                    {
                        *tt++ = '\0' ;
                        if(setVar((meUByte *)ss,(meUByte *)tt,meRegCurr) <= 0)  /* set a variable */
                        {
                            sprintf((char *)evalResult,"%s Error: Unable to set variable [%s]\n",argv[0],ss) ;
                            mePrintMessage(evalResult) ;
                            meExit(1) ;
                        }
                        execstr = NULL ;
                        clexec = meFALSE ;
                    }
                    break ;
                }
#ifdef _UNIX
            case 'x':
                sigcatch = 0 ;
                break;
#endif

            default:
                {
                    sprintf((char *)evalResult,"%s Error: Unknown option %s\nOption -h gives further help\n",argv[0],argv[carg]) ;
                    mePrintMessage(evalResult) ;
                    meExit(1) ;
                }
            }
        }
        else if(argv[carg][0] == '@')
            file = (meUByte *) argv[carg] + 1 ;
        else if((argv[carg][0] == '+') && (argv[carg][1] == '\0'))
            goto missing_arg ;
        else
        {
            argv[rarg++] = argv[carg] ;
            noFiles = 1 ;
        }
    }
    /* Set up the path information. */
    meSetupPathsAndUser(argv[0]) ;

#if MEOPT_CLIENTSERVER
    if(userClientServer && TTconnectClientServer())
    {
        int      binflag=0 ;         /* load next file as a binary file*/
        meInt    gline = 0 ;         /* what line? (-l option)         */
        meUShort gcol = 0 ;
        meUByte  buff[meBUF_SIZE_MAX+32] ;
        int      nn ;
        char    *ss ;

        for(carg=1 ; carg < rarg ; carg++)
        {
            /* if its a switch, process it */
            if(argv[carg][0] == '-')
            {
                switch(argv[carg][1])
                {
                case 'l':    /* -l for initial goto line */
                    if (argv[carg][2] == 0)
                        ss = argv[++carg] ;
                    else
                        ss = (argv[carg])+2 ;
                    gline = meAtoi(ss);
                    if((ss=strchr(ss,':')) != NULL)
                        gcol = (meUShort) meAtoi(ss+1) ;
                    break;
                case 'b':    /* -b bin flag */
                    binflag |= BFND_BINARY ;
                    break;
                case 'k':    /* -k crypt flag */
                    binflag |= BFND_CRYPT ;
                    break;
                case 's':
                    /* -s not supported across client-server */
                    if (argv[carg][2] == 0)
                        carg++ ;
                    break ;
                case 'y':    /* -y rbin flag */
                    binflag |= BFND_RBIN ;
                    break;
                }
            }
            else if (argv[carg][0] == '+')
            {
                gline = meAtoi((argv[carg])+1);
                if((ss=strchr(argv[carg],':')) != NULL)
                    gcol = (meUShort) meAtoi(ss+1) ;
            }
            else
            {
                /* set up a buffer for this file */
                findFileList((meUByte *)argv[carg],(BFND_CREAT|BFND_MKNAM|binflag),gline,gcol) ;
                gline = 0 ;
                gcol = 0 ;
                binflag = 0 ;
            }
        }
        if(clientMessage != NULL)
        {
            buff[0] = '"' ;
            meStrncpy(buff+1,clientMessage,meBUF_SIZE_MAX) ;
            meStrcat(buff+1,"\"") ;
            token(buff,resultStr) ;
            nn = meStrlen(resultStr) ;
            if((nn <= 1) || (resultStr[nn-1] != '\n'))
            {
                resultStr[nn++] = '\n' ;
                resultStr[nn] = '\0' ;
            }
            TTsendClientServer(resultStr+1) ;
        }
        bp = bheadp ;
        while(bp != NULL)
        {
            if(bp->fileName != NULL)
            {
                nn = 1 ;
                if(meModeTest(bp->mode,MDBINARY))
                    nn |= BFND_BINARY ;
                if(meModeTest(bp->mode,MDCRYPT))
                    nn |= BFND_CRYPT ;
                if(meModeTest(bp->mode,MDRBIN))
                    nn |= BFND_RBIN ;
                sprintf((char *)buff,"C:ME:%d find-file \"%s\"\n",nn,bp->fileName) ;
                TTsendClientServer(buff) ;
                if(bp->dotLineNo != 0)
                {
                    sprintf((char *)buff,"C:ME:goto-line %d\n",(int) bp->dotLineNo) ;
                    TTsendClientServer(buff) ;
                }
            }
            bp = bp->next ;
        }
        if(userClientServer & 2)
            /* send a 'make-current' command to server */
            TTsendClientServer((meUByte *) "C:ME:2 popup-window\n") ;
#ifdef _WIN32
        /* send a WM_USR message to the main window to wake ME up */
        SendMessage(baseHwnd,WM_USER,1,0) ;
        /* if only a -o (not -m) then pause before exiting to give the server me a chance to
         * load the files, the file could be a temp which will be removed once this me exits */
        if(userClientServer == 2)
            Sleep(250) ;
#endif
        meExit(0) ;
    }
    else if(userClientServer & 1)
    {
        sprintf((char *)evalResult,"%s Error: Unable to connect to server\n",argv[0]) ;
        mePrintMessage(evalResult) ;
        meExit(1) ;
    }
#endif

    meInit(BmainN);           /* Buffers, windows.    */

#ifdef _DOS
    if(dumpScreen)
    {
        extern void TTdump(meBuffer *) ;
        TTdump(frameCur->bufferCur) ;
        windowGotoBob(meFALSE,1) ;
        carg++ ;
    }
#endif

    mlerase(0);                /* Delete command line */
    /* disable screen updates to reduce the flickering and startup time */
    screenUpdateDisabledCount = -9999 ;
    /* run me.emf unless an @... arg was given in which case run that */
    execFile(file,meTRUE,noFiles) ;

    /* initalize *scratch* colors & modes to global defaults & check for a hook */
    if((mainbp=bfind(BmainN,0)) != NULL)
    {
        meModeCopy(mainbp->mode,globMode) ;
#if MEOPT_COLOR
        mainbp->scheme = globScheme;
#endif
#if MEOPT_FILEHOOK
        setBufferContext(mainbp) ;
#endif
        /* make *scratch*'s history number very low so any other
         * buffer is preferable */
        mainbp->histNo = -1 ;
    }
#if MEOPT_COLOR
#if MEOPT_CLIENTSERVER
    /* also initialize the client server color scheme */
    if((ipipes != NULL) && (ipipes->pid == 0))
        ipipes->bp->scheme = globScheme;
#endif
#endif
    screenUpdateDisabledCount = 0 ;
#ifdef _CLIPBRD
    /* allow interaction with the clipboard now that me has initialized */
    clipState &= ~CLIP_DISABLED ;
#endif

    {
        meUByte  *searchStr=NULL, *cryptStr=NULL ;
        int       binflag=0 ;         /* load next file as a binary file*/
        meInt     gline = 0 ;         /* what line? (-l option)         */
        meUShort  gcol = 0 ;
        char     *ss ;
        int       stdinflag=0 ;
        int       obufHistNo ;

        obufHistNo = bufHistNo ;
        noFiles = 0 ;
        /* scan through the command line and get the files to edit */
        for(carg=1 ; carg < rarg ; carg++)
        {
            /* if its a switch, process it */
            if(argv[carg][0] == '-')
            {
                switch(argv[carg][1])
                {
#if MEOPT_EXTENDED
                case 0:
                    bufHistNo = obufHistNo + rarg - carg ;
                    bp = bfind(BstdinN,BFND_CREAT|binflag);
                    bp->dotLineNo = gline ;
                    bufHistNo = obufHistNo + rarg - carg + 1 ;
                    bp->histNo = bufHistNo ;
                    bp->intFlag |= BIFFILE ;
                    noFiles++ ;
#ifdef _WIN32
                    meio.rp = GetStdHandle(STD_INPUT_HANDLE) ;
#else
                    meio.rp = stdin ;
#endif
                    stdinflag = 1 ;
                    goto handle_stdin ;
#endif
                case 'l':    /* -l for initial goto line */
                    if (argv[carg][2] == 0)
                        ss = argv[++carg] ;
                    else
                        ss = (argv[carg])+2 ;
                    gline = meAtoi(ss);
                    if((ss=strchr(ss,':')) != NULL)
                        gcol = (meUShort) meAtoi(ss+1) ;
                    break;
                case 'b':    /* -b bin flag */
                    binflag |= BFND_BINARY ;
                    break;
                case 'k':    /* -k crypt flag */
                    binflag |= BFND_CRYPT ;
                    if (argv[carg][2] != 0)
                        cryptStr = (meUByte *) argv[carg] + 2 ;
                    break;
                case 's':
                    /* -s not supported across client-server */
                    if (argv[carg][2] == 0)
                        searchStr = (meUByte *) argv[++carg] ;
                    else
                        searchStr = (meUByte *) argv[carg]+2 ;
                    break ;
                case 'y':    /* -y rbin flag */
                    binflag |= BFND_RBIN ;
                    break;
                }
            }
            else if(argv[carg][0] == '+')
            {
                gline = meAtoi((argv[carg])+1);
                if((ss=strchr(argv[carg],':')) != NULL)
                    gcol = (meUShort) meAtoi(ss+1) ;
            }
            else
            {
                /* set up a buffer for this file - force the history so the first file
                 * on the command-line has the highest bufHistNo so is shown first */
                bufHistNo = obufHistNo + rarg - carg ;
                noFiles += findFileList((meUByte *)argv[carg],(BFND_CREAT|BFND_MKNAM|binflag),gline,gcol) ;
                if((cryptStr != NULL) || (searchStr != NULL))
                {
#if MEOPT_EXTENDED
handle_stdin:
#endif
                    /* Deal with -k<key> and -s <search> */
                    bp = bheadp ;
                    while(bp != NULL)
                    {
                        if(bp->histNo == bufHistNo)
                        {
#if MEOPT_CRYPT
                            if(cryptStr != NULL)
                                setBufferCryptKey(bp,cryptStr) ;
#endif
                            if((searchStr != NULL) || stdinflag)
                            {
                                meBuffer *cbp ;
                                int histNo, flags ;
                                cbp = frameCur->bufferCur ;
                                histNo = cbp->histNo ;
                                swbuffer(frameCur->windowCur,bp) ;
                                if(searchStr != NULL)
                                {
                                    flags = ISCANNER_QUIET ;
                                    if(meModeTest(bp->mode,MDMAGIC))
                                        flags |= ISCANNER_MAGIC ;
                                    if(meModeTest(bp->mode,MDEXACT))
                                        flags |= ISCANNER_EXACT ;
                                    /* what can we do if we fail to find it? */
                                    iscanner(searchStr,0,flags,NULL) ;
                                }
                                swbuffer(frameCur->windowCur,cbp) ;
                                cbp->histNo = histNo ;
                                bp->histNo = bufHistNo ;
                            }
                        }
                        bp = bp->next ;
                    }
                    cryptStr = NULL ;
                    searchStr = NULL ;
                }
                gcol = 0 ;
                gline = 0 ;
                binflag = 0 ;
                stdinflag = 0 ;
            }
        }
        bufHistNo = obufHistNo + rarg ;
    }

    if(noFiles > 0)
    {
        if((bp = replacebuffer(NULL)) != mainbp)
        {
            swbuffer(frameCur->windowCur,bp) ;
	    mainbp->histNo = -1 ;
            if((noFiles > 1) && ((bp = replacebuffer(NULL)) != mainbp) &&
               (windowSplitDepth(meTRUE,2) > 0))
            {
                swbuffer(frameCur->windowCur,replacebuffer(NULL)) ;
                windowGotoPrevious(meFALSE,1) ;
            }
        }
    }
#ifdef _UNIX
    /*
     * Catch nasty signals when running under UNIX unless the -x
     * option appeared on the command line.
     *
     * These can't be trapped as caused by ME failure and will cause loop
     *
     * signal(SIGILL, sigeat);
     * signal(SIGTRAP, sigeat);
     * signal(SIGBUS, sigeat);
     * signal(SIGSEGV, sigeat);
     * signal(SIGSYS, sigeat);
     * signal(SIGIOT, sigeat);
     * signal(SIGEMT, sigeat);
     */
    if(sigcatch)
    {
#ifdef _POSIX_SIGNALS
        struct sigaction sa ;

        sigemptyset(&sa.sa_mask) ;
        sa.sa_flags=0;
        sa.sa_handler=sigeat ;
        sigaction(SIGHUP,&sa,NULL) ;
        sigaction(SIGINT,&sa,NULL) ;
        sigaction(SIGQUIT,&sa,NULL) ;
        sigaction(SIGTERM,&sa,NULL) ;
        sigaction(SIGUSR1,&sa,NULL) ;
#else /* _POSIX_SIGNALS */
        signal(SIGHUP,sigeat) ;
        signal(SIGINT,sigeat) ;
        signal(SIGQUIT,sigeat) ;
        signal(SIGTERM,sigeat) ;
        signal(SIGUSR1,sigeat) ;
#endif /* _POSIX_SIGNALS */
    }
#endif /* _UNIX */

    /* setup to process commands */
    lastflag = 0;                       /* Fake last flags.     */

    /* Set the screen for a redraw at this point cos its after the
     * startup which can screw things up
     */
    sgarbf = meTRUE;			 /* Erase-page clears */

    carg = decode_fncname((meUByte *)"start-up",1) ;
    if(carg >= 0)
        execFunc(carg,meFALSE,1) ;
#if MEOPT_EXTENDED
    if(alarmState & meALARM_PIPED)
    {
        /* if we got here then the exit strategy of the pipe script failed - get out */
        mlwrite(MWABORT|MWSTDERRWRT,(meUByte *)"[Error: Pipe script failed to exit!]");
        alarmState = meALARM_DIE;
        exitEmacs(1,0x30);
    }
#endif

}

#ifndef NDEBUG

/* _meAssert
 * The receiving end of the macro meAssert(<boolean>). This keeps microEMACS
 * up allowing the debugger to be attached. This is essential under Windows
 * since Microsofts assert() is the pits and invariably just exits the
 * program without invoking the debugger.
 */

void
_meAssert (char *file, int line)
{
    meUByte buf[meBUF_SIZE_MAX];                  /* String buffer */
    meUByte cc;                           /* Character input */

    /* Put out the string */
    sprintf ((char *) buf,
             "!! ASSERT FAILED %s:%d !! - <S>ave. <Q>uit. <C>ontinue",
             file, line);
    mlwrite (MWABORT, buf);

    TTinflush ();                       /* Flush the input buffer */
    for (;;)
    {
        cc = (meUByte) TTgetc() ;         /* Get character from keyboard */
        if (cc == 'S')
        {
            /* Try and perform an emergency save for the user - no guarantees
             * here I am afraid - we may be totally screwed at this point in
             * time. */
            alarmState = meALARM_DIE ;
            exitEmacs(1,0x28) ;
        }
        else if (cc == 'Q')
            meExit (1);                 /* Die the death */
        else if (cc == 'C')
            break;                      /* Let the sucker go !! */
    }
}
#endif

#if MEOPT_EXTENDED
int
commandWait(int f, int n)
{
    meVarList *varList=NULL ;
    meUByte clexecSv ;
    int execlevelSv ;
    meUByte *ss ;

    if(f && n)
    {
        f = clexec ;
        clexec = meFALSE ;
        if(n < 0)
            TTsleep(0-n,1,NULL) ;
        else
            TTsleep(n,0,NULL) ;
        clexec = f ;
        return meTRUE ;
    }
    if((meRegCurr->commandName == NULL) ||
       ((f=decode_fncname(meRegCurr->commandName,1)) < 0) ||
       ((varList = &(cmdTable[f]->varList)) == NULL))
        return meTRUE ;

    if(n == 0)
    {
        TTsleep(-1,0,varList) ;
    }
    else
    {
        clexecSv = clexec;
        execlevelSv = execlevel ;
        clexec = meFALSE ;
        execlevel = 0 ;
        do
        {
            /* Fix up the screen before waiting for input otherwise TTahead
             * will be true and the screen will never be refreshed - test with gdiff */
            update(meFALSE);
            TTsleep(-1,1,varList) ;
            if(((ss=getUsrLclCmdVar((meUByte *)"wait",varList)) == errorm) || !meAtoi(ss))
                break ;
            doOneKey() ;
            if(TTbreakFlag)
            {
                TTinflush() ;
                if((selhilight.flags & (SELHIL_ACTIVE|SELHIL_KEEP)) == SELHIL_ACTIVE)
                    selhilight.flags &= ~SELHIL_ACTIVE ;
                TTbreakFlag = 0 ;
            }
        } while((varList != NULL) &&
                ((ss=getUsrLclCmdVar((meUByte *)"wait",varList)) != errorm) && meAtoi(ss)) ;
        clexec = clexecSv ;
        execlevel = execlevelSv ;
    }
    return meTRUE ;
}
#endif

#ifndef _WIN32
int
main(int argc, char *argv[])
{
    //return(0);
    mesetup(argc,argv) ;
#ifdef _ME_CONSOLE
    //printf("\033[?47h\n");
#endif          
    while(1)
    {
        doOneKey() ;
        if(TTbreakFlag)
        {
            TTinflush() ;
            if((selhilight.flags & (SELHIL_ACTIVE|SELHIL_KEEP)) == SELHIL_ACTIVE)
                selhilight.flags &= ~SELHIL_ACTIVE ;
            TTbreakFlag = 0 ;
        }

#ifdef _DRAGNDROP
        /* if drag and drop is enabled then process the drag and drop
         * files now. Note we make the invocation here as we
         * know we have returned to a base state; any macro's would have
         * been aborted. However it is better that macro debugging
         * is disabled so explicitly disable it. */
        if (dadHead != NULL)
        {
            struct s_DragAndDrop *dadp; /* Drag and drop pointer */
#if MEOPT_FRAME
            meFrame *startFrame;        /* Starting frame */
            startFrame = frameCur;
#endif
            /* Disable the cursor to allow the mouse position to be
             * artificially moved */
#if MEOPT_DEBUGM
            macbug = 0;                 /* Force macro debugging off */
#endif
            /* Iterate down the list and get the files. */
            while ((dadp = dadHead) != NULL)
            {
#if MEOPT_FRAME
                /* Change to the correct frame */
                meFrameMakeCur(dadp->frame, 1);
#endif

#if MEOPT_MOUSE
                /* Re-position the mouse */
                mouse_X = clientToCol (dadp->mouse_x);
                mouse_Y = clientToRow (dadp->mouse_y);
                if (mouse_X > frameCur->width)
                    mouse_X = frameCur->width;
                if (mouse_Y > frameCur->depth)
                    mouse_Y = frameCur->depth;

                /* Find the window with the mouse */
                setCursorToMouse (meFALSE, 0);
#endif
#if MEOPT_EXTENDED
                /* if the current window is locked to a buffer find another */
                if(frameCur->windowCur->flags & meWINDOW_LOCK_BUFFER)
                    meWindowPopup(NULL,WPOP_MKCURR|WPOP_USESTR,NULL) ;
#endif

                /* Find the file into buffer */
                findSwapFileList (dadp->fname,BFND_CREAT|BFND_MKNAM,0,0);

                /* Destruct the list */
                dadHead = dadp->next;
                meFree (dadp);
            }

#if MEOPT_FRAME
            /* Restore the starting frame */
            meFrameMakeCur (startFrame, 1);
#endif

            /* Display a message indicating last trasaction */
            mlwrite (0,(meUByte *) "Drag and Drop transaction completed");
        }
#endif /* _DRAGNDROP */
    }
    return 0 ;
}
#endif
