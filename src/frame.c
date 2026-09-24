/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * frame.c - Multiple frame support.
 *
 * Copyright (C) 2002-2009 JASSPA (www.jasspa.com)
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
 * Created:     Fri Mar 1 2002
 * Synopsis:    Multiple frame support.
 * Authors:     Steven Phillips
 * Description:
 *     Adds support for multiple frames in MicroEmacs, in all versions, this
 *     allows the current 'frame' to be hidden and a new frame to be created/
 *     displayed, in windowing versions the new frame can be in a new window.
 */

#define	__FRAMEC

#include "emain.h"

/*
 * meFrameEnlargeVideo
 * Enlarge the video frames attached to the virtual video structures.
 * Usually follows an enlargement of the screen. */
int
meFrameEnlargeVideo(meFrame *frame, int rows)
{
    meVideo *vvptr;                      /* Pointer to the video blocks */
    meVideoLine  *vs;                         /* Video store line */

    for (vvptr = &frame->video; vvptr != NULL; vvptr = vvptr->next)
    {
        /* Try and allocate new video frame */
        if ((vs = (meVideoLine *) meMalloc (rows * sizeof (meVideoLine))) == NULL)
            return (meFALSE);

        /* Allocation successful. Reset the contents to zero and swap
           for the existing one */
        memset (vs, 0, rows * sizeof (meVideoLine));  /* Reset to zero */
        meFree (vvptr->lineArray);                    /* Dispose of old one */
        vvptr->lineArray = vs;                        /* Swap in new one */
    }

    return (meTRUE);
}


int
meFrameChangeWidth(meFrame *frame, int ww)
{
    /* ensure the value is in range */
    if(ww < 8)
        ww = 8 ;
    else if(ww > 400)
        ww = 400 ;
    
    /* Already this size ?? Nothing to do */
    if(frame->width == ww)
        return meTRUE;
    
    meFrameLoopBegin() ;
    
    /* only process frames which use the same screen window */
    meFrameLoopContinue(loopFrame->mainId != frame->mainId) ;
    
    /* Only process if the window size is different from the current
     * window size. If we got here that is true, */
    if(ww > loopFrame->widthMax)
    {
        /* Must extend the length of loopFrame->mlLine, loopFrame->mlLineStore, and all window
         * mode lines */
        meWindow *wp ;                /* Temporary window pointer */
        meFrameLine *flp;             /* Frame line pointer */
        meFrameLine fl;               /* Temporary frame line */
        int ii, jj;                   /* Local loop counters */
        meLine *ml ;
        meUByte *mls ;

        if(((ml = meLineMalloc(ww,0)) == NULL) ||
           ((mls = meMalloc(ww+1)) == NULL))
            return meFALSE ;
            
        /* Fix up the frame store by growing the lines. Do a safe
         * grow where by we can recover if a malloc fails. */
        for (flp = loopFrame->store, ii = 0; ii < loopFrame->depthMax; ii++, flp++)
        {
            if ((fl.scheme = meMalloc(ww*(sizeof(meUByte)+sizeof(meStyle)))) == NULL)
                return meFALSE ;
            fl.text = (meUByte *) (fl.scheme+ww) ;
#if defined(_WIN32) && defined(_ME_WINDOW)
            if ((fl.wtext = (unsigned short *) meMalloc(ww*sizeof(unsigned short))) == NULL)
                return meFALSE ;
#endif

            /* Data structures allocated. Copy accross the new screen
             * information and pad endings with valid data. Strictly we
             * do not need to do this for all platforms, however if it
             * is safer if we make sure the data is valid. Resize is an
             * infrequent operation and time is not critical here */
            memcpy(fl.text, flp->text, sizeof(meUByte) * loopFrame->widthMax);
            memcpy(fl.scheme, flp->scheme, sizeof(meScheme) * loopFrame->widthMax);
#if defined(_WIN32) && defined(_ME_WINDOW)
            if(flp->wtext != NULL)
                memcpy(fl.wtext, flp->wtext, sizeof(unsigned short) * loopFrame->widthMax);
#endif
            jj = ww ;
            while(--jj >= loopFrame->widthMax)
            {
                fl.text[jj] = ' ' ;
                fl.scheme[jj] = globScheme ;
#if defined(_WIN32) && defined(_ME_WINDOW)
                fl.wtext[jj] = ' ' ;
#endif
            }
            /* Free off old data and copy in new */
            meFree (flp->scheme);
#if defined(_WIN32) && defined(_ME_WINDOW)
            if(flp->wtext != NULL)
                meFree(flp->wtext);
#endif
            flp->text = fl.text;
            flp->scheme = fl.scheme;
#if defined(_WIN32) && defined(_ME_WINDOW)
            flp->wtext = fl.wtext;
#endif
        }
        /* Fix up the window structures */
        memcpy(ml,loopFrame->mlLine,meLINE_SIZE+loopFrame->mlLine->length) ;
        if(loopFrame->mlStatus & MLSTATUS_KEEP)
        {
            meStrcpy(mls,loopFrame->mlLine->text);
            loopFrame->mlColumnStore = loopFrame->mlColumn ;
            loopFrame->mlStatus = (loopFrame->mlStatus & ~MLSTATUS_KEEP) | MLSTATUS_RESTORE ;
        }
        else if(loopFrame->mlStatus & MLSTATUS_RESTORE)
            meStrcpy(mls,loopFrame->mlLineStore) ;
        
        free(loopFrame->mlLine) ;
        free(loopFrame->mlLineStore) ;
        loopFrame->video.lineArray[loopFrame->depth].line = (loopFrame->mlLine = ml) ;
        loopFrame->mlLineStore = mls ;
        
        wp = loopFrame->windowList ;
        while(wp != NULL)
        {
            if((ml = meLineMalloc(ww,0)) == NULL)
                return meFALSE ;
            memcpy(ml,wp->modeLine,meLINE_SIZE+wp->modeLine->length) ;
            free(wp->modeLine) ;
            wp->modeLine = ml ;
            wp = wp->next ;
        }
        loopFrame->widthMax = ww ;
    }

    loopFrame->width = ww ;
    
    /* Fix up the windows */
#if MEOPT_FRAME
    {
        meFrame *fc=frameCur ;
        frameCur = loopFrame ;
        meFrameResizeWindows(frameCur,meFRAMERESIZEWIN_WIDTH);
        frameCur = fc ;
    }
#else
    meFrameResizeWindows(frameCur,meFRAMERESIZEWIN_WIDTH);
#endif
    
    meFrameLoopEnd() ;
    return meTRUE ;
}


