/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * region.c - Region (dot to cursor) routines.
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
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:	
 *     The routines in this file deal with the region, that magic space
 *     between "." and mark. Some functions are commands. Some functions are
 *     just for internal use.
 */

#define	__REGIONC			/* Define filename */

#include "emain.h"

/*
 * This routine figures out the
 * bounds of the region in the current window, and
 * fills in the fields of the "meRegion" structure pointed
 * to by "rp". Because the dot and mark are usually very
 * close together, we scan outward from dot looking for
 * mark. This should save time. Return a standard code.
 * Callers of this routine should be prepared to get
 * an "meABORT" status; we might make this have the
 * conform thing later.
 */
int
getregion(register meRegion *rp)
{
    meLine *lp, *elp;
    long  size;
    
    if (frameCur->windowCur->markLine == NULL)
        return noMarkSet() ;
    if(frameCur->windowCur->dotLine == frameCur->windowCur->markLine)
    {
        rp->line = frameCur->windowCur->dotLine;
        if (frameCur->windowCur->dotOffset < frameCur->windowCur->markOffset) 
        {
            rp->offset = frameCur->windowCur->dotOffset;
            rp->size = (long)(frameCur->windowCur->markOffset-frameCur->windowCur->dotOffset);
        }
        else
        {
            rp->offset = frameCur->windowCur->markOffset;
            rp->size = (long)(frameCur->windowCur->dotOffset-frameCur->windowCur->markOffset);
        }
        rp->lineNo = frameCur->windowCur->dotLineNo;
        return meTRUE ;
    }
    if(frameCur->windowCur->dotLineNo < frameCur->windowCur->markLineNo)
    {
        elp = frameCur->windowCur->markLine ;
        lp = frameCur->windowCur->dotLine ;
        rp->lineNo = frameCur->windowCur->dotLineNo ;
        rp->offset = frameCur->windowCur->dotOffset ;
        size = frameCur->windowCur->dotOffset ;
        size = frameCur->windowCur->markOffset + meLineGetLength(lp) - size -
                  frameCur->windowCur->dotLineNo + frameCur->windowCur->markLineNo ;
    }
    else
    {
        elp = frameCur->windowCur->dotLine ;
        lp = frameCur->windowCur->markLine ;
        rp->lineNo = frameCur->windowCur->markLineNo ;
        rp->offset = frameCur->windowCur->markOffset ;
        size = frameCur->windowCur->markOffset ;
        size = frameCur->windowCur->dotOffset + meLineGetLength(lp) - size +
                  frameCur->windowCur->dotLineNo - frameCur->windowCur->markLineNo ;
    }
    rp->line = lp ;
    while((lp=meLineGetNext(lp)) != elp)
        size += meLineGetLength(lp) ;
    rp->size = size ;
    return meTRUE ;
}

/*
 * Kill the region. Ask "getregion"
 * to figure out the bounds of the region.
 * Move "." to the start, and kill the characters.
 * Bound to "C-W".
 */
int
killRegion(int f, int n)
{
    meRegion region;
    
    if(n == 0)
        return meTRUE ;
    if(getregion(&region) <= 0)
        return meFALSE ;
    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meFALSE ;
    frameCur->windowCur->dotLine = region.line ;
    frameCur->windowCur->dotLineNo = region.lineNo ;
    frameCur->windowCur->dotOffset = region.offset ;   
    
    return ldelete(region.size,(n > 0) ? 3:2);
}

/*
 * Copy all of the characters in the
 * region to the kill buffer. Don't move dot
 * at all. This is a bit like a kill region followed
 * by a yank. Bound to "M-W".
 */
