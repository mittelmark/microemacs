/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * display.c - Display routines.
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
 * Synopsis:    Display routines.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     The functions in this file handle redisplay. There are two halves, the
 *     ones that update the virtual display screen, and the ones that make the
 *     physical display screen the same as the virtual display screen. These
 *     functions use hints that are left in the windows by the commands.
 */

#define	__DISPLAYC				/* Name file */

#include "emain.h"
#include "eskeys.h"		/* External Defintions */
#include "evers.h"                      /* Version information */

#ifdef _STDARG
#include <stdarg.h>		/* Variable Arguments */
#endif

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#endif

#ifdef _DOS
#include <pc.h>
#endif

/*
 * Set the virtual cursor to the specified row and column on the virtual
 * screen. There is no checking for nonsense values; this might be a good
 * idea during the early stages.
 */

/* Poke screen flags
 * Poke screen maintains the rectangular extents of the region of the
 * screen that has been damaged. These screen areas are fixed on the next
 * invocation to update(). Note that we only initialise the flag, the other
 * fields should be automatically initialised since we start in a garbaged
 * state.
 *
 * Note that all poke extents are 'inclusive' values.
 */
static void pokeUpdate (void);          /* The poke screen update function */

/*
 * convertSchemeToStyle
 * Validate a hilighting colour.
 */
int
convertUserScheme(int n, int defaultStyle)
{
    if ((n < 0) || (n >= styleTableSize))
    {
        mlwrite (MWABORT|MWPAUSE,(meUByte *)"[Invalid color-scheme index %d]", n);
        return defaultStyle ;
    }
    return (n*meSCHEME_STYLES) ;
}

/*
 * Apply selection hilighting to the window
 */
static void
shilightWindow(meWindow *wp)
{
    meInt lineNo;           /* Toprow line number */

    /* Check to see if any selection hilighting is to be removed. */
    if(((selhilight.flags & SELHIL_ACTIVE) == 0) ||
       (wp->buffer != selhilight.bp))
        goto remove_hilight;

    if((wp->updateFlags & (WFSELHIL|WFREDRAW)) == 0)
        return ;

    /* Hilight selection has been modified. Work out the start and
     * end points
     */
    if (selhilight.flags & SELHIL_CHANGED)
    {
        selhilight.flags &= ~(SELHIL_CHANGED|SELHIL_SAME);

        /* dot ahead of mark line */
        if (selhilight.dotLineNo > selhilight.markLineNo)
        {
            selhilight.sline = selhilight.markLineNo;
            selhilight.eline = selhilight.dotLineNo;
            selhilight.soff = selhilight.markOffset;
            selhilight.eoff = selhilight.dotOffset;
        }
        /* dot behind mark line */
        else if (selhilight.dotLineNo < selhilight.markLineNo)
        {
            selhilight.sline = selhilight.dotLineNo;
            selhilight.eline = selhilight.markLineNo;
            selhilight.soff = selhilight.dotOffset;
            selhilight.eoff = selhilight.markOffset;
        }
        /* dot and mark on same line */
        else
        {
            selhilight.sline = selhilight.markLineNo;
            selhilight.eline = selhilight.markLineNo;
            /* dot ahead of mark */
            if (selhilight.markOffset > selhilight.dotOffset)
            {
                selhilight.soff = selhilight.dotOffset;
                selhilight.eoff = selhilight.markOffset;
            }
            /* dot behind mark line */
            else if (selhilight.markOffset < selhilight.dotOffset)
            {
                selhilight.soff = selhilight.markOffset;
                selhilight.eoff = selhilight.dotOffset;
            }
            /* dot and mark on same offset */
            else
                selhilight.flags |= SELHIL_SAME;
        }
    }

    lineNo = wp->vertScroll ;
    if ((selhilight.flags & SELHIL_SAME) ||
        (lineNo > selhilight.eline) ||
        (lineNo+wp->textDepth-1 < selhilight.sline))
    {
remove_hilight:
        if(wp->updateFlags & WFSELDRAWN)
        {
            meVideoLine  *vptr;               /* Pointer to the video block */
            meUShort  sline, eline ;          /* physical screen line to update */

            /* Determine the video line position and determine the video block that
             * is being used. */
            vptr = wp->video->lineArray;      /* Video block */
            sline = wp->frameRow;             /* Start line */
            eline = sline + wp->textDepth ;  /* End line */

            while (sline < eline)
            {
                meUShort vflag;

                if ((vflag = vptr[sline].flag) & VFSHMSK)
                    vptr[sline].flag = (vflag & ~VFSHMSK)|VFCHNGD;
                sline++;
            }
            wp->updateFlags = (wp->updateFlags & ~WFSELDRAWN) | WFMAIN ;
        }
    }
    else
    {
        meVideoLine *vptr;                /* Pointer to the video block */
        meUShort  sline, eline ;          /* physical screen line to update */

        /* Determine the video line position and determine the video block that
         * is being used. */
        vptr = wp->video->lineArray;      /* Video block */
        sline = wp->frameRow;             /* Start line */
        eline = sline + wp->textDepth ;  /* End line */

        /* Mark all of the video lines with the update. */
        while (sline < eline)
        {
            meUShort vflag;

            vflag = vptr[sline].flag;

            if ((lineNo < selhilight.sline) || (lineNo > selhilight.eline))
            {
                if (vflag & VFSHMSK)
                    vptr[sline].flag = (vflag & ~VFSHMSK)|VFCHNGD;
            }
            else
            {
                if (lineNo == selhilight.sline)
                {
                    if (lineNo == selhilight.eline)
                        vptr[sline].flag = (vflag & ~VFSHMSK)|(VFSHBEG|VFSHEND|VFCHNGD);
                    else
                        vptr[sline].flag = (vflag & ~VFSHMSK)|(VFSHBEG|VFCHNGD);
                }
                else if (lineNo == selhilight.eline)
                    vptr[sline].flag = (vflag & ~VFSHMSK)|(VFSHEND|VFCHNGD);
                else if ((vflag & VFSHALL) == 0)
                    vptr[sline].flag = (vflag & ~VFSHMSK)|(VFSHALL|VFCHNGD);
            }
            /* on to the next one */
            sline++;                        /* Next video line */
            lineNo++;                       /* Next line number */
        }
        wp->updateFlags |= (WFSELDRAWN|WFMAIN) ;
    }
}

#if MEOPT_EXTENDED
int
showCursor(int f, int n)
{
    int ii ;

    ii = cursorState ;
    if(f)
        cursorState += n ;
    else
        cursorState = 0 ;
    if((cursorState >= 0) && (ii < 0))
    {
        if(cursorBlink)
            /* kick off the timer */
            TThandleBlink(1) ;
        else
            TTshowCur() ;
    }
    else if((cursorState < 0) && (ii >= 0))
        TThideCur() ;
    return meTRUE ;
}

int
showRegion(int f, int n)
{
    int absn ;

    absn = (n < 0) ? -n:n ;

    if(((absn == 1) || (absn == 2)) && !(selhilight.flags & SELHIL_FIXED))
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No current region]") ;
    if(((absn == 2) || (n == 4)) && (selhilight.bp != frameCur->bufferCur))
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Current region not in this buffer]") ;
    switch(n)
    {
    case -3:
        selhilight.bp = frameCur->bufferCur;                  /* Select the current buffer */
        selhilight.flags = SELHIL_ACTIVE|SELHIL_CHANGED ;
        selhilight.markOffset = frameCur->windowCur->dotOffset;   /* Current mark offset */
        selhilight.markLineNo = frameCur->windowCur->dotLineNo;    /* Current mark line number */
        selhilight.dotOffset = frameCur->windowCur->dotOffset;   /* Current mark offset */
        selhilight.dotLineNo = frameCur->windowCur->dotLineNo;    /* Current mark line number */
        break ;

    case -2:
        frameCur->windowCur->updateFlags |= WFMOVEL ;
        if((windowGotoLine(meTRUE,selhilight.markLineNo+1) > 0) &&
           (selhilight.markOffset <= meLineGetLength(frameCur->windowCur->dotLine)))
        {
            frameCur->windowCur->dotOffset = (meUShort) selhilight.markOffset ;
            return meTRUE ;
        }
        return meFALSE ;

    case -1:
        selhilight.flags &= ~SELHIL_ACTIVE ;
        break ;
    case 0:        /* Show status of hilighting */
        n = 0 ;
        if(selhilight.flags & SELHIL_ACTIVE)
            n |= 1 ;
        if(selhilight.flags & SELHIL_FIXED)
            n |= 2 ;
        if(selhilight.bp == frameCur->bufferCur)
        {
            n |= 4 ;
            /* check for d >= dot > m */
            if(((selhilight.dotLineNo < frameCur->windowCur->dotLineNo) ||
                ((selhilight.dotLineNo == frameCur->windowCur->dotLineNo) && (selhilight.dotOffset <= frameCur->windowCur->dotOffset))) &&
               ((selhilight.markLineNo > frameCur->windowCur->dotLineNo) ||
                ((selhilight.markLineNo == frameCur->windowCur->dotLineNo) && (selhilight.markOffset >  frameCur->windowCur->dotOffset))))
                n |= 8 ;
            /* check for m >= dot > d */
            else if(((selhilight.markLineNo < frameCur->windowCur->dotLineNo) ||
                     ((selhilight.markLineNo == frameCur->windowCur->dotLineNo) && (selhilight.markOffset <= frameCur->windowCur->dotOffset))) &&
                    ((selhilight.dotLineNo > frameCur->windowCur->dotLineNo) ||
                     ((selhilight.dotLineNo == frameCur->windowCur->dotLineNo) && (selhilight.dotOffset >  frameCur->windowCur->dotOffset))))
                n |= 8 ;
        }
        sprintf((char *)resultStr,"%d",n) ;
        return meTRUE ;

    case 1:
        selhilight.flags |= SELHIL_ACTIVE;
        if(f || (selhilight.uFlags & SELHILU_KEEP))
            selhilight.flags |= SELHIL_KEEP ;
        break ;

    case 2:
        frameCur->windowCur->updateFlags |= WFMOVEL ;
        if((windowGotoLine(meTRUE,selhilight.dotLineNo+1) > 0) &&
           (selhilight.dotOffset <= meLineGetLength(frameCur->windowCur->dotLine)))
        {
            frameCur->windowCur->dotOffset = (meUShort) selhilight.dotOffset ;
            return meTRUE ;
        }
        return meFALSE ;

    case 3:
        if(((selhilight.flags & (SELHIL_FIXED|SELHIL_ACTIVE)) == 0) ||
           (selhilight.bp != frameCur->bufferCur))
        {
            selhilight.bp = frameCur->bufferCur ;
            selhilight.markOffset = frameCur->windowCur->dotOffset;  /* Current mark offset */
            selhilight.markLineNo = frameCur->windowCur->dotLineNo;  /* Current mark line number */
        }
        selhilight.flags |= SELHIL_FIXED|SELHIL_CHANGED|SELHIL_ACTIVE ;
        if(selhilight.uFlags & SELHILU_KEEP)
            selhilight.flags |= SELHIL_KEEP ;
        selhilight.dotOffset = frameCur->windowCur->dotOffset;      /* Current mark offset */
        selhilight.dotLineNo = frameCur->windowCur->dotLineNo;      /* Current mark line number */
        break ;

    case 4:
        selhilight.flags = SELHIL_ACTIVE|SELHIL_CHANGED ;
        selhilight.dotOffset = frameCur->windowCur->dotOffset;      /* Current mark offset */
        selhilight.dotLineNo = frameCur->windowCur->dotLineNo;      /* Current mark line number */
        break ;

   default:
        return meABORT ;
    }
    meBufferAddModeToWindows(selhilight.bp, WFSELHIL);
    return meTRUE ;
}
#endif

void
windCurLineOffsetEval(meWindow *wp)
{
    if((wp->dotCharOffset->next == wp->dotLine) &&
       !(wp->dotLine->flag & meLINE_CHANGED))
        return ;
    if(wp->dotLine->length > meLineGetMaxLength(wp->dotCharOffset))
    {
        meFree(wp->dotCharOffset) ;
        wp->dotCharOffset = meLineMalloc(wp->dotLine->length,0) ;
    }
    /* store dotp as the last line done */
    wp->dotCharOffset->next = wp->dotLine ;
#if MEOPT_COLOR
#if MEOPT_HILIGHT
    if((wp->buffer->hilight != 0) &&
       (meHilightGetFlags(hilights[wp->buffer->hilight]) & HFRPLCDIFF))
        hilightCurLineOffsetEval(wp) ;
    else
#endif
#endif
    {
        register meUByte cc, *ss, *off ;
        register int pos, ii ;

        ss = wp->dotLine->text ;
        off = wp->dotCharOffset->text ;
        pos = 0 ;
#ifndef NDEBUG
        if(wp->dotLine->text[wp->dotLine->length] != '\0')
        {
            _meAssert(__FILE__,__LINE__) ;
            wp->dotLine->text[wp->dotLine->length] = '\0' ;
        }
#endif
        while((cc=*ss++) != 0)
        {
            if(isDisplayable(cc))
                ii = 1 ;
            else if(cc == meCHAR_TAB)
                ii = get_tab_pos(pos,wp->buffer->tabWidth) + 1;
            else if (cc < 0x20)
                ii = 2 ;
            else
                ii = 4 ;
            *off++ = (meUByte) ii ;
            pos += ii ;
        }
        *off = 0 ;
    }
}