int
meFrameChangeDepth(meFrame *frame, int dd)
{
    /* ensure the value is in range */
    if(dd < 4)
        dd = 4 ;
    else if(dd > 400)
        dd = 400 ;
        
    /* Already this size ?? Nothing to do */
    if((frame->depth+1) == dd)
        return meTRUE;
    
    meFrameLoopBegin() ;
    
    /* only process frames which use the same screen window */
    meFrameLoopContinue(loopFrame->mainId != frame->mainId) ;
    
    /* Only process if the window size is different from the current
     * window size. If we got here that is true. 
     * Go and get some more screen space */
    if (dd > loopFrame->depthMax)
    {
        meFrameLine *flp;             /* Temporary frame line */
        int ii, jj;                 /* Local loop counter */
        
        if (meFrameEnlargeVideo(loopFrame,dd) == meFALSE)
            return meFALSE ;
            
        /* Grow the Frame store depthwise do this safely so that 
         * we do not cause a crash at the video end. 
         *
         * Grow the frame store first. Reallocate and then
         * copy across the old information. 
         */
        if ((flp = (meFrameLine *) meMalloc (sizeof (meFrameLine) * dd)) == NULL)
            return meFALSE ;
        
        memcpy (flp, loopFrame->store, sizeof (meFrameLine) * loopFrame->depthMax);
        meFree (loopFrame->store);        /* Free off old store */
        loopFrame->store = flp;           /* Re-assign */
        
        /* Allocate a new set of lines for the remainder of the space */
        for (flp += loopFrame->depthMax, ii = loopFrame->depthMax; ii < dd; ii++, flp++)
        {
            if ((flp->scheme = meMalloc(loopFrame->widthMax*(sizeof(meUByte)+sizeof(meScheme)))) == NULL)
                return meFALSE ;

            flp->text = (meUByte *) (flp->scheme+loopFrame->widthMax) ;
#if defined(_WIN32) && defined(_ME_WINDOW)
            if ((flp->wtext = (unsigned short *) meMalloc(loopFrame->widthMax*sizeof(unsigned short))) == NULL)
                return meFALSE ;
#endif
            /* Initialise the data to something valid */
            jj = loopFrame->widthMax ;
            while(--jj >= 0)
            {
                flp->text[jj] = ' ' ;
                flp->scheme[jj] = globScheme ;
#if defined(_WIN32) && defined(_ME_WINDOW)
                flp->wtext[jj] = ' ' ;
#endif
            }
        }
        loopFrame->depthMax = dd ;
    }

    /* Fix up the message line by binding to the new video frame */
    loopFrame->video.lineArray[loopFrame->depth].flag = 0 ;    /* Decouple the old one */
    loopFrame->video.lineArray[loopFrame->depth].line = NULL ;
    loopFrame->video.lineArray[dd-1].flag = VFMESSL;    /* Bind in new message line */
    loopFrame->video.lineArray[dd-1].line = loopFrame->mlLine ;
    loopFrame->video.lineArray[dd-1].endp = loopFrame->width;
    loopFrame->depth = dd-1 ;                       /* Set up global number of rows */

#if MEOPT_OSD
    if (loopFrame->menuDepth > 0)
        loopFrame->video.lineArray[0].line = loopFrame->menuLine;
#endif
    
    /* Fix up the windows */
#if MEOPT_FRAME
    {
        meFrame *fc=frameCur ;
        frameCur = loopFrame ;
        meFrameResizeWindows(frameCur,meFRAMERESIZEWIN_DEPTH);
        frameCur = fc ;
    }
#else
    meFrameResizeWindows(frameCur,meFRAMERESIZEWIN_DEPTH);
#endif
    
    meFrameLoopEnd() ;

    return meTRUE ;
}

