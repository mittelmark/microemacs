/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * word.c - Word processing routines.
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
 * Synopsis:    Word processing routines.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     The routines in this file implement commands that work word or a
 *     paragraph at a time. There are all sorts of word mode commands.
 *     If I do any sentence mode commands, they are likely to be put in
 *     this file. 
 *
 * Notes:
 * 
 *     Jon: 99/07/09
 *
 *     I have just added the new Anchor routines in here to preserve the
 *     character position of point and mark on fill paragraph. This was a
 *     fairly quick modification that disturbed as little of the code as
 *     possible, thereby maintaining the integrity of the release.
 *    
 *     The anchor routines are written ready for export because I think that
 *     they might be useful, although they have not actually been exported yet
 *     because with misuse they will crash editor. So I'm in two minds about
 *     this.
 *    
 *     The fill paragraph is now in need of a severe overhaul to reduce the
 *     amount of work that it performs. It currently performs a destructive
 *     justification process which is deeply unpleasant. In addition it must
 *     scan through the source copying it multiple times - very un-nice.
 *    
 *     I have started to prototype a replacement which is not ready yet. This
 *     will overcome all of the above. In addition I want to add a new Flow
 *     mode, this will allow MicroEmacs to act more like a word processor and
 *     throw words during the editing process onto the next line, and then
 *     automatically re-format the remaining part of the paragraph. Obviously
 *     handling left/right/centre correctly.
 *    
 *     On top of all of that. I want to change the paragraph recognition
 *     markers to be a buffer dependent regexp, thereby allowing the different
 *     paragraph types to be recognised and handled correctly. And finally,
 *     allow for text leaders (i.e. typically programming language comment
 *     delimiters) and correctly handle text wrapping with leader insertion.
 *    
 *     One could argue that some of the above belongs in ifill-paragraph
 *     (macro wrapper around fill-paragraph). However for smatter flow control
 *     I do not think that this is the case and would like to pursue a generic
 *     method that handles a multitude of cases, given the minimal amount of
 *     setup information in the buffer
 *    
 *     All of the above is possible, it just takes a little time for some of
 *     these things to come together - a commodity I've not got much of at the
 *     moment.
 */

#define	__WORDC			/* Define filename */

#include "emain.h"

/*
 * Move the cursor backward by "n" words. All of the details of motion are
 * performed by the "windowBackwardChar" and "windowForwardChar" routines. Error if you try to
 * move beyond the buffers.
 */
