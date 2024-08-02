/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * buffer.c - Buffer operations.
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
 * Synopsis:    Buffer operations.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     Buffer management.
 *     Some of the functions are internal, and some are actually attached to user
 *     keys. Like everyone else, they set hints for the display system.
 */

#define __BUFFERC                       /* Define filename */

#include "emain.h"
#include "efunc.h"
#include "esearch.h"

int
getBufferName(meUByte *prompt, int opt, int defH, meUByte *buf)
{
    /* Only setup the history to a specific buffer if we're
     * told to and we're not running a macro
     */
    if(defH && (clexec != meTRUE))
    {
        register meBuffer *bp ;
        if(defH == 1)
            bp = frameCur->bufferCur ;
        else
        {
            bp = replacebuffer(frameCur->bufferCur) ;
            defH = 1 ;
        }
        addHistory(MLBUFFER, bp->name) ;
    }
    return meGetString(prompt,opt|MLBUFFERCASE,defH,buf,meBUF_SIZE_MAX) ;
}

#if MEOPT_FILEHOOK
static meUByte    fileHookCount ;
static meUByte  **fileHookExt ;
static meUByte  **fileHookFunc ;
static meShort   *fileHookArg ;
static meUByte    defaultHookName[] = "fhook-default" ;
static meUByte    binaryHookName[]  = "fhook-binary" ;
static meUByte    rbinHookName[]    = "fhook-rbin" ;
static meUByte    triedDefault=0 ;
static meUByte    triedBinary=0 ;
static meUByte    triedRbin=0 ;

int
addFileHook(int f, int n)
{
    if(n == 0)
    {
        /* arg of 0 frees all hooks off */
        if(fileHookCount)
        {
            n = fileHookCount ;
            while(--n >= 0)
            {
                free(fileHookExt[n]) ;
                free(fileHookFunc[n]) ;
            }
            free(fileHookExt) ;
            free(fileHookFunc) ;
            free(fileHookArg) ;
            fileHookCount=0 ;
            fileHookExt=NULL ;
            fileHookFunc=NULL ;
            fileHookArg=NULL ;
        }
    }
    else
    {
        meUByte buff[meBUF_SIZE_MAX] ;
    
        if(((fileHookExt = meRealloc(fileHookExt,(fileHookCount+1)*sizeof(char *))) == NULL) ||
           ((fileHookFunc = meRealloc(fileHookFunc,(fileHookCount+1)*sizeof(char *))) == NULL) ||
           ((fileHookArg = meRealloc(fileHookArg,(fileHookCount+1)*sizeof(short))) == NULL))
        {
            fileHookCount=0 ;
            return meFALSE ;
        }
        if((meGetString((meUByte *)"File id",0,0,buff,meBUF_SIZE_MAX) <= 0) ||
           ((fileHookExt[fileHookCount] = meStrdup(buff)) == NULL) ||
           (meGetString((meUByte *)"Hook func",0,0,buff,meBUF_SIZE_MAX) <= 0) ||
           ((fileHookFunc[fileHookCount] = meStrdup(buff)) == NULL))
            return meFALSE ;
        if(f == meFALSE)
            fileHookArg[fileHookCount] = 0 ;
        else
            fileHookArg[fileHookCount] = n ;
        fileHookCount++ ;
    }
    return meTRUE ;
}

/* Check extent.
   Check the extent string for the filename extent. */

static int
checkExtent(meUByte *filename, int len, meUByte *sptr, meIFuncSSI cmpFunc)
{
    int  extLen ;
    char cc ;
    
    if(sptr == NULL)
        return 0 ;              /* Exit - fail */
    filename += len ;
    /*---       Run through the list, checking the extents as we go. */
    for(;;)
    {
        while(*sptr == ' ')
            sptr++ ;
        extLen = 0 ;
        while(((cc=sptr[extLen]) != ' ') && ((cc=sptr[extLen]) != '\0'))
            extLen++ ;
        if((extLen <= len) &&
           !cmpFunc(sptr,filename-extLen,extLen))
            return 1 ;
        if(cc == '\0')
            return 0 ;
        sptr += extLen+1 ;
    }
}

/*
 * assignHooks.
 * Given a buffer and a hook name, determine the file
 * hooks associated with the file and assign.
 *
 * Note that the hookname (hooknm) is modified, ultimatly
 * returning the 'f' prefixed name.
 */
static void
assignHooks (meBuffer *bp, meUByte *hooknm)
{
    int fhook;                          /* File hook indentity */

    /* Get the identity of the hook */
    fhook = decode_fncname(hooknm,1);

    /* Get the file into memory if we have to */
    if((fhook == -1) &&
       ((hooknm != defaultHookName) || !triedDefault) &&
       ((hooknm != binaryHookName) || !triedBinary) &&
       ((hooknm != rbinHookName) || !triedRbin) &&
       (meStrlen(hooknm) > 6))
    {
        meUByte fn[meBUF_SIZE_MAX], buff[meBUF_SIZE_MAX] ;             /* Temporary buffer */

	buff[0] = 'h' ;
	buff[1] = 'k' ;
	meStrcpy(buff+2,hooknm+6) ;
	if(!fileLookup(buff,(meUByte *)".emf",meFL_CHECKDOT|meFL_USESRCHPATH,fn))
	{
	    if(hooknm == defaultHookName)
		triedDefault++ ;
	    else if(hooknm == binaryHookName)
		triedBinary++ ;
	    else if(hooknm == rbinHookName)
		triedRbin++ ;
	    else
		mlwrite(MWABORT|MWWAIT,(meUByte *)"Failed to find file [%s]",buff);
	}
	else
	{
	    execFile(fn,0,1) ;
	    bp->fhook = decode_fncname(hooknm,1) ;
	}
    }
    if((bp->fhook = decode_fncname(hooknm,1)) >= 0)
        resultStr[0] = '\0' ;
    *hooknm   = 'b' ;
    bp->bhook = decode_fncname(hooknm,1) ;
    *hooknm   = 'd' ;
    bp->dhook = decode_fncname(hooknm,1) ;
    *hooknm   = 'e' ;
    bp->ehook = decode_fncname(hooknm,1) ;
    *hooknm   = 'f';
    return;
}

/*
 * findBufferContext - buffer context hook identification.
 *
 * Search for buffer context hook information this informs MicroEmacs
 * at the last knockings what buffer hook it should be using. This
 * over-rides the file extension buffer hook which is used as a
 * fall-back.
 *
 * The first line is searched for magic strings embedded in the text
 * performing a case insensitive search for a magic string as defined
 * by add-magic-hook. This is higher priority than the file extension, but
 * lower priority than than the explit mode string.
 *
 * Search for information on the first non-blank line of the form
 * "-*- name -*-" or "-*- mode: name -*-" in much the same way as
 * GNU Emacs. There are so many files in Linux with these attributes
 * we may as well use them.
 */