int	
copyRegion(int f, int n)
{
    meRegion region ;
    meLine *line;
    meUByte  *ss, *dd ;
    meInt left, ii, ret ;
#if MEOPT_NARROW
    meUShort eoffset ;
    meLine *markLine, *dotLine, *elp ;
    meInt markLineNo, dotLineNo, expandNarrows=meFALSE ;
    meUShort markOffset, dotOffset ;
#endif
    
    if(getregion(&region) <= 0)
        return meFALSE ;
    left = region.size ;
    line = region.line ;
#if MEOPT_NARROW
    if((n & 0x01) && (frameCur->bufferCur->narrow != NULL) &&
       (frameCur->windowCur->dotLine != frameCur->windowCur->markLine))
    {
        /* expand narrows that are within the region */
        expandNarrows = meTRUE ;
        dotLine = frameCur->windowCur->dotLine ;
        dotLineNo = frameCur->windowCur->dotLineNo ;
        dotOffset = frameCur->windowCur->dotOffset ;
        markLine = frameCur->windowCur->markLine ;
        markLineNo = frameCur->windowCur->markLineNo ;
        markOffset = frameCur->windowCur->markOffset ;
        if(line == dotLine)
        {
            elp = markLine ;
            eoffset = markOffset ;
        }
        else
        {
            elp = dotLine ;
            eoffset = dotOffset ;
        }
        left += meBufferRegionExpandNarrow(frameCur->bufferCur,&line,region.offset,elp,eoffset,0) ;
        if(((frameCur->windowCur->dotLine != dotLine) && dotOffset) ||
           ((frameCur->windowCur->markLine != markLine) && markOffset))
        {
            ret = mlwrite(MWABORT,(meUByte *)"[Bad region definition]") ;
            goto copy_region_exit ;
        }
    }
#endif
    if(lastflag != meCFKILL)                /* Kill type command.   */
        killSave();
    if(left == 0)
    {
        thisflag = meCFKILL ;
        ret =  meTRUE ;
        goto copy_region_exit ;
    }
    
    if((dd = killAddNode(left)) == NULL)
    {
        ret = meFALSE ;
        goto copy_region_exit ;
    }
    if((ii = region.offset) == meLineGetLength(line))
        goto add_newline ;                 /* Current offset.      */
    ss = line->text+ii ;
    ii = meLineGetLength(line) - ii ;
    goto copy_middle ;
    while(left)
    {
        ss = line->text ;
        ii = meLineGetLength(line) ;
copy_middle:
        /* Middle of line.      */
        if(ii > left)
        {
            ii = left ;
            left = 0 ;
        }
        else
            left -= ii ;
        while(ii--)
            *dd++ = *ss++ ;
add_newline:
        if(left != 0)
        {
            *dd++ = meCHAR_NL ;
            line = meLineGetNext(line);
            left-- ;
        }
    }
    thisflag = meCFKILL ;
    ret = meTRUE ;

#ifdef _CLIPBRD
    TTsetClipboard() ;
    {
        static int xclipChecked = 0;
        static int xclipAvailable = 0;
        meUByte *sessionType;
        
        /* After explicit copy, run xclip to take over clipboard.
         * This pipes clipboard content through xclip, making xclip the owner
         * instead of ME, so subsequent mouse selections don't affect clipboard.
         * Only run on pure X11 (not Wayland, not console). */
        sessionType = meGetenv("XDG_SESSION_TYPE");
        if(!xclipChecked && sessionType != NULL && meStrcmp(sessionType, "wayland") == 0)
        {
            /* On Wayland - let Wayland clipboard handle it */
            xclipChecked = 1;
            xclipAvailable = 0;
        }
        else if(!xclipChecked)
        {
            xclipChecked = 1;
            xclipAvailable = (meGetenv("DISPLAY") != NULL) && (meGetenv("PATH") != NULL) &&
                (system((char *)"which xclip >/dev/null 2>&1") == 0);
        }
        if(xclipAvailable)
        {
            /* Fork and pipe through xclip to hand ownership to xclip */
            if(meFork() == 0)
            {
                execlp("sh", "sh", "-c", "xclip -selection clipboard -o | xclip -selection clipboard -i", NULL);
                _exit(1);
            }
        }
    }
#endif

copy_region_exit:

#if MEOPT_NARROW
    if(expandNarrows)
    {
        meBufferCollapseNarrowAll(frameCur->bufferCur) ;
        frameCur->windowCur->dotLine = dotLine ;
        frameCur->windowCur->dotLineNo = dotLineNo ;
        frameCur->windowCur->dotOffset = dotOffset ;
        frameCur->windowCur->markLine = markLine ;
        frameCur->windowCur->markLineNo = markLineNo ;
        frameCur->windowCur->markOffset = markOffset ;
    }
#endif
    return ret ;
}