void
updCursor(register meWindow *wp)
{
    register meUInt ii, jj ;

    if(wp->updateFlags & (WFREDRAW|WFRESIZE))
        /* reset the dotCharOffset to force a recalc when needed */
        wp->dotCharOffset->next = NULL ;

    /* set ii to cursors screen col & jj to the require screen scroll */
    if(wp->dotOffset == 0)
    {
        /* simple express case when offset is zero */
        ii = 0 ;
        jj = 0 ;
        /* must reset the dotCharOffset if its changed cos the flag will be lost
         * for next time! Nasty bug!! Must check the line is still dotp if we
         * are to keep it cos it could have been freed.
         */
        if((wp->dotCharOffset->next != wp->dotLine) || (wp->dotLine->flag & meLINE_CHANGED))
            wp->dotCharOffset->next = NULL ;
    }
    else
    {
        register meUByte *off ;
        int leftMargin;                 /* Base left margin position (scrolled) */

        windCurLineOffsetEval(wp) ;
        jj = 0 ;
        ii = wp->dotOffset ;
        off = wp->dotCharOffset->text ;
        while(ii--)
            jj += *off++ ;

        /* jj - is the character offset of the cursor on the screen */
        if((scrollFlag & 0x0f) == 3)
            leftMargin = wp->horzScrollRest ;
        else
            leftMargin = wp->horzScroll;

        /* Cursor behind the current scroll position */
        if(((int) jj) <= leftMargin)
        {
            ii = wp->textWidth - (((leftMargin - jj) % (wp->textWidth - (wp->marginWidth << 1))) + wp->marginWidth);
            if (ii > jj)                   /* Make sure we have enough info to show */
                ii = jj;                   /* No - shoe the start of the line */
            jj -= ii;                      /* Correct the screen offset position */
        }
        /* Cursor ahead of the scroll position and off screen */
        else if (((int) jj) >= leftMargin + (wp->textWidth-1))
        {
            /* Move onto the next screen */
            ii = ((jj - leftMargin - wp->textWidth) % (wp->textWidth - (wp->marginWidth << 1))) + wp->marginWidth;
            jj -= ii;                      /* Screen offset */
        }
        /* Cursor ahead of scroll position and on screen */
        else
        {
            /* Check to see if a separate line scroll is in effect and can be
             * knocked off because the text is on screen. */
            if ((wp->horzScroll != wp->horzScrollRest) &&
                ((scrollFlag & 0x0f) != 3) &&
                ((jj - wp->horzScrollRest) < wp->textWidth))
            {
                /* The current scrolls are not aligned and can be. Fix by
                 * reseting the left margin with the scroll reset. */
                ii = jj - wp->horzScrollRest;
                jj = wp->horzScrollRest;
            }
            else
            {
                /* Cursor ahead of scroll position and on screen */
                ii = jj - leftMargin ;
                jj = leftMargin ;
            }
        }
    }

    frameCur->mainRow = wp->frameRow + (wp->dotLineNo - wp->vertScroll) ;
    frameCur->mainColumn = wp->frameColumn + ii ;
    if(wp->horzScroll != (meInt) jj)         /* Screen scroll correct ?? */
    {
        wp->horzScroll = (meInt) jj;    /* Scrolled line offset */
        wp->updateFlags |= WFDOT ;
        /* must make sure the disLineBuff is at least as long as the current scroll
         * otherwise if the horizontal scroll is set to 2 and the first line is short
         * updateline will scribble as it assumes the buffer is long enough */
        if(((int) jj) >= disLineSize)
        {
            do
                disLineSize += 512;
            while(disLineSize < ((int) jj));
            disLineBuff = meRealloc(disLineBuff,disLineSize+32);
        }
    }
    switch(scrollFlag & 0x0f)
    {
    case 0:
        if(wp->horzScrollRest)
        {
            /* Reset scroll column */
            wp->horzScrollRest = 0;             /* Set scroll column to base position */
            wp->updateFlags |= WFREDRAW;        /* Force a screen update */
        }
        break ;
    case 1:
        if((wp->horzScrollRest != (meInt) jj) && wp->horzScrollRest)
        {
            /* Reset scroll column */
            wp->horzScrollRest = 0;             /* Set scroll column to base position */
            wp->updateFlags |= WFREDRAW;        /* Force a screen update */
        }
        break ;
    case 2:
        if(wp->horzScrollRest != (meInt) jj)
        {
            /* Reset scroll column */
            wp->horzScrollRest = (meInt) jj;    /* Set scroll column to base position */
            wp->updateFlags |= WFREDRAW;        /* Force a screen update */
        }
        break ;
        /* case 3 leaves the scroll alone */
    }
}

#define DEBUGGING 0
#if DEBUGGING
static char drawno = 'A';
#endif

/*
 * renderLine
 * This function renders a non-hilighted text line.
 */
int
renderLine (meUByte *s1, int len, int wid, meBuffer *bp)
{
    register meUByte cc;
    register meUByte *s2;

    s2 = disLineBuff + wid;
    while(len-- > 0)
    {
        /* the largest character size is a tab which is user definable */
        if (wid >= disLineSize)
        {
            disLineSize += 512 ;
            disLineBuff = meRealloc(disLineBuff,disLineSize+32) ;
            s2 = disLineBuff + wid ;
        }
        cc = *s1++ ;
        if(isDisplayable(cc))
        {
            wid++ ;
            if(cc == ' ')
                *s2++ = displaySpace ;
            else if(cc == meCHAR_TAB)
                *s2++ = displayTab ;
            else
                *s2++ = cc ;
        }
        else if(cc == meCHAR_TAB)
        {
            int ii=get_tab_pos(wid,bp->tabWidth) ;

            wid += ii+1 ;
            *s2++ = displayTab ;
            while(--ii >= 0)
                *s2++ = ' ' ;
        }
        else if(cc < 0x20)
        {
            wid += 2 ;
            *s2++ = '^' ;
            *s2++ = cc ^ 0x40 ;
        }
        else
        {
            /* Its a nasty character */
            wid += 4 ;
            *s2++ = '\\' ;
            *s2++ = 'x' ;
            *s2++ = hexdigits[cc/0x10] ;
            *s2++ = hexdigits[cc%0x10] ;
        }
    }
    return wid;
}