void
setBufferContext(meBuffer *bp)
{
    int ii ;
    
#if MEOPT_COLOR
    /* First setup the global scheme - this can be missed by buffers loaded with -c */
    bp->scheme = globScheme;
#endif
    
    /* Search the buffer for alternative hook information */
    if ((bp->intFlag & BIFFILE) && !meModeTest(bp->mode,MDBINARY) &&
        !meModeTest(bp->mode,MDRBIN) && ((bp->fileFlag & meBFFLAG_DIR) == 0) &&
        ((ii = fileHookCount) > 0))
    {
	meUByte *pp, cc ;
	meLine *lp, *tlp ;
	int nn ;
	
        /* search for the first non-blank line */
	for(lp=meLineGetNext(bp->baseLine) ; lp != bp->baseLine ; lp = meLineGetNext(lp))
        {
            for (pp = lp->text; (cc=*pp++) != '\0' ; )
            {
                if(!isSpace(cc))
                {
                    while(--ii >= 0)
                    {
                        if(((nn=fileHookArg[ii]) != 0) &&
                           (meRegexComp(&meRegexStrCmp,fileHookExt[ii],(nn < 0) ? meREGEX_ICASE:0) == meREGEX_OKAY))
                        {
                            if(nn < 0)
                                nn = -nn ;
                            tlp = lp ;
                            do {
                                if(meRegexMatch(&meRegexStrCmp,meLineGetText(tlp),
                                                meLineGetLength(tlp),0,meLineGetLength(tlp),0))
                                {
                                    assignHooks(bp,fileHookFunc[ii]) ;
                                    if(bp->fhook >= 0)
                                    {
                                        ii = meRegexStrCmp.group[0].end - meRegexStrCmp.group[0].start ;
                                        if(ii >= meBUF_SIZE_MAX)
                                            ii = meBUF_SIZE_MAX - 1 ;
                                        meStrncpy(resultStr,meLineGetText(tlp)+
                                                  meRegexStrCmp.group[0].start,ii) ;
                                        resultStr[ii] = '\0' ;
                                        ii = 0 ;
                                        break ;
                                    }
                                }
                            } while((--nn > 0) && ((tlp=meLineGetNext(tlp)) != bp->baseLine)) ; 
                        }
                    }
                    break ;
                }
            }
        }
    }
    if(bp->fhook < 0)
    {
	meUByte *hooknm ;
	
	/* Do file hooks */
	if(meModeTest(bp->mode,MDBINARY))
	    hooknm = binaryHookName ;
	else if(meModeTest(bp->mode,MDRBIN))
	    hooknm = rbinHookName ;
	else
	{
	    meUByte *sp, *bn ;
	    int ll, bnll ;
	    
            /* Find the length of the string to pass into check_extension.
             * Check if its name has a <?> and/or a backup ~, if so reduce
             * the length so '<?>' & '~'s not inc. */
            sp = bp->name ;
            ll = meStrlen(sp) ;
	    if((sp[ll-1] == '>') && (bp->fileName != NULL) &&
               ((bn = getFileBaseName(bp->fileName)) != NULL) &&
               ((bnll = meStrlen(bn)) > 0) && (ll > bnll) &&
               (sp[bnll] == '<')  && !meStrncmp(sp,bn,bnll))
                ll = bnll ;
	    if(sp[ll-1] == '~')
		ll-- ;
            if(ll)
            {
                meUByte cc = sp[ll-1] ;
                if(cc == '~')
                {
                    ll-- ;
                    if((ll > 2) && (sp[ll-1] == '~') && (sp[ll-2] == '.'))
                        ll -= 2 ;
                }
                else if(isDigit(cc))
                {
                    int ii=ll-2 ;
                    while(ii > 0)
                    {
                        cc = sp[ii--] ;
                        if(!isDigit(cc))
                        {
                            if((cc == '~') && (sp[ii] == '.'))
                                ll = ii ;
                            break ;
                        }
                    }
                }
            }
	    ii = fileHookCount ;
	    while(--ii >= 0)
		if((fileHookArg[ii] == 0) &&
		   checkExtent(sp,ll,fileHookExt[ii],
#ifdef _INSENSE_CASE
			       meStrnicmp
#else
			       (meIFuncSSI) strncmp
#endif
			       ))
	    {
		hooknm = fileHookFunc[ii] ;
		break ;
	    }
	    if(ii < 0)
		hooknm = defaultHookName ;
	}
	assignHooks(bp,hooknm) ;
    }
    if(bp->fhook >= 0)
        execBufferFunc(bp,bp->fhook,meEBF_ARG_GIVEN,(bp->intFlag & BIFFILE)) ;
}
#endif

int
swbuffer(meWindow *wp, meBuffer *bp)        /* make buffer BP current */
{
    meWindow *twp ;
    meBuffer *tbp ;
    meInt     dotLineNo=0, markLineNo ;
    meUShort  dotCol, markCol ;
    int       reload=0 ;
    
    if(meModeTest(bp->mode,MDNACT) && ((bp->intFlag & BIFNACT) == 0))
    {   
	/* buffer not active yet */
	if(bp->intFlag & BIFFILE)
	{
	    /* The buffer is linked to a file, load it in */
	    dotLineNo = bp->dotLineNo ;
            dotCol = bp->dotOffset;
	    markLineNo = bp->markLineNo ;
            markCol = bp->markOffset;
	    bp->dotLineNo = 0 ;
            bp->dotOffset = 0;
	    bp->markLineNo = 0 ;
            bp->markOffset = 0;
	    readin(bp,bp->fileName);
            /* if this is also the current buffer then we have an out-of-date
             * buffer being reloaded. We must be careful of what hooks we
             * execute and the state stored */
            if((reload = (wp->buffer == bp)) != 0)
            {
               /* the buffer was empty, it now probably isn't and the window
                * is wrong (dotLine is the base, lineNo is 0). Must correct this
                * before the call to setBufferContext */
                meAssert(wp->markLine == NULL) ;
                meAssert(wp->dotLineNo == 0) ;
                wp->dotLine = meLineGetNext(bp->baseLine) ;
            }
	}
	meModeClear(bp->mode,MDNACT) ;
#if MEOPT_FILEHOOK
	/* Now set the buffer context */
        if((bp->intFlag & BIFNOHOOK) == 0)
        {
            setBufferContext(bp) ;
            /* the buffer's fhook could have done some really whacky things like
             * delete the buffer. If it has avoid crashing by checking the buffer
             * is still a buffer! */
            tbp = bheadp ;
            while(tbp != bp)
                if((tbp=tbp->next) == NULL)
                    return meFALSE ;
        }
#endif
    }
    tbp = wp->buffer ;
#if MEOPT_FILEHOOK
    /* If this is an out-of-date reload find-file, the call to bclear will
     * have executed the ehook and dhook so do do this now. */
    if((wp == frameCur->windowCur) && !reload && (frameCur->bufferCur->ehook >= 0))
    {
        execBufferFunc(frameCur->bufferCur,frameCur->bufferCur->ehook,0,1) ;
        if(wp->buffer != tbp)
            return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Error: Buffer ehook has altered windows]") ;
    }