/*
 * frameChangeWidth - Change the width of the screen.
 * Resize the screen, re-writing the screen
 */

int
frameChangeWidth(int f, int n)
{
    /* if no argument is given then prompt for the new width */
    if (f == meFALSE)
    {
        meUByte buff[meSBUF_SIZE_MAX] ;
        
        if (meGetString((meUByte *)"New width", 0, 0, buff, meSBUF_SIZE_MAX) <= 0) 
            return meFALSE ;
        n = meAtoi(buff) ;
    }
    else
        /* n is a delta, add this to the current width to get the new width */
        n = frameCur->width + n ;
    
    if ((n < 8) || (n > 400))           /* In range ?? */
        return mlwrite(MWABORT,(meUByte *)"Screen width %d out of range", n);
    if (n == frameCur->width)                    /* Already this size ?? */
        return meTRUE;
    
    if(meFrameChangeWidth(frameCur,n) <= 0)
        return meFALSE ;

#ifdef _WINDOW
    meFrameSetWindowSize(frameCur) ;    /* Change the size of the window */
#endif
    return(meTRUE);
}

/*
 * frameChangeDepth  - Change the depth of the screen.
 * Resize the screen, re-writing the screen
 */
int
frameChangeDepth(int f, int n)
{
    /* if no argument is given then prompt for the new depth */
    if (f == meFALSE)
    {
        meUByte buff[meSBUF_SIZE_MAX] ;
        
        if (meGetString((meUByte *)"New depth", 0, 0, buff, meSBUF_SIZE_MAX) <= 0) 
            return meFALSE ;
        n = meAtoi(buff) ;
    }
    else
        /* n is a delta, add this to the current width to get the new width */
        n = frameCur->depth + 1 + n ;
    if ((n < 4) || (n > 400))           /* Argument in range ?? */
        return mlwrite(MWABORT,(meUByte *)"[Screen depth %d out of range]", n);
    if (n == (frameCur->depth+1))
        return meTRUE;                    /* Already the right size */
    
    if(meFrameChangeDepth(frameCur,n) <= 0)
        return meFALSE ;
    
#ifdef _WINDOW
    meFrameSetWindowSize(frameCur) ;    /* Change the size of the window */
#endif
    return meTRUE ;
}

/* initialize a frame, mallocing required video and framestore space.
 * 
 * The frame depthMax and widthMax must be set */