/*
 * Lower case region. Zap all of the upper
 * case characters in the region to lower case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lineSetChanged" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-L".
 */
int
lowerRegion(int f, int n)
{
    meLine *line ;
    int   loffs ;
    meInt lline ;
    register meUByte c;
    register int   s;
    meRegion  region;
    
    if((s=getregion(&region)) <= 0)
        return (s);
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s ;
    line = frameCur->windowCur->dotLine ;
    loffs = frameCur->windowCur->dotOffset ;
    lline = frameCur->windowCur->dotLineNo ;
    frameCur->windowCur->dotLine = region.line ;
    frameCur->windowCur->dotOffset = region.offset ;
    frameCur->windowCur->dotLineNo = region.lineNo ;
    while (region.size--)
    {
        if((c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset)) == '\0')
        {
            frameCur->windowCur->dotLine = meLineGetNext(frameCur->windowCur->dotLine);
            frameCur->windowCur->dotOffset = 0;
            frameCur->windowCur->dotLineNo++ ;
        }
        else
        {
            if(isUpper(c))
            {
                lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                meLineSetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset, c);
            }
            (frameCur->windowCur->dotOffset)++ ;
        }
    }
    frameCur->windowCur->dotLine = line ;
    frameCur->windowCur->dotOffset = loffs ;
    frameCur->windowCur->dotLineNo = lline ;
    return meTRUE ;
}

/*
 * Upper case region. Zap all of the lower
 * case characters in the region to upper case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lineSetChanged" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-U".
 */
int
upperRegion(int f, int n)
{
    meLine *line ;
    int   loffs ;
    long  lline ;
    register char  c;
    register int   s;
    meRegion         region;
    
    if((s=getregion(&region)) <= 0)
        return (s);
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s ;
    line = frameCur->windowCur->dotLine ;
    loffs = frameCur->windowCur->dotOffset ;
    lline = frameCur->windowCur->dotLineNo ;
    frameCur->windowCur->dotLine = region.line ;
    frameCur->windowCur->dotOffset = region.offset ;
    frameCur->windowCur->dotLineNo = region.lineNo ;
    while (region.size--)
    {
        if((c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset)) == '\0')
        {
            frameCur->windowCur->dotLine = meLineGetNext(frameCur->windowCur->dotLine);
            frameCur->windowCur->dotOffset = 0;
            frameCur->windowCur->dotLineNo++ ;
        }
        else
        {
            if(isLower(c))
            {
                lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                meLineSetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset, c);
            }
            (frameCur->windowCur->dotOffset)++ ;
        }
    }
    frameCur->windowCur->dotLine = line ;
    frameCur->windowCur->dotOffset = loffs ;
    frameCur->windowCur->dotLineNo = lline ;
    return meTRUE ;
}