#endif
    tbp->histNo = ++bufHistNo ;
    tbp->intFlag &= ~BIFNACT ;
    if((--tbp->windowCount == 0) && !reload)
    {
        /* don't do this on a reload as the bclear means the window state is
         * rubbish! */
        storeWindBSet(tbp,wp) ;
    }
    
    /* Switch. reset any scroll */
    wp->buffer = bp ;
    wp->horzScrollRest = 0 ;
    wp->horzScroll = 0 ;
    wp->updateFlags |= WFMODE|WFMAIN|WFSBOX ;

    if(wp == frameCur->windowCur)
    {
        frameCur->bufferCur = bp;
#if MEOPT_EXTENDED
        if(isWordMask != frameCur->bufferCur->isWordMask)
        {
            isWordMask = frameCur->bufferCur->isWordMask ;
#if MEOPT_MAGIC
            mereRegexClassChanged() ;
#endif
        }
#endif
    }
    if((bp->windowCount++ == 0) || reload)
    {
	/* First use.           */
	restoreWindBSet(wp,bp) ;
        if(meModeTest(frameCur->bufferCur->mode,MDPIPE))
        {
            /* if its a piped command we can't rely on the bp markp being correct,
             * reset */
	    wp->markLine = NULL;                 /* Invalidate "mark"    */
	    wp->markOffset = 0;
	    wp->markLineNo = 0;
        }            
	if(dotLineNo > 0)
        {
            if(markLineNo > 0)
            {
                windowGotoLine(meTRUE,markLineNo) ;
                frameCur->windowCur->markLine = frameCur->windowCur->dotLine;
                frameCur->windowCur->markLineNo = frameCur->windowCur->dotLineNo;
                if(markCol > 0)
                {
                    if(markCol > meLineGetLength(frameCur->windowCur->markLine))
                        frameCur->windowCur->markOffset = meLineGetLength(frameCur->windowCur->markLine) ;
                    else
                        frameCur->windowCur->markOffset = markCol ;
                }
            }
	    windowGotoLine(meTRUE,dotLineNo) ;
            if(dotCol > 0)
            {
                if(dotCol > meLineGetLength(frameCur->windowCur->dotLine))
                    frameCur->windowCur->dotOffset = meLineGetLength(frameCur->windowCur->dotLine) ;
                else
                    frameCur->windowCur->dotOffset = dotCol ;
            }
        }
        /* on a reload reset all other windows displaying this buffer to the tob */
        if(reload && (bp->windowCount > 1))
        {
            meFrameLoopBegin() ;
            twp = loopFrame->windowList;
            while(twp != NULL)
            {
                if((twp != wp) && (twp->buffer == bp)) 
                    restoreWindWSet(twp,wp) ;
                twp = twp->next;
            }
            meFrameLoopEnd() ;
        }
    }
    else if(bp != tbp)
    {
        meFrameLoopBegin() ;
	twp = loopFrame->windowList;                            /* Look for old.        */
	while (twp != NULL)
	{
	    if (twp!=wp && twp->buffer==bp) 
	    {
		restoreWindWSet(wp,twp) ;
		break;
	    }
	    twp = twp->next;
	}
        meFrameLoopBreak(twp!=NULL) ;
        meFrameLoopEnd() ;
    }
#if MEOPT_IPIPES
    if(meModeTest(bp->mode,MDPIPE) &&
       ((wp == frameCur->windowCur) || (bp->windowCount == 1)))
	ipipeSetSize(wp,bp) ;
#endif        
#if MEOPT_FILEHOOK
    if((wp == frameCur->windowCur) && (frameCur->bufferCur->bhook >= 0))
        execBufferFunc(frameCur->bufferCur,frameCur->bufferCur->bhook,0,1) ;
#endif        
    
    if((bp->intFlag & (BIFLOAD|BIFNACT)) == BIFLOAD)
    {
        bp->intFlag &= ~BIFLOAD ;
	if(bufferOutOfDate(bp))
	{
            update(meTRUE) ;
            dotLineNo = wp->dotLineNo ;
            if(((bp->fileFlag & meBFFLAG_DIR) || (mlyesno((meUByte *)"File changed on disk, reload") > 0)) &&
               (bclear(bp) > 0))
            {
                /* achieve cheekily by calling itself as the bclear make it inactive,
                 * This ensures the re-evaluation of the hooks etc.
                 */
                /* SWP 7/7/98 - If the file did not exist to start with, i.e. new file,
                 * and now it does then we need to set the BIFFILE flag */
                bp->intFlag |= BIFFILE ;
                bp->dotLineNo = dotLineNo+1 ;
		return swbuffer(wp,bp) ;
	    }
	}
	else
	    mlwrite(MWCLEXEC,(meUByte *)"[Old buffer]");
    }
    return meTRUE ;
}


static int
HideBuffer(meBuffer *bp, int forceAll)
{
    meBuffer *bp1 ;
    meWindow *wp ;
    
    if(bp->windowCount > 0)
    {
#if MEOPT_FRAME
        meFrame *fc=frameCur ;
        for(frameCur=frameList ; frameCur!=NULL ; frameCur=frameCur->next)
        {
#endif
            for(wp = frameCur->windowList; wp != (meWindow*) NULL; wp = wp->next)
            {
                if(wp->buffer == bp)
                {
                    
#if MEOPT_EXTENDED
                    if(forceAll || ((wp->flags & meWINDOW_LOCK_BUFFER) == 0))
#endif
                    {
                        /* find a replacement buffer */
                        /* if its the same then can't replace so return */
                        if((bp1 = replacebuffer(bp)) == bp)
                        {
                            /* this is true if only *scratch* is left, ensure
                             * the buffer hooks are correctly setup and
                             * executed */
#if MEOPT_FILEHOOK
                            setBufferContext(bp) ;
#endif
                            return meFALSE ;
                        }
                        
                        if(swbuffer(wp, bp1) <= 0)
                            return meFALSE ;
                    }
                }
            }
#if MEOPT_FRAME
        }
        frameCur = fc ;
#endif
    }
    bp->histNo = 0 ;
    
    return meTRUE ;
}


/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
int
findBuffer(int f, int n)
{
    meBuffer *bp, *cbp;
    meUByte bufn[meBUF_SIZE_MAX];
    meInt chistNo ;
    int s;

    if((s = getBufferName((meUByte *)"Find buffer",0,2,bufn)) <= 0)
	return s ;
    if(n < 0)
        n = (n < -1) ? 24:8 ;
#if MEOPT_EXTENDED
    if(n & 2)
    {
	bp = bheadp;
	while(bp != NULL)
	{
            if((bp->fileName != NULL) &&
#ifdef _INSENSE_CASE
               (meStricmp(bp->fileName,bufn) == 0)
#else
               (meStrcmp(bp->fileName,bufn) == 0)
#endif
               )
                break ;
	    bp = bp->next;
	}
        if(bp == NULL)
        {
            if(((n & 5) != 5) ||
               ((bp = bfind(bufn,BFND_CREAT)) == NULL))
                return meFALSE ;
            meModeSet(bp->mode,MDNACT) ;
            bp->fileName = meStrdup(bufn) ;
            bp->intFlag |= BIFFILE ;
        }
    }
    else
#endif
    {
        if(n & 1)
            f = BFND_CREAT ;
        else
            f = 0 ;
        if((bp = bfind(bufn,f)) == NULL)
            return meFALSE ;
    }
    if(n & 8)
        return HideBuffer(bp,(n & 16) ? 1:0) ;
    if((n & 4) && meModeTest(bp->mode,MDNACT))
        bp->intFlag |= BIFNACT ;
    if(n & 32)
        chistNo = (cbp = frameCur->bufferCur)->histNo ;
    s = swbuffer(frameCur->windowCur, bp) ;
    if(n & 32)
        cbp->histNo = chistNo ;
    return s ;
}

