/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * line.c - Line handling operations.
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
 * Synopsis:    Line handling operations.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     The functions in this file are a general set of line management
 *     utilities. They are the only routines that touch the text. They also
 *     touch the buffer and window structures, to make sure that the
 *     necessary updating gets done. There are routines in this file that
 *     handle the kill buffer too. It isn't here for any good reason.
 *
 * Notes:
 *     This code only updates the dot and mark values in the window list.
 *     Since all the code acts on the current window, the buffer that we are
 *     editing must be being displayed, which means that "windowCount" is non zero,
 *     which means that the dot and mark values in the buffer headers are
 *     nonsense.
 */

#define	__LINEC				/* Define filename */

#include "emain.h"
#include "efunc.h"

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#endif

#include <assert.h>

static meKillNode **currkill=NULL ;	/* current kill buffer */
static meKill *reyankLastYank=NULL;

static meLine *
meLineShrink(meLine *lp, int newlen)
{
    int unused = meLineGetMaxLength(lp) - newlen ;
    if(unused > 0xff)
    {
        meLine *nlp ;
        
        unused = meLineMallocSize(newlen+meLINE_BLOCK_SIZE) ;
        if((nlp = meRealloc(lp,unused)) != lp)
        {
            meWindow *wp ;
            assert (nlp != NULL) ;
            nlp->next->prev = nlp ;
            nlp->prev->next = nlp ;
            meFrameLoopBegin() ;
            wp = loopFrame->windowList;                    /* Update windows       */
            while (wp != NULL)
            {
                if(wp->buffer == frameCur->bufferCur)
                {
                    if(wp->dotLine == lp)
                        wp->dotLine = nlp ;
                    if(wp->markLine == lp)
                        wp->markLine = nlp ;
                }
                wp = wp->next;
            }
            meFrameLoopEnd() ;
            lp = nlp ;
        }
        lp->unused = unused - newlen - meLINE_SIZE ;
    }
    else
        lp->unused = unused ;
    lp->length = newlen ;
    return lp ;
}

/****************************************************************************
 * This routine allocates a block of memory large enough to hold a meLine
 * containing "used" characters. The block is always rounded up a bit. Return
 * a pointer to the new block, or NULL if there isn't any memory left. Print a
 * message in the message line if no space.
 ******************************************************************************/

meLine *
meLineMalloc(int length, int editLine)
{
    register meLine *lp ;
    register size_t ss ;
    
    if(!editLine)
        ss = meLineMallocSize(length) ;
    else
        ss = meLineMallocSize(length+meLINE_BLOCK_SIZE) ;
    if((lp = malloc(ss)) != NULL)
    {
        lp->length = length ;
        lp->unused = ss - length - meLINE_SIZE ;
        lp->text[length] = '\0' ;
        /* Always flag the line as changed, this may seem redundant but some
         * macros can blow aware a buffer and reconstruct an almost identical
         * buffer which use the same lines (eg *buffer-info* toolbar tool) and
         * unless the lines are flagged as changed the update does not happen
         * correctly */
        lp->flag = meLINE_CHANGED ;
    }
    else
        mlwrite(MWCURSOR|MWABORT|MWWAIT,(meUByte *)"Warning! Malloc failure") ;
    return lp ;
}

void
meLineResetAnchors(meInt flags, meBuffer *bp, meLine *lp, meLine *nlp,
                   meUShort offset, meInt adjust)
{
    /*
     * The line "lp" is marked by an alphabetic mark. Look through
     * the linked list of marks for the one that references this
     * line and update to the new line
     */
    register meAnchor *p=bp->anchorList;
    
    /* remove the AMARK flag */
    lp->flag &= ~meLINE_ANCHOR ;
    
    while(p != NULL)
    {
        if(p->line == lp)
        {
            if(((flags & (meLINEANCHOR_IF_LESS|meLINEANCHOR_IF_GREATER)) == 0) ||
               ((flags & meLINEANCHOR_IF_GREATER) && (p->offs > offset)) ||
               ((flags & meLINEANCHOR_IF_LESS) && (p->offs <= offset)))
            {
                p->line = nlp ;
                nlp->flag |= meAnchorGetLineFlag(p) ;
                if(flags & meLINEANCHOR_FIXED)
                    p->offs = (meUShort) adjust ;
                else if(flags & meLINEANCHOR_RELATIVE)
                    p->offs = (meUShort) (((meInt) p->offs) + adjust) ;
            }
            else if((flags & meLINEANCHOR_COMPRESS) && (p->offs > (offset + adjust)))
            {
                p->line = nlp ;
                nlp->flag |= meAnchorGetLineFlag(p) ;
                p->offs = (meUShort) (offset + adjust) ;
            }
            else
                lp->flag |= meAnchorGetLineFlag(p) ;
        }
        p = p->next;
    }
}

/*
 * This routine gets called when a character is changed in place in the current
 * buffer. It updates all of the required flags in the buffer and window
 * system. The flag used is passed as an argument; if the buffer is being
 * displayed in more than 1 window we change EDIT t HARD. Set MODE if the
 * mode line needs to be updated (the "*" has to be set).
 */
int
bufferSetEdit(void)
{
    if(meModeTest(frameCur->bufferCur->mode,MDVIEW))
        return rdonly() ;
    if(!meModeTest(frameCur->bufferCur->mode,MDEDIT))      /* First change, so     */
    {
        if((frameCur->bufferCur->name[0] != '*') &&
           (bufferOutOfDate(frameCur->bufferCur)) &&
           (mlyesno((meUByte *)"File changed on disk, really edit") <= 0))
            return ctrlg(meTRUE,1) ;
        meModeSet(frameCur->bufferCur->mode,MDEDIT);
        frameAddModeToWindows(WFMODE) ;
#if MEOPT_UNDO
        if(meModeTest(frameCur->bufferCur->mode,MDUNDO))
        {
            meUndoNode        *uu ;

            uu = meUndoCreateNode(sizeof(meUndoNode)) ;
            uu->type |= meUNDO_SPECIAL|meUNDO_SET_EDIT ;
            /* Add and narrows, must get the right order */
        }
#endif
    }
    if((autoTime > 0) && (frameCur->bufferCur->autoTime < 0) &&
       meModeTest(frameCur->bufferCur->mode,MDAUTOSV) &&
       (frameCur->bufferCur->name[0] != '*'))
    {
        struct   meTimeval tp ;

        gettimeofday(&tp,NULL) ;
        frameCur->bufferCur->autoTime = ((tp.tv_sec-startTime+autoTime)*1000) +
                  (tp.tv_usec/1000) ;
        if(!isTimerExpired(AUTOS_TIMER_ID) &&
           (!isTimerSet(AUTOS_TIMER_ID) || 
            (meTimerTime[AUTOS_TIMER_ID] > frameCur->bufferCur->autoTime)))
            timerSet(AUTOS_TIMER_ID,frameCur->bufferCur->autoTime,((long)autoTime) * 1000L) ;
    }
    return meTRUE ;
}