meFrame *
meFrameInit(meFrame *sibling)
{
    meFrame *frame ;
    meFrameLine *flp;                     /* Frame store line pointer */
    int ii, jj ;                        /* Local loop counters */
    
    /* add 2 to hilBlockS to allow for a double trunc-scheme change */
    if((frame = meMalloc(sizeof(meFrame))) == NULL)
        return NULL ;
    memset(frame,0,sizeof(meFrame)) ;
    
#if MEOPT_FRAME
    if(sibling != NULL)
    {
        frame->mainId = sibling->mainId ;
        frame->width = sibling->width ;
        frame->depth = sibling->depth ;
        frame->widthMax = sibling->widthMax ;
        frame->depthMax = sibling->depthMax ;
    }
    else
#else
    meAssert(sibling == NULL) ;
#endif
    {
        frame->mainId = 0 ;
#if MEOPT_FRAME
        {
            meFrame *ff = frameList ;
            while(ff != NULL)
            {
                if(ff->mainId == frame->mainId)
                {
                    if(frame->mainId == 255)
                    {
                        free(frame) ;
                        return NULL ;
                    }
                    frame->mainId++ ;
                    ff = frameList ;
                }
                ff = ff->next ;
            }
        }
#endif
        frame->width = TTwidthDefault ;
        frame->depth = TTdepthDefault-1 ;
        frame->widthMax = TTwidthDefault ;
        frame->depthMax = TTdepthDefault ;
    }
    if(meFrameTermInit(frame,sibling) <= 0)
        return NULL ;
    
    if(((frame->video.lineArray = meMalloc(frame->depthMax * sizeof(meVideoLine))) == NULL) ||
       ((frame->mlLine = meLineMalloc(frame->widthMax,0)) == NULL) ||
       ((frame->mlLineStore = meMalloc(frame->widthMax+1)) == NULL))
        return NULL ;

    /* Initialise the virtual video structure. */
    memset(frame->video.lineArray,0,frame->depthMax*sizeof(meVideoLine)) ;
    frame->video.lineArray[frame->depth].flag = VFMESSL ;
    frame->video.lineArray[frame->depth].line = frame->mlLine ;
    /* Reset the video list block */
    frame->video.next = NULL;                 /* No next block */
    frame->video.window = NULL;               /* No windows attached */

    memset(frame->mlLine->text,'\0',frame->widthMax) ;
    frame->mlLineStore[0] = '\0';
#if MEOPT_EXTENDED
    frame->id = ++nextFrameId ;
#endif

    /* Frame Store storage
     * The frame store hold's 'n' lines of video information.
     * The frame->store is an array of points to meFrameLine structures
     * which hold the physical line contents.
     *
     * Allocate lines in the frame store to hold the video information.
     */
    if((frame->store = meMalloc(sizeof(meFrameLine) * frame->depthMax)) == NULL)
        return NULL ;
    for (flp = frame->store, ii = 0; ii < frame->depthMax; ii++, flp++)
    {
        if ((flp->scheme = meMalloc(frame->widthMax*(sizeof(meUByte)+sizeof(meStyle)))) == NULL)
            return NULL ;
        flp->text = (meUByte *) (flp->scheme+frame->widthMax) ;
#if defined(_WIN32) && defined(_ME_WINDOW)
        if ((flp->wtext = (unsigned short *) meMalloc(frame->widthMax*sizeof(unsigned short))) == NULL)
            return NULL ;
#endif
        /* Fill with data */
        jj = frame->widthMax ;
        while(--jj >= 0)
        {
            flp->text[jj] = ' ' ;
            flp->scheme[jj] = meSCHEME_NDEFAULT ;
#if defined(_WIN32) && defined(_ME_WINDOW)
            flp->wtext[jj] = ' ' ;
#endif
        }
    }
    return frame ;
}

/* meFrameInitWindow
 * 
 * Creates the initial window for a frame, would be part of
 * meFrameInit except the order of events at startup make this
 * impossible (no buffer)
 */
int
meFrameInitWindow(meFrame *frame, meBuffer *buffer)
{
    meWindow *wp;
    meLine   *lp, *off;
    
    if(((wp = meMalloc(sizeof(meWindow))) == NULL) ||
       ((lp = meLineMalloc(frame->widthMax,0)) == NULL) ||
       ((off= meLineMalloc(frame->widthMax,0)) == NULL))
        return meFALSE ;
    
    frame->bufferCur   = buffer ;              /* Make this current    */
    frame->windowList  = wp ;
    frame->windowCur   = wp ;
    frame->windowCount = 1 ;
    memset(wp,0,sizeof(meWindow)) ;
    /* Window # columns & rows */
    wp->width = frame->width;
    wp->depth = frame->depth;
    wp->modeLine = lp ;
    off->next = NULL ;
    wp->dotCharOffset= off ;
    wp->buffer = buffer ;
    wp->dotLine = buffer->baseLine ;
    wp->updateFlags = WFMODE|WFRESIZE|WFSBAR;
#if MEOPT_EXTENDED
    wp->id = ++nextWindowId ;
#endif
    /* Flag buffer as displayed. */
    buffer->windowCount++ ;
    meWindowFixTextSize(wp) ;
    meVideoAttach(&(frame->video), wp) ;
    return meTRUE ;
}