#if MEOPT_EXTENDED
/*
 * Find a buffer into another window. This is an alternative to mapping
 * ^X2 (ie C-x 2) onto windowSplitDepth().
 *
 *                                      Tony Bamford 31/8/86 tony@root.UUCP
 */
int     
nextWndFindBuf(int f, int n)
{
    meBuffer *bp;
    meUByte bufn[meBUF_SIZE_MAX];
    int s;
    
    if((s = getBufferName((meUByte *)"Use buffer", 0, 2, bufn)) <= 0)
	return(s);
    if(n)
	n = BFND_CREAT ;
    if((meWindowPopup(NULL,WPOP_MKCURR,NULL) == NULL) ||
       ((bp=bfind(bufn,n)) == NULL))
	return meFALSE ;
    return swbuffer(frameCur->windowCur, bp) ;
}
#endif

int     
nextBuffer(int f, int n)   /* switch to the next buffer in the buffer list */
{
    meBuffer *bp, *pbp;
    
    bp = frameCur->bufferCur ;
    if(n < 0)
    {
        /* cycle backward through the buffers to find an eligable one */
        while(n < 0)
        {
            if(bp == bheadp)
                bp = NULL;
            pbp = bheadp ;
            while(pbp->next != bp)
                pbp = pbp->next;
            bp = pbp ;
            if(bp == frameCur->bufferCur)
                break ;
            if(!meModeTest(bp->mode,MDHIDE))
                n++ ;
        }
    }
    else
    {
        /* cycle forwards through the buffers to find an eligable one */
        while(n > 0)
        {
            bp = bp->next;
            if(bp == NULL)
                bp = bheadp;
            if(bp == frameCur->bufferCur)
                break ;
            if(!meModeTest(bp->mode,MDHIDE))
                n-- ;
        }
    }
    if(bp == frameCur->bufferCur)
        return meTRUE ;
    return(swbuffer(frameCur->windowCur,bp));
}


/*
** Find a replacement buffer.
** given a buffer oldbuf (assumed to be displayed), then find the best
** buffer to replace it by. There are 3 main catagories
** 1) undisplayed normal buffer.
** 2) displayed normal buffer.
** 3) default buffer main. (created).
** buffers higher in the history have preference.
** returns a pointer to the replacement (guaranteed not to be the oldbuf).
*/
meBuffer *
replacebuffer(meBuffer *oldbuf)
{
    meBuffer *bp, *best=NULL, *next=NULL ;
    int histNo=-1, nextNo=-1, wc ;
    
    for(bp=bheadp ; bp != NULL;bp = bp->next)
    {
	if((bp != oldbuf) && !meModeTest(bp->mode,MDHIDE))
	{
	    if(bp->histNo > histNo)
            {
                wc = bp->windowCount ;
#if MEOPT_MWFRAME
                if(wc)
                {
                    /* check that the buffer is visible on the current frame */
                    meWindow *wp ;
                    for(wc = 0, wp = frameCur->windowList ; wp != NULL ; wp = wp->next)
                    {
                        if(wp->buffer == bp)
                        {
                            wc = 1 ;
                            break ;
                        }
                    }
                }
#endif
                if(wc == 0)
                {
                    histNo = bp->histNo ;
                    best = bp ;
                }
                else if(nextNo <= bp->histNo)
                {
                    nextNo = bp->histNo ;
                    next = bp ;
                }
            }
	}
    }
    /* if didnt find a buffer, create a new *scratch* */
    if(best == NULL)
	best = next ;
    if(best == NULL)
	best = bfind(BmainN,BFND_CREAT) ;
    
    return best ;
}


void
resetBufferWindows(meBuffer *bp)
{
    register meWindow *wp ;
    
    meFrameLoopBegin() ;
    wp = loopFrame->windowList ;
    while(wp != NULL)
    {
	if(wp->buffer == bp)
	{
            restoreWindBSet(wp,bp) ;
	    wp->updateFlags |= WFMOVEL|WFREDRAW|WFSBOX ;
	}
	wp = wp->next ;
    }   
    meFrameLoopEnd() ;
}
/*
 * This routine blows away all of the text
 * in a buffer. If the buffer is marked as changed
 * then we ask if it is ok to blow it away; this is
 * to save the user the grief of losing text. The
 * window chain is nearly always wrong if this gets
 * called; the caller must arrange for the updates
 * that are required. Return meTRUE if everything
 * looks good.
 */
/* Note for Dynamic file names
 * bclear does not touch the file name.
 */