void
lineSetChanged(register int flag)
{
    frameCur->windowCur->dotLine->flag |= meLINE_CHANGED ;

    if (frameCur->bufferCur->windowCount != 1)             /* If more than 1 window, update rest */
    {
        register meWindow *wp;

        meFrameLoopBegin() ;
        wp = loopFrame->windowList;
        while (wp != NULL)
        {
            if(wp->buffer == frameCur->bufferCur)
                wp->updateFlags |= flag;
            wp = wp->next;
        }
        meFrameLoopEnd() ;
    }
    else
        frameCur->windowCur->updateFlags |= flag;              /* update current window */
}

/*
 * Insert "n" characters (not written) at the current location of dot. In
 * the easy case all that happens is the text is stored in the line. In the
 * hard case, the line has to be reallocated. When the window list is updated,
 * take special care; I screwed it up once. You always update dot in the
 * current window. You update mark, and a dot in another window, if it is
 * greater than the place where you did the insert. Return line pointer if all
 * is well, and NULL on errors.
 */
meUByte *
lineMakeSpace(int n)
{
    meUByte  *cp1;
    meWindow *wp;
    meLine   *lp_new;
    meLine   *lp_old;
    meUShort  doto ;
    int     newl ;

    lp_old = frameCur->windowCur->dotLine;			/* Current line         */
    if((newl=(n+((int) meLineGetLength(lp_old)))) > 0x0fff0)
    {
        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Line too long!]") ;
        return NULL ;
    }
    if (lp_old == frameCur->bufferCur->baseLine)		/* At the end: special  */
    {
        /*---	Allocate a new line */

        if ((lp_new=meLineMalloc(n,1)) == NULL)    /* Allocate new line    */
            return NULL ;

        /*---	Link in with the previous line */
        frameCur->bufferCur->lineCount++ ;
        lp_new->next = lp_old;
        lp_new->prev = lp_old->prev;
        lp_new->flag = (lp_old->flag & ~meLINE_ANCHOR) | meLINE_CHANGED ;
#if MEOPT_EXTENDED
        /* only the new line should have any $line-scheme */
        lp_old->flag &= ~meLINE_SCHEME_MASK ;
#endif
        lp_old->prev->next = lp_new;
        lp_old->prev = lp_new;

        if(lp_old->flag & meLINE_ANCHOR)
            /* update the position of any anchors - important for narrows */
            meLineResetAnchors(meLINEANCHOR_IF_LESS|meLINEANCHOR_RETAIN,frameCur->bufferCur,
                               lp_old,lp_new,0,0) ;
        cp1 = lp_new->text ;	/* Return pointer */
        doto = 0 ;
#if MEOPT_UNDO
        meUndoAddInsChar() ;
#endif
    }
    else
    {
        doto = frameCur->windowCur->dotOffset;           /* Save for later.      */
        if (n > (int) lp_old->unused)                    /* Hard: reallocate     */
        {
            meUByte *cp2 ;
            meInt ii ;

            /*---	Allocate new line of correct length */

            if ((lp_new = meLineMalloc(newl,1)) == NULL)
                return NULL ;

            /*---	Copy line */

            cp1 = &lp_old->text[0];
            cp2 = &lp_new->text[0];
            ii = (meInt) doto ;
            while(--ii >= 0)
                *cp2++ = *cp1++ ;
            cp2 += n;
            while((*cp2++ = *cp1++))
                ;

            /*---	Re-link in the new line */

            lp_new->prev = lp_old->prev;
            lp_new->next = lp_old->next;
            /* preserve the $line-scheme */
            lp_new->flag = (lp_old->flag & ~meLINE_ANCHOR) | meLINE_CHANGED ;
            lp_old->prev->next = lp_new;
            lp_old->next->prev = lp_new;
            if(lp_old->flag & meLINE_ANCHOR)
            {
                /* update the position of any anchors - important for narrows */
                meLineResetAnchors(meLINEANCHOR_IF_GREATER|meLINEANCHOR_RELATIVE,frameCur->bufferCur,
                                   lp_old,lp_new,doto,n);
                meLineResetAnchors(meLINEANCHOR_ALWAYS|meLINEANCHOR_RETAIN,frameCur->bufferCur,
                                   lp_old,lp_new,0,0) ;
            }
            meFree(lp_old);
            cp1 = lp_new->text + doto ;		/* Save position */
        }
        else                                /* Easy: in place       */
        {
            meUByte *cp2 ;
            meUShort ii ;

            lp_new = lp_old;                      /* Pretend new line     */
            lp_new->unused -= n ;
            lp_new->length = newl ;
            lp_new->flag |= meLINE_CHANGED ;
            cp2 = lp_old->text + lp_old->length ;
            cp1 = cp2 - n ;
            ii  = cp1 - lp_old->text - doto ;
            do
                *cp2-- = *cp1-- ;
            while(ii--) ;
            cp1++ ;
            if(lp_old->flag & meLINE_ANCHOR)
                /* update the position of any anchors - important for narrows */
                meLineResetAnchors(meLINEANCHOR_IF_GREATER|meLINEANCHOR_RELATIVE,frameCur->bufferCur,
                                   lp_old,lp_new,doto,n);
        }
    }

    meFrameLoopBegin() ;
    wp = loopFrame->windowList;                    /* Update windows       */
    while (wp != NULL)
    {
        if(wp->buffer == frameCur->bufferCur)
        {
            if (wp->dotLine == lp_old)
            {
                wp->dotLine = lp_new;
                if(wp->dotOffset >= doto)
                    wp->dotOffset += n;
            }
            if (wp->markLine == lp_old)
            {
                wp->markLine = lp_new;
                if (wp->markOffset > doto)
                    wp->markOffset += n;
            }
        }
        wp = wp->next;
    }
    meFrameLoopEnd() ;
    return cp1 ;
}

/*
 * Insert "n" copies of the character "c" at the current location of dot. In
 * the easy case all that happens is the text is stored in the line. In the
 * hard case, the line has to be reallocated. When the window list is updated,
 * take special care; I screwed it up once. You always update dot in the
 * current window. You update mark, and a dot in another window, if it is
 * greater than the place where you did the insert. Return meTRUE if all is
 * well, and meFALSE on errors.
 */