int
backWord(int f, int n)
{
    if(n < 0)
        return forwWord(f,-n) ;
    if(meWindowBackwardChar(frameCur->windowCur, 1) == meFALSE)
        return meErrorBob() ;
    while (n--)
    {
        while(inWord() == meFALSE)
            if(meWindowBackwardChar(frameCur->windowCur, 1) == meFALSE)
                return meErrorBob() ;
        while(inWord() != meFALSE)
            if(meWindowBackwardChar(frameCur->windowCur, 1) == meFALSE)
                /* We can't move back any more cos we're at the start,
                 * BUT as we have moved and we are in the buffers first word, 
                 * we should succeed */
                return meTRUE ;
    }
    return meWindowForwardChar(frameCur->windowCur,1) ;
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "windowForwardChar". Error if you try and move beyond the buffer's end.
 */
int
forwWord(int f, int n)
{
    if(n < 0)
        return backWord(f,-n) ;
    while(n--)
    {
        while(!inWord())
        {
            if(meWindowForwardChar(frameCur->windowCur, 1) == meFALSE)
                return meErrorEob() ;
        }
        while(inWord())
            meWindowForwardChar(frameCur->windowCur,1) ;
    }
    return meTRUE ;
}

/*
 * Move the cursor forward by the specified number of words. As you move,
 * convert any characters to upper case. Error if you try and move beyond the
 * end of the buffer. Bound to "M-U".
 */
int
upperWord(int f, int n)
{
    register int    c;
    
    if (n < 0)
        return (meFALSE);
    if((c=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return c ;
    while (n--)
    {
        while (inWord() == meFALSE) 
        {
            if(meWindowForwardChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
        while (inWord() != meFALSE)
        {
            c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset);
            if (isLower(c))
            {
                lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                meLineSetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset, c);
            }
            if (meWindowForwardChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
    }
    return (meTRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert characters to lower case. Error if you try and move over the end of
 * the buffer. Bound to "M-L".
 */
int
lowerWord(int f, int n)
{
    register int    c;
    
    if (n < 0)
        return (meFALSE);
    if((c=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return c ;
    while (n--)
    {
        while (inWord() == meFALSE) 
        {
            if (meWindowForwardChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
        while (inWord() != meFALSE)
        {
            c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset);
            if (isUpper(c))
            {
                lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                meLineSetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset, c);
            }
            if (meWindowForwardChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
    }
    return (meTRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert the first character of the word to upper case, and subsequent
 * characters to lower case. Error if you try and move past the end of the
 * buffer. Bound to "M-C".
 */
int
capWord(int f, int n)
{
    register int    c;
    
    if(n < 0)
        return (meFALSE);
    if((c=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return c ;
    while (n--)
    {
        while (inWord() == meFALSE)
        {
            if (meWindowForwardChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
        }
        if (inWord() != meFALSE) {
            c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset);
            if (isLower(c))
            {
                lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                c = toggleCase(c) ;
                meLineSetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset,c);
            }
            if (meWindowForwardChar(frameCur->windowCur, 1) == meFALSE)
                return (meFALSE);
            while (inWord() != meFALSE)
            {
                c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset);
                if (isUpper(c))
                {
                    lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                    meUndoAddRepChar() ;
#endif
                    c = toggleCase(c) ;
                    meLineSetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset,c);
                }
                if (meWindowForwardChar(frameCur->windowCur, 1) == meFALSE)
                    return meFALSE ;
            }
        }
    }
    return meTRUE ;
}

/*
 * Kill forward by "n" words. Remember the location of dot. Move forward by
 * the right number of words. Put dot back where it was and issue the kill
 * command for the right number of characters. Bound to "M-d".
 */
int
forwDelWord(int f, int n)
{
    meLine  *dotp ;
    meInt    size, lineno ;
    meUShort doto ;    
    
    if(n == 0)
        return meTRUE ;
    if(n < 0)
        return backDelWord(f,-n) ;
    
    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meABORT ;
    dotp   = frameCur->windowCur->dotLine;
    doto   = frameCur->windowCur->dotOffset;
    lineno = frameCur->windowCur->dotLineNo ;
    size = 0;
    /* if no argument was given then only kill the white space before the word */
    if(!f && !inWord())
    {
        while(!inWord() && (meWindowForwardChar(frameCur->windowCur, 1) > 0))
            size++ ;
        if(size)
            n = 0 ;
    }
    else
    {
        do {
            while(!inWord() && (meWindowForwardChar(frameCur->windowCur, 1) > 0))
                size++ ;
            if(!inWord())
                break ;
            while(inWord())
            {
                meWindowForwardChar(frameCur->windowCur, 1) ;
                size++ ;
            }
        } while(--n) ;
    }
    frameCur->windowCur->dotLine = dotp;
    frameCur->windowCur->dotOffset = doto;
    frameCur->windowCur->dotLineNo = lineno ;
    if(ldelete(size,3) <= 0)
        return meFALSE ;
    return ((n) ? meErrorEob():meTRUE) ;
}

/*
 * Kill backwards by "n" words. Move backwards by the desired number of words,
 * counting the characters. When dot is finally moved to its resting place,
 * fire off the kill command. Bound to "M-Backspace".
 */
int
backDelWord(int f, int n)
{
    meInt size;
    
    if(n == 0)
        return meTRUE ;
    if(n < 0)
        return forwDelWord(f,-n) ;
    
    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meABORT ;
    if(meWindowBackwardChar(frameCur->windowCur, 1) == meFALSE)
        return meErrorBob() ;
    size = 0;
    /* if no argument was given then only kill the white space before the word */
    if(!f && !inWord())
    {
        do
            size++ ;
        while((meWindowBackwardChar(frameCur->windowCur, 1) > 0) && !inWord()) ;
        if(inWord())
            meWindowForwardChar(frameCur->windowCur, 1) ;
        n = 0 ;
    }
    else
    {
        do {
            while(!inWord() && (meWindowBackwardChar(frameCur->windowCur, 1) > 0))
                size++ ;
            if(!inWord())
            {
                size++ ;
                break ;
            }
            while(inWord() && (meWindowBackwardChar(frameCur->windowCur, 1) > 0))
                size++ ;
        } while(--n) ;
        
        if((n == 0) && !inWord())
            meWindowForwardChar(frameCur->windowCur, 1) ;
    }

    if(ldelete(size,3) <= 0)
        return meFALSE ;
    return ((n) ? meErrorBob():meTRUE) ;
}

#if MEOPT_WORDPRO

#define FILL_LEFT    0x0001             /* Left fill                    */
#define FILL_RIGHT   0x0002             /* Right fill                   */
#define FILL_BOTH    0x0004             /* Both fill                    */
#define FILL_CENTRE  0x0008             /* Center fill                  */
#define FILL_NONE    0x0010             /* None fill                    */
#define FILL_TMASK   0x001f             /* Type mask for fill           */
#define FILL_INDENT  0x0020             /* Do Indented fill             */
#define FILL_FORCE   0x0040             /* Forced line output           */
#define FILL_DOT     0x0080             /* Last character was a period. */
#define FILL_EOP     0x0100             /* End of paragraph detected    */
#define FILL_FIRST   0x0200             /* First word ? (needs no space */
#define FILL_JUSTIFY 0x0400             /* Status of the justify flag   */
#define FILL_AUTO    0x0800             /* Automatic fill               */
#define FILL_LINE    0x1000             /* Fill to a line               */
#define FILL_MARGIN  0x2000             /* Fill to margin (AUTO+LINE)   */
#define FILL_INDALL  0x4000             /* Indent all                   */
#define FILL_INDNVR  0x8000             /* Never indent                 */
#define FILL_SPACE  0x10000             /* Kept one space               */

/* Word wrap on n-spaces. Back-over whatever precedes the point on the current
 * line and stop on the first word-break or the beginning of the line. If we
 * reach the beginning of the line, jump back to the end of the word and start
 * a new line.  Otherwise, break the line at the word-break, eat it, and jump
 * back to the end of the word.
 * Returns meTRUE on success, meFALSE on errors.
 */

int
wrapWord(int f, int n)
{
    register meUShort cnt, off ;		/* size of word wrapped to next line */
    register meUByte c, last=2 ;		/* charector temporary */
    
    if(bufferSetEdit() <= 0)               /* Check we can change the buffer */
        return meABORT ;
    
    /* Make sure we are not in no fill mode */
    if (toLower(frameCur->bufferCur->fillmode) == 'n')
        return meTRUE;
    
    /* back up until we aren't in a word,
       make sure there is a break in the line */
    cnt = 0 ;
    off = frameCur->windowCur->dotOffset ;
    do
    {
        c = meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset) ;
        if((c == ' ') || (c == meCHAR_TAB))
        {
            if(last == 1)
                cnt = frameCur->windowCur->dotOffset+1 ;
            last = 0 ;
        }
        else if(!last && (getccol() < frameCur->bufferCur->fillcol))
            break ;
        else
            last = 1 ;
    } while((frameCur->windowCur->dotOffset)-- > 0) ;
    
    if(cnt != 0)
        frameCur->windowCur->dotOffset = cnt ;
    else
    {
        frameCur->windowCur->dotOffset = off ;
        if(((c = meLineGetChar(frameCur->windowCur->dotLine,off)) != ' ') && (c != meCHAR_TAB))
            return meTRUE ;
    }
    off = meLineGetLength(frameCur->windowCur->dotLine) - off ;
#if MEOPT_HILIGHT
    if(frameCur->bufferCur->indent)
        cnt = indentInsert() ;
    else
#endif
    if(meModeTest(frameCur->bufferCur->mode,MDINDENT))
        cnt = winsert() ;
    else
    {
        cnt = lineInsertNewline(0) ;
#if MEOPT_UNDO
        if(cnt > 0)
            meUndoAddInsChars(n) ;
#endif
    }    
    if(meModeTest(frameCur->bufferCur->mode,MDJUST))
        justify(-1,-1,frameCur->bufferCur->fillmode) ;
    
    frameCur->windowCur->dotOffset = meLineGetLength(frameCur->windowCur->dotLine) - off ;
    return cnt ;
}



static int
countGaps(void)
{
    meUByte last;       /* Last character read. */
    meUByte c;          /* Current character */
    int	count=0 ;	/* Number of gaps encountered */

    last = '\0';				/* Reset word */
    while((c=meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset)) != '\0')
    {
        if((last == ' ') && (c != ' '))
           ++count ;
        last = c;		/* Save last character */
        frameCur->windowCur->dotOffset++;	/* Next character position */
    }
    return count ;		/* Record word */
    
}	/* End of "goto_gap" */

static char gapDir = 0;                 /* Gap direction */

static void
getBestGap(void)
{
    meUByte last;       /* Last character read. */
    meUByte c;          /* Current character */
    int	score=0, bestscore=0, currscore=0 ;	/* Number of gaps encountered */
    meUShort bestoff, curroff, off ;
    
    last = 'z';				/* Reset word */
    off = frameCur->windowCur->dotOffset ;
    for(;;)
    {
        if(((c=meLineGetChar(frameCur->windowCur->dotLine,off)) == '\0') || (c == ' '))
        {
            if(last != ' ')
            {
                if(!isWord(last))
                    score -= 10 ;
                currscore += score ;
/*                currscore += off & 0x7 ;*/
                currscore += gapDir ;
                if(currscore > bestscore)
                {
                    bestscore = currscore ;
                    bestoff = curroff ;
                }
                currscore = score+0x0fff0000 ;
                curroff = off ;
            }
            else
                currscore -= 0x10000 ;
            if(c == '\0')
                break ;
        }
        else
        {
            if(last == ' ')
            {
                if(!isWord(c))
                    score = 10 ;
                else
                    score = 20 ;
            }
            else
                score += 20 ;
        }
        last = c;		/* Save last character */
        off++;	                /* Next character position */
    }
    frameCur->windowCur->dotOffset = bestoff ;		/* Record word */
    
}	/* End of "goto_gap" */

/* justify
 * This routine will justify a block of text. The start of the text is at
 * doto, and the scaller specifies the length of the text to be justified, as
 * well as the new required length. This routine assumes the caller has
 * already removed tabs as justification is performed on spaces only.
 * Additionally the caller has already formatted the line to the correct
 * length. Justification is performed on the right margin only. (The left
 * margin is assumed to be done).
 * 
 * leftMargin is a instruction from fillPara() to inform justify where it's
 * left margin is. justify() determines it's own left margin and tabMode if
 * passed a -ve values.
 * 
 * This is required for the bulleted "* " or "a) " and "ii - " type
 * indentation where a leading space must not be altered.
 * 
 * Note that on first inspection justify appears to be doing too much work in
 * cleaning up the line. This is true for a fillPara() invocation but it is
 * not true for any other invocation.
 */
int
justify(int leftMargin, int leftDoto, meUByte jmode)
{
    meUShort  len, doto, ss ;
    meUShort  odoto;              /* Starting doto backup */
    meUByte   cc ;
    int       undoMode;           /* Undo operation */
    int       col ;               /* left indent with tab expansion */
    
    /* Save old context */
    odoto = frameCur->windowCur->dotOffset;
    frameCur->windowCur->dotLine = meLineGetPrev(frameCur->windowCur->dotLine) ;
    (frameCur->windowCur->dotLineNo)-- ;
    undoMode = (leftMargin < 0) ? 2:0;  /* Compute the undo mode */ 
    
    /* lowercase the given jmode to remove the auto-detect */
    jmode = toLower(jmode);
    
    /* We are always filling the previous line */
    len = meLineGetLength(frameCur->windowCur->dotLine) ; 
    ss = len ;
    
    /* Strip any spaces from the right of the string */
    while(len > 0)
    {
        len-- ;
        if(((cc = meLineGetChar(frameCur->windowCur->dotLine,len)) != ' ') && (cc != meCHAR_TAB))
        {
            len++ ;
            break ;
        }
    }
    if((ss = ss - len) != 0)
    {
        frameCur->windowCur->dotOffset = len ;
        ldelete (ss, undoMode) ;
    }

    /* If line is empty or left justification is required then do nothing */
    if((len == 0) || (strchr ("bcr", jmode) == NULL))
        goto finished;
    
    if(leftDoto < 0)
    {
	/* move to the left hand margin */
	frameCur->windowCur->dotOffset = 0 ;
	while(((cc = meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset)) != '\0') &&
	      ((cc == ' ') || (cc == '\t')))
	    frameCur->windowCur->dotOffset++ ;
	doto = frameCur->windowCur->dotOffset ;
	col = getccol() ;
	if(leftMargin < 0)
	    leftMargin = col ;
    }
    else
    {
	doto = leftDoto ;
	frameCur->windowCur->dotOffset = doto ;
	col = leftMargin ;
    }
    /* check for and trash any TABs on the rest of the line */
    while((cc=meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset)) != '\0')
    {
	if (cc == meCHAR_TAB)
	{
	    /* Get expansion width of TAB */ 
	    int jj = next_tab_pos(getccol(), frameCur->bufferCur->tabWidth);
	    /* Replace the TAB with equivelent SPACE's */
	    ldelete (1L,undoMode);
	    lineInsertChar (jj, ' ');
#if MEOPT_UNDO
	    if (undoMode)
		meUndoAddInsChars(jj) ;
#endif
	}
	frameCur->windowCur->dotOffset++ ;
    }
    len = meLineGetLength(frameCur->windowCur->dotLine) - doto + leftMargin ;
    
    /* If there is leading white space and we are justifying centre or right
     * then delete the white space. */
    if(jmode != 'b')
    {
        /* Centre and right justification are pretty similar. For right
         * justification we insert "fillcol-linelen" spaces. For centre
         * justification we insert "fillcol-linelen/2" spaces. */
        if(len > frameCur->bufferCur->fillcol)
        {
            /* line is already too long - this can happen when filling a
             * paragraph to the right and the first line is not the longest -
             * pull back minimum to stay within the fillcol */
            len -= frameCur->bufferCur->fillcol ;
            if(leftMargin < len)
                leftMargin = 0 ;
            else
                leftMargin -= len ;
        }
        else
        {
            len = frameCur->bufferCur->fillcol - len ;
            if(jmode == 'c')                /* Centre ?? */
                leftMargin = (len + leftMargin) >> 1 ;
            else
                leftMargin += len ;
        }
    }
    /* Both left and right margin justification. */
    else if(len < frameCur->bufferCur->fillcol)
    {
        int gaps ;			/* The number of gaps in the line */
        
        ss = frameCur->bufferCur->fillcol - len ;
        frameCur->windowCur->dotOffset = doto ;	        /* Save doto position */
        gaps = countGaps() ;	        /* Get number of gaps */
        
        if(gaps == 0)			/* No gaps ?? */
        {
            lineInsertChar(ss, ' ') ;	/* Fill at end */
#if MEOPT_UNDO
            if (undoMode)
                meUndoAddInsChars(ss) ;
#endif
        }
        else                            /* Start filling the gaps with spaces */
        {
            int incGaps ;
            
            gapDir ^= 1 ;               /* Direction of gaps */
            do
            {
                frameCur->windowCur->dotOffset = doto ;	/* Save doto position */
                getBestGap() ;
                
                incGaps = (ss+gaps-1)/gaps ;
                lineInsertChar(incGaps, ' ') ;
#if MEOPT_UNDO
                if (undoMode)
                    meUndoAddInsChars(incGaps) ;
#endif
                ss -= incGaps ;
                gaps-- ;
            } while(ss) ;
        }
    }

    /* Set the left margin for center and right modes,
     * and change the left margin back to tabs */
    if(col != leftMargin)
	meLineSetIndent(doto,leftMargin,undoMode) ;
finished:
    
    /* Restore context - return the new line length */
    ss = meLineGetLength (frameCur->windowCur->dotLine);
    frameCur->windowCur->dotLine = meLineGetNext(frameCur->windowCur->dotLine) ;
    frameCur->windowCur->dotOffset = odoto ;
    (frameCur->windowCur->dotLineNo)++ ;
    return ss;
}

/*
 * fillLookahead
 * Helper for fillPara(). Search forward in the paragraph for any magic
 * paragraph characters which are used for auto indentation. If found 
 * prompt the user that they have been found and ask for the left-hand
 * fill column. 
 * 
 * A change of lefthand fill column modifies the buffers doto position.
 * 
 * form is a bit of a hang over which I must come back and sort (Jon Green).
 * TABS are also bogus here !!
 */

static meInt
lookahead(meInt fillState)
{
    int status;                         /* Status of command. */
    int	ii;                             /* Loop counter */
    int	limit;		                /* Limit */
    char c;                             /* Current character */
    char last_c;                        /* Last character */
    
    /* Nothing to do if there are no paragraph markers */
    if (fillbullet[0] == '\0')
        return fillState ;
    limit = frameCur->windowCur->dotOffset+fillbulletlen;/* Define the limit of search */
    if (limit > meLineGetLength(frameCur->windowCur->dotLine))
        limit = meLineGetLength (frameCur->windowCur->dotLine);
    
    /* Scan the line for a paragraph starting point. Start from the beginning
       of the line */
    for (ii = 0, last_c = (char) 1; ii < limit; ii++)
    {
        c = meLineGetChar (frameCur->windowCur->dotLine, ii);
        if ((meStrchr (fillbullet, last_c) != NULL) && (c == ' ' || c == meCHAR_TAB))
        {
            /* Go to the end of the spaces. */
            while (++ii < limit)
            {
                c = meLineGetChar(frameCur->windowCur->dotLine, ii);
                if ((c != ' ') && (c != meCHAR_TAB))
                    break;
            }
            
            /* flag that we will have to prompt the user */
            status = 1 ;
            
            /* If we are automatically detecting then try to make a 
             * sensible decision on the paragraph indentation without
             * troubling the user */
            if (fillState & FILL_AUTO)
            {
                meLine *temp_dotp;      /* Save our current line */
                
                /* Move to the next line and scan it */
                temp_dotp = frameCur->windowCur->dotLine;  
                frameCur->windowCur->dotLine = meLineGetNext (frameCur->windowCur->dotLine);
                if (frameCur->windowCur->dotLine != frameCur->bufferCur->baseLine)	/* EOF  ?? */
                {
                    int jj;
                    int jlen;
                    
                    /* Build up a profile of the spaces on the next line */
                    jlen = meLineGetLength (frameCur->windowCur->dotLine);
                    for (jj = 0; jj < jlen; jj++)
                    {
                        c = meLineGetChar(frameCur->windowCur->dotLine, jj);
                        if ((c != ' ') && (c != meCHAR_TAB))
                            break;
                    }
                
                    /* Only prompt the user if the indent on the next line
                     * does not match the current line expectation. Or the
                     * current line is liable to spill onto the next line
                     * which is empty */
                    if ((jj == ii) ||   /* Next line indented to 'ii' */
                        ((jj == jlen) && (meLineGetLength(temp_dotp) <= frameCur->bufferCur->fillcol)))
                    {
                        frameCur->windowCur->dotOffset = ii; /* Assume position 'i' */
                        status = 0;     /* Disable prompt */
                    }
                }
                /* Restore previous position */
                frameCur->windowCur->dotLine = temp_dotp;
            }
            
            /* If auto detect is disabled or failed then manually prompt the user
             * for the indented position. */
            if(status)
            {
                short temp_doto;        /* Temporary character position */
                
                temp_doto = frameCur->windowCur->dotOffset;
                frameCur->windowCur->dotOffset = ii;
                
                if(fillState & FILL_INDALL)
                    status = 'y' ;
                else if(fillState & FILL_INDNVR)
                    status = 'n' ;
                else
                {
                    if((status = mlCharReply((meUByte *)"Indent to <<<<<<<<<< (?/y/n/a/o) ? ",mlCR_QUIT_ON_USER|mlCR_LOWER_CASE,(meUByte *)"ynao",NULL)) == -2)
                    {
                        meUByte scheme=(frameCur->bufferCur->scheme/meSCHEME_STYLES) ;
            
                        /* Force an update of the screen to to ensure that the user
                         * can see the information in the correct location */
                        update (meTRUE);
                        pokeScreen(0x10,frameCur->mainRow,frameCur->mainColumn,&scheme,(meUByte *)"<<<<<<<<<<") ;
                        status = mlCharReply((meUByte *)"Indent to <<<<<<<<<< (?/y/n/a/o) ? ",mlCR_LOWER_CASE,(meUByte *)"ynao",
                                             (meUByte *)"(Y)es, (N)o, Yes to (a)ll, N(o) to all, (C-g)Abort ? ") ;
                        mlerase(0);
                    }
                    if (status == -1)
                    {
                        frameCur->windowCur->dotOffset = temp_doto;	/* Restore */
                        return -1 ;
                    }
                    if(status == 'a')
                    {
                        fillState |= FILL_INDALL ;
                        status = 'y' ;
                    }
                    else if(status == 'o')
                    {
                        fillState |= FILL_INDNVR ;
                        status = 'n' ;
                    }
                }
                if(status == 'y')
                    frameCur->windowCur->dotOffset = ii;
                else
                    frameCur->windowCur->dotOffset = temp_doto;	/* Restore */
            }
            return fillState ;          /* Return potentially modified fillState */
        }
        else
            last_c = c;
    }
    return fillState ;
}


/*
 * Try to autodetect the fill mode from the formation of the first line
 * in the paragraph. This will assume that the first line. is correctly
 * formatted.
 */
static char 
fillAutoDetect (char mode)
{
    int doto;                           /* Doto position */
    int len;                            /* lenght of the line */
    int textLen;                        /* Length of the text */
    
    doto = frameCur->windowCur->dotOffset;               /* Save current position */
    len = meLineGetLength (frameCur->windowCur->dotLine);      /* Length of the line */
    textLen = (len - doto);             /* Text length */
    mode = toLower (mode);              /* Align mode to regular operation */
    
    /* Test for centre */
    if (len < frameCur->bufferCur->fillcol)   /* On the right margin */
    {
        int sdiff;                      /* Start difference */
        int ediff;                      /* End difference */
        
        /* Check for balanced line. We expect the start and end of the
         * line to be approximatly balanced. Allow a margin of error of
         * 6 characters */
        sdiff = (frameCur->bufferCur->fillcol >> 1) - (doto + (textLen >> 1));
        if (sdiff < 0)                  /* Quick abs !! */
            sdiff = -sdiff;
        ediff = (frameCur->bufferCur->fillcol >> 1) - (len  - (textLen >> 1));
        if (ediff < 0)                  /* Quick abs !! */
            ediff = -ediff;
        sdiff += ediff;
        
        if ((sdiff == 0) ||             /* We are certain !! */
            ((sdiff < 2) && (textLen < (frameCur->bufferCur->fillcol >> 1))))
            return ('c');               /* We are centred */
    }
    
    /* Test for right aligned text */
    if ((len >= frameCur->bufferCur->fillcol) && (doto > (frameCur->bufferCur->fillcol >> 1)))
        return ('r');                   /* We are right */
    
    /* Check for text on the left-hand edge of line. If it meets our
     * criteria for not filling the whole paragraph then apply 'n' which
     * formats each line separately. */
    if ((doto == 0) && (len <= (frameCur->bufferCur->fillcol >> 1)))
        return ('n');                   /* No justification for left */
    
    return mode ;                       /* Returned modified mode */
}              
    
/* Fill the current paragraph according to the current fill column */
int
fillPara(int f, int n)	
{
    meLine *eopline;		        /* ptr to line just past EOP	*/
    meInt eoplno;		        /* line no of line just past EOP*/
    meInt ilength;			/* Initial line length          */
    meUByte jmode;                      /* justification mode           */
    register meInt fillState;           /* State of the fill            */
    int c, lastc;		        /* current char durring scan	*/
    int ccol;				/* position on line during fill	*/
    int newcol;			        /* tentative new line length	*/
    int wordlen;		        /* length of current word	*/
    int icol;				/* Initial line columns         */
    int fdoto;                          /* The left doto for 1st line   */

#if MEOPT_UNDO
    int paralen ;
#endif
    
    if (frameCur->bufferCur->fillcol == 0)  /* Fill column set ??*/
        return mlwrite(MWABORT,(meUByte *)"No fill column set");
    if ((c=bufferSetEdit()) <= 0)       /* Check we can change the buffer */
        return c ;
    
    /* A -ve cont indicates that we do not want to be prompted for indentation
     * We also do not prompt if we are right or centre justifying */
    if (n < 0)
    {
        fillState = 0 ;                 /* Disable indented fill */
        n = 0 - n ;
    }
    else
        fillState = FILL_INDENT;        /* Enable indented fill */
    
    /* Sort out the justification mode that we are running in.
     * An upper case justification mode implies an automatic mode
     * where we detect the current state of the line and apply the 
     * appropriate formatting.
     *
     * If the One Line mode (o) is operational then fill compleate 
     * paragraphs to a single line. Indentation is disabled */
    jmode = frameCur->bufferCur->fillmode;
    if (isUpper(jmode))
    {
        fillState |= FILL_AUTO;         /* Auto paragraph type detection */
        jmode = toLower(jmode);
        if (jmode == 'o')
            fillState = (fillState | FILL_LINE | FILL_MARGIN) & ~FILL_INDENT; 

    }
    else if (jmode == 'o')
        fillState = (fillState | FILL_LINE) & ~FILL_INDENT; 
    
    /* In none mode then we do not touch the paragraph. */
    if (jmode == 'n') 
    {
        /* If there are no arguments then do nothing, otherwise advance the
         * paragraph. */
        if (!f)
            return meTRUE;
        return windowForwardParagraph(meFALSE, n) ;
    }
    
    /* Record if justify mode is enabled. Record in our local context Also
     * knock off indent if disabled. We do not want to be prompting the user. */
    if(meModeTest(frameCur->bufferCur->mode,MDJUST)) /* Justify enabled ?? */
        fillState |= FILL_JUSTIFY;      /* Yes - Set justification state */
    else
    {
        /* Justification is disabled. If centre, right or both is enabled
         * then reduce to left. */
        jmode = 'l';
        fillState &= ~FILL_INDENT;      /* No - Knock indent off */
    }
    
    /* If fill paragraph is called with no arguments then we must retain the
     * position of dot so that we can preserve the users position after we
     * have filled the paragraph, but only if we are not at the top of the
     * paragraph. */
    if(f == 0)
    {
        /* See if this is a paragraph separator line. If it is not a
         * separator then the paragraph position must be restored. */
        f = 1;                          /* Anchor will be set */
        if ((icol = meLineGetLength (frameCur->windowCur->dotLine)) != 0)
        {
            for (ccol = 0; ccol < icol; ccol++)
            {
                /* Check that this is not a paragraph sepearator line */
                c = meLineGetChar (frameCur->windowCur->dotLine, ccol);
                if ((c != meCHAR_TAB) && (c != ' '))	           
                {
                    f = 2;              /* Char exists on line, restore pos */
                    break;
                }
            }
        }
        
        meAnchorSet(frameCur->bufferCur,meANCHOR_FILL_DOT,
                    frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset,1) ;
        ilength = frameCur->windowCur->dotLineNo ;
        icol = frameCur->windowCur->dotOffset ;
    }
    else
        f = -1 ;
    
    /* Fill 'n' paragraphs */
    while (--n >= 0)
    {
        /* go to the beginning of the line and use forward-paragraph
         * to go to the end of the current or next paragraph, if this
         * fails we are at the end of buffer */
        frameCur->windowCur->dotOffset = 0 ;             /* Got start of current line */
        if (windowForwardParagraph(meFALSE, 1) <= 0)
            break;
        
	/* record the pointer to the line just past the EOP 
         * and then back top the beginning of the paragraph.
         * doto is at the beginning of the first word */
        eopline = frameCur->windowCur->dotLine ;
        eoplno = frameCur->windowCur->dotLineNo ;
	windowBackwardParagraph(meFALSE, 1);
        
        /* Advance to the first character in the paragraph. */
        while(!inPWord())
            if(meWindowForwardChar(frameCur->windowCur, 1) <= 0)
                break;
        
        /* Skip non-formatting paragraphs */
        if ((fillignore[0] != '\0') &&
            (meStrchr (fillignore, meLineGetChar (frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset)) != NULL))
        {
            frameCur->windowCur->dotLine = eopline;    /* Goto the end of the paragraph */
            frameCur->windowCur->dotLineNo = eoplno ;
            frameCur->windowCur->dotOffset = 0;
            continue;			/* Next one */
	}
				
#if MEOPT_EXTENDED
        {
            /* check that this paragraph can be filled */
            meLine *lp ;
            lp = frameCur->windowCur->dotLine ;
            do
            {
                if(meLineGetFlag(lp) & meLINE_PROTECT)
                    return mlwrite(MWABORT,(meUByte *)"[Protected line in paragraph!]") ;
#if MEOPT_NARROW
                if((lp != frameCur->windowCur->dotLine) &&
                   (meLineGetFlag(lp) & meLINE_ANCHOR_NARROW))
                    return mlwrite(MWABORT,(meUByte *)"[Narrow in paragraph!]") ;
#endif
            } while((lp=meLineGetNext(lp)) != eopline) ;
        }
#endif
        /* Quick auto test to determine what mode the current paragraph is.
         * Set up the modes in the fill status mask */
        if (fillState & FILL_AUTO)
            jmode = fillAutoDetect(frameCur->bufferCur->fillmode);
        fillState &= ~FILL_TMASK;
        switch (jmode)
        {
        case 'b':                       /* Both mode */
            fillState |= FILL_BOTH;
            break;
        case 'r':                       /* Right mode */
            if (fillState & FILL_MARGIN)
                goto noIndent;          /* Reduce to 'none' */
            fillState |= FILL_RIGHT;
            break;
        case 'c':                       /* Center mode */
            if (fillState & FILL_MARGIN)
                goto noIndent;          /* Reduce to 'none' */
            fillState |= FILL_CENTRE;
            break;
noIndent:            
        case 'g':                       /* Guffer mode */
            jmode = 'n';
            /* Drop through */
        case 'n':                       /* None mode */
            fillState |= FILL_NONE;
            break;
        case 'o':                       /* One line justification */
            jmode = 'l';                /* Force to left */
            /* Drop through */
        default:                        /* Left justification */
            fillState |= FILL_LEFT;
            break;
        }
        
	fdoto = frameCur->windowCur->dotOffset ;
        /* Get the initial string from the start of the paragraph.
         * lookahead() modifies the current doto value. */
        if (((fillState & (FILL_INDENT|FILL_BOTH|FILL_LEFT)) > FILL_INDENT) &&
            ((fillState=lookahead(fillState)) == -1))
            return meABORT ;
	
        /* if lookahead has found a new left indent position, this position
	 * must be passed to justify so justify ignores the text to the left
	 * of this indent point */
	if(fdoto == frameCur->windowCur->dotOffset)
	    fdoto = -1 ;
	else
	    fdoto = frameCur->windowCur->dotOffset ;
        
        ilength = frameCur->windowCur->dotOffset;
#if MEOPT_UNDO
        frameCur->windowCur->dotOffset = 0;
        meUndoAddReplaceBgn(eopline,0) ;
        frameCur->windowCur->dotOffset = (meUShort) ilength;
        paralen = 0 ;
#endif
        /* Get the indentation column, this is the real left indentation position */
	icol = getccol() ;
        
	/* Reset to initial settings.
         * Modes are:-
         * 
         *     ~EOP    - Not at end of paragraph
         *     ~FORCE  - Not a forced paragraph line
         *     ~DOT    - Not a period present
         *      FIRST  - This is the first word
         */
        ccol = icol;
        wordlen = 0;                    /* No word is present */
 
        fillState = ((fillState & ~(FILL_EOP|FILL_FORCE|FILL_DOT)) |
                     (FILL_FIRST));
        
        /* scan through lines, filling words */
        while ((fillState & FILL_EOP) == 0)
        {
            /* get the next character in the paragraph */
            if((c = meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset)) == '\0')
            {
                c = meCHAR_TAB ;
                if (meLineGetNext(frameCur->windowCur->dotLine) == eopline)
                    fillState |= FILL_EOP;  /* End of paragraph */
                
                if (fillState & (FILL_CENTRE|FILL_NONE|FILL_RIGHT))
                    fillState |= FILL_FORCE; /* Force paragraph output */
            }
            
            /* if not a separator, just add it in */
            if ((c == ' ') || (c == meCHAR_TAB))
            {
                if (wordlen)
                {
                    /* at a word break with a word waiting - reset offset to start of word */
                    frameCur->windowCur->dotOffset -= wordlen ;
                    if(fillState & FILL_SPACE)
                        frameCur->windowCur->dotOffset-- ;
                    
                    /* calculate tantitive new length with word added */
                    newcol = ccol + 1 + wordlen;
                    if (fillState & FILL_DOT)
                        newcol += filleoslen-1 ;
                    if ((newcol <= frameCur->bufferCur->fillcol) || (fillState & FILL_LINE))
                    {
                        /* add word to current line */
                        if((fillState & FILL_FIRST) == 0)
                        {
                            int nosp = (fillState & FILL_DOT) ? filleoslen:1 ;
                            ccol += nosp ;
                            if(fillState & FILL_SPACE)
                            {
                                frameCur->windowCur->dotOffset++ ;
                                nosp-- ;
                            }
                            if(nosp)
                                lineInsertChar(nosp, ' ') ; /* the space */
                        }
                        fillState &= ~(FILL_FIRST|FILL_DOT);
                    }
                    else
                    {
                        ccol = frameCur->windowCur->dotOffset;
                        if(fillState & FILL_SPACE)
                            /* the saved space is not required, delete it */
                            ldelete(1L,0);
                        lineInsertNewline(0);
                        if (fillState & FILL_JUSTIFY)
                        {
                            ccol = justify(icol,fdoto,jmode);
                            /* reset the indent offset as following lines will not
                             * have the bullet text to the left of the indent
                             * column */
                            fdoto = -1 ;
                        }
#if MEOPT_UNDO
                        paralen += ccol + 1;
#endif
                        meLineSetIndent(0,icol,0) ;
                        ccol = (int) icol;
                        fillState &= ~FILL_DOT;
                    }
                    
                    /* and add the length of the word in either case */
                    ccol += wordlen;
                    frameCur->windowCur->dotOffset += wordlen;
                    if(meStrchr(filleos,lastc) != NULL)
                        fillState |= FILL_DOT;
                    fillState &= ~FILL_SPACE ;
                    wordlen = 0;
                }
                
                if (fillState & FILL_FORCE)
                {
                    if (fillState & FILL_JUSTIFY)
                    {
                        lineInsertNewline(0);
                        ccol = justify(icol,fdoto,jmode);
                        fdoto = -1 ;
                    }
                    else
                    {
                        ccol = frameCur->windowCur->dotOffset;
                        lineInsertNewline(0);
                    }
#if MEOPT_UNDO
                    paralen += ccol + 1;
#endif
                    /* Turn off dot marker and forced line */
                    ccol = 0;
                    if ((fillState & FILL_EOP) == 0)
                        fillState = ((fillState & ~(FILL_DOT|FILL_FORCE)) |
                                     FILL_FIRST);
                }
                /* delete the space */
                if((c == ' ') && ((fillState & (FILL_FIRST|FILL_SPACE)) == 0))
                {
                    frameCur->windowCur->dotOffset++ ;
                    fillState |= FILL_SPACE ;
                }
                else
                    ldelete(1L,0);
            }
            else
            {
                lastc = c ;
                wordlen++ ;
                frameCur->windowCur->dotOffset++ ;
            }
        }

        if ((fillState & FILL_FORCE) == 0)
        {
#if MEOPT_UNDO
            paralen += frameCur->windowCur->dotOffset + 1;
#endif
            lineInsertNewline(0);
        }
#if MEOPT_UNDO
        meUndoAddReplaceEnd(paralen);
#endif
    }
    
    if(f >= 0)
    {
        /* Restore starting point */
        if((f == 2) && (meAnchorGet(frameCur->bufferCur,meANCHOR_FILL_DOT) > 0))
        {
            frameCur->windowCur->dotLine = frameCur->bufferCur->dotLine ;
            frameCur->windowCur->dotOffset = frameCur->bufferCur->dotOffset ;
            frameCur->windowCur->dotLineNo = frameCur->bufferCur->dotLineNo ;
        }
        meAnchorDelete(frameCur->bufferCur,meANCHOR_FILL_DOT) ;
    }
    return meTRUE ;
}

/* Delete n paragraphs starting with the current one. -ve will kill 'n'
 * paragraphs in the backward direction. */
int	
killPara(int f, int n)
{
    int ss ;
    int sep_line = 1;                   /* Paragraph separator line */
    
    /* A zero kill then do not kill anything. */
    if(n == 0)
        return meTRUE ;
    
    /* Check we can change the buffer */
    if(bufferSetEdit() <= 0)
        return meABORT ;
    
    /* Set the mark at the current position. */
    frameCur->windowCur->markLine = frameCur->windowCur->dotLine ;
    frameCur->windowCur->markLineNo = frameCur->windowCur->dotLineNo ;
    frameCur->windowCur->markOffset = frameCur->windowCur->dotOffset ;
    
    /* See if this is a paragraph separator line. If it is then the kill is
     * performed differently. */
    if ((ss = meLineGetLength (frameCur->windowCur->dotLine)) != 0)
    {
        int ii;
        
        for (ii = 0; ii < ss; ii++)
        {
            char cc;
            
            /* Check that this is not a paragraph sepearator line */
            cc = meLineGetChar (frameCur->windowCur->dotLine, ii);
            if ((cc != meCHAR_TAB) && (cc != ' '))	           
            {
                /* Character exists on the line. */
                sep_line = 0;
                break;
            }
        }
    }
               
    /* Move to the appropriate paragraph position and delete the region. */
    ss = windowForwardParagraph(meTRUE,n) ;
    
    /* If we commenced on a separator line then we do not need to protect the 
     * paragraph separator. */
    if (sep_line == 0)
    {
        /* Did not start on a separator line, protect the paragraph space. */
        if (n < 0)
        {
            /* If killing backwards advance to the first character of the line in
             * the paragraph so that we do not delete the paragraph space. */
            while(!inPWord())
                if(meWindowForwardChar(frameCur->windowCur, 1) <= 0)
                    break;
            frameCur->windowCur->dotOffset = 0;        /* Beginning of line */
        }
        else
        {
            /* If killing forwards goto the previous character so that we do not
             * destroy the line separating the paragraph. */
            meWindowBackwardChar(frameCur->windowCur, 1);
        }
    }
    
    /* Kill the region containing the paragraphs */
    if(killRegion(meTRUE,1) <= 0)
        return meFALSE ;
    
    return ss ;
}

/* Count the # of words in the marked region, along with average word sizes,
 * # of chars, etc, and report on them. */
int	
countWords(int f, int n)
{
    register meLine *lp;		/* current line to scan */
    register int offset;	/* current char to scan */
    long size;			/* size of region left to count */
    register int ch;		/* current character to scan */
    register int wordflag;	/* are we in a word now? */
    register int lastword;	/* were we just in a word? */
    long nwords;		/* total # of words */
    long nchars;		/* total number of chars */
    int nlines;			/* total number of lines in region */
    int status;			/* status return code */
    meRegion region;		/* region to look at */

    /* make sure we have a region to count */
    if((status = getregion(&region)) <= 0)
        return status ;
    lp = region.line ;
    offset = region.offset ;
    size = region.size ;
    
    /* count up things */
    lastword = meFALSE;
    nchars = 0L;
    nwords = 0L;
    nlines = 0;
    while(size--) 
    {
        /* get the current character */
        if((ch = meLineGetChar(lp, offset)) == '\0')
        {   /* end of line */
            ch = meCHAR_NL;
            lp = meLineGetNext(lp);
            offset = 0;
            nlines++ ;
        } 
        else
            offset++ ;
        
        /* and tabulate it */
        wordflag = isWord(ch) ;
        if((wordflag != meFALSE) && (lastword == meFALSE))
            ++nwords;
        lastword = wordflag;
        ++nchars;
    }

    /* and report on the info */
    sprintf((char *)resultStr,"%ld Words, %ld Chars, %d Lines",
            nwords,nchars,nlines+1);
    return mlwrite(MWSPEC,resultStr) ;
}
#endif