int
bclear(register meBuffer *bp)
{
    meMode bmode ;
    register meAnchor *mk, *nmk ;
    register meLine    *lp ;
    register int      s;
    
#if MEOPT_IPIPES
    if(meModeTest(bp->mode,MDPIPE))
    {
	meIPipe *ipipe ;
	
	if(!(bp->intFlag & BIFBLOW) &&
	   ((s=mlyesno((meUByte *)"Kill active process")) <= 0))
	    return s ;
	
        /* while waiting for a Yes from the user, the process may
         * have finished, so be careful */
        ipipe = ipipes ;
	while(ipipe != NULL)
        {
            if(ipipe->bp == bp)
            {
                ipipeRemove(ipipe) ;
                break ;
            }
	    ipipe = ipipe->next ;
        }
    }
#endif        
    if(bufferNeedSaving(bp) && !(bp->intFlag & BIFBLOW))
    {   /* Something changed    */
	meUByte prompt[meBUF_SIZE_MAX+20] ;

	if(bp->fileName != NULL)
	    meStrcpy(prompt,bp->fileName) ;
	else
	    meStrcpy(prompt,bp->name) ;
        meStrcat(prompt,": Discard changes") ;
	s = mlyesno(prompt) ;
	if(s <= 0)
	    return s ;
        autowriteremove(bp) ;
    }
    
#if MEOPT_FILEHOOK
    /* There is a problem when we kill a buffer, if it is the current
     * buffer (frameCur->windowCur) then the end hook needs to be executed before
     * the local variables are blown away. It would seem sensible that
     * it is done here. Execute the end hook and then kill off the end
     * hook to ensure that it is not executed again. On a re-read of
     * a file the file hooks are re-assigned so this should not cause
     * a problem - I hope !!.
     * 
     * Note that this problem has been introduced by local variables
     * it is not a problem if global variables are used.
     * 
     * zotbuf() calls bclear; hence any operation to delete a buffer
     * will cause the end hook to be executed.
     */
    if((frameCur->bufferCur == bp) && (bp->ehook >= 0))
    {
	execBufferFunc(bp,bp->ehook,0,1) ;      /* Execute the end hook */
	bp->ehook = -1;                         /* Disable the end hook */
    }
    if(!meModeTest(bp->mode,MDNACT) && (bp->dhook >= 0))
	execBufferFunc(bp,bp->dhook,0,1) ;      /* Execute the delete hook */
#endif    
    /* Continue the destruction of the buffer space. */
    /* The following modes to preserve are: MDBINARY, MDRBIN, MDCRYPT, MDDEL */
    meModeCopy(bmode,bp->mode) ;
    meModeCopy(bp->mode,globMode) ;
    meModeSet(bp->mode,MDNACT) ;
    if(meModeTest(bmode,MDBINARY))
        meModeSet(bp->mode,MDBINARY) ;
    else if(meModeTest(bmode,MDRBIN))
        meModeSet(bp->mode,MDRBIN) ;
    if(meModeTest(bmode,MDCRYPT))
        meModeSet(bp->mode,MDCRYPT) ;
    bp->intFlag &= ~(BIFLOAD|BIFBLOW|BIFLOCK) ;
    lp = bp->baseLine ;
    meLineLoopFree(lp,0) ;
    lp->flag  = 0 ;
    bp->dotLine  = lp ;
    bp->vertScroll = 0 ;
    bp->dotOffset  = 0;
    bp->dotLineNo = 0;
    bp->markLine = NULL;                     /* Invalidate "mark"    */
    bp->markOffset = 0;
    bp->markLineNo = 0;
    bp->lineCount = 0;
    bp->tabWidth = tabWidth;
    bp->indentWidth = indentWidth;
#if MEOPT_WORDPRO
    bp->fillcol = fillcol;
    bp->fillmode = fillmode;
#endif
#if MEOPT_FILEHOOK
    bp->fhook = bp->dhook = bp->bhook = bp->ehook = -1 ;
#endif
#if MEOPT_HILIGHT
    bp->hilight = 0 ;
    bp->indent = 0 ;
#endif
#if MEOPT_UNDO
    meUndoRemove(bp) ;
#endif
#if MEOPT_EXTENDED
    /* Clean out the local buffer variables */
    if (bp->varList.head != NULL)
    {
	meVariable *bv, *tv;
	bv = bp->varList.head ;
	do
	{
	    tv = bv;
	    bv = bv->next;
	    meNullFree(tv->value) ;
	    free(tv) ;
	} while (bv != NULL);
	bp->varList.head = NULL ;
	bp->varList.count = 0 ;
    }
#endif
    mk = bp->anchorList;
    while(mk != NULL)
    {
	nmk = mk->next ;
	meFree(mk) ;
	mk = nmk ;
    }
    bp->anchorList = NULL ;
#if MEOPT_LOCALBIND
    if(bp->bindList != NULL)
	meFree(bp->bindList) ;
    bp->bindList = NULL ;
    bp->bindCount = 0 ;
#endif
#if MEOPT_NARROW
    if(bp->narrow != NULL)
    {
        meNarrow *nw, *nnw ;
        nnw = bp->narrow ;
        do
        {
            nw = nnw ;
            nnw = nw->next ;
            /* Create the loop for freeing */
            nw->elp->next = nw->slp ;
            meLineLoopFree(nw->slp,1) ;
            meFree(nw) ;
        } while(nnw != NULL) ;
        bp->narrow = NULL ;
    }
#endif
    resetBufferWindows(bp) ;
    return meTRUE ;
}


void
linkBuffer(meBuffer *bp)
{    
    register meBuffer *sb, *lb;   /* buffer to insert after */
    register meUByte  *bname=bp->name ;
    
    /* find the place in the list to insert this buffer */
    lb = NULL ;
    sb = bheadp;
    while(sb != NULL)
    {
#ifdef _INSENSE_CASE
	if(meStridif(sb->name,bname) >= 0)
#else
	if(meStrcmp(sb->name,bname) >= 0)
#endif
	    break ;
	lb = sb ;
	sb = sb->next;
    }
    /* and insert it */
    bp->next = sb ;
    if(lb == NULL)
	/* insert at the begining */
	bheadp = bp;
    else
	lb->next = bp ;
}

void
unlinkBuffer(meBuffer *bp)
{
    register meBuffer *bp1 ;
    register meBuffer *bp2 ;
    
    bp1 = NULL;                             /* Find the header.     */
    bp2 = bheadp;
    while (bp2 != bp)
    {
	bp1 = bp2;
	bp2 = bp2->next;
    }
    bp2 = bp2->next;                      /* Next one in chain.   */
    if (bp1 == NULL)                        /* Unlink it.           */
	bheadp = bp2;
    else
	bp1->next = bp2;
}

/* Note for Dynamic file names
 * zotbuf frees the file name so it must be dynamic and unique to buffer 
 *        or NULL.
 */
int      
zotbuf(register meBuffer *bp, int silent) /* kill the buffer pointed to by bp */
{
    register int     s ;
    
    /* Blow text away. last chance for user to abort so do first */
    if((s=bclear(bp)) <= 0)
	return (s);
    /* if this is the scratch and the only buffer then don't delete */
    
    /*
    ** If the buffer we want to kill is displayed on the screen, make
    ** it the current window and then remove it from the screen. Once
    ** this has been done, we can then go about zapping it.
    */
    /* This must really be the frameCur->windowCur, ie frameCur->bufferCur
    ** must be bp otherwise swbuffer stuffs up big time.
    */
    if(HideBuffer(bp,1) <= 0)
	/* only scratch left */
	return meTRUE ;
    
    meFree(bp->baseLine);             /* Release header line. */
    unlinkBuffer(bp) ;
    meNullFree(bp->fileName) ;
#if MEOPT_CRYPT
    meNullFree(bp->cryptKey) ;
#endif
#if MEOPT_EXTENDED
    meNullFree(bp->modeLineStr) ;
#endif
    if(!silent)
	/* say it went ok       */
	mlwrite(0,(meUByte *)"[buffer %s killed]", bp->name);
    meNullFree(bp->name) ;
    meFree(bp);                             /* Release buffer block */
    return meTRUE ;
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header. Bound to "C-X K".
 */
int
bufferDelete(int f, int n)
{
    register meBuffer *bp;
    register int s;
    meUByte bufn[meBUF_SIZE_MAX];

    if((s = getBufferName((meUByte *)"Delete buffer", MLNOSTORE,1,bufn)) <= 0)
	return(s);
    
    if((bp=bfind(bufn, 0)) == NULL) 
	/* Easy if unknown.     */
	return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s: no such buffer]", bufn);
    if(bp->intFlag & BIFNODEL)
	return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s cannot be deleted]", bufn);
        
    if(!(n & 0x01))
	bp->intFlag |= BIFBLOW ;

    return zotbuf(bp,clexec) ;
}