int
lineInsertChar(int n, int c)
{
    meUByte *cp;

    lineSetChanged(WFMOVEC|WFMAIN);		/* Declare editied buffer */
    if ((cp = lineMakeSpace(n))==NULL)  /* Make space for the characters */
        return meFALSE ;	        /* Error !!! */
    while (n-- > 0)                     /* Copy in the characters */
        *cp++ = c;
    return meTRUE ;
}

/*---	As insrt char, insert string */
int
lineInsertString(register int n, register meUByte *cp)
{
    meUByte *lp ;

    /* if n == 0 the length if the string is unknown calc the length */
    if((n == 0) &&
       ((n = meStrlen(cp)) == 0))
        return meTRUE ;
    
    lineSetChanged(WFMAIN) ;
    
    if ((lp = lineMakeSpace(n))==NULL)	/* Make space for the characters */
        return meFALSE ;		/* Error !!! */
    while(n-- > 0)			/* Copy in the characters */
        *lp++ = *cp++;
    return meTRUE ;
}

/*
 * Insert a newline into the buffer at the current location of dot in the
 * current window. The funny ass-backwards way it does things is not a botch;
 * it just makes the last line in the file not a special case. Return meTRUE if
 * everything works out and meFALSE on error (memory allocation failure). The
 * update of dot and mark is a bit easier then in the above case, because the
 * split forces more updating.
 */
int
lineInsertNewline(meInt flags)
{
    meLine   *lp1;
    meLine   *lp2;
    meWindow *wp;
    meInt     lineno ;
    meUShort  doto, llen ;
    meLineFlag lflag ;
    
    lp1 = frameCur->windowCur->dotLine ;
    doto = frameCur->windowCur->dotOffset;
    lflag = meLineGetFlag(lp1) ;
#if MEOPT_EXTENDED
    if((lflag & meLINE_PROTECT) == 0)
        flags = 0 ;
    else if(!(flags & meBUFINSFLAG_UNDOCALL))
        return mlwrite(MWABORT,(meUByte *)"[Protected Line!]") ;
#endif
    lineSetChanged(WFMOVEL|WFMAIN|WFSBOX);
    llen = meLineGetLength(lp1)-doto ;
    if((doto == 0) || (llen > doto+32))
    {
        /* the length of the rest of the line is greater than doto+32
         * so use the current line for the next line and create a new this line */
        lp2 = lp1 ;
        if ((lp1=meLineMalloc(doto,1)) == NULL) /* New second line      */
            return meFALSE ;
        lp1->prev = lp2->prev;
        lp2->prev->next = lp1;
        lp1->next = lp2 ;
        lp2->prev = lp1 ;
        if(doto)
        {
            /* Shuffle text around  */
            register meUByte *cp1, *cp2 ;
            int ii ;
            
            ii = doto ;
            cp1 = lp2->text ;
            cp2 = lp1->text ;
            while(ii--)
                *cp2++ = *cp1++ ;
            cp2 = lp2->text ;
            while((*cp2++ = *cp1++))
                ;
            *cp2 = '\0' ;
            lp2 = meLineShrink(lp2,llen) ;
        }
#if MEOPT_EXTENDED
        /* special case: when undoing the PROTECTed flag is overridden, but if the
         * offest is 0 the anchors must remain on the original line */
        if(doto || !(flags & meBUFINSFLAG_UNDOCALL))
#endif
        {
            /* sort out the line anchors */ 
            if(lp2->flag & meLINE_ANCHOR)
            {
                /* update the position of any anchors */
                /* move ones before split over to the first line (lp1) */
                meLineResetAnchors(meLINEANCHOR_IF_LESS|meLINEANCHOR_RETAIN,frameCur->bufferCur,
                                   lp2,lp1,doto,0) ;
                /* adjust offset of remaining ones on the second line (lp2) */
                if(doto && (lp2->flag & meLINE_ANCHOR))
                    meLineResetAnchors(meLINEANCHOR_IF_GREATER|meLINEANCHOR_RELATIVE,frameCur->bufferCur,
                                       lp2,lp2,doto,0-((meInt)doto)) ;
            }
        }
    }
    else
    {
        register meUByte *cp1, *cp2, *cp3 ;
        
        if ((lp2=meLineMalloc(llen,1)) == NULL) /* New second line      */
            return meFALSE ;
        cp2 = cp1 = lp1->text+doto ;                /* Shuffle text around  */
        cp3 = lp2->text ;
        while((*cp3++=*cp2++))
            ;
        *cp1 = '\0' ;
        lp1 = meLineShrink(lp1,doto) ;
        lp2->next = lp1->next;
        lp1->next->prev = lp2;
        lp2->prev = lp1 ;
        lp1->next = lp2 ;
        
        if(lp1->flag & meLINE_ANCHOR)
            /* move to the second line anchors with an offset greater than the split offset */
            meLineResetAnchors(meLINEANCHOR_IF_GREATER|meLINEANCHOR_RELATIVE,frameCur->bufferCur,
                               lp1,lp2,doto,0-((meInt)doto)) ;
    }
    if(lflag & meLINE_NOEOL)
    {
        lp1->flag &= ~meLINE_NOEOL ;
        lp2->flag |= meLINE_NOEOL ;
    }
#if MEOPT_EXTENDED
    /* scheme & user set flags should remain on the top line even if offset was 0 - important for narrows etc */ 
    if(doto || !(flags & meBUFINSFLAG_UNDOCALL))
    {
        lp1->flag |= lflag & (meLINE_MARKUP|meLINE_PROTECT|meLINE_SET_MASK|meLINE_SCHEME_MASK) ;
        lp2->flag &= ~(meLINE_MARKUP|meLINE_PROTECT|meLINE_SET_MASK|meLINE_SCHEME_MASK) ;
    }
#endif
    lineno = frameCur->windowCur->dotLineNo ;
    meFrameLoopBegin() ;
    wp = loopFrame->windowList ;
    while(wp != NULL)
    {
        if(wp->buffer == frameCur->bufferCur)
        {
            if(wp->vertScroll > lineno)
                wp->vertScroll++ ;
            if(wp->dotLineNo == lineno)
            {
                if (wp->dotOffset >= doto)
                {
                    wp->dotLine = lp2;
                    wp->dotLineNo++ ;
                    wp->dotOffset -= doto;
                }
                else
                    wp->dotLine = lp1;
            }
            else if(wp->dotLineNo > lineno)
                wp->dotLineNo++ ;
            if(wp->markLineNo == lineno)
            {
                if (wp->markOffset > doto)
                {
                    wp->markLine = lp2;
                    wp->markLineNo++ ;
                    wp->markOffset -= doto;
                }
                else
                    wp->markLine = lp1;
            }
            else if(wp->markLineNo > lineno)
                wp->markLineNo++ ;
        }
        wp = wp->next;
    }
    meFrameLoopEnd() ;
    frameCur->bufferCur->lineCount++ ;
    return meTRUE ;
}