#if MEOPT_FRAME ||  (defined _ME_FREE_ALL_MEMORY)
void
meFrameFree(meFrame *frame)
{
    meFrame     *ff ;
    meFrameLine *flp ;
    meWindow    *wp ;
    meVideo    *vvptr;
    int        ii ;
    
    /* take frame out of the list */
    if(frame == frameList)
        frameList = frame->next ;
    else
    {
        ff = frameList ;
        while(ff->next != frame)
            ff = ff->next ;
        ff->next = frame->next ;
    }
#if MEOPT_MWFRAME
    if(frameFocus == frame)
        frameFocus = NULL ;
#endif
    
    /* try to find a sibling of this frame */
    ff = frameList ;
    while((ff != NULL) && (ff->mainId != frame->mainId))
        ff = ff->next ;
    
    meFrameTermFree(frame,ff) ;
    
    while (frame->windowList != NULL)
    {
        wp = frame->windowList ;
        frame->windowList = wp->next ;
        if(--wp->buffer->windowCount == 0)
        {
            storeWindBSet(wp->buffer,wp) ;
            wp->buffer->histNo = bufHistNo ;
        }
        meFree(wp->modeLine) ;
        meFree(wp->dotCharOffset) ;
        meFree(wp);
    }
    /* Destruct all virtual video blocks */
    for(vvptr=frame->video.next; vvptr != NULL; vvptr = frame->video.next)
    {
        frame->video.next = vvptr->next;
        meFree (vvptr->lineArray);
        meFree (vvptr);
    }
    meFree(frame->video.lineArray);
    for(flp=frame->store,ii=0; ii<frame->depthMax; ii++, flp++)
    {
#if defined(_WIN32) && defined(_ME_WINDOW)
        if(flp->wtext != NULL)
            meFree(flp->wtext) ;
#endif
        meFree(flp->scheme) ;
    }
    meFree(frame->mlLine) ;
    meFree(frame->mlLineStore) ;
    meFree(frame->store) ;
    meFree(frame) ;
}
#endif

#if MEOPT_FRAME
int
meFrameDelete(meFrame *frame, int flags)
{
    /* use next frame to get a replacement frame if required, always try to get an
     * internal (sibling) replacement frame as if one exists then the window
     * must not be deleted */
    if((flags & 1) && (frame == frameCur))
        frameNext(meTRUE,1) ;
    if((frame == frameCur) ||
       (((flags & 1) == 0) && (frame->mainId == frameCur->mainId)))
    {
        if(flags & 2)
            frameNext(meTRUE,2) ;
        if(frame->mainId == frameCur->mainId)
        {
            /* no replacement could be found! */
            if((flags & 4) == 0)
                mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Last frame]") ;
            return meABORT ;
        }
    }
    
    if(frame->mainId != frameCur->mainId)
    {
        meFrame *nf ;
        meUByte mainId=frame->mainId ;
        
        frame = frameList ;
        while(frame != NULL)
        {
            nf = frame->next ;
            if(frame->mainId == mainId)
                meFrameFree(frame) ;
            frame = nf ;
        }
    }
    else
        meFrameFree(frame) ;
    
    return meTRUE ;
}

/*
 * Make the frame current.
 * 
 * The quiet flag performs a frame swap but does not call the terminal driver
 * to raise the window. This is used in the UNIX drag and drop so that the
 * desktop window order is not changed while the drag and drop files are being
 * inserted into the appropriate window(s).
 */