#if MEOPT_EXTENDED
int
deleteSomeBuffers(int f, int n)
{
    meBuffer *bp, *nbp ;
    meUByte   prompt[meBUF_SIZE_MAX] ;
    int     s ;
    
    bp = bheadp ;
    while(bp != NULL)
    {
	nbp = bp->next ;
	if(meModeTest(bp->mode,MDNACT))
        {
            if((n & 0x06) == 0)
            {
                if((s = mlCharReply((meUByte *)"Delete buffers not yet active (?/y/n/a) ? ",mlCR_LOWER_CASE,(meUByte *)"yna",(meUByte *)"(Y)es, (N)o, Yes to (a)ll, (C-g)Abort ? ")) < 0)
                    return ctrlg(meFALSE,1) ;
                if(s == 'n')
                    break ;
                n |= (s == 'a') ? 2:4 ;
            }
            bp->intFlag |= BIFBLOW ;
            zotbuf(bp,clexec) ;
        }
	bp = nbp ;
    }
    bp = bheadp ;
    while(bp != NULL)
    {
	nbp = bp->next ;
	if(!meModeTest(bp->mode,MDHIDE))
	{
	    if(n & 0x02)
                s = meTRUE ;
            else
            {
                sprintf((char *)prompt,"Delete buffer [%s]  (?/y/n/a) ? ",bp->name) ;
                if((s = mlCharReply(prompt,mlCR_LOWER_CASE,(meUByte *)"yna",(meUByte *)"(Y)es, (N)o, Yes to (a)ll, (C-g)Abort ? ")) < 0)
                    return ctrlg(meFALSE,1) ;
                if(s == 'n')
                    s = meFALSE ;
                else
                {
                    if(s == 'a')
                        n |= 2 ;
                    s = meTRUE ;
                }
            }
            if(s)
	    {
                if((n & 0x01) == 0)
                    bp->intFlag |= BIFBLOW ;
		if(zotbuf(bp,clexec) < 0)
		    return meABORT ;
	    }
	}
	bp = nbp ;
    }
    return meTRUE ;
}

int
changeBufName(int f, int n)     /*      Rename the current buffer       */
{
    meUByte bufn[meBUF_SIZE_MAX], *nn;     /* buffer to hold buffer name */
    meBuffer *bp1, *bp2 ;
    int s ;
    
    /* prompt for and get the new buffer name */
    if((s = getBufferName((meUByte *)"New buffer name", 0, 0, bufn)) <= 0)
	return s ;
    
    bp1 = frameCur->bufferCur ;
    unlinkBuffer(bp1) ;
    
    /* check for duplicates */
    if((bp2 = bfind(bufn,0)) != NULL)
    {
        if(n & 1)
        {
            /* if the names the same */
            linkBuffer(bp1) ;
            return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Already exists!]") ;
        }
        /* if bit one is clear then make it a valid name by generating a
         * valid name or by moving the other buffer out of the way */
        nn = bufn ;
        if(n & 2)
        {
            meUByte *name ;
            unlinkBuffer(bp2) ;
            name = bp1->name ;
            bp1->name = bp2->name ;
            bp2->name = name ;
            linkBuffer(bp1) ;
            bp1 = bp2 ;
            if(bp1->fileName != NULL)
                nn = bp1->fileName ;
        }
        makename(bufn,nn) ;
    }
    
    frameAddModeToWindows(WFMODE) ;
    if((nn = meStrdup(bufn)) == NULL)
    {
        linkBuffer(bp1) ;
        return meABORT ;
    }
    meFree(bp1->name) ;
    bp1->name = nn ;
    linkBuffer(bp1) ;
    
    return meTRUE ;
}
#endif


/*
 * The argument "text" points to
 * a string. Append this line to the
 * given buffer. Handcraft the EOL
 * on the end. Return meTRUE if it worked and
 * meFALSE if you ran out of room.
 * if pos is meTRUE then insert to current line
 * else to the end of the buffer
 */
int 
addLine(register meLine *ilp, meUByte *text)
{
    meUByte cc, *stext ;
    meLine *lp, *plp ;
    int len, lineCount=0 ;
    
    plp = ilp->prev ;
    do
    {
        len = 0 ;
        stext = text ;
        while(((cc=*text++) != '\0') && (cc != '\n'))
            len++ ;
        if((lp=meLineMalloc(len,0)) == NULL)
            break ;
        lp->text[len] = '\0' ;
        while(--len >= 0)
            lp->text[len] = stext[len] ;
        plp->next = lp ;
        lp->prev = plp ;
        plp = lp ;
        lineCount++ ;
    } while(cc != '\0') ;
    ilp->prev = lp;
    lp->next = ilp ;
    return lineCount ;
}

/*
 * This routine rebuilds the
 * text in the given buffer
 * that holds the buffer list. It is called
 * by the list buffers command. Return meTRUE
 * if everything works. Return meFALSE if there
 * is an error (if there is no memory).
 */
static void
makelist(meBuffer *blistp, int verb)
{
    register meBuffer *bp ;
    register meUByte   cc, *cp1, *cp2 ;
    register int     ii, ff ;
    meUByte line[meBUF_SIZE_MAX] ;
    
#if MEOPT_EXTENDED
    if(verb & 0x02)
    {
        addLineToEob(blistp,(meUByte *)
                     "AC    Size Buffer              File Size  Mem Size   Undo Size\n"
                     "--    ---- ------              ---------  --------   ---------") ;
    }
    else
#endif
    {
        addLineToEob(blistp,(meUByte *)
                     "AC    Size Buffer          File\n"
                     "--    ---- ------          ----") ;
    }
        
    bp = bheadp;                            /* For all buffers      */
    
    /* output the list of buffers */
    while (bp != NULL)
    {
        if((verb & 0x01) && meModeTest(bp->mode,MDHIDE))
        {
	    bp = bp->next;
	    continue;
	}
	cp1 = line ;                      /* Start at left edge   */
      
	/* output status of ACTIVE flag (has the file been read in? */
        if(meModeTest(bp->mode,MDNACT))   /* "@" if activated     */
	    *cp1++ = ' ';
	else
	    *cp1++ = '@';
	
	/* output status of changed flag */
        if(meModeTest(bp->mode,MDEDIT))       /* "*" if changed       */
	    *cp1++ = '*';
	else if(meModeTest(bp->mode,MDVIEW))  /* "%" if in view mode  */
	    *cp1++ = '%';
	else
	    *cp1++ = ' ';
	if((bp == blistp) || meModeTest(bp->mode,MDNACT))
	    cp2 = (meUByte *) "-" ;
	else
	    cp2 = meItoa(bp->lineCount+1);
	ii = 8 - meStrlen(cp2) ;
	while(ii-- > 0)
	    *cp1++ = ' ' ;
	while((cc = *cp2++) != '\0')
	    *cp1++ = cc ;
	cp2 = bp->name ;
	ii = 16 ;
	if((ff=(meStrchr(cp2,' ') != NULL)))
	    *cp1 = '"';
	else
	    *cp1 = ' ';
	while((*++cp1 = *cp2++) != 0)
	    ii-- ;
	if(ff)
	    *cp1++ = '"';
	else
	    *cp1++ = ' ';
	ii-- ;
	
	for( ; ii>0 ; ii--)
	    *cp1++ = ' ';
	
#if MEOPT_EXTENDED
	if(verb & 0x02)
	{
#if MEOPT_UNDO
	    meUndoNode *nn ;
#endif
	    meLine *ll ;
	    int jj ;
	    
	    ii = 0 ;
	    jj = 0 ;
	    ll = bp->baseLine ;
	    do
	    {
		ii += meLineGetLength(ll) + 1 ;
		jj += meLineGetMaxLength(ll) ;
		ll = meLineGetNext(ll) ;
	    } while(ll != bp->baseLine) ;
	    ff = line + 30 - cp1 ;
	    while(--ff >= 0)
		*cp1++ = ' ' ;
	    *cp1++ = ' ';
	    cp2 = meItoa(ii) ;
	    while((cc = *cp2++) != '\0')
		*cp1++ = cc ;
	    ff = line + 42 - cp1 ;
	    while(--ff >= 0)
		*cp1++ = ' ' ;
	    jj += (meLINE_SIZE*(bp->lineCount+1)) + sizeof(meBuffer) ;
	    cp2 = meItoa(jj) ;
	    while((cc = *cp2++) != '\0')
		*cp1++ = cc ;
#if MEOPT_UNDO
	    ff = line + 53 - cp1 ;
	    while(--ff >= 0)
		*cp1++ = ' ' ;
	    ii = 0 ;
	    nn = bp->undoHead ;
	    while(nn != NULL)
	    {
		if(meUndoIsLineSort(nn))
                    ii += sizeof(meUndoNode) + (sizeof(meInt) * (nn->count + 1)) ;
#if MEOPT_NARROW
		else if(meUndoIsNarrow(nn))
                    ii += sizeof(meUndoNarrow) ;
#endif
                else
                {
                    ii += sizeof(meUndoNode) ;
                    if(nn->type & meUNDO_DELETE)
                        ii += meStrlen(nn->str) ;
                    if(meUndoIsReplace(nn))
                        ii += sizeof(meUndoCoord)*nn->doto ;
                }
		nn = nn->next ;
	    }
	    cp2 = meItoa(ii) ;
	    while((cc = *cp2++) != '\0')
		*cp1++ = cc ;
#endif
	}
	else
#endif
            if((bp->name[0] != '*') &&
               ((cp2 = bp->fileName) != NULL))
	{
	    if((ii=meStrlen(cp2))+(cp1-line) > frameCur->width)
	    {
		*cp1++ = '$' ;
		cp2 += ii - (frameCur->width-(1+cp1-line)) ;
	    }
	    while((cc = *cp2++) != '\0')
		*cp1++ = cc ;
	}
	*cp1 = '\0' ;
	addLineToEob(blistp,line) ;
	bp = bp->next;
    }
    meModeSet(blistp->mode,MDVIEW) ;        /* put this buffer view mode */
}