#if MEOPT_EXTENDED
static int
bufferIsTextInsertLegal(meUByte *str)
{
    if((str[0] != '\0') &&
       (meLineGetFlag(frameCur->windowCur->dotLine) & meLINE_PROTECT) &&
       (meStrchr(str,meCHAR_NL) != NULL))
        return mlwrite(MWABORT,(meUByte *)"[Protected Line!]") ;
    return meTRUE ;
}
#endif

/* insert the given text string into the current buffer at dot
 * Note - this function avoids any meLINE_PROTECT issues, it assumes that
 * the legality of the action is already checked */
int
bufferInsertText(meUByte *str, meInt flags)
{
    meUByte *ss=str, cc, ccnl ;
    meInt status, lineCount=0, lineNo, len, tlen=0 ;
    meLine *line ;
    
    if(str[0] == '\0')
        return 0 ;
    
    ccnl = (flags & meBUFINSFLAG_LITERAL) ? '\0':meCHAR_NL ;
    for(;;)
    {
        while(((cc=*ss++) != '\0') && (cc != ccnl))
            ;
        ss-- ;
        len = (((size_t) ss) - ((size_t) str)) ;
        if(len > 0x0fff0)
        {
            mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Line too long!]") ;
            return tlen ;
        }
        if(cc == '\0')
        {
            if(len && (lineInsertString(len,str) > 0))
                tlen += len ;
            break ;
        }
        if(frameCur->windowCur->dotOffset)
        {
            assert(lineCount == 0) ;
            if(len && (lineInsertString(len,str) <= 0))
                break ;
            tlen += len ;
            if(lineInsertNewline(flags) <= 0)
                break ;
            tlen++ ;
        }
        else
        {
            if(lineCount == 0)
            {
                line = meLineGetPrev(frameCur->windowCur->dotLine) ;
                lineNo = frameCur->windowCur->dotLineNo ;
            }
            *ss = '\0' ;
            status = addLine(frameCur->windowCur->dotLine,str) ;
            *ss = meCHAR_NL ;
            if(status <= 0)
                break ;
            if(!tlen & !(flags & meBUFINSFLAG_UNDOCALL))
            {
                /* Update the position of any anchors at the start of the line, these are left behind */
                if(frameCur->windowCur->dotLine->flag & meLINE_ANCHOR)
                    meLineResetAnchors(meLINEANCHOR_IF_LESS|meLINEANCHOR_RETAIN,frameCur->bufferCur,
                                       frameCur->windowCur->dotLine,meLineGetNext(line),0,0) ;
#if MEOPT_EXTENDED
                /* scheme & user set flags should remain on the top line as well - important for narrows etc */ 
                meLineGetNext(line)->flag |= frameCur->windowCur->dotLine->flag & (meLINE_MARKUP|meLINE_PROTECT|meLINE_SET_MASK|meLINE_SCHEME_MASK) ;
                frameCur->windowCur->dotLine->flag &= ~(meLINE_MARKUP|meLINE_PROTECT|meLINE_SET_MASK|meLINE_SCHEME_MASK) ;
#endif
            }
            tlen += len+1 ;
            lineCount++ ;
        }
        str = ++ss ;
    }
    if(lineCount > 0)
    {
        meWindow *wp;
        
        frameCur->bufferCur->lineCount += lineCount ;
        meFrameLoopBegin() ;
        wp = loopFrame->windowList ;
        while(wp != NULL)
        {
            if(wp->buffer == frameCur->bufferCur)
            {
                if(wp->vertScroll > lineNo)
                    wp->vertScroll += lineCount ;
                if(wp->dotLineNo >= lineNo)
                    wp->dotLineNo += lineCount ;
                if(wp->markLineNo >= lineNo)
                {
                    if((wp->markLineNo == lineNo) && (wp->markOffset == 0))
                        /* the mark gets left behind */
                        wp->markLine = meLineGetNext(line) ;
                    else
                        wp->markLineNo += lineCount ;
                }
                wp->updateFlags |= WFMOVEL|WFMAIN|WFSBOX ;
            }
            wp = wp->next;
        }
        meFrameLoopEnd() ;
    }
    return tlen ;
}

int
bufferInsertSpace(int f, int n)	/* insert spaces forward into text */
{
    register int s ;
    
    if(n <= 0)
        return meTRUE ;
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s ;
    s = lineInsertChar(n, ' ');
#if MEOPT_UNDO
    if(s > 0)
        meUndoAddInsChars(n) ;
#endif
    return s ;
}

int
bufferInsertTab(int f, int n)	/* insert tabs forward into text */
{
    register int s ;

    if(n <= 0)
        return meTRUE ;
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s ;
    s = lineInsertChar(n, '\t');
#if MEOPT_UNDO
    if(s > 0)
        meUndoAddInsChars(n) ;
#endif
    return s ;
}

/* ask for and insert a string into the current buffer at the current point */
int
bufferInsertString(int f, int n)
{
    meUByte tstring[meBUF_SIZE_MAX];    /* string to add */
    register int status;		/* status return code */
    register int count=0;		/* char insert count */
    
    /* ask for string to insert */
    if((status=meGetString((meUByte *)"String", 0, 0,tstring, meBUF_SIZE_MAX)) <= 0)
        return status ;
#if MEOPT_EXTENDED
    if((status=bufferIsTextInsertLegal(tstring)) <= 0)
        return status ;
#endif
    if((status=bufferSetEdit()) <= 0)   /* Check we can change the buffer */
        return status ;
    
    if(n < 0)
    {
        status = meBUFINSFLAG_LITERAL ;
        n = -n ;
    }
    else
        status = 0 ;
    
    /* insert it */
    for(; n>0 ; n--)
        count += bufferInsertText(tstring,status) ;

#if MEOPT_UNDO
    meUndoAddInsChars(count) ;
#endif
    return meTRUE ;
}


/*
 * Open up some blank space. The basic plan is to insert a bunch of newlines,
 * and then back up over them. Everything is done by the subcommand
 * procerssors. They even handle the looping. Normally this is bound to "C-O".
 */
int
bufferInsertNewline(int f, int n)
{
    register int    i;
    register int    s;
    
    if (n <= 0)
        return meTRUE ;
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s ;
    i = n;                                  /* Insert newlines.     */
    do 
        s = lineInsertNewline(0) ;
    while((s > 0) && --i);
#if MEOPT_UNDO
    meUndoAddInsChars(n-i) ;
#endif
    return (s);
}