/* row of screen to update, virtual screen image */
static int
updateline(register int row, register meVideoLine *vp1, meWindow *window)
{
    register meUByte *s1;       /* Text line pointer */
    register meUShort flag ;    /* Video line flag */
    meSchemeSet *blkp;             /* Style change list */
    meScheme *fssp;             /* Frame store - colour pointer */
    meUByte    *fstp;           /* Frame store - text pointer */
    meUShort noColChng;         /* Number of colour changes */
    meUShort scol;              /* Lines starting column */
    meUShort ncol;              /* Lines number of columns */

    /* Get column size/offset */
    s1 = vp1->line->text;
    if(window != NULL)
    {
        scol = window->frameColumn ;
        ncol = window->textWidth ;
    }
    else
    {
        scol = 0 ;
        ncol = frameCur->width ;
    }

    if((flag = vp1->flag) & VFMAINL)
    {
        meScheme scheme;
#if MEOPT_HILIGHT
        if(meLineIsSchemeSet(vp1->line))
        {
            scheme = window->buffer->lscheme[meLineGetSchemeIndex(vp1->line)] ;
            /* We have to assume this line is an exception and the hilno & bracket
             * for the next line should be what this line would have been */
            if(window->buffer->hilight &&
               ((vp1[1].hilno != vp1[0].hilno) || (vp1[1].bracket != vp1[0].bracket)))
            {
                vp1[1].flag |= VFCHNGD ;
                vp1[1].hilno = vp1[0].hilno ;
                vp1[1].bracket = vp1[0].bracket ;
            }
            goto hideLineJump ;
        }
        else if(window->buffer->hilight)
        {
            meUByte tempIsWordMask;

            /* The highlighting relies on the correct setting of the
             * isWordMask to find word boundaries. Context save the existing
             * word mask and restore from the buffer. Do the hilighting
             * and then restore the previous setting. */
            tempIsWordMask = isWordMask;
            isWordMask = window->buffer->isWordMask;
            noColChng = hilightLine(vp1,0) ;
            isWordMask = tempIsWordMask;

            /* Export the information from the higlighting request */
            blkp = hilBlock + 1 ;
            scheme = meHilightGetTruncScheme(hilights[vp1->hilno]) ;
        }
        else
#endif
        {
            meInt lineLen;

#if MEOPT_COLOR
            scheme = window->buffer->scheme;
#else
            scheme = globScheme;
#endif
#if MEOPT_HILIGHT
hideLineJump:
#endif
            blkp = hilBlock + 1 ;
            lineLen = vp1->line->length;

            if (flag & VFCURRL)
                scheme += meSCHEME_CURRENT;

            /* Determine if there is any selection hilighting on the line. */
            if (flag & VFSHMSK)
            {
                /* Check for whole line hilighted */
                if (flag & VFSHALL)
                {
                    if(lineLen > 0)
                        blkp[0].column = renderLine(s1,lineLen,0,window->buffer) ;
                    else
                        blkp[0].column = 0 ;
                    blkp[0].scheme = scheme + meSCHEME_SELECT;
                    noColChng = 1 ;
                }
                else
                {
                    register meInt wid, len;

                    /* Region hilight commences in this line */
                    if((flag & VFSHBEG) && (selhilight.soff > 0))
                    {
                        len = selhilight.soff ;
                        wid = renderLine(s1, len, 0, window->buffer) ;
                        blkp[0].scheme = scheme ;
                        blkp[0].column = wid ;
                        noColChng = 1 ;
                    }
                    else
                    {
                        wid = 0 ;
                        len = 0 ;
                        noColChng = 0 ;
                    }

                    /* Region hilight terminates in this line */
                    if (flag & VFSHEND)
                    {
                        if (selhilight.eoff > len)
                        {
                            /* Set up the colour change */
                            wid = renderLine(s1+len,selhilight.eoff-len,wid,window->buffer) ;
                            blkp[noColChng].scheme = scheme + meSCHEME_SELECT;
                            blkp[noColChng++].column = wid ;
                            len = selhilight.eoff ;
                        }
                        blkp[noColChng].scheme = scheme ;
                    }
                    else
                        /* Set up the colour change */
                        blkp[noColChng].scheme = scheme + meSCHEME_SELECT;

                    /* Render the rest of the line in the standard colour */
                    if (lineLen > len)
                        wid = renderLine(s1+len, lineLen-len,wid,window->buffer) ;
                    blkp[noColChng].column = wid ;
                    noColChng += 1 ;
                }
            }
            else
            {
                /* Render the rest of the line in the standard colour */
                if (lineLen > 0)
                    blkp[0].column = renderLine(s1,lineLen,0,window->buffer) ;
                else
                    blkp[0].column = 0 ;
                blkp[0].scheme = scheme ;
                noColChng = 1 ;
            }
            scheme = trncScheme ;
        }

        s1 = disLineBuff ;
        {
            meInt scroll;

            if (flag & VFCURRL)
                scroll = window->horzScroll ;   /* Current line scroll */
            else
                scroll = window->horzScrollRest ;    /* Hard window scroll */
            if(scroll != 0)
            {
                meInt ii;

                /* Line is scrolled. The effect we want is if any text at all
                 * is on the line we place a dollar at the start of the line
                 * in the last highlighting colour. If the line is empty then
                 * we do not want the dollar.
                 *
                 * Only process the line if it is not empty, this ensures that
                 * we do not insert a dollar where not required. */
                if (blkp->column > 0)
                {
                    s1 += scroll ;
                    while((blkp->column <= scroll))
                    {
                        if(noColChng == 1)
                        {
                            blkp->column = scroll+1 ;
                            break ;
                        }
                        blkp++ ;
                        noColChng-- ;
                    }

                    for(ii=0 ; ii<noColChng ; ii++)
                        blkp[ii].column -= scroll ;

                    /* set the first char to the truncate '$' and set the scheme */
                    *s1 = windowChars[WCDISPTXTLFT] ;
                    blkp-- ;
                    blkp[0].column = 1 ;
                    blkp[0].scheme = scheme | (blkp[1].scheme & (meSCHEME_CURRENT|meSCHEME_SELECT)) ;
                    noColChng++ ;
                }
            }
        }
        if(blkp[noColChng-1].column >= ncol)
        {
            while((noColChng > 1) && (blkp[noColChng-2].column >= (ncol-1)))
                noColChng-- ;
            blkp[noColChng-1].column = ncol-1 ;
            /* set the last char to the truncate '$' and set the scheme */
            if(meSchemeTestStyleHasFont(scheme))
            {
                /* remove the fonts as these can effect the next char which will probably be the scroll bar */
                scheme = meSchemeSetNoFont(scheme) ;
            }
            s1[ncol-1] = windowChars[WCDISPTXTRIG] ;
            blkp[noColChng].column = ncol ;
            blkp[noColChng].scheme = scheme | (blkp[noColChng-1].scheme & (meSCHEME_CURRENT|meSCHEME_SELECT)) ;
            noColChng++ ;
        }
        else
        {
            /* An extra space is added to the last column so that
             * the cursor will be the right colour */
            if(vp1->line != window->buffer->baseLine)
                s1[blkp[noColChng-1].column] = displayNewLine ;
            else
                s1[blkp[noColChng-1].column] = ' ' ;
            if(meSchemeTestStyleHasFont(blkp[noColChng-1].scheme))
            {
                /* remove the fonts as these can effect the next char which will probably be the scroll bar */
                if(((noColChng == 1) && (blkp[0].column == 0)) ||
                   ((noColChng > 1) && (blkp[noColChng-1].column == blkp[noColChng-2].column)))
                {
                    blkp[noColChng-1].column++ ;
                    blkp[noColChng-1].scheme = meSchemeSetNoFont(blkp[noColChng-1].scheme) ;
                }
                else
                {
                    blkp[noColChng].column = blkp[noColChng-1].column+1 ;
                    blkp[noColChng].scheme = meSchemeSetNoFont(blkp[noColChng-1].scheme) ;
                    noColChng++ ;
                }
            }
            else
            {
                blkp[noColChng-1].column++ ;
            }
        }
    }
    else
    {
        blkp = hilBlock ;
        blkp->column = vp1->line->length ;
        noColChng = 1 ;
        if(flag & VFMODEL)
            blkp->scheme = mdLnScheme + ((flag & VFCURRL) ? meSCHEME_CURRENT:meSCHEME_NORMAL);
#if MEOPT_OSD
        else if (flag & VFMENUL)        /* Menu line */
            blkp->scheme = osdScheme;
#endif
        else
            blkp->scheme = mlScheme;
    }

    /* Get the frame store colour and text pointers */
    fssp = &frameCur->store[row].scheme[scol] ;
    fstp = &frameCur->store[row].text[scol] ;

#ifdef _UNIX

#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
    {
#ifdef _TCAP
        /********************************************************************
         * TERMCAP                                                          *
         ********************************************************************/
        meInt ii, col, cno;
        meScheme scheme;

        TCAPmove(row,scol);	/* Go to start of line. */

        col = 0 ;
        for(cno=0 ; cno<noColChng ; cno++)
        {
            scheme = blkp->scheme ;
            TCAPschemeSet(scheme) ;

            /* Output the character in the specified colour.
             * Maintain the frame store */
            for(ii = blkp->column ; col<ii ; col++)
            {
                *fssp++ = scheme;
                *fstp++ = *s1;
                TCAPputc(*s1++) ;
            }
            blkp++;
        }
        if (meStyleCmpBColor(meSchemeGetStyle(vp1->eolScheme),meSchemeGetStyle(scheme)))
        {
            ii = ncol ;
            vp1->eolScheme = scheme ;
        }
        else
            ii = vp1->endp ;
        vp1->endp = col ;
        if((ii -= col) > 0)
        {
            /* Output the end of the line - maintain the frame store */
            while(--ii >= 0)
            {
                *fssp++ = scheme ;
                *fstp++ = ' ' ;
                TCAPputc(' ') ;
            }
        }
#if DEBUGGING
        TCAPputc(drawno) ;
#endif
        TCAPschemeReset() ;
#endif /* _TCAP */
    }
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
    {
#ifdef _XTERM
        /********************************************************************
         * X-WINDOWS                                                        *
         ********************************************************************/
        register int ii, len, col, cno ;
        register meScheme scheme ;

        row = rowToClient(row) ;
        col = cno = 0 ;                 /* Virtual column start */

        if (meSystemCfg & meSYSTEM_FONTFIX)
        {
            meUByte cc, *sfstp=fstp;
            meInt spFlag = 0, ccol=col;
            do {
                scheme = blkp->scheme ;
                meFrameXTermSetScheme(frameCur,scheme) ;
                ii = blkp->column ;
                ii -= col;
                len = ii ;
                blkp++ ;
                ccol = col ;
                spFlag = 0 ;
                /* Maintain the frame store and copy the string into
                 * the frame store with the colour information
                 * copy a space in place of special chars, they are
                 * drawn separately after the XDraw, the spaces are
                 * replaced with the correct chars */
                while (--len >= 0)
                {
                    *fssp++ = scheme;
                    if(((cc=s1[col++]) & 0xe0) == 0)
                    {
                        cc = ' ' ;
                        spFlag++ ;
                    }
                    *fstp++ = cc ;
                }
                meFrameXTermDrawString(frameCur,colToClient(scol+ccol),row,(char *)sfstp+ccol,ii);
                while(--spFlag >= 0)
                {
                    while (((cc=s1[ccol]) & 0xe0) != 0)
                        ccol++ ;
                    sfstp[ccol] = cc ;
                    meFrameXTermDrawSpecialChar(frameCur,colToClient(scol+ccol),row-mecm.ascent,cc) ;
                    ccol++ ;
                }
            } while(++cno < noColChng) ;
        }
        else
        {
            do {
                scheme = blkp->scheme ;
                meFrameXTermSetScheme(frameCur,scheme) ;
                ii = blkp->column ;
                len = ii-col;
                meFrameXTermDrawString(frameCur,colToClient(scol+col),row,(char *)s1+col,len);
                blkp++ ;

                /* Maintain the frame store and copy the string into
                 * the frame store with the colour information */
                while (--len >= 0)
                {
                    *fssp++ = scheme;
                    *fstp++ = s1[col++];
                }
            } while(++cno < noColChng) ;
        }
        if (meStyleCmpBColor(meSchemeGetStyle(vp1->eolScheme),meSchemeGetStyle(scheme)))
        {
            ii = ncol ;
            vp1->eolScheme = scheme;
        }
        else
            ii = vp1->endp ;
        vp1->endp = col ;

        if(ii > col)
        {
            char buff[meBUF_SIZE_MAX] ;

            ii -= col ;
            cno = ii ;
            s1 = (meUByte *) buff ;
            do
            {
                *s1++ = ' ' ;           /* End fill with spaces */
                *fstp++ = ' ';         /* Frame store space fill */
                *fssp++ = scheme;      /* Frame store colour fill */
            }
            while(--cno) ;
            meFrameXTermDrawString(frameCur,colToClient(scol+col),row,(char *)buff,ii);
        }
#if DEBUGGING
        else
            ii = 0 ;
        meFrameXTermDrawString(frameCur,colToClient(scol+col+ii-1),row,&drawno,1);
#endif
#endif /* _XTERM */
    }
#endif /* _ME_WINDOW */

#endif /* _UNIX */

    /************************************************************************
     *
     * MS-DOS
     *
     ***********************************************************************/
#ifdef _DOS
    {
        meScheme scheme;
        meInt ii, len;
        meUByte  cc;

        ii = 0 ;
        do {
            scheme = blkp->scheme ;
            cc = TTschemeSet(scheme) ;

            /* Update the screen.
             * Maintain the frame store and copy the sting into the
             * frame store with the colour information */
            len = blkp->column ;
            for(; ii < len; ii++)
            {
                *fssp++ = scheme ;
                *fstp++ = *s1 ;
                ScreenPutChar(*s1++, cc, scol++, row);
            }
            blkp++;
        } while(--noColChng) ;

        if (meStyleCmpBColor(meSchemeGetStyle(vp1->eolScheme),meSchemeGetStyle(scheme)))
        {
            len = ncol ;
            vp1->eolScheme = scheme;
        }
        else
            len = vp1->endp ;
        vp1->endp = ii ;
        /* Update the screen for end of the line. Maintain the
         * frame store copy. */
        for( ; ii<len ; ii++)
        {
            *fssp++ = scheme;
            *fstp++ = ' ';
            ScreenPutChar(' ', cc, scol++, row);
        }
#if DEBUGGING
        ScreenPutChar(drawno,TTcolorSet(1,7),scol-1,row) ;
#endif
    }
#endif /* _DOS */
    /*************************************************************************
     *
     * MS-WINDOWS
     *
     ************************************************************************/
#ifdef _WIN32
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    if (meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
    {
        meScheme scheme;
        meInt ll, ii, ccol;
        WORD  cc;

        ccol = 0 ;
        do {
            scheme = blkp->scheme ;
            cc = (WORD) TTschemeSet(scheme) ;

            /* Update the screen.
             * Maintain the frame store and copy the sting into the
             * frame store with the colour information */
            ii = blkp->column;
            ll = ii - ccol ;
            ConsoleDrawString (s1, cc, scol+ccol, row, ll);
            ccol = ii ;
            while(--ll >= 0)
            {
                *fssp++ = scheme ;
                *fstp++ = *s1++;
            }
            blkp++;
        } while(--noColChng) ;

        if (meStyleCmpBColor(meSchemeGetStyle(vp1->eolScheme),meSchemeGetStyle(scheme)))
        {
            ii = ncol ;
            vp1->eolScheme = scheme;
        }
        else
            ii = vp1->endp ;
        vp1->endp = ccol ;
        /* Update the screen for end of the line. Maintain the
         * frame store copy. */
        if(ccol < ii)
        {
            s1 = fstp ;
            ll = ii - ccol ;
            while(--ll >= 0)
            {
                *fssp++ = scheme;
                *fstp++ = ' ';
            }
            ll = ii - ccol ;
            ConsoleDrawString (s1, cc, scol+ccol, row, ll);
#if DEBUGGING
            ccol = ii ;
#endif
        }
#if DEBUGGING
        ConsoleDrawString (&drawno, cc, scol+ccol, row, 1);
#endif
/*        TTflush() ;*/
    }
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
    {
        meScheme scheme;
        meInt offset;                     /* Offset into the line */
        meInt len;                        /* Local line column */

        /* Iterate through the colour changes */
        len = 0;
        do {
            /* Set up the colour maps */
            offset = blkp->column - len;   /* Get width of coloured block */
            scheme = blkp->scheme;
            blkp++;                     /* Next colour pair */
            len += offset;              /* Increase line length */
            while (--offset >= 0)
            {
                *fssp++ = scheme;
                *fstp++ = *s1++;
            }
        } while (--noColChng > 0);

        /* Clear to the end of the line */
        if (meStyleCmpBColor(meSchemeGetStyle(vp1->eolScheme),meSchemeGetStyle(scheme)))
        {
            offset = ncol - len;
            vp1->eolScheme = scheme;
        }
        else if (vp1->endp < len)
            offset = 0;
        else
            offset = vp1->endp - len;
        vp1->endp = len;
        len += offset;

        if (offset != 0)
        {
            /* Line terminates before right margin. */
            /* Fill in the frame store */
            do
            {
                *fssp++ = scheme;
                *fstp++ = ' ';
            }
            while (--offset > 0);
        }
#if DEBUGGING
        fstp[-1] = drawno ;
#endif
        /* Invalidate the screen */
        if(flag & VFMAINL)
            /* in the main window just add the area */
            TTaddArea(row,scol,1,len) ;
        else
            /* other areas we want to set and apply */
            TTputs(row,scol,len) ;
    }
#endif /* _ME_WINDOW */
#endif /* _WIN32 */

    return meTRUE ;
}

/*
 * updateWindow() - update all the lines in a window on the virtual screen
 * The screen now consists of frameCur->depth array of meVideoLine's which point to the
 * actual meLine's (dummy ones if its a modeline) the flags indicate if the
 * meLine is mess or mode or main line and if its current
 */
static void
updateWindow(meWindow *wp)
{
    meBuffer *bp = wp->buffer;
    meVideoLine *vptr;           /* Pointer to the video block */
    meLine *lp;                  /* Line to update */
    int row, nrows;              /* physical screen line to update */
    meUByte force;

    force = (meUByte) (wp->updateFlags & (WFREDRAW|WFRESIZE)) ;
    /* Determine the video line position and determine the video block that
     * is being used. */
    row   = wp->frameRow ;
    vptr  = wp->video->lineArray + row ;  /* Video block */
    nrows = wp->textDepth ;

    /* Search down the lines, updating them */
    {
        int ii;
        ii = (wp->dotLineNo - wp->vertScroll) ;
        lp = wp->dotLine ;
        while(--ii >= 0)
            lp = meLineGetPrev(lp) ;
    }
#if MEOPT_COLOR
#if MEOPT_HILIGHT
    if(bp->hilight)
    {
        if(force)
        {
            vptr->hilno   = bp->hilight ;
            vptr->bracket = NULL ;
            wp->updateFlags |= WFLOOKBK ;
        }
        if(wp->updateFlags & WFLOOKBK)
        {
            if(meHilightGetFlags(hilights[bp->hilight]) & (HFLOOKBLN|HFLOOKBSCH))
                hilightLookBack(wp) ;
            wp->updateFlags &= ~WFLOOKBK ;
        }
    }
#endif
#endif
#ifdef _WIN32
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
        TTinitArea() ;
#endif /* _ME_WINDOW */
#endif /* _WIN32 */
    while(--nrows >= 0)
    {
        register meUByte update=force|(vptr->flag & VFCHNGD) ;

        /*---	 and update the virtual line */
        if(lp == wp->dotLine)
        {
            if(((vptr->flag & VFTPMSK) != (VFMAINL|VFCURRL)) ||
               (wp->updateFlags & WFDOT))
                update = 1 ;
            vptr->flag = (vptr->flag & ~VFTPMSK) | VFMAINL | VFCURRL ;
        }
        else
        {
            if((vptr->flag & VFTPMSK) != VFMAINL)
                update = 1 ;
            vptr->flag = (vptr->flag & ~VFTPMSK) | VFMAINL ;
        }
        if(force)
        {
            if(force & WFRESIZE)
                vptr->endp = wp->textWidth ;
            vptr->wind = wp ;
            vptr->line = lp ;
#if MEOPT_COLOR
#if MEOPT_HILIGHT
            vptr[1].hilno = bp->hilight ;
            vptr[1].bracket = NULL ;
#endif
#endif
        }
        else if((lp->flag & meLINE_CHANGED) ||
                (vptr->line != lp))
        {
            vptr->line = lp ;
            update = 1 ;
        }
        if(update)
            updateline(row,vptr,wp);
        row++ ;
        vptr++ ;
        if(lp == bp->baseLine)
            break ;
        lp=meLineGetNext(lp) ;
    }

    if(nrows > 0)
    {
        /* if we are at the end */
        meScheme scheme = vptr[-1].eolScheme ;
        meLineFlag flag ;
        /* store the baseLine flag and set to 0 as it may have a $line-scheme */
        flag = lp->flag ;
        lp->flag = 0 ;
        for( ; (--nrows >= 0) ; row++,vptr++)
        {
            vptr->line = lp ;
            if(force || (vptr->endp > 1) || (vptr->flag & (VFCHNGD|VFCURRL)) ||
               (vptr->eolScheme != scheme))
            {
                vptr->flag = VFMAINL ;
                vptr->wind = wp ;
                if(force & WFRESIZE)
                    vptr->endp = wp->textWidth ;
                updateline(row,vptr,wp);
            }
        }
        lp->flag = flag ;
    }
#ifdef _WIN32
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
        TTapplyArea() ;
#endif /* _ME_WINDOW */
#endif /* _WIN32 */
}

meUByte
assessModeLine(meUByte *ml)
{
    meUByte flags=WFMODE|WFRESIZE ;
    meUByte cc ;

    while((cc = *ml++) != '\0')
    {
        if(cc == '%')
        {
            switch((cc = *ml++))
            {
            case 'l':
                flags |= WFMOVEL ;
                break ;
            case 'c':
                /* The column often changes with the line,
                 * so add that too
                 */
                flags |= WFMOVEC|WFMOVEL ;
                break ;
            }
        }
    }
    return flags ;
}

/*
 * updateModeLine()
 * Redisplay the mode line for the window pointed to by the "wp". This is the
 * only routine that has any idea of how the modeline is formatted. You can
 * change the modeline format by hacking at this routine. Called by "update"
 * any time there is a dirty window.
 */
static void
updateModeLine(meWindow *wp)
{
    time_t          clock;		    /* Time in machine format. */
    struct tm	   *time_ptr;	            /* Pointer to time frame. */
    register meUByte *ml, *cp, *ss ;
    register meUByte  cc, lchar ;
    register meBuffer *bp ;
    register int    ii ;	            /* loop index */
    register int    lineLen ;               /* Max length of the line */

    /* See if there's anything to do first */
    bp = wp->buffer;
#if MEOPT_EXTENDED
    if((ml = bp->modeLineStr) != NULL)
    {
        if((wp->updateFlags & bp->modeLineFlags) == 0)
            return ;
    }
    else
#endif
    {
        if((wp->updateFlags & modeLineFlags) == 0)
            return ;
        ml = modeLineStr ;
    }

    lineLen = wp->textWidth ;              /* Max length of line. Only need to
                                             * evaluate this many characters */
    wp->modeLine->length = lineLen ;

    if (wp == frameCur->windowCur)                        /* mark the current buffer */
        lchar = windowChars [WCMLCWSEP];    /* Typically '=' */
    else
        lchar = windowChars [WCMLIWSEP];    /* Typically '-' */

    clock = time(0);	                    /* Get system time */
    time_ptr = (struct tm *) localtime (&clock);	/* Get time frame */
    cp = wp->modeLine->text ;
    while((lineLen > 0) && ((cc = *ml++) != '\0'))
    {
        if(cc == '%')
        {
            switch((cc = *ml++))
            {
            case 's':
                /* Horizontal split line character. Only displayed if split
                 * is enabled in the scroll bars */
                if (gsbarmode & WMSPLIT)
                {
                    *cp++ = windowChars [WCHSBSPLIT];
                    lineLen-- ;
                }
                break;
            case 'r':
                /* root user - unix only */
#ifdef _UNIX
                /*
                 * The idea of this nastiness is that the mode line is
                 * changed to give some visual indication when running as root.
                 */
                if(meUid == 0)
                    *cp++ = '#' ;
                else
#endif
                    *cp++ = lchar ;
                lineLen-- ;
                break ;

            case 'u':
                /* buffer changed */
                if(meModeTest(bp->mode,MDEDIT))        /* "*" if changed. */
                    *cp++ = windowChars [WCMLBCHNG];     /* Typically '*' */
                else if (meModeTest(bp->mode,MDVIEW))  /* "%" if view. */
                    *cp++ = windowChars [WCMLBVIEW];     /* Typically '%' */
                else
                    *cp++ = lchar ;
                lineLen--;
                break ;

            case 'e':
                /* add in the mode flags */
                for (ii = 0; ii < MDNUMMODES; ii++)
                    if(meModeTest(modeLineDraw,ii) &&
                       meModeTest(bp->mode,ii))
                    {
                        *cp++ = modeCode[ii] ;
                        if (--lineLen == 0)
                            break;
                    }
                break ;

            case 'k':
                if (kbdmode == meRECORD)           /* if playing macro */
                {
                    ss = (meUByte *) "REC" ;
                    goto model_copys ;
                }
                break ;

            case 'l':
                ss = meItoa(wp->dotLineNo+1);
                goto model_copys ;

            case 'n':
                ss = meItoa(wp->buffer->lineCount+1);
                goto model_copys ;

            case 'c':
                ss = meItoa(wp->dotOffset) ;
                goto model_copys ;

            case 'b':
                ss = bp->name ;
                goto model_copys ;

            case 'f':
                if((bp->name[0] == '*') || (bp->fileName == NULL))
                    break ;
                ss = bp->fileName ;
                if((ii=meStrlen(ss)) > lineLen)
                {
                    *cp++ = windowChars[WCDISPTXTLFT] ;
                    if(--lineLen == 0)
                        break ;
                    ss += ii-lineLen ;
                }
                goto model_copys ;

            case 'D':
                ss = meItoa(time_ptr->tm_mday);
                goto model_copys ;

            case 'M':
                ss = meItoa(time_ptr->tm_mon+1);
                goto model_copys ;

            case 'y':
                ss = meItoa(1900+time_ptr->tm_year);
                goto model_copys ;

            case 'Y':
                ss = meItoa(time_ptr->tm_year%100);
                goto model_2n_copy ;

            case 'h':
                ss = meItoa(time_ptr->tm_hour);
                goto model_2n_copy ;

            case 'm':
                ss = meItoa(time_ptr->tm_min);
model_2n_copy:
                if(ss[1] == '\0')
                {
                    *cp++ = '0' ;
                    if (--lineLen == 0)
                        break;
                }
model_copys:
                while((cc = *ss++) != '\0')
                {
                    *cp++ = cc ;
                    if (--lineLen == 0)
                        break;
                }
                break ;
            default:
                *cp++ = cc ;
                lineLen--;
            }
        }
        else
        {
            if(cc == windowChars [WCMLIWSEP])  /* Is it a '-' ?? */
                *cp++ = lchar ;
            else
                *cp++ = cc ;
            lineLen--;
        }
    }
    while(lineLen-- > 0)                       /* Pad to full width. */
        *cp++ = lchar ;

    {
        register meVideoLine *vptr ;           /* Pointer to the video block */

        /* Determine the video line position and determine the video block that
         * is being used. */
        ii = wp->frameRow + wp->textDepth ;
        vptr = wp->video->lineArray + ii ;     /* Video block */

        if(wp->updateFlags & WFRESIZE)
            vptr->endp = wp->textWidth ;
        if(frameCur->windowCur == wp)
            vptr->flag = VFMODEL|VFCURRL ;
        else
            vptr->flag = VFMODEL ;
        vptr->line = wp->modeLine ;
        updateline(ii,vptr,wp) ;
    }
}

/*	reframe: check to see if the cursor is on in the window
        and re-frame it if needed or wanted		*/

void
reframe(meWindow *wp)
{
    register long  ii ;

    /* See if the selection hilighting is enabled for the buffer */
    if ((selhilight.flags & SELHIL_ACTIVE) &&
        (selhilight.bp == wp->buffer))
        wp->updateFlags |= WFSELHIL;

#if MEOPT_IPIPES
    if(meModeTest(wp->buffer->mode,MDLOCK) &&
       (wp->dotLine->flag & meLINE_ANCHOR_AMARK))
    {
        meAnchor *ap=wp->buffer->anchorList ;

        /* Are we at the input line? */
        while((ap != NULL) && (ap->name != 'I'))
            ap = ap->next ;

        if((ap != NULL) &&
           (ap->line == wp->dotLine) &&
           (ap->offs == wp->dotOffset))
        {
            meIPipe *ipipe ;

            /* Yes we are at the right place, find the ipipe node */
            ipipe = ipipes ;
            while(ipipe->bp != wp->buffer)
                ipipe = ipipe->next ;

            if((ii = ipipe->curRow) >= wp->textDepth)
                ii = wp->textDepth-1 ;
            if((ii = wp->dotLineNo-ii) < 0)
                ii = 0 ;
            if((wp->updateFlags & WFFORCE) || (ii != wp->vertScroll))
            {
                wp->vertScroll = ii ;
                /* Force the scroll box to be updated if present. */
                wp->updateFlags |= WFSBOX|WFLOOKBK ;
            }
            return ;
        }
    }
#endif
    /* if not a requested reframe, check for a needed one */
    if(!(wp->updateFlags & WFFORCE))
    {
        ii = wp->dotLineNo - wp->vertScroll ;
        if((ii >= 0) && (ii < wp->textDepth))
            return ;
    }
    /* reaching here, we need a window refresh */
    ii = wp->windowRecenter ;

    /* how far back to reframe? */
    if(ii > 0)
    {	/* only one screen worth of lines max */
        if (--ii >= wp->textDepth)
            ii = wp->textDepth - 1;
    }
    else if(ii < 0)
    {	/* negative update???? */
        ii += wp->textDepth;
        if(ii < 0)
            ii = 0;
    }
    else
        ii = wp->textDepth / 2;

    ii = wp->dotLineNo-ii ;
    if(ii != wp->vertScroll)
    {
        if(ii <= 0)
            wp->vertScroll = 0 ;
        else
            wp->vertScroll = ii ;
        /* Force the scroll box and lookBack to be updated if present. */
        wp->updateFlags |= WFSBOX|WFLOOKBK ;
    }
}

#if MEOPT_SCROLL
/*
 * updateSrollBar
 * Update the scroll bar
 */
static void
updateScrollBar (meWindow *wp)
{
    int ii;                             /* index into split string */
    int col;                            /* The column */
    int row;                            /* Current row */
    int endrow;                         /* Terminating row */
    meScheme scheme;                    /* Region scheme */
    meUByte *wbase;                       /* Base window character */
    int  len;                           /* Length of bar */
    int  flipBox;                       /* Flip colours for the box */

    /* Has this window got a bar present ?? */
    if (!(wp->vertScrollBarMode & WMVBAR))
        return;                         /* No quit */

    /* Sort out the colours we should be using */
    scheme = sbarScheme + ((frameCur->windowCur == wp) ? meSCHEME_CURRENT:meSCHEME_NORMAL);

    /* Get the extents of the bar line */
    row = wp->frameRow;                 /* Start of row */
    col = wp->frameColumn + wp->textWidth;  /* Start column */

    if (wp->vertScrollBarMode & WMVWIDE)
    {
        len = 2;
        wbase = &windowChars[WCVSBSPLIT1];
    }
    else
    {
        len = 1;
        wbase = &windowChars[WCVSBSPLIT];
    }

    /* Flipbox is provided where we have not got a good reverse video character
     * for the scroll box (UNIX !!). When WMRVBOX is defined we may use a <SPACE>
     * character for the scroll box and paint in reverse video, this then makes a
     * better scroll box as required.
     *
     * Simply get the status and record the offset of the box, we will flip fcol
     * and bcol when we get there. If it is not defined then set to the end of
     * scroll bar + 1.
     */
    if (wp->vertScrollBarMode & WMRVBOX)
        flipBox = 1 << (WCVSBBOX-WCVSBSPLIT) ;
    else
        flipBox = 0 ;

#ifdef _WIN32
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
        /* Initialize the invalid screen area */
        TTinitArea() ;
#endif /* _ME_WINDOW */
#endif /* _WIN32 */

    /* Iterate down the length of the split line */
    for (ii = 0; ii <= (WCVSBML-WCVSBSPLIT); ii++, wbase += len)
    {
        scheme = (scheme & ~1) ^ (flipBox & 1) ;  /* Select component scheme */
        endrow = wp->vertScrollBarPos [ii] ;
        flipBox >>= 1;

        /* See if there is anything to do. */
        if ((wp->updateFlags & (WFSBSPLIT << ii)) == 0)
        {
            row = endrow ;
            continue ;
        }

        /* Perform the update */
#ifdef _WIN32
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
        if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
            /* Invalidate the screen */
            TTaddArea (row, col, endrow-row, len);
#endif /* _ME_WINDOW */
#endif /* _WIN32 */

        for (; row < endrow; row++)
        {
            /* Update the frame store with the character and scheme
             * information - this is nearly the same for all platforms
             */
            meUByte    *fstp ;      /* Frame store text pointer */
            meScheme *fssp ;      /* Frame store scheme pointer */

            fstp = frameCur->store[row].text + col ;
            fssp = frameCur->store[row].scheme + col ;

#ifdef _WIN32
            /****************************************************************
             * MS-Windows                                                   *
             ****************************************************************/

#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
            if (meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
            {
                WORD cc ;
                cc = (WORD) TTschemeSet(scheme) ;
                ConsoleDrawString(wbase,cc,col,row,len) ;
            }
#endif /* _ME_CONSOLE */
            fssp[0] = scheme;         /* Assign the scheme */
            fstp[0] = wbase[0];       /* Assign the text */
            if(len > 1)
            {
                fssp[1] = scheme;     /* Assign the scheme */
                fstp[1] = wbase[1];   /* Assign the text */
            }
#endif /* _WIN32 */

#ifdef _UNIX
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
            if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
            {
#ifdef _TCAP
                /************************************************************
                 * TERMCAP                                                  *
                 ************************************************************/
                meUByte cc;

                TCAPmove(row, col);    /* Go to correct place. */
                TCAPschemeSet(scheme) ;

                fssp[0] = scheme;      /* Assign the colour */
                cc = *wbase;
                fstp[0] = cc ;           /* Assign the text */
                TCAPputc(cc) ;
                if(len > 1)
                {
                    fssp[1] = scheme; /* Assign the colour */
                    cc = wbase[1] ;
                    fstp[1] = cc ;    /* Assign the text */
                    TCAPputc(cc) ;
                }
                TCAPschemeReset() ;
#endif /* _TCAP */
            }
#ifdef _ME_WINDOW
            else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
            {
#ifdef _XTERM
                /************************************************************
                 * X-WINDOWS                                                *
                 ************************************************************/
                meFrameXTermSetScheme(frameCur,scheme) ;

                if (meSystemCfg & meSYSTEM_FONTFIX)
                {
                    meUByte cc;
                    fssp[0] = scheme;  /* Assign the colour */
                    if(!((cc = *wbase) & 0xe0))
                        cc = ' ' ;    /* Assign the text */
                    fstp[0] = cc ;    /* Assign the text */

                    if(len > 1)
                    {
                        fssp[1] = scheme;  /* Assign the colour */
                        if(!((cc = wbase[1]) & 0xe0))
                            cc = ' ' ;    /* Assign the text */
                        fstp[1] = cc ;    /* Assign the text */
                    }
                    meFrameXTermDrawString(frameCur,colToClient(col),rowToClient(row),fstp,len);
                    if(!((cc = *wbase) & 0xe0))
                    {
                        meFrameXTermDrawSpecialChar(frameCur,colToClient(col),rowToClientTop(row),cc) ;
                        fstp[0] = cc ;   /* Assign the text */
                    }
                    if((len > 1) && !((cc = *wbase) & 0xe0))
                    {
                        meFrameXTermDrawSpecialChar(frameCur,colToClient(col+1),rowToClientTop(row),cc) ;
                        fstp[1] = cc ;   /* Assign the text */
                    }
                }
                else
                {
                    fssp[0] = scheme ;   /* Assign the colour */
                    fstp[0] = *wbase ;   /* Assign the text */
                    if(len > 1)
                    {
                        fssp[1] = scheme ;  /* Assign the colour */
                        fstp[1] = wbase[1] ;
                    }
                    meFrameXTermDrawString(frameCur,colToClient(col),rowToClient(row),wbase,len);
                }
#endif /* _XTERM */
            }
#endif /* _ME_WINDOW */
#endif /* _UNIX */

#ifdef _DOS
            /****************************************************************
             * MS-DOS                                                       *
             ****************************************************************/
            {
                meUByte ss ;
                int cc ;

                cc = TTschemeSet(scheme) ;

                fssp[0] = scheme ;  /* Assign the colour */
                ss = *wbase ;
                fstp[0] = ss ;      /* Assign the text */
                ScreenPutChar(ss,cc,col,row);
                if(len > 1)
                {
                    fssp[1] = scheme ;  /* Assign the colour */
                    ss = wbase[1] ;
                    fstp[1] = ss ;      /* Assign the text */
                    ScreenPutChar(ss,cc,col+1,row);
                }
            }
#endif /* _DOS */
        }
    }
#ifdef _WIN32
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
        if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
            /* Apply the invalid area */
            TTapplyArea () ;
#endif /* _ME_WINDOW */
#endif /* _WIN32 */
}
#endif /* MEOPT_SCROLL */

/*
 * pokeUpdate()
 * Update all of the areas of the screen that have been modified.
 * This is a simple bounding box test against the extents of the
 * window. If any part of the poke screen box lies within the window
 * then the lines in the virtual video structure must be marked as
 * changed in order to refresh them.
 */
static void
pokeUpdate (void)
{
    meWindow *wp;                         /* Window pointer */
    meVideoLine  *vp1;                        /* Video pointer */

    /* Fix up the message line - special case */
    if (frameCur->pokeRowMax == frameCur->depth)
    {
        vp1 = frameCur->video.lineArray + frameCur->depth;
        vp1->flag |= VFCHNGD;
        vp1->endp = frameCur->width;             /* Force update to EOL        */
    }

#if MEOPT_OSD
    /* Fix up the menu line - spacial case */
    if (frameCur->pokeRowMin < frameCur->menuDepth)
    {
        frameCur->video.lineArray->flag |= VFCHNGD;
        frameCur->video.lineArray->endp = frameCur->width;
    }
#endif
    /* Fix up the windows */
    for (wp = frameCur->windowList; wp != NULL; wp = wp->next)
    {
        int ii, jj;                      /* Offset/Length counter */

        /* Bounding box test agains the window extents */
        if (wp->frameColumn > frameCur->pokeColumnMax)
            continue;
        if (wp->frameRow > frameCur->pokeRowMax)
            continue;
        if ((wp->frameColumn + wp->width) <= frameCur->pokeColumnMin)
            continue;
        if ((wp->frameRow + wp->depth) <= frameCur->pokeRowMin)
            continue;

        /* Got here - therefore overlaps the window.
         * Determine vertical starting position on the frame.
         *
         * ii initially contains the startOffset */
        if (wp->frameRow > frameCur->pokeRowMin)
            ii = wp->frameRow;            /* Start at top of window */
        else
            ii = frameCur->pokeRowMin;             /* Start at top of poke */

        /* Get starting point og the Video frame pointer */
        vp1 = (meVideoLine *)(wp->video->lineArray) + ii;

        /* Determine the end position on the frame and work out
         * the number of lines that need to be updated.
         *
         * ii will contain the (endOffset - startOffset) i.e the
         * number of video rows to be updated */
        if ((jj=wp->frameRow+wp->depth-1) <= frameCur->pokeRowMax)
        {
            ii = jj - ii - 1 ;
            wp->updateFlags |= WFMODE ;
        }
        else
            ii = frameCur->pokeRowMax - ii;        /* End at poke max */

        /* Iterate down the video frame until we have exhausted our
         * row count. Note that we only mark the underlying video structure
         * as changed and NOT the window contents. This ensures that the
         * video is re-drawn without checking the window flags
         */
        do
        {
            vp1->flag |= VFCHNGD;       /* Mark video line as changed */
            vp1->endp = wp->textWidth; /* Force update to EOL        */
            vp1++;                      /* Next line                  */
        }
        while (ii-- > 0);               /* Until exhaused line count  */

        /* flag the window as needing attension */
        wp->updateFlags |= WFMAIN ;

#if MEOPT_SCROLL
        /* Fix up the vertical scroll bar if we have invaded it's space,
         * note that we do not modify the underlying separator bar in the
         * VVIDOE structure since it has not been re-sized. It is only
         * necessary to mark the scroll bar itself as changed */
        if ((frameCur->pokeColumnMax >= (wp->frameColumn + wp->textWidth)) &&
            (wp->vertScrollBarMode & WMVBAR))
            wp->updateFlags |= WFSBAR;
#endif /* MEOPT_SCROLL */
    }

    /* Reset the poke flags for next time. Set to their maximal limits
     * that will force a fail on the next poke screen operation */
    frameCur->pokeColumnMin = frameCur->width;
    frameCur->pokeColumnMax = 0;
    frameCur->pokeRowMin = frameCur->depth;
    frameCur->pokeRowMax = 0;
    frameCur->pokeFlag = 0;
}

/*
 * Make sure that the display is right. This is a three part process. First,
 * scan through all of the windows looking for dirty ones. Check the framing,
 * and refresh the screen. Second, make sure that "currow" and "curcol" are
 * correct for the current window. Third, make the virtual and physical
 * screens the same.
 */
/* screenUpdate:	user routine to force a screen update
		always finishes complete update		*/
int screenUpdateDisabledCount=0 ;

int
screenUpdate(int f, int n)
{
#if MEOPT_MWFRAME
    meFrame *fc ;
#endif
    meWindow *wp ;
    int force ;

#if DEBUGGING
    if(drawno++ == 'Z')
        drawno = 'A' ;
#endif
    if(n <= 0)
    {
        screenUpdateDisabledCount = n ;
        return meTRUE ;
    }
    if(n == 3)
    {
        /* only update the screen enough to get the $window vars correct */
        if(frameCur->windowCur->bufferLast != frameCur->bufferCur)
        {
            frameCur->windowCur->bufferLast = frameCur->bufferCur ;
            frameCur->windowCur->updateFlags |= WFMODE|WFREDRAW|WFMOVEL|WFSBOX ;
        }
        /* if top of window is the last line and there's more than
         * one, force refame and draw */
        if((frameCur->windowCur->vertScroll == frameCur->bufferCur->lineCount) && frameCur->windowCur->vertScroll)
            frameCur->windowCur->updateFlags |= WFFORCE ;

        /* if the window has changed, service it */
        if(frameCur->windowCur->updateFlags & (WFMOVEL|WFFORCE))
            reframe(frameCur->windowCur) ;	        /* check the framing */

        if(frameCur->windowCur->updateFlags & (WFREDRAW|WFRESIZE|WFSELHIL|WFSELDRAWN))
            shilightWindow(frameCur->windowCur);         /* Update selection hilight */

        /* check the horizontal scroll and cursor position */
        updCursor(frameCur->windowCur) ;
        return meTRUE ;
    }

    force = (n == 1) ;

#if MEOPT_MWFRAME
    fc = frameCur ;
    for(frameCur=frameList ; frameCur!=NULL ; frameCur=frameCur->next)
    {
        if(frameCur->flags & meFRAME_HIDDEN)
            continue ;
#else
    /* else required to keep the auto-indent right */
#endif

    if(force)
    {
#if MEOPT_OSD
        /* if screen is garbage, re-plot it all */
        if (frameCur->menuDepth > 0)
        {
            /* Mark menu as changed */
            frameCur->video.lineArray[0].flag = VFMENUL|VFCHNGD;
            frameCur->video.lineArray[0].endp = frameCur->width;
        }
#endif
        /* Mark message line as changed */
        frameCur->video.lineArray[frameCur->depth].flag |= VFCHNGD ;
        frameCur->video.lineArray[frameCur->depth].endp = frameCur->width;
        /* Reset the poke flags */
        frameCur->pokeColumnMin = frameCur->width;
        frameCur->pokeColumnMax = 0;
        frameCur->pokeRowMin = frameCur->depth;
        frameCur->pokeRowMax = 0;
        frameCur->pokeFlag = 0;
    }
    else
    {
#if MEOPT_OSD
        if((osdDisplayHd != NULL)
#if MEOPT_MWFRAME
           && (frameCur == fc)
#endif
           )
            /* else we must store any osd dialogs */
            osdStoreAll() ;
#endif
        /* if the screen has been poked then fix it */
        if (frameCur->pokeFlag)                      /* Screen been poked ??           */
            pokeUpdate();                   /* Yes - determine screen changes */
    }

    /* Does the title bar need updating? */
#ifdef _WINDOW
    if(force || (frameCur->bufferCur->name != frameCur->titleBufferName))
    {
        frameCur->titleBufferName = frameCur->bufferCur->name ;
        meFrameSetWindowTitle(frameCur,frameCur->bufferCur->name) ;
    }
#endif
#if MEOPT_OSD
    /* Does the menu line need updating? if so do it! */
    if((frameCur->menuDepth > 0) && (frameCur->video.lineArray[0].flag & VFCHNGD))
        osdMainMenuUpdate(force) ;
#endif
    /* update any windows that need refreshing */
    for (wp = frameCur->windowList; wp != NULL; wp = wp->next)
    {
        if(force)
        {
            wp->bufferLast = wp->buffer ;
            wp->updateFlags |= WFUPGAR ;
        }
        else if(wp->bufferLast != wp->buffer)
        {
            wp->bufferLast = wp->buffer ;
            wp->updateFlags |= WFMODE|WFREDRAW|WFMOVEL|WFSBOX ;
        }
        /* if top of window is the last line and there's more than
         * one, force refame and draw */
        if((wp->vertScroll == wp->buffer->lineCount) && wp->vertScroll)
            wp->updateFlags |= WFFORCE ;

        /* if the window has changed, service it */
        if(wp->updateFlags & (WFMOVEL|WFFORCE))
            reframe(wp) ;	        /* check the framing */

        if(wp->updateFlags & (WFREDRAW|WFRESIZE|WFSELHIL|WFSELDRAWN))
            shilightWindow(wp);         /* Update selection hilight */

        /* check the horizontal scroll and cursor position */
        if(wp == frameCur->windowCur)
            updCursor(wp) ;

        if(wp->updateFlags & ~(WFSELDRAWN|WFLOOKBK))
        {
            /* if the window has changed, service it */
#if MEOPT_SCROLL
            if(wp->updateFlags & WFSBOX)
                meWindowFixScrollBox(wp);     /* Fix the scroll bars */
#endif
            if(wp->updateFlags & ~(WFMODE|WFSBAR|WFLOOKBK))
                updateWindow(wp) ;          /* Update the window */
            updateModeLine(wp);	    /* Update mode line */
#if MEOPT_SCROLL
            if(wp->updateFlags & WFSBAR)
                updateScrollBar(wp);        /* Update scroll bar  */
#endif
            wp->updateFlags &= WFSELDRAWN ;
        }
        wp->windowRecenter = 0;
    }

    /* If forced then sort out the message-line as well */
    if(frameCur->video.lineArray[frameCur->depth].flag & VFCHNGD)
    {
        updateline(frameCur->depth,frameCur->video.lineArray+frameCur->depth,NULL) ;
        frameCur->video.lineArray[frameCur->depth].flag &= ~VFCHNGD ;
    }
#if MEOPT_OSD
    /* If we are in osd then update the osd menus */
    if((osdDisplayHd != NULL)
#if MEOPT_MWFRAME
        && (frameCur == fc)
#endif
       )
        osdRestoreAll(force) ;
#endif
    /* update the cursor and flush the buffers */
    resetCursor() ;

    TTflush();

#if MEOPT_MWFRAME
    }
    frameCur = fc ;
#else
    /* else required to keep the auto-indent right */
#endif

#if MEOPT_MWFRAME
    for(fc=frameList ; fc!=NULL ; fc=fc->next)
    {
        if(fc->flags & meFRAME_HIDDEN)
            continue ;
        wp = fc->windowList ;
#else
    wp = frameCur->windowList ;
#endif

    /* Now rest all the window line meLINE_CHANGED flags */
    do
    {
        meLine *flp, *blp ;
        meLineFlag flag ;
        int ii, jj ;

        ii = (wp->dotLineNo - wp->vertScroll) ;
        jj = (wp->textDepth - ii) ;
        blp = flp = wp->dotLine ;
        while(--ii >= 0)
        {
            blp = meLineGetPrev(blp) ;
            if((flag=blp->flag) & meLINE_CHANGED)
                blp->flag = (flag & ~meLINE_CHANGED) ;
        }
        while(--jj >= 0)
        {
            if((flag=flp->flag) & meLINE_CHANGED)
                flp->flag = (flag & ~meLINE_CHANGED) ;
            flp = meLineGetNext(flp) ;
        }
    } while((wp = wp->next) != NULL) ;

#if MEOPT_MWFRAME
    }
#else
    /* else required to keep the auto-indent right */
#endif

    return(meTRUE);
}

int
update(int flag)    /* force=meTRUE update past type ahead? */
{
#if MEOPT_CALLBACK
    register int index;
    meUInt arg ;
#endif

    if((alarmState & meALARM_PIPED) ||
       (!(flag & 0x01) && ((kbdmode == mePLAY) || clexec || TTahead())))
        return meTRUE ;

    if(screenUpdateDisabledCount)
    {
        screenUpdateDisabledCount++ ;
        return meTRUE ;
    }

#if MEOPT_CALLBACK
    if((index = decode_key(ME_SPECIAL|SKEY_redraw,&arg)) >= 0)
        execFuncHidden(ME_SPECIAL|SKEY_redraw,index,0x80000002-sgarbf) ;
    else
#endif
        screenUpdate(1,2-sgarbf) ;
    /* reset garbled status */
    sgarbf = meFALSE ;

    return meTRUE ;
}

/*
 * pokeScreen.
 * Poke some text on the screen.
 *
 * flags are a bit pattern with the following meanings
 *
 * 0x01 POKE_NOMARK - Dont remember (or mark) the poke so that at the next
 *                    update this may not be erased (depending on what upates
 *                    are required.
 * 0x02 POKE_NOFLUSH- Don't flush the output terminal, great for splatting a
 *                    lot of pokes up at once (only the last one calls a flush)
 *                    This has limited success on Xterms as they seem to flush
 *                    anyway.
 * 0x04 POKE_COLORS - If not set then fbuf and bbuf point to a single fore and
 *                    back-ground color to be used for the whole poke. If set
 *                    then a fore & back color is given per poke char.
 */
/* poking can draw onto of the cursor and erase it, this is often an unwanted
 * side effect (particularly with the matching fence drawing).
 * Win32 Window and UNIX termcap do not suffer from this */
#ifdef _DOS
#define _ME_POKE_REDRAW_CURSOR
#endif
#if (defined _UNIX) && (defined _ME_WINDOW)
#ifdef _ME_CONSOLE
#define _ME_POKE_REDRAW_CURSOR !(meSystemCfg & meSYSTEM_CONSOLE) &&
#else
#define _ME_POKE_REDRAW_CURSOR
#endif /* _ME_CONSOLE */
#endif /* _UNIX */
#if (defined _WIN32) && (defined _ME_CONSOLE)
#ifdef _ME_WINDOW
#define _ME_POKE_REDRAW_CURSOR  (meSystemCfg & meSYSTEM_CONSOLE) &&
#else
#define _ME_POKE_REDRAW_CURSOR
#endif /* _ME_WINDOW */
#endif /* _WIN32 */
void
pokeScreen(int flags, int row, int col, meUByte *scheme,
           meUByte *str)
{
    meUByte  cc, *ss ;
    meScheme *fssp;               /* Frame store scheme pointer */
    int len, schm, off ;
#ifdef _ME_POKE_REDRAW_CURSOR
    int drawCursor ;
#endif

    /* Normalise to the screen rows */
    if((row < 0) || (row > frameCur->depth))     /* Off the screen in Y ?? */
        return ;                        /* Yes - quit */

    /* Normalise to the screen columns. Chop the string up if necessary
     * and change any bad chars to '.'s
     */
    len = 0 ;
    ss = str ;
    while((cc = *ss) != '\0')
    {
        len++ ;
        if(!isPokable(cc))
            *ss = '.' ;
        ss++ ;
    }
    if(col < 0)
    {
        if((len += col) <= 0)
            return ;
        str -= col ;
#if MEOPT_EXTENDED
        if(flags & POKE_COLORS)
            scheme -= col ;
#endif
        col = 0 ;
    }
    else if(col >= frameCur->width)
        return ;
    if((col+len) >= frameCur->width)
        len = frameCur->width - col ;

    /* Sort out the poke flags. Just keep the minimal and maximal extents
     * of the change. We will sort out what needs to be updated on the
     * next update() call. */
    if((flags & POKE_NOMARK) == 0)
    {
        frameCur->pokeFlag |= 1;
        if (row < frameCur->pokeRowMin)
            frameCur->pokeRowMin = row;
        if (row > frameCur->pokeRowMax)
            frameCur->pokeRowMax = row;
        if (col < frameCur->pokeColumnMin)
            frameCur->pokeColumnMin = col;
        if ((col+len) > frameCur->pokeColumnMax)
            frameCur->pokeColumnMax = col+len;
    }

    /* Write the text to the frame store. Note that the colour still
     * needs to be updated. */
    memcpy(frameCur->store[row].text+col,str,len) ;      /* Write text in */
    fssp = frameCur->store[row].scheme + col ;           /* Get the scheme pointer */
    off  = (flags >> 4) & 0x07 ;

#ifdef _ME_POKE_REDRAW_CURSOR
    /* Must redraw the cursor if we have zapped it */
    drawCursor = ((cursorState >= 0) && blinkState &&
                  _ME_POKE_REDRAW_CURSOR
                  (row == frameCur->cursorRow) &&
                  (col <= frameCur->cursorColumn) && ((col+len) > frameCur->cursorColumn)) ;
#endif /* _ME_POKE_REDRAW_CURSOR */

#ifdef _WIN32
#ifdef _ME_WINDOW
#ifdef _ME_CONSOLE
    if (!(meSystemCfg & meSYSTEM_CONSOLE))
#endif /* _ME_CONSOLE */
        TTputs(row,col,len) ;
#endif /* _ME_WINDOW */
#endif /* _WIN32 */

#if MEOPT_EXTENDED
    /* Write to the screen */
    if(flags & POKE_COLORS)
    {
#ifdef _UNIX

#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
        if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
        {
#ifdef _TCAP
            /****************************************************************
             * TERMCAP                                                      *
             ****************************************************************/
            TCAPmove(row, col);	/* Go to correct place. */
            while(len--)
            {
                schm = *scheme++ ;
                if((schm == meCHAR_LEADER) && ((schm = *scheme++) == meCHAR_TRAIL_NULL))
                    schm = 0 ;
                else
                    schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
                schm += off ;
                *fssp++ = schm ;
                TCAPschemeSet(schm) ;
                cc = *str++ ;
                TCAPputc(cc) ;

            }
            TCAPschemeReset() ;
#endif /* _TCAP */
        }
#ifdef _ME_WINDOW
        else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
        {
#ifdef _XTERM
            /****************************************************************
             * X-WINDOWS                                                    *
             ****************************************************************/
            col = colToClient(col) ;
            row = rowToClient(row) ;

            if (meSystemCfg & meSYSTEM_FONTFIX)
            {
                meUByte cc ;
                str-- ;
                while(len--)
                {
                    schm = *scheme++ ;
                    if((schm == meCHAR_LEADER) && ((schm = *scheme++) == meCHAR_TRAIL_NULL))
                        schm = 0 ;
                    else
                        schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
                    schm += off ;
                    /* Update the frame store colour */
                    *fssp++ = schm ;
                    meFrameXTermSetScheme(frameCur,schm) ;
                    if((cc=*++str) & 0xe0)
                        meFrameXTermDrawString(frameCur,col,row,str,1);
                    else
                    {
                        static char ss[1]={' '} ;
                        meFrameXTermDrawString(frameCur,col,row,ss,1);
                        meFrameXTermDrawSpecialChar(frameCur,col,row-mecm.ascent,cc) ;
                    }
                    col += mecm.fwidth ;
                }
            }
            else
            {
                while(len--)
                {
                    schm = *scheme++ ;
                    if((schm == meCHAR_LEADER) && ((schm = *scheme++) == meCHAR_TRAIL_NULL))
                        schm = 0 ;
                    else
                        schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
                    schm += off ;
                    /* Update the frame store colour */
                    *fssp++ = schm ;
                    meFrameXTermSetScheme(frameCur,schm) ;
                    meFrameXTermDrawString(frameCur,col,row,str,1);
                    str++ ;
                    col += mecm.fwidth ;
                }
            }
#endif /* _XTERM */
        }
#endif /* _ME_WINDOW */

#endif /* _UNIX */

#ifdef _DOS
        /********************************************************************
         * MS-DOS                                                           *
         ********************************************************************/
        {
            while(len--)
            {
                schm = *scheme++ ;
                if((schm == meCHAR_LEADER) && ((schm = *scheme++) == meCHAR_TRAIL_NULL))
                    schm = 0 ;
                else
                    schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
                schm += off ;
                *fssp++ = schm ;
                cc = TTschemeSet(schm) ;
                ScreenPutChar(*str++, cc, col++, row);
            }
        }
#endif /* _DOS */

#ifdef _WIN32
        /********************************************************************
         * MS-WINDOWS                                                       *
         ********************************************************************/
        {
            while(len--)
            {
                schm = *scheme++ ;
                if((schm == meCHAR_LEADER) && ((schm = *scheme++) == meCHAR_TRAIL_NULL))
                    schm = 0 ;
                else
                    schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
                schm += off ;
                /* Update the frame store colour */
                *fssp++ = schm ;
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
                if (meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
                {
                    WORD att ;
                    att = (WORD) TTschemeSet(schm) ;
                    ConsoleDrawString(str++, att, col++, row, 1);
                }
#endif /* _ME_CONSOLE */
            }
        }
#endif /* _WIN32 */
    }
    else
#endif
    {
        schm = *scheme ;
        if((schm == meCHAR_LEADER) && ((schm = *scheme++) == meCHAR_TRAIL_NULL))
            schm = 0 ;
        else
            schm = meSchemeCheck(schm)*meSCHEME_STYLES ;
        schm += off ;

#ifdef _UNIX

#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
        if(meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
        {
#ifdef _TCAP
            /****************************************************************
             * TERMCAP                                                      *
             ****************************************************************/
            TCAPmove(row, col);	/* Go to correct place. */
            TCAPschemeSet(schm) ;

            while(len--)
            {
                cc = *str++ ;
                TCAPputc(cc) ;
                /* Update the frame store colour */
                *fssp++ = schm ;
            }
            TCAPschemeReset() ;
            /* restore cursor position */
            if((cursorState >= 0) && blinkState)
                TTshowCur() ;
            else
                TThideCur() ;
#endif /* _TCAP */
        }
#ifdef _ME_WINDOW
        else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
        {
#ifdef _XTERM
            /****************************************************************
             * X-WINDOWS                                                    *
             ****************************************************************/
            meFrameXTermSetScheme(frameCur,schm) ;
            if (meSystemCfg & meSYSTEM_FONTFIX)
            {
                meUByte cc, *sfstp, *fstp ;
                int ii, spFlag=0 ;

                ii = len ;
                sfstp = fstp = &(frameCur->store[row].text[col]) ;
                row = rowToClient(row) ;
                while (--len >= 0)
                {
                    *fssp++ = schm ;
                    if(((cc=*fstp++) & 0xe0) == 0)
                    {
                        spFlag++ ;
                        fstp[-1] = ' ' ;
                    }
                }
                meFrameXTermDrawString(frameCur,colToClient(col),row,sfstp,ii);
                ii = 0 ;
                while(--spFlag >= 0)
                {
                    while (((cc=str[ii]) & 0xe0) != 0)
                        ii++ ;
                    sfstp[ii] = cc ;
                    meFrameXTermDrawSpecialChar(frameCur,colToClient(col+ii),row-mecm.ascent,cc) ;
                    ii++ ;
                }
            }
            else
            {
                meFrameXTermDrawString(frameCur,colToClient(col),rowToClient(row),str,len);
                /* Update the frame store colour */
                while (--len >= 0)
                    *fssp++ = schm ;
            }
#endif /* _XTERM */
        }
#endif /* _ME_WINDOW */
#endif /* _UNIX */

#ifdef _DOS
        /********************************************************************
         * MS-DOS                                                           *
         ********************************************************************/
        {
            cc = TTschemeSet(schm) ;
            while(len--)
            {
                ScreenPutChar(*str++, cc, col++, row);
                /* Update the frame store colour */
                *fssp++ = schm;
            }
        }
#endif /* _DOS */

#ifdef _WIN32
        /********************************************************************
         * MS-WINDOWS                                                       *
         ********************************************************************/
        {
#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
            if (meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
            {
                WORD att ;
                att = (WORD) TTschemeSet(schm) ;
                ConsoleDrawString (str, att, col, row, len);
            }
#endif /* _ME_CONSOLE */
            /* Update the frame store colours */
            while (--len >= 0)
                *fssp++ = schm ;
        }
#endif /* _WIN32 */

    }

#ifdef _ME_POKE_REDRAW_CURSOR
    if(drawCursor)
        TTshowCur() ;
#endif

    if((flags & POKE_NOFLUSH) == 0)
        TTflush() ;                         /* Force update of screen */
}

#if MEOPT_EXTENDED
/*
 * screenPoke.
 * The macro interface to the pokeScreen function. This accepts a
 * numeric argument which is a bit map of the form passed into pokeScreen.
 * As the default argument is 1 this will set just the POKE_NOMARK flag
 * which means that the poke will be visible after an ecs x invocation of
 * screen-poke.
 */
int
screenPoke(int f, int n)
{
    int row, col ;
    meUByte fbuf[meBUF_SIZE_MAX] ;
    meUByte sbuf[meBUF_SIZE_MAX] ;

    if((meGetString((meUByte *)"row",0,0,fbuf,meBUF_SIZE_MAX) <= 0) ||
       ((row = meAtoi(fbuf)),(meGetString((meUByte *)"col",0,0,fbuf,meBUF_SIZE_MAX) <= 0)) ||
       ((col = meAtoi(fbuf)),(meGetString((meUByte *)"scheme",0,0,fbuf,meBUF_SIZE_MAX) <= 0)) ||
       (meGetString((meUByte *)"string",0,0,sbuf,meBUF_SIZE_MAX) <= 0))
        return meFALSE ;
    if((n & POKE_COLORS) == 0)
        fbuf[0] = (meUByte) meAtoi(fbuf) ;
    pokeScreen(n,row,col,fbuf,sbuf) ;
    return meTRUE ;
}
#endif

/*
 * Write out a long integer, in the specified radix. Update the physical cursor
 * position.
 *
 * As usual, if the last argument is not NULL, write characters to it and
 * return the number of characters written.
 */
static	int
mlputi(long i, int r, meUByte *buf)
{
    meUByte	    temp_buf [22];		/* Temporary buffer */
    register meUByte *temp = temp_buf;	/* Temporaty buffer pointer */
    register int    count=0, neg=0 ;

    *temp++ = '\0' ;
    if (i < 0)
    {	/* Number is negetive */
        i = -i;
        neg = 1 ;
    }
    do
    {
        count++;
        *temp++ = hexdigits [i%r];
    } while ((i = i/r) > 0) ;

    if(neg)
    {
        count++ ;
        *temp++ = '-';
    }

    /*---	Write the information out */

    while ((*buf++ = *--temp) != 0)
        ;
    return(count);
}

/*
 * Write a message into the message line. Keep track of the physical cursor
 * position. A small class of printf like format items is handled. Assumes the
 * stack grows down; this assumption is made by the "++" in the argument scan
 * loop. Set the "message line" flag meTRUE.
 */
#ifdef _STDARG
int
mlwrite(int flags, meUByte *fmt, ...)
#else
int
mlwrite(int flags, meUByte *fmt, int arg)
#endif
{
#ifdef _STDARG
    va_list	 ap;
#else
    register char *ap;
#endif
    register meUByte c;
    register meVideoLine *vp1;
    register int offset ;	/* offset into ???		*/
    meUByte  mlw[meBUF_SIZE_MAX];		/* what we want to be there	*/
    meUByte *mlwant;		/* what we want to be there	*/
    meUByte *s1 ;

    /* If this is a termination request then exit immediately as the output
     * terminal window may have been destroyed so there is no canvas onto
     * which the message may be written. */
    if (alarmState & meALARM_DIE)
        goto mlwrite_exit ;
    /* If the system is not initialised then the output window does not
     * exist, attempt to write on the console. */
    if((alarmState & meALARM_INITIALIZED) == 0)
    {
        /* an mlwrite at this stage is fatal - usually a malloc failure,
         * print message and exit */
        mePrintMessage(fmt) ;
        meExit(1) ;
    }
    if((alarmState & meALARM_PIPED_QUIET) && ((flags & MWSTDALLWRT) == 0))
        goto mlwrite_exit ;
    if(clexec && (flags & MWCLEXEC))
    {
        meRegister *rr ;

        if(!(flags & MWABORT))
            goto mlwrite_exit ;

        rr = meRegCurr ;
        do {
            if(rr->force)
                goto mlwrite_exit ;
            rr = rr->prev ;
        } while(rr != meRegHead) ;
    }

    if(frameCur->mlStatus & MLSTATUS_KEEP)
    {
        meUByte *_ss, *_dd ;
        _ss = frameCur->mlLine->text ;
        _dd = frameCur->mlLineStore ;
        while((*_dd++ = *_ss++))
            ;
        frameCur->mlColumnStore = frameCur->mlColumn ;
        frameCur->mlStatus = (frameCur->mlStatus & ~MLSTATUS_KEEP) | MLSTATUS_RESTORE ;
    }
    else
        frameCur->mlStatus |= MLSTATUS_CLEAR ;

    if(flags & MWSPEC)
    {
        mlwant = (meUByte *) fmt ;
        offset = meStrlen(mlwant) ;
    }
    else
    {
#ifdef _STDARG
        va_start(ap,fmt);
#else
        ap = (char *) &arg;
#endif
        mlwant = mlw ;
        while ((c = *fmt++) != '\0')
        {
            if(c != '%')
                *mlwant++ = c;
            else
            {
                c = *fmt++;
                switch (c)
                {
                case 'c':
#ifdef _STDARG
                    *mlwant++ = va_arg(ap, int) ;
#else
                    *mlwant++ = *(meUByte *)ap ;
                    ap += sizeof(int);
#endif
                    break;

                case 'd':
#ifdef _STDARG
                    mlwant += mlputi((long)(va_arg(ap, int)),10,mlwant);
#else
                    mlwant += mlputi((long)(*(int *)ap), 10,mlwant);
                    ap += sizeof(int);
#endif
                    break;

                case 'o':
#ifdef _STDARG
                    mlwant += mlputi((long)(va_arg(ap, int)),8,mlwant);
#else
                    mlwant += mlputi((long)(*(int *)ap),8,mlwant);
                    ap += sizeof(int);
#endif
                    break;

                case 'x':
#ifdef _STDARG
                    mlwant += mlputi((long)(va_arg(ap, int)),16,mlwant);
#else
                    mlwant += mlputi((long)(*(int *)ap),16,mlwant);
                    ap += sizeof(int);
#endif
                    break;

                case 'D':
#ifdef _STDARG
                    mlwant += mlputi(va_arg(ap, long), 10,mlwant) ;
#else
                    mlwant += mlputi(*(long *)ap,10,mlwant) ;
                    ap += sizeof(long);
#endif
                    break;

                case 's':
#ifdef _STDARG
                    s1 = va_arg(ap, meUByte *) ;
#else
                    s1 = *(meUByte **)ap ;
                    ap += sizeof(meUByte *);
#endif
                    /* catch a string which is too long */
                    offset = meStrlen(s1) ;
                    if(offset > frameCur->width)
                    {
                        mlwant = mlw ;
                        s1 += offset - frameCur->width ;
                        offset = frameCur->width ;
                    }
                    meStrcpy(mlwant,s1) ;
                    mlwant += offset ;
                    break;

                default:
                    *mlwant++ = c;
                    break;
                } /* switch */
            } /* if */
        } /* while */
#ifdef _STDARG
        va_end (ap);
#endif
        *mlwant= '\0' ;
        offset = mlwant - mlw ;
        mlwant = mlw ;
    }
    if((alarmState & meALARM_PIPED) || (flags & MWSTDALLWRT))
    {
#ifdef _WIN32
        HANDLE fp ;
        DWORD written ;
        if(flags & MWSTDOUTWRT)
            fp = GetStdHandle(STD_OUTPUT_HANDLE) ;
        else
            fp = GetStdHandle(STD_ERROR_HANDLE) ;
        WriteFile(fp,mlwant,offset,&written,NULL) ;
        WriteFile(fp,"\n",1,&written,NULL) ;
#else
        fprintf((flags & MWSTDOUTWRT) ? stdout:stderr,"%s\n",mlwant) ;
#endif
        goto mlwrite_exit ;
    }
    vp1 = frameCur->video.lineArray + frameCur->depth ;
    /* JDG End */
    /* SWP - changed the mlwrite when the string is to long for the ml line
     * so that it always displays the last part. This is for two reasons:
     * 1) If this is an MWSPEC then it must not touch the string, so can't
     *    null terminate.
     * 2) The latter part is usually the more important, i.e. where the
     *    prompt is
     */
    if(offset > frameCur->width)
    {
        mlwant += offset-frameCur->width ;
        offset = frameCur->width ;
        if(!(flags & MWUSEMLCOL))
            frameCur->mlColumn = frameCur->width-1 ;
    }
    else if(!(flags & MWUSEMLCOL))
        frameCur->mlColumn = offset ;
    s1 = frameCur->mlLine->text ;
    while((*s1++ = *mlwant++))
        ;
    frameCur->mlLine->length=offset ;
    /* JDG 02/03/97 - This line was removed, however when it is then the
     * bottom line is not reset properly.
     * SWP - I've commented it out again - this only alleviates another bug
     * which should be/should have been fixed.
     */
    /* vp1->endp = frameCur->width ;*/

    updateline(frameCur->depth,vp1,NULL);
    mlResetCursor() ;
    TTflush();

    if(flags & MWABORT)
        TTbell() ;
    if(flags & MWPAUSE)
    {
        /* Change the value of frameCur->mlStatus to MLSTATUS_KEEP cos we want to keep
         * the string for the length of the sleep
         */
        meUByte oldMlStatus = frameCur->mlStatus ;
        frameCur->mlStatus = MLSTATUS_KEEP ;
        TTsleep(pauseTime,1,NULL)  ;
        frameCur->mlStatus = oldMlStatus ;
    }
    else if((flags & MWWAIT) || (clexec && (flags & MWCLWAIT)))
    {
        /* Change the value of frameCur->mlStatus to MLSTATUS_KEEP cos we want to keep
         * the string till we get a key
         */
        meUByte scheme=(globScheme/meSCHEME_STYLES), oldMlStatus = frameCur->mlStatus, oldkbdmode=kbdmode ;
        pokeScreen(POKE_NOMARK+0x10,frameCur->depth,frameCur->width-9,&scheme,
                   (meUByte *) "<ANY KEY>") ;
        vp1->endp = frameCur->width ;
        frameCur->mlStatus = MLSTATUS_KEEP ;
        /* avoid any recursive call to an idle macro */
        kbdmode = meSTOP ;
        TTgetc() ;
        kbdmode = oldkbdmode ;
        frameCur->mlStatus = oldMlStatus ;
        mlerase(0) ;
    }
    if(!(flags & MWCURSOR))
        resetCursor();
    TTflush();

mlwrite_exit:
    if(!(flags & MWABORT))
        return meTRUE ;

    return meABORT ;
}

/* mlerase relegated to here due to its use of mlwrite */
/*
 * Erase the message line. This is a special routine because the message line
 * is not considered to be part of the virtual screen. It always works
 * immediately; the terminal buffer is flushed via a call to the flusher.
 */
void
mlerase(int flag)
{
    if(clexec && (flag & MWCLEXEC))
        return ;
    if((frameCur->mlStatus & MLSTATUS_RESTORE) && !(flag & MWERASE))
    {
        frameCur->mlColumn = frameCur->mlColumnStore ;
        mlwrite(((frameCur->mlStatus & MLSTATUS_POSML) ? MWSPEC|MWUSEMLCOL|MWCURSOR:MWSPEC),frameCur->mlLineStore) ;
        frameCur->mlStatus = (frameCur->mlStatus & ~MLSTATUS_RESTORE) | MLSTATUS_KEEP ;
    }
    else if(!(frameCur->mlStatus & MLSTATUS_KEEP))
    {
        mlwrite(MWSPEC|(flag&MWCURSOR),(meUByte *)"") ;
        frameCur->mlStatus &= ~MLSTATUS_CLEAR ;
    }
}

#if MEOPT_COLOR
/*
 * addColor
 * Add a new colour palett entry, or modify existing colour
 * palette entry.
 *
 * add-colour <index> <red> <green> <blue>
 *
 * Modify the colour  palette  values. The default is black (0) and white (1).
 * Any number of colours may be specified. Colors are specified as rgb values
 * in the range 0-255 where 0 is the black level; 255 is bright.
 *
 * The  <index>  need not  necesserily  commence  from zero and may exceed 256
 * entries. The support  offered by the system is dependent  upon the graphics
 * sub-system.
 */
int
addColor(int f, int n)
{
    meColor index ;
    meUByte r, g, b ;
    meUByte buff[meBUF_SIZE_MAX] ;

    if((meGetString((meUByte *)"Index",0,0,buff,meBUF_SIZE_MAX) == meABORT) ||
       ((index = (meColor) meAtoi(buff)) == meCOLOR_INVALID) ||
       (meGetString((meUByte *)"Red",0,0,buff,meBUF_SIZE_MAX) == meABORT) ||
       ((r = (meUByte) meAtoi(buff)),
        (meGetString((meUByte *)"Green",0,0,buff,meBUF_SIZE_MAX) == meABORT)) ||
       ((g = (meUByte) meAtoi(buff)),
        (meGetString((meUByte *)"Blue",0,0,buff,meBUF_SIZE_MAX) == meABORT)))
        return meFALSE ;
    b = (meUByte) meAtoi(buff) ;

    n = TTaddColor(index,r,g,b) ;
    if(index == (meColor) meStyleGetBColor(meSchemeGetStyle(globScheme)))
        TTsetBgcol() ;
    return n ;
}

/*
 * addColorScheme
 * Get the hilight colour definition.
 */
int
addColorScheme(int f, int n)
{
    /* Brought out as add-font also uses the same prompts */
    static meUByte *schmPromptM[4] = {(meUByte *)"Normal",(meUByte *)"Current",(meUByte *)"Select",(meUByte *)"Cur-Sel" } ;
    static meUByte *schmPromptP[4] = {(meUByte *)" fcol",(meUByte *)" bcol",(meUByte *)" nfont",(meUByte *)"rfont" } ;
    meStyle *hcolors ;
    meStyle *dcolors ;
    meColor  scheme[4][4] ;
    meUByte  fcol, bcol, font ;
    meUByte  prompt[20] ;
    meUByte  buf[meBUF_SIZE_MAX] ;
    int index, ii, jj, col ;

    /* Get the hilight colour index */
    if ((meGetString((meUByte *)"Color scheme index",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
        ((index = meAtoi(buf)) < 0))
        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Invalid scheme index number");

    /* See if we are replicating a color and validate the existing index */
    if ((f) && ((n > styleTableSize) || (n < 0)))
        return mlwrite (MWABORT|MWPAUSE,(meUByte *)"Cannot duplicate undefined scheme %d.", n);
    else
        dcolors = styleTable + (n*meSCHEME_STYLES);

    /* Allocate a new table if required */
    if (index >= styleTableSize)
    {
        /* Allocate a new table */
        if((styleTable = meRealloc(styleTable,(index+1)*meSCHEME_STYLES*sizeof(meStyle))) == NULL)
            return mlwrite (MWABORT|MWPAUSE,(meUByte *)"Cannot allocate scheme table");
        for(ii=styleTableSize ; ii<=index ; ii++)
            memcpy(styleTable+(ii*meSCHEME_STYLES),defaultScheme,meSCHEME_STYLES*sizeof(meStyle)) ;
        styleTableSize = index+1 ;
    }
    hcolors = styleTable+(index*meSCHEME_STYLES) ;

    if (f)
    {
        memcpy(hcolors,dcolors,meSCHEME_STYLES*sizeof(meStyle));
        return meTRUE ;
    }

    /* Get the parameters from the user. */
    for (ii = 0; ii < 4; ii++)
    {
        for (jj = 0; jj < 2; jj++)
        {
            meStrcpy(prompt,schmPromptM[ii]) ;
            meStrcat(prompt,schmPromptP[jj]) ;
            if((meGetString(prompt,0,0,buf,meBUF_SIZE_MAX) <= 0) ||
               ((col=meAtoi(buf)) < 0) || (col >= noColors))
                return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Invalid scheme entry");
            scheme[ii][jj] = col;
        }
        scheme[ii][2] = 0 ;
        scheme[ii][3] = meFONT_REVERSE ;
    }
    for (ii = 0; ii < 4; ii++)
    {
        for (jj = 2; jj < 4; jj++)
        {
            meStrcpy(prompt,schmPromptM[ii]) ;
            meStrcat(prompt,schmPromptP[jj]) ;
            if(meGetString(prompt,MLEXECNOUSER,0,buf,meBUF_SIZE_MAX) <= 0)
                break;
            scheme[ii][jj] = ((meUByte) meAtoi(buf)) & meFONT_MASK ;
        }
    }
    /* now copy the scheme into the table */
    for (ii = 0; ii < meSCHEME_STYLES; ii++)
    {
        font = scheme[ii>>1][2+(ii&0x1)] ;
        if(font & meFONT_REVERSE)
        {
            fcol = scheme[ii>>1][1] ;
            bcol = scheme[ii>>1][0] ;
        }
        else
        {
            fcol = scheme[ii>>1][0] ;
            bcol = scheme[ii>>1][1] ;
        }
        if(!(meSystemCfg & meSYSTEM_FONTS))
            /* fonts not being used */
            font = 0 ;
        /* if color is used then reverse font is done using color
         * so remove this font. If not using color then set the
         * color to the defaults */
        if(meSystemCfg &  (meSYSTEM_RGBCOLOR|meSYSTEM_ANSICOLOR|meSYSTEM_XANSICOLOR))
            font &= ~meFONT_REVERSE ;
        else
        {
            fcol = meCOLOR_FDEFAULT ;
            bcol = meCOLOR_BDEFAULT ;
        }
        meStyleSet(hcolors[ii],fcol,bcol,font) ;
    }
    if(index == globScheme)
        TTsetBgcol() ;
    return meTRUE;
}
#endif

/************************** VIRTUAL meVideoLine INTERFACES **************************
 *
 * meVideoDetach
 * Detach the window from the virtual video block. If the virtual video
 * block has no more windows attached then destruct the block itself.
 */

void
meVideoDetach (meWindow *wp)
{
    meVideo *vvptr;
    meWindow *twp;

    vvptr = wp->video;               /* Get windows virtual video block */
    meAssert (vvptr != NULL);

    if ((twp = vvptr->window) == wp)    /* First window in the chain ?? */
    {
        vvptr->window = wp->videoNext;   /* Detach from the head */
        wp->video = NULL;
    }
    else                                /* Not first in chain - chase block */
    {
        while (twp != NULL)
        {
            if (twp->videoNext == wp)
            {
                twp->videoNext = wp->videoNext;
                wp->video = NULL;
                break;
            }
            twp = twp->videoNext;
        }
    }

    meAssert (twp != NULL);

    /* Clean up the Virtual video frame if it is empty. Note
     * that vvptr == frameCur->video is handled correctly and is not
     * deleted. */
    if (vvptr->window == NULL)
    {
        meVideo *vv;

        for (vv = &frameCur->video; vv != NULL; vv = vv->next)
        {
            if (vv->next == vvptr)
            {
                vv->next = vvptr->next;
                meNullFree (vvptr->lineArray);
                meFree (vvptr);
                break;
            }
        }
    }
}

/*
 * meVideoAttach
 * Attach a window to the virtual video block. If no virtual video block
 * is specified then construct a new block and link it into the existing
 * structure.
 */

int
meVideoAttach (meVideo *vvptr, meWindow *wp)
{
    /* Construct a new virtual video block if required */
    if (vvptr == NULL)
    {
        meVideoLine  *vs;                     /* New video structure */

        /* Allocate a new video structure */
        if (((vvptr = (meVideo *) meMalloc (sizeof (meVideo))) == NULL) ||
            ((vs = (meVideoLine *) meMalloc(frameCur->depthMax*sizeof(meVideoLine))) == NULL))
        {
            meNullFree (vvptr);
            return (meFALSE);
        }

        /* Reset the strucure and chain to the existing frameCur->video list */
        memset (vs, 0, frameCur->depthMax * sizeof(meVideoLine));
        memset (vvptr, 0, sizeof (meVideo));
        vvptr->lineArray = vs;                    /* Attach the video structure */
        vvptr->next = frameCur->video.next;      /* Chain into frameCur->vvideo list */
        frameCur->video.next = vvptr;

        /* Reset information in the Window block */
        wp->videoNext = NULL;
    }

    /* Link the window to the frameCur->video block */
    wp->video = vvptr;
    wp->videoNext = vvptr->window;
    vvptr->window = wp;
    return (meTRUE);
}