void
meFrameMakeCur(meFrame *frame, int quiet)
{
    if(frame != frameCur)
    {
        meFrame *frameOld=frameCur ;
        
        /* if frame is not the current frame in this window then make it */
        if(frame->flags & meFRAME_HIDDEN)
        {
            meFrame *ff ;
            ff = frameList ;
            while((ff->mainId != frame->mainId) ||
                  (ff->flags & meFRAME_HIDDEN))
                ff = ff->next ;
            frame->flags = (frame->flags & ~(meFRAME_HIDDEN|meFRAME_NOT_FOCUS)) |
                      (ff->flags & meFRAME_NOT_FOCUS) ;
            ff->flags |= meFRAME_HIDDEN ;
        }            
        frameCur = frame ;
        sgarbf = meTRUE ;                      /* Garbage the screen */
        if(frameOld->mlStatus & (MLSTATUS_RESTORE|MLSTATUS_KEEP))
        {
            meUByte mlStatus ;
            meUByte *mlStr ;
            
            /* move the current mlstatus over to the new frame, this should
             * usually be zero but in some cases (e.g. notes) the swapping of
             * frames can occur during a ml prompt. There are potantial side
             * effects to doing this (e.g. what if its an OSD dialog and the
             * position is outside the the new frame), so this should be avoid if
             * at all possible */
            if(frameOld->mlStatus & MLSTATUS_RESTORE)
            {
                frame->mlColumn = frameOld->mlColumnStore ;
                mlStatus = (frameOld->mlStatus & ~MLSTATUS_RESTORE) | MLSTATUS_KEEP ;
                mlStr = frameOld->mlLineStore ;
            }
            else if(frameOld->mlStatus & MLSTATUS_KEEP)
            {
                frame->mlColumn = frameOld->mlColumn ;
                mlStatus = frameOld->mlStatus ;
                mlStr = frameOld->mlLine->text ;
            }
            frame->mlStatus = 0 ;
            mlwrite(((mlStatus & MLSTATUS_POSML) ? MWSPEC|MWUSEMLCOL|MWCURSOR:MWSPEC),mlStr) ;
            frame->mlStatus = mlStatus ;
        }
        frameOld->mlStatus = 0 ;
        if(frameOld->mlLine->length > 0)
        {
            /* erase the ml line of old current frame */
            frameOld->mlLine->text[0] = '\0' ;
            frameOld->mlLine->length = 0 ;
            frameOld->mlLine->flag |= meLINE_CHANGED ;
        }
#if MEOPT_MWFRAME
        if(frame->mainId == frameOld->mainId)
            frameFocus = frame ;
        else if(!quiet)
            /* A quiet operation will not change the terminal to perform the swap */
            meFrameTermMakeCur(frame) ;
#endif
    }
}

int
frameCreate(int f, int n)
{
    meFrame *frame, *sf ;
    int menuDepth ;
    
#if MEOPT_MWFRAME
    if(!f || (n & 2))
        sf = NULL ;
    else if(n & 1)
        sf = frameCur ;
    else
        return ctrlg(0,1) ;
#else
    if((n & 1) == 0)
        return ctrlg(0,1) ;
    sf = frameCur ;
#endif
    
    /* these functions cannot use meMalloc as they are used to init ME,
     * so if they fail we must warn of the malloc failure */
    if(((frame = meFrameInit(sf)) == NULL) ||
       (meFrameInitWindow(frame,frameCur->bufferCur) <= 0))
        return mlwrite(MWCURSOR|MWABORT|MWWAIT,(meUByte *)"[Failed to create new frame]") ;
#if MEOPT_MWFRAME
    if(sf != NULL)
#endif
        frame->flags |= meFRAME_HIDDEN ;
    menuDepth = frameCur->menuDepth ;
    frame->next = frameCur->next ;
    restoreWindWSet(frame->windowCur,frameCur->windowCur) ;
    frameCur->next = frame ;
    meFrameMakeCur(frame, 0) ;
    if(menuDepth)
        frameSetupMenuLine(menuDepth) ;
    sgarbf = meTRUE;                      /* Garbage the screen */
    
    return meTRUE ;
}

int
frameDelete(int f, int n)
{
    if(f == meFALSE)
        n = 3 ;
    return meFrameDelete(frameCur,n) ;
}

int
frameNext(int f, int n)
{
    meFrame *frame=frameCur ;
    
    if(f == meFALSE)
        n = 2 ;
    
    for(;;)
    {
        if((frame = frame->next) == NULL)
            frame = frameList ;
        if(frame == frameCur)
        {
            if(f)
                return meTRUE ;
            /* failed to find an external, try again */
            f = 1 ;
            n = 1 ;
        }
        else if(((n & 0x03) == 3) ||
                ((n & 0x01) && (frame->mainId == frameCur->mainId)) ||
                ((n & 0x02) && (frame->mainId != frameCur->mainId) &&
                 ((frame->flags & meFRAME_HIDDEN) == 0)))
            break ;
    }
    meFrameMakeCur(frame, 0) ;
    return meTRUE ;
}

#endif