/*
 * Save the current kill context. This involves making a new klist element and
 * putting it at the head of the klists (ie make khead point at it).
 *
 * The next kill will go into khead->kill, the previous kill can be accessed
 * via khead->k_next->kill
 *
 * The routine is made slightly more complicated because the number of elements
 * in the klist is kept to meKILL_MAX (defined in edef.h). Every time we enter this
 * routine we have to count the number of elements in the list and throw away
 * (ie free) the excess ones.
 *
 * Return meTRUE, except when we cannot malloc enough space for the new klist
 * element.
 */
int
killSave(void)
{
    meShort count=0 ;
    meKill *thiskl, *lastkl ;/* pointer to last klist element */

    /*
     * see if klhead points to anything, if not then our job is easy,
     * we setup klhead and make currkill point to the kill buffer that
     * klhead points to.
     */
    thiskl = klhead ;
    /*
     * There is at least one element in the klist. Count the total number
     * of elements and if over meKILL_MAX, delete the oldest kill.
     */
    while(thiskl != NULL)
    {
        if(count++ == meKILL_MAX)
            break ;
        lastkl = thiskl;
        thiskl = thiskl->next;
    }
    if(thiskl != NULL)
    {
        meKillNode *next, *kill ;
        /*
         * One element too many. Free the kill buffer(s) associated
         * with this klist, and finally the klist itself.
         */
        kill = thiskl->kill ;
        while(kill != NULL)
        {
            next = kill->next;
            meFree(kill);
            kill = next;
        }
        meFree(thiskl) ;
        /*
         * Now make sure that the previous element in the list does
         * not point to the thing that we have freed.
         */
        lastkl->next = (meKill*) NULL;
        reyankLastYank = NULL ;
    }

    /*
     * There was a klhead. malloc a new klist element and wire it in to
     * the head of the list.
     */
    if((thiskl = (meKill *) meMalloc(sizeof(meKill))) == NULL)
        return meFALSE ;
    thiskl->next = klhead ;
    klhead = thiskl ;
    thiskl->kill = NULL ;
    currkill = &(thiskl->kill) ;

    return meTRUE ;
}

/* Add a new kill block of size count to the current kill.
 * returns NULL on failure, pointer to the 1st byte of
 * the block otherwise.
 */
meUByte *
killAddNode(meInt count)
{
    meKillNode *nbl ;

    if((nbl = (meKillNode*) meMalloc(sizeof(meKillNode)+count)) == NULL)
        return NULL ;
    nbl->next = NULL ;
    nbl->data[count] = '\0' ;
    *currkill = nbl ;
    currkill  = &(nbl->next) ;
    return nbl->data ;
}

/*
 * This function deletes "n" bytes, starting at dot. It understands how do deal
 * with end of lines, etc. It returns meTRUE if all of the characters were
 * deleted, and meFALSE if they were not (because dot ran into the end of the
 * buffer. The characters deleted are returned in the string. It is assumed
 * that the kill buffer is not in use.
 * SWP - changed because
 * a. Incredibly slow and inefficient
 * b. The mark line wasn't being undated properly
 * c. Needs to return the number of chars not deleted so that the undo can
 *    be corrected
 * 
 *   noChrs   - # of chars to delete
 *   kstring  - put killed text into kill string
 * 
 * Notes: 
 *   This function does not check that the deletion is legal (not PROTECTED), nor 
 *   does it handle narrows, this is assumed to be done already (see ldelete).
 * 
 *   Line Flags - Given a start line & offset (slp,soff) and end point
 *        (elp,eoff), the resultant joined line's flags will be derived as
 *        follows:
 * 
 *     ANCHOR  - true if any line contains an anchor.
 *     NOEOL   - derived from elp
 *     MARKUP  - derived from slp if soff > 0, elp otherwise
 *     SCHEME  - derived from slp if soff > 0, elp otherwise
 */