/*---   List all  of the  active buffers.   First update  the special buffer
	that holds the list.  Next make sure at least 1 window is displaying
	the buffer  list, splitting  the screen  if this  is what  it takes.
	Lastly, repaint  all of  the windows  that are  displaying the list.
	Bound to "C-X C-B".   */
	
int
listBuffers (int f, int n)
{
    meBuffer *bp ;
    
    if((bp=bfind(BbuffersN,BFND_CREAT|BFND_CLEAR)) == NULL)
	return mlwrite(MWABORT,(meUByte *)"[Failed to create blist]") ;
    makelist(bp,n) ;
    
    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotOffset = 0 ;
    bp->dotLineNo = 0 ;
    
    resetBufferWindows(bp) ;
    meWindowPopup(bp->name,WPOP_USESTR,NULL) ;
    return meTRUE ;
}

/*
** Returns :-
** meTRUE  if the given buffer needs saving
** meFALSE if it doesnt
 */
int
bufferNeedSaving(meBuffer *bp)
{
    /* If changed (edited) and valid buffer name (not system) */
    if(meModeTest(bp->mode,MDEDIT) && (bp->name[0] != '*'))
	return meTRUE ;
    return meFALSE ;
}

/*
 * Look through the list of
 * buffers. Return meTRUE if there
 * are any changed buffers. Buffers
 * that hold magic internal stuff are
 * not considered; who cares if the
 * list of buffer names is hacked.
 * Return meFALSE if no buffers
 * have been changed.
 */
int
anyChangedBuffer(void)
{
    register meBuffer *bp;
    
    bp = bheadp;
    while (bp != NULL)
    {
	if(bufferNeedSaving(bp))
	    return (meTRUE);
	bp = bp->next;
    }
    return (meFALSE);
}

meBuffer *
createBuffer(register meUByte *bname)
{
    register meBuffer *bp;
    register meLine   *lp;

    if((bp=(meBuffer *)meMalloc(sizeof(meBuffer))) == NULL)
	return NULL ;
    if((lp=meLineMalloc(0,0)) == NULL)
    {
	meFree(bp);
	return NULL ;
    }
    /* set everything to 0 */
    memset(bp,0,sizeof(meBuffer)) ;
    if((bp->name = meStrdup(bname)) == NULL)
	return NULL ;
    linkBuffer(bp) ;
    
    /* and set up the non-zero buffer fields */
    meModeCopy(bp->mode,globMode) ;
    meModeSet(bp->mode,MDNACT) ;
    meFiletimeInit(bp->stats.stmtime) ;
    bp->dotLine = bp->baseLine = lp;
    bp->autoTime = -1 ;
    bp->stats.stmode  = meUmask ;
#ifdef _UNIX
    bp->stats.stuid = meUid ;
    bp->stats.stgid = meGid ;
    bp->stats.stdev = -1 ;
    bp->stats.stino = -1 ;
#endif
#if MEOPT_IPIPES
    bp->ipipeFunc = -1 ;
#endif
#if MEOPT_EXTENDED
    bp->inputFunc = -1 ;
    bp->isWordMask = CHRMSK_DEFWORDMSK ;
#endif
#if MEOPT_FILEHOOK
    bp->fhook = -1 ;
    bp->bhook = -1 ;
    bp->dhook = -1 ;
    bp->ehook = -1 ;
#endif
#if MEOPT_COLOR
    /* set the colors of the new window */
    bp->scheme = globScheme;
#endif
    bp->indentWidth = indentWidth;
    bp->tabWidth = tabWidth;
#if MEOPT_WORDPRO
    bp->fillcol = fillcol;
    bp->fillmode = fillmode;
#endif
    lp->next = lp;
    lp->prev = lp;
    
    return bp ;
}


/*
 * Find a buffer, by name. Return a pointer
 * to the meBuffer structure associated with it.
 * If the buffer is not found
 * and the "cflag" is meTRUE, create it. The "bflag" is
 * the settings for the flags in in buffer.
 */