#if MEOPT_EXTENDED
int
killRectangle(int f, int n)
{
    meUByte *kstr ;
    meInt slno, elno, size ;
    int soff, eoff, coff, llen, dotPos ;
    
    if (frameCur->windowCur->markLine == NULL)
        return noMarkSet() ;
    if(frameCur->windowCur->dotLine == frameCur->windowCur->markLine)
        return killRegion(f,n) ;
    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meFALSE ;
    
    eoff = getcwcol() ;
    /* remember we have swapped */
    windowSwapDotAndMark(0,1) ;
    soff = getcwcol() ;
    if(soff > eoff)
    {
        llen = eoff ;
        eoff = soff ;
        soff = llen ;
    }
    llen = eoff - soff ;
    if((dotPos=frameCur->windowCur->dotLineNo > frameCur->windowCur->markLineNo))
        windowSwapDotAndMark(0,1) ;
    slno = frameCur->windowCur->dotLineNo ;
    elno = frameCur->windowCur->markLineNo ;
    /* calculate the maximum length */
    size = (elno - slno + 1) * (llen + 1) ;
    
    /* sort out the kill buffer */
    if((lastflag != meCFKILL) && (thisflag != meCFKILL))
        killSave();
    if((kstr = killAddNode(size)) == NULL)
        return meFALSE ;
    thisflag = meCFKILL;
    for(;;)
    {
        meUByte *off, cc ;
        int lspc=0, ii, jj, kk, ll ;
        
        windCurLineOffsetEval(frameCur->windowCur) ;
        off = frameCur->windowCur->dotCharOffset->text ;
        coff = 0 ;
        ii = 0 ;
        kk = frameCur->windowCur->dotLine->length ;
        while(ii < kk)
        {
            if(coff == soff)
                break ;
            if((coff+off[ii]) > soff)
            {
                lspc = soff - coff ;
                /* as we can convert tabs to spaces, adjust the offset */
                if(meLineGetChar(frameCur->windowCur->dotLine,ii) == meCHAR_TAB)
                    off[ii] -= lspc ;
                coff = soff ;
                break ;
            }
            coff += off[ii++] ;
        }
        jj = ii ;
        if(coff < soff)
        {
            /* line too short to even get to the start point */
            lspc = soff - coff ;
            memset(kstr,' ',llen) ;
            kstr += llen ;
        }
        else
        {
            while(jj < kk)
            {
                if(coff == eoff)
                    break ;
                if((ll = off[jj]) != 0)
                {
                    cc = meLineGetChar(frameCur->windowCur->dotLine,jj) ;
                    if((coff+ll) > eoff)
                    {
                        /* as we can convert tabs to spaces, delete and convert */
                        if(cc == meCHAR_TAB)
                        {
                            lspc += coff + ll - eoff ;
                            jj++ ;
                        }
                        break ;
                    }
                    coff += ll ;
                    if(cc == meCHAR_TAB)
                    {
                        /* convert tabs to spaces for better column support */
                        do
                            *kstr++ = ' ' ;
                        while(--ll > 0) ;
                    }
                    else
                        *kstr++ = cc ;
                }
                jj++ ;
            }
            if(coff < eoff)
            {
                /* line too short to get to the end point, fill with spaces */
                coff = eoff - coff ;
                memset(kstr,' ',coff) ;
                kstr += coff ;
            }
            
        }
        *kstr++ = '\n' ;
        frameCur->windowCur->dotOffset = ii ;
        if((jj -= ii) > 0)
        {
#if MEOPT_UNDO
            meUndoAddDelChars(jj) ;
#endif
            mldelete(jj,NULL) ;
        }
        if(lspc)
        {
            if(lineInsertChar(lspc,' ') <= 0)
            {
                *kstr = '\0' ;
                return meFALSE ;
            }
#if MEOPT_UNDO
            if(jj > 0)
                meUndoAddReplaceEnd(lspc) ;
            else
                meUndoAddInsChars(lspc) ;
#endif
        }
        if(frameCur->windowCur->dotLineNo == elno)
            break ;
        /* move on to the next line */
        frameCur->windowCur->dotLineNo++ ;
        frameCur->windowCur->dotLine  = meLineGetNext(frameCur->windowCur->dotLine);
        frameCur->windowCur->dotOffset  = 0;
    }
    *kstr = '\0' ;
#ifdef _CLIPBRD
    TTsetClipboard() ;
#endif
    if(dotPos)
        windowSwapDotAndMark(0,1) ;
    return meTRUE ;
}