int
mldelete(meInt noChrs, meUByte *kstring)
{
    meLine   *slp, *elp, *fslp, *felp ;
    meInt   slno, elno, nn=noChrs, ii, soff, eoff ;
    meUByte  *ks, *ss ;
    meWindow *wp ;
    
    ks = kstring ;
    if(noChrs == 0)
    {
        if(ks != NULL)
            *ks = '\0' ;
        return 0 ;
    }
    slp  = frameCur->windowCur->dotLine ;
    soff = frameCur->windowCur->dotOffset ;
    ii = meLineGetLength(slp) - soff ;
    if(nn <= ii)
    {
        /* this can be treated as a special quick case because
         * it is simply removing a few chars from the middle of
         * the line - removal as window position updates are
         * special case and very easy
         */
        meUByte *s1, *s2 ;
        
        s1 = slp->text+soff ;
        if(ks != NULL)
        {
            s2 = s1 ;
            ii = nn ;
            while(ii--)
                *ks++ = *s2++ ;
            *ks = '\0' ;
        }
        s2 = s1+nn ;
        while((*s1++ = *s2++) != '\0')
            ;
        slp = meLineShrink(slp,slp->length-nn) ;
        if(slp->flag & meLINE_ANCHOR)
            meLineResetAnchors(meLINEANCHOR_IF_GREATER|meLINEANCHOR_COMPRESS|meLINEANCHOR_RELATIVE,
                               frameCur->bufferCur,slp,slp,(meUShort) (soff+nn),-nn);
        slp->flag |= meLINE_CHANGED ;
        
        /* Last thing to do is update the windows */
        meFrameLoopBegin() ;
        wp = loopFrame->windowList ;
        while (wp != NULL)
        {
            if(wp->buffer == frameCur->bufferCur)
            {
                if((wp->dotLine == slp) && (((meInt) wp->dotOffset) > soff))
                {
                    if((ii=wp->dotOffset - nn) < soff)
                        wp->dotOffset = (meUShort) soff ;
                    else
                        wp->dotOffset = (meUShort) ii ;
                }
                if((wp->markLine == slp) && (((meInt) wp->markOffset) > soff))
                {
                    if((ii=wp->markOffset - nn) < soff)
                        wp->markOffset = (meUShort) soff ;
                    else
                        wp->markOffset = (meUShort) ii ;
                }
                wp->updateFlags |= WFMAIN ;
            }
            wp = wp->next;
        }
        meFrameLoopEnd() ;
        /* Can't fail, must have remove all chars */
        return 0 ;
    }
    slno = frameCur->windowCur->dotLineNo ;
    elp = meLineGetNext(slp) ;
    elno = slno+1 ;
    eoff = 0 ;
    nn -= ii + 1 ;
    if(ks != NULL)
    {
        ss = slp->text+soff ;
        while(ii--)
            *ks++ = *ss++ ;
        *ks++ = meCHAR_NL ;
    }
    /* The deletion involves more than one line, lets find the end point */
    while((nn != 0) && (elp != frameCur->bufferCur->baseLine))
    {
        ii = meLineGetLength(elp) ;
        if(nn <= ii)
        {
            /* Ends somewhere in this line, set the offset and break */
            eoff = (meUShort) nn ;
            if(ks != NULL)
            {
                ss = elp->text ;
                for( ; nn ; nn--)
                    *ks++ = *ss++ ;
            }
            else
                nn = 0 ;
            break ;
        }
        /* move to the next line, decrementing the counter */
        nn -= ii+1 ;
        if(ks != NULL)
        {
            ss = elp->text ;
            for( ; ii ; ii--)
                *ks++ = *ss++ ;
            *ks++ = meCHAR_NL ;
        }
        elp = meLineGetNext(elp) ;
        elno++ ;
    }
    if(ks != NULL)
        *ks = '\0' ;
    /* quick test first for speed, see if we can simply use the last line */
    if(soff == 0)
    {
        /* here we can simply use the end line */
        if(eoff != 0)
        {
            meUByte *s1, *s2 ;
            
            s1 = elp->text ;
            s2 = s1+eoff ;
            while((*s1++ = *s2++) != '\0')
                ;
            elp = meLineShrink(elp,elp->length-eoff) ;
        }
        fslp = slp ;
        felp = elp ;
        slp = elp ;
    }
    else
    {
        /* construct the new joined start and end line, 3 posibilities
         * a. use the start line
         * b. use the end line
         * c. create a new line
         * 
         * NOTE There is a 4th possibility, the two joined lines could be
         * too long (>0xfff0) for a single line. This is tested for in mldelete
         * line and avoid so if this function is used else where, beware!
         */
        int newl ;
        ii = meLineGetLength(elp) - eoff ;
        newl = ii + soff ;
        if(newl > 0x0fff0)
            /* this deletion will leave the joined line too long, abort */
            return noChrs ;
        if(meLineGetMaxLength(slp) >= newl)
        {
            /* here we have got to test for one special case, if the elp is
             * the buffer baseLine then eoff must be 0, we can't remove this
             * line so can't remove last '\n' and increment nn or but don't
             * consider this an error? Not sure yet
             */
            if(elp == frameCur->bufferCur->baseLine)
            {
                slp->text[soff] = '\0' ;
                felp = elp ;
                /* increment the no-lines cause we're only pretending we've
                 * removed the last line
                 */
                frameCur->bufferCur->lineCount++ ;
            }
            else
            {
                meUByte *s1, *s2 ;
            
                s1 = slp->text+soff ;
                s2 = elp->text+eoff ;
                while((*s1++ = *s2++) != '\0')
                    ;
                felp = meLineGetNext(elp) ;
            }
            slp = meLineShrink(slp,newl) ;
            if(slp->flag & meLINE_ANCHOR)
                meLineResetAnchors(meLINEANCHOR_IF_GREATER|meLINEANCHOR_FIXED,frameCur->bufferCur,
                                   slp,slp,(meUShort) soff,soff) ;
            if(elp->flag & meLINE_ANCHOR)
                meLineResetAnchors(meLINEANCHOR_IF_GREATER|meLINEANCHOR_RELATIVE,frameCur->bufferCur,
                                   elp,slp,(meUShort) eoff,soff-eoff);
            slp->flag = (slp->flag & ~meLINE_NOEOL) | (elp->flag & meLINE_NOEOL) ;
            fslp = meLineGetNext(slp) ;
        }
        else if(meLineGetMaxLength(elp) >= newl)
        {
            meUByte *s1, *s2 ;
            
            if(soff > eoff)
            {
                s1 = elp->text+newl ;
                s2 = elp->text+elp->length ;
                do
                    *s1-- = *s2-- ;
                while(ii--) ;
            }
            else if(soff < eoff)
            {
                s1 = elp->text+soff ;
                s2 = elp->text+eoff ;
                while((*s1++ = *s2++) != '\0')
                    ;
            }
            elp = meLineShrink(elp,newl) ;
            s1 = elp->text ;
            s2 = slp->text ;
            ii = soff ;
            while(ii--)
                *s1++ = *s2++ ;
            if(elp->flag & meLINE_ANCHOR)
            {
                meLineResetAnchors(meLINEANCHOR_IF_LESS|meLINEANCHOR_FIXED,frameCur->bufferCur,
                                   elp,elp,(meUShort) eoff,eoff);
                meLineResetAnchors(meLINEANCHOR_ALWAYS|meLINEANCHOR_RELATIVE,frameCur->bufferCur,
                                   elp,elp,(meUShort) 0,soff-eoff);
            }
            if(slp->flag & meLINE_ANCHOR)
                meLineResetAnchors(meLINEANCHOR_IF_LESS|meLINEANCHOR_RETAIN,frameCur->bufferCur,
                                   slp,elp,(meUShort) soff,0) ;
#if MEOPT_EXTENDED
            elp->flag = (elp->flag & ~(meLINE_MARKUP|meLINE_SCHEME_MASK)) | (slp->flag & (meLINE_MARKUP|meLINE_SCHEME_MASK)) ;
#endif
            fslp = slp ;
            felp = elp ;
            slp = elp ;
        }
        else
        {
            meUByte *s1, *s2 ;
            fslp = slp ;
            if((slp=meLineMalloc(newl,1)) == NULL)
                return noChrs ;
            s1 = slp->text ;
            s2 = fslp->text ;
            ii = soff ;
            while(ii--)
                *s1++ = *s2++ ;
            s2 = elp->text+eoff ;
            while((*s1++ = *s2++) != '\0')
                ;
            slp->next = elp->next ;
            elp->next->prev = slp ;
            slp->prev = elp ;
            elp->next = slp ;
            if(fslp->flag & meLINE_ANCHOR)
                meLineResetAnchors(meLINEANCHOR_IF_LESS|meLINEANCHOR_RETAIN,frameCur->bufferCur,
                                   fslp,slp,(meUShort) soff,0) ;
            if(elp->flag & meLINE_ANCHOR)
                meLineResetAnchors(meLINEANCHOR_IF_GREATER|meLINEANCHOR_RELATIVE,frameCur->bufferCur,
                                   elp,slp,(meUShort) eoff,soff-eoff);
#if MEOPT_EXTENDED
            slp->flag |= (fslp->flag & (meLINE_MARKUP|meLINE_SCHEME_MASK)) | (elp->flag & meLINE_NOEOL) ;
#endif
            felp = slp ;
        }
    }
    
    /* due to the last '\n' & long line special cases above, it is possible
     * that we aren't actually removing any lines, check this
     */
    if(fslp != felp)
    {
        /* link out the unwanted section fslp -> meLineGetPrev(felp) and free */
        felp->prev->next = NULL ;
        fslp->prev->next = felp ;
        felp->prev = fslp->prev ;
        
        while(fslp != NULL)
        {
            if(fslp->flag & meLINE_ANCHOR)
                meLineResetAnchors(meLINEANCHOR_ALWAYS|meLINEANCHOR_FIXED,frameCur->bufferCur,
                                   fslp,slp,(meUShort) 0,soff);
            felp = meLineGetNext(fslp) ;
            meFree(fslp) ;
            fslp = felp ;
        }
    }
    slp->flag |= meLINE_CHANGED ;
    
    /* correct the buffers number of lines */
    frameCur->bufferCur->lineCount -= elno-slno ;
    
    /* Last thing to do is update the windows */
    meFrameLoopBegin() ;
    wp = loopFrame->windowList;
    while (wp != NULL)
    {
        if(wp->buffer == frameCur->bufferCur)
        {
            if(wp->vertScroll > slno)
            {
                if(wp->vertScroll > elno)
                    wp->vertScroll -= elno-slno ;
                else if(scrollFlag & 0x10)
                    wp->vertScroll = slno ;
            }
            if(wp->dotLineNo >= slno)
            {
                if(wp->dotLineNo > elno)
                    wp->dotLineNo -= elno-slno ;
                else
                {
                    if((wp->dotLineNo == elno) && (wp->dotOffset > eoff))
                        wp->dotOffset = (meUShort) (soff + wp->dotOffset - eoff) ;
                    else if((wp->dotLineNo != slno) || (wp->dotOffset > soff))
                        wp->dotOffset = (meUShort) soff ;
                    wp->dotLine = slp ;
                    wp->dotLineNo = slno ;
                }
            }
            if(wp->markLineNo >= slno)
            {
                if(wp->markLineNo > elno)
                    wp->markLineNo -= elno-slno ;
                else
                {
                    if((wp->markLineNo == elno) && (wp->markOffset > eoff))
                        wp->markOffset = (meUShort) (soff + wp->markOffset - eoff) ;
                    else if((wp->markLineNo != slno) || (wp->markOffset > soff))
                        wp->markOffset = (meUShort) soff ;
                    wp->markLine = slp ;
                    wp->markLineNo = slno ;
                }
            }
            wp->updateFlags |= WFMOVEL|WFMAIN|WFSBOX ;
        }
        wp = wp->next;
    }
    meFrameLoopEnd() ;
    
    /* return the number of chars left to delete, usually 0 */
    return nn ;
}