meBuffer *
bfind(register meUByte *bname, int cflag)
{
    meBuffer *bp;
    meMode  sglobMode ;
    meUByte intFlag=0 ;
    meUByte *bnm ;
    meUByte bn[meBUF_SIZE_MAX] ;
    int ii ;
    
    if(cflag & BFND_MKNAM)
    {
	makename(bn,bname) ;
	bnm = bn ;
    }
    else
    {
	bp = bheadp;
	while(bp != NULL)
	{
#ifdef _INSENSE_CASE
	    if((ii=meStricmp(bp->name,bname)) == 0)
#else
	    if((ii=meStrcmp(bp->name,bname)) == 0)
#endif
	    {
		if(cflag & BFND_CLEAR)
		{
		    bp->intFlag |= BIFBLOW ;             /* Don't complain!      */
                    bclear(bp) ;
                    /* The BFND_CLEAR functionality is only used for system buffers
                     * such as *help* *command* etc. i.e. not user loaded buffer. 
                     * These buffers do not and must not be linked to a filename.
                     * This can happen if the user renames "./foo" to *compile* say
                     * and then runs another compile, it its still linked to foo then
                     * it gets loaded */
                    bp->intFlag &= ~BIFFILE ;
                    if(bp->fileName != NULL)
                    {
                        meFree(bp->fileName) ;
                        bp->fileName = NULL ;
                    }
		    /* set the modes correctly */
		    if(cflag & BFND_BINARY)
			meModeSet(bp->mode,MDBINARY) ;
		    else if(cflag & BFND_RBIN)
			meModeSet(bp->mode,MDRBIN) ;
		    if(cflag & BFND_CRYPT)
			meModeSet(bp->mode,MDCRYPT) ;
		}
		return bp ;
	    }
	    if(ii>0)
		break ;
	    bp = bp->next;
	}
	if(!cflag)
	    return NULL ;
	bnm = bname ;
    }
    meModeCopy(sglobMode,globMode) ;
    if(cflag & BFND_BINARY)
	meModeSet(globMode,MDBINARY) ;
    else if(cflag & BFND_RBIN)
	meModeSet(globMode,MDRBIN) ;
    if(cflag & BFND_CRYPT)
	meModeSet(globMode,MDCRYPT) ;
    bp = createBuffer(bnm) ;
    bp->intFlag |= intFlag ;
    if(cflag & BFND_NOHOOK)
        bp->intFlag |= BIFNOHOOK ;
    meModeCopy(globMode,sglobMode) ;
    return bp ;
}



int
adjustMode(meBuffer *bp, int nn)  /* change the editor mode status */
{
    meMode  mode ;
    int     func ;
    meUByte prompt[50];            /* string to prompt user with */
    meUByte cbuf[meSBUF_SIZE_MAX];            /* buffer to recieve mode name into */
    
    if(nn >= 128)
    {
	nn -= 128 ;
	func = 0 ;
    }
    else if(nn < 0)
    {
	nn = 0 - nn ;
	func = -1 ;
    }
    else if(nn == 0)
	func = 0 ;
    else
	func = 1 ;
    
    if(nn < 2)
    {
	/* build the proper prompt string */
	meStrcpy(prompt,"Buffer mode to ");
	if(bp == NULL)
	    memcpy(prompt,"Global",6);
	
	if(func == 0)
	    meStrcpy(prompt+15, "toggle");
	else if(func > 0)
	    meStrcpy(prompt+15, "add");
	else
            meStrcpy(prompt+15, "delete");
	
	/* Setup the completion */
	mlgsStrList = modeName ;
	mlgsStrListSize = MDNUMMODES ;
	
	/* prompt the user and get an answer */
	if(meGetString(prompt, MLLOWER|MLUSER, 0, cbuf, meSBUF_SIZE_MAX) == meABORT)
	    return meFALSE ;
    
	/* test it against the modes we know */
	for (nn=0; nn < MDNUMMODES ; nn++) 
	    if(meStrcmp(cbuf,modeName[nn]) == 0) 
		break ;
    }
    else
	nn -= 2 ;
    
    if(nn >= MDNUMMODES)
	return mlwrite(MWABORT,(meUByte *)"[No such mode]");
    
    switch(nn)
    {
    case MDEDIT:
	if(bp == NULL)
	    goto invalid_global ;
		
	if(meModeTest(bp->mode,MDEDIT))
	{
	    if(func <= 0)
	    {
		autowriteremove(bp) ;
#if MEOPT_UNDO
		meUndoRemove(bp) ;
#endif
		meModeClear(bp->mode,MDEDIT) ;
		frameAddModeToWindows(WFMODE) ;
	    }
	}
	else if(func >= 0)
	{
	    if(bp != frameCur->bufferCur)
	    {
		register meBuffer *tbp = frameCur->bufferCur ;
		
		storeWindBSet(tbp,frameCur->windowCur) ;
		swbuffer(frameCur->windowCur,bp) ;
		
		nn = bufferSetEdit() ;
		
		swbuffer(frameCur->windowCur,tbp) ;
		restoreWindBSet(frameCur->windowCur,tbp) ;
	    }
	    else
		nn = bufferSetEdit() ;
            
            if(nn <= 0)
                return nn ;
	}
        if((func == 0) && !clexec)
            mlwrite(0,(meUByte *)"[Buffer edit mode is now %s]",
                    meModeTest(bp->mode,MDEDIT) ? "on":"off");
	return meTRUE ;
	
    case MDLOCK:
	if((bp == NULL) || !meModeTest(bp->mode,MDPIPE)) 
	    goto invalid_global ;
	break ;
        
    case MDHIDE:
	if(bp != NULL)
	    break ;
    case MDNACT:
    case MDNARROW:
    case MDPIPE:
invalid_global:
	return mlwrite(MWABORT,(meUByte *)"[Cannot change this mode]");
    }
    /* make the mode change */
    if (bp == NULL)
	meModeCopy(mode,globMode) ;
    else
	meModeCopy(mode,bp->mode) ;
    if(func == 0)
    {
        meModeToggle(mode,nn) ;
        if(!clexec)
            mlwrite(0,(meUByte *)"[%s %s mode is now %s]",
                    (bp == NULL) ? "Global":"Buffer",modeName[nn],
                    meModeTest(mode,nn) ? "on":"off");
    }
    else if(func > 0)
	meModeSet(mode,nn) ;
    else
	meModeClear(mode,nn) ;
    
    if(meModeTest(mode,nn))
    {
	/* make sure that, if setting RBIN the clear BINARY and vice versa */
        if(nn == MDBINARY)
	    meModeClear(mode,MDRBIN) ;
	else if(nn == MDRBIN)
	    meModeClear(mode,MDBINARY) ;
    }
    else if(bp != NULL)
    {
#if MEOPT_UNDO
	if(nn == MDUNDO)
	    meUndoRemove(bp) ;
	else
#endif
            if(nn == MDAUTOSV)
	    bp->autoTime = -1 ;
    }
    if (bp == NULL)
        meModeCopy(globMode,mode) ;
    else
    {
	meModeCopy(bp->mode,mode) ;
        /* update  mode line in all buffer windows */
        meBufferAddModeToWindows(bp,WFMODE) ;
    }    
    return meTRUE ;
}
   
int     
bufferMode(int f, int n)        /* prompt and set an editor mode */
{
    return adjustMode(frameCur->bufferCur,(f) ? n:0) ;
}

int     
globalMode(int f, int n)        /* prompt and set a global editor mode */
{
    return adjustMode(NULL,(f) ? n:0);
}

#if MEOPT_EXTENDED
int     
namedBufferMode(int f, int n)   /* prompt and set an editor mode */
{
    register meBuffer *bp;
    register int s;
    meUByte bufn[meBUF_SIZE_MAX];
    
    if((s = getBufferName((meUByte *)"Buffer name", MLNOSTORE,1,bufn)) <= 0)
	return s ;
    
    if((bp=bfind(bufn, 0)) == NULL) 
	/* Easy if unknown.     */
	return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No such buffer %s]", bufn);
    return adjustMode(bp,(f) ? n:0) ;
}
#endif