static int
yankRectangleKill(struct meKill *pklist, int soff, int notLast)
{
    meUByte *off, *ss, *tt, *dd=NULL, cc ;
    int ii, jj, kk, lsspc, lespc, ldel, linsc, coff ;
    meKillNode *killp ;
    
    killp = pklist->kill ;
    while (killp != NULL)
    {
        ss = killp->data ;
        while(*ss != '\0')
        {
            tt = ss ;
            while(((cc = *ss) != '\0') && (cc != meCHAR_NL))
                ss++ ;
            ii = (size_t) ss - (size_t) tt ;
            windCurLineOffsetEval(frameCur->windowCur) ;
            off = frameCur->windowCur->dotCharOffset->text ;
            ldel = lsspc = lespc = 0 ;
            coff = 0 ;
            jj = 0 ;
            kk = frameCur->windowCur->dotLine->length ;
            while(jj < kk)
            {
                if(coff == soff)
                    break ;
                if((coff+off[jj]) > soff)
                {
                    /* if its a tab we can remove the tab and replace with spaces */
                    if(meLineGetChar(frameCur->windowCur->dotLine,jj) == meCHAR_TAB)
                    {
                        ldel = 1 ;
                        lespc = off[jj] - soff + coff ;
                    }
                    break ;
                }
                coff += off[jj++] ;
            }
            lsspc = soff - coff ;
            linsc = ii + lsspc + lespc - ldel ;
            frameCur->windowCur->dotOffset = jj ;
            /* Make space for the characters */
            if((dd = lineMakeSpace(linsc)) == NULL)
                return meABORT ;
            lineSetChanged(WFMOVEC|WFMAIN) ;
#if MEOPT_UNDO
            if(ldel > 0)
            {
                *dd = meCHAR_TAB ;
                frameCur->windowCur->dotOffset = jj ;
                meUndoAddDelChars(ldel) ;
            }
#endif
            /* add the starting spaces */
            while(--lsspc >= 0)
                *dd++ = ' ' ;
            while(--ii >= 0)
                *dd++ = *tt++ ;
            /* add the ending spaces (only if we've deleted a TAB),
             * preserve dd so the end point is correct */
            tt = dd ;
            while(--lespc >= 0)
                *tt++ = ' ' ;
#if MEOPT_UNDO
            if(ldel > 0)
            {
                frameCur->windowCur->dotOffset += linsc+ldel ;
                meUndoAddReplaceEnd(linsc+ldel) ;
            }
            else
                meUndoAddInsChars(linsc) ;
#endif
            if(*ss == meCHAR_NL)
                ss++ ;
            frameCur->windowCur->dotLineNo++ ;
            frameCur->windowCur->dotLine  = meLineGetNext(frameCur->windowCur->dotLine);
            frameCur->windowCur->dotOffset  = 0;
        }
        killp = killp->next;
    }
    if((dd != NULL) && !notLast)
    {
        frameCur->windowCur->dotLineNo-- ;
        frameCur->windowCur->dotLine = meLineGetPrev(frameCur->windowCur->dotLine);
        frameCur->windowCur->dotOffset = ((size_t) dd - (size_t) meLineGetText(frameCur->windowCur->dotLine)) ;
    }
    return meTRUE ;
}

int
yankRectangle(int f, int n)
{
    int col ;
#ifdef _CLIPBRD
    TTgetClipboard() ;
#endif
    /* make sure there is something to yank */
    if(klhead == NULL)
        return mlwrite(MWABORT,(meUByte *)"[nothing to yank]");
    /* Check we can change the buffer */
    if(bufferSetEdit() <= 0)
        return meABORT ;
    
    /* get the current column */
    col = getcwcol() ;
    
    /* place the mark on the current line */
    windowSetMark(meFALSE, meFALSE);
    
    /* for each time.... */
    while(--n >= 0)
        if(yankRectangleKill(klhead,col,n) <= 0)
            return meABORT ;
    return meTRUE ;
}
#endif