/*
 * This function deletes "n" bytes, starting at dot. It understands how do deal
 * with end of lines, etc. It returns meTRUE if all of the characters were
 * deleted, and meFALSE if they were not (because dot ran into the end of the
 * buffer. The "kflag" is meTRUE if the text should be put in the kill buffer.
 */
/* nn    -  # of chars to delete */
/* kflag -  put killed text in kill buffer flag */
int
ldelete(meInt nn, int kflag)
{
    meLine *ll ;
    meUByte *kstring ;
    meLineFlag lflag=0 ;
    meInt len, ret ;
    
    /* A quick test to make failure handling easier */
    ll = frameCur->windowCur->dotLine ;
    if((ll == frameCur->bufferCur->baseLine) && nn)
        return meFALSE ;
    
    /* Must get the # chars we can delete */
    if((len = nn + frameCur->windowCur->dotOffset - meLineGetLength(ll)) > 0)
    {
        do
        {
            lflag |= meLineGetFlag(ll) ;
            ll = meLineGetNext(ll) ;
            if(ll == frameCur->bufferCur->baseLine)
            {
                /* The last but ones line can only be removed if the current 
                 * offset is 0
                 */
                if(frameCur->windowCur->dotOffset == 0)
                    len-- ;
                break ;
            }
            len -= meLineGetLength(ll)+1 ;
        } while(len > 0) ;
    }
#if MEOPT_EXTENDED
    if((frameCur->windowCur->dotLine != ll) &&
       ((frameCur->windowCur->dotOffset && 
         (frameCur->windowCur->dotLine->flag & meLINE_PROTECT)) ||
        ((ll->flag & meLINE_PROTECT) &&
         (frameCur->windowCur->dotOffset || (meLineGetLength(ll) != -len)))))
    {
        mlwrite(MWABORT,(meUByte *)"[Protected Line!]") ;
        return meFALSE ;
    }
#endif
    if(len > 0)
    {
        nn -= len ;
        /* This is a bit grotty, we cannot remove the last \n, so the count has
         * been adjusted appropriately, but neither should we generate an error.
         * So only generate an error if the number of chars we want to delete but
         * can't is greater that 1
         */
        if(len > 1)
            ret = meFALSE ;
        else
            ret = meTRUE ;
        len = 0 ;
    }
    else if((((long) frameCur->windowCur->dotOffset) - len) > 0xfff0)
    {
        /* The last line will be too long - dont remove the last \n and the offset
         * on the last line, i.e. if G is a wanted char and D is a char that should
         * be deleted, we start with:
         * 
         * GGGGGGDDDDDDDDDD
         * DDDDDDDDDDDDDDDD
         * DDDDDDDDDDDDDDDD
         * DDDDDDDGGGGGGGG
         * 
         * We will try to get it to
         * 
         * GGGGGG
         * DDDDDDDGGGGGGGG
         * 
         * All we have to do is reduce the no chars appropriately.
         * 
         * Note: if there is a narrow on the last line we should that that too so set
         * len to 1.
         */
        nn -= meLineGetLength(ll)+len+1 ;
        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Line too long!]") ;
        ret = meFALSE ;
        len = 1 ;
    }        
    else
    {
        ret = meTRUE ;
        len += meLineGetLength(ll) ;
    }
#if MEOPT_NARROW
    if((lflag & meLINE_ANCHOR_NARROW) ||
       (len && (meLineGetFlag(ll) & meLINE_ANCHOR_NARROW)))

    {
        /* the kill probably includes one or more narrows, find them, expand then and
         * add them to the size of the kill */
        meLine *slp, *dlp ;
        meInt dln ;
        
        slp = frameCur->windowCur->dotLine ;
        nn += meBufferRegionExpandNarrow(frameCur->bufferCur,&slp,frameCur->windowCur->dotOffset,ll,(meUShort) len,1) ;
        if((dlp = frameCur->windowCur->dotLine) != slp)
        {
            dln = frameCur->windowCur->dotLineNo ;
            do
                dln-- ;
            while((dlp = meLineGetPrev(dlp)) != slp) ;
            frameCur->windowCur->dotLine = dlp ;
            frameCur->windowCur->dotLineNo = dln ;
        }
    }
#endif
    if(kflag & 1)
    {   /* Kill?                */
        if((lastflag != meCFKILL) && (thisflag != meCFKILL))
            killSave();
        if((kstring = killAddNode(nn)) == NULL)
            return meFALSE ;
        thisflag = meCFKILL;
    }
    else
        kstring = NULL ;
#if MEOPT_UNDO
    if(kflag & 2)
    {
        if(!(kflag & 4) && (nn == 1))
            meUndoAddDelChar() ;
        else
            meUndoAddDelChars(nn) ;
    }
#endif
    mldelete(nn,kstring) ;
    return ret ;
}

/*
 * yank text back via a klist.
 *
 * Return meTRUE if all goes well, meFALSE if the characters from the kill buffer
 * cannot be inserted into the text.
 */

int
yankfrom(struct meKill *pklist)
{
    int len=0 ;
    meKillNode *killp ;

#if MEOPT_EXTENDED
    if(meLineGetFlag(frameCur->windowCur->dotLine) & meLINE_PROTECT)
    {
        killp = pklist->kill;
        while(killp != NULL)
        {
            if(meStrchr(killp->data,meCHAR_NL) != NULL)
                return mlwrite(MWABORT,(meUByte *)"[Protected Line!]") ;
            killp = killp->next;
        }
    }
#endif
    killp = pklist->kill;
    while (killp != NULL)
    {
        len += bufferInsertText(killp->data,0) ;
        killp = killp->next;
    }
    return len ;
}

/*
 * Yank text back from the kill buffer. This is really easy. All of the work
 * is done by the standard insert routines. All you do is run the loop, and
 * check for errors. Bound to "C-Y".
 */
int
yank(int f, int n)
{
    register int ret ;		/* return from "yankfrom()" */
    register int len = 0 ;	/* return from "yankfrom()" */

    commandFlag[CK_YANK] = (comSelStart|comSelSetDot|comSelSetMark|comSelSetFix) ;
    if(n == 0)
    {
        /* place the mark on the current line */
        windowSetMark(meFALSE, meFALSE);
        /* remember that this was a yank command */
        thisflag = meCFYANK ;
        return meTRUE ;
    }
    if(n < 0)
    {
        meKill *kl ;
        
        while((++n <= 0) && ((kl = klhead) != NULL))
        {
            meKillNode *next, *kill ;
            kill = kl->kill ;
            while(kill != NULL)
            {
                next = kill->next;
                meFree(kill);
                kill = next;
            }
            klhead = kl->next ;
            meFree(kl) ;
        }
        reyankLastYank = NULL ;
        commandFlag[CK_YANK] = comSelKill ;
        return meTRUE ;
    }
        
#ifdef _CLIPBRD
    TTgetClipboard() ;
#endif
    /* make sure there is something to yank */
    if(klhead == NULL)
        return mlwrite(MWABORT,(meUByte *)"[nothing to yank]");
    /* Check we can change the buffer */
    if((ret=bufferSetEdit()) <= 0)
        return ret ;

    /* place the mark on the current line */
    windowSetMark(meFALSE, meFALSE);

    /* for each time.... */
    while(n--)
    {
        if((ret = yankfrom(klhead)) < 0)
            break;
        len += ret ;
    }
#if MEOPT_UNDO
    if(len > 0)
        meUndoAddInsChars(len) ;
#endif
    if(ret >= 0)
    {
        /* remember that this was a yank command */
        thisflag = meCFYANK ;
        return meTRUE ;
    }
    return meFALSE ;
}

int
reyank(int f, int n)
{
    meRegion region ;
    int ret, len ;
    
    if(f && (n >= 0))
    {
        reyankLastYank = klhead;
        while((--n >= 0) && ((reyankLastYank = reyankLastYank->next) != NULL))
            ;
        if(reyankLastYank == NULL)
            return mlwrite(MWABORT,(meUByte *)"[not enough in the kill-ring]");
    }
    else
    {
        if(lastflag == meCFYANK)
            /* Last command was yank. Set reyankLastYank to the most recent
             * delete, so we reyank the second most recent */
            reyankLastYank = klhead;
        else if(lastflag != meCFRYANK)
            /* Last command was not a reyank or yank, set reyankLastYank to
             * NULL so we bitch */
            reyankLastYank = NULL ;
    
        if(reyankLastYank == NULL)
            /*
             * Last command was not a yank or reyank (or someone was
             * messing with @cc and got it wrong). Complain and return
             * immediately, this is because the first thing we do is to
             * delete the current region without saving it. If we've just
             * done a yank, or reyank, the mark and cursor will surround
             * the last yank so this will be ok, but otherwise the user
             * would loose text with no hope of recovering it.
             */
            return mlwrite(MWABORT,(meUByte *)"[reyank must IMMEDIATELY follow a yank or reyank]");
    
        if(((reyankLastYank = reyankLastYank->next) == NULL) && (n < 0))
            /* Fail if got to the end and n is -ve */
            return meFALSE ;
        
        /* Get the current region */
        if((ret = getregion(&region)) <= 0)
            return ret ;
        
        /* Delete the current region. */
        frameCur->windowCur->dotLine  = region.line;
        frameCur->windowCur->dotOffset = region.offset;
        frameCur->windowCur->dotLineNo = region.lineNo;
        ldelete(region.size,6);
    }
    /* Set the mark here so that we can delete the region in the next
     * reyank command. */
    windowSetMark(meFALSE, meFALSE);

    /* If we've fallen off the end of the klist and there are no more
     * elements, wrap around to the most recent delete. This makes it
     * appear as if the linked list of kill buffers is actually
     * implemented as the GNU style "kill ring".
     */
    if(reyankLastYank == NULL)
    {
        mlwrite(MWABORT,(meUByte *)"[start of kill-ring]");
        reyankLastYank = klhead;
    }
    
    if(n < 0)
        n = 0 - n ;
    len = 0 ;
    while(n--)
    {
        if((ret = yankfrom(reyankLastYank)) < 0)
            return meFALSE ;
        len += ret ;
    }
#if MEOPT_UNDO
    if(len > 0)
        meUndoAddReplaceEnd(len) ;
#endif
    /* Remember that this is a reyank command for next time. */
    thisflag = meCFRYANK;

    return meTRUE ;
}

void
meLineLoopFree(meLine *lp, int flag)
{
    meLine *clp, *nlp ;
    clp = meLineGetNext(lp) ;
    while(clp != lp)
    {
        nlp = meLineGetNext(clp) ;
        meFree(clp) ;
        clp = nlp ;
    }
    if(flag)
        meFree(clp) ;
    else
        clp->next = clp->prev = clp ;
}
