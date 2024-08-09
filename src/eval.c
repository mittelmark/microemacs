/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * eval.c - Expresion evaluation functions.
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
 * Created:     1986
 * Synopsis:    Expresion evaluation functions.
 * Authors:     Daniel Lawrence, Jon Green & Steven Phillips
 */ 

#define __EVALC 1       /* Define program name */

#include "emain.h"
#include "evar.h"
#include "efunc.h"
#include "eskeys.h"
#include "esearch.h"
#include "evers.h"
#include "eopt.h"

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#ifdef _UNIX
#include <sys/stat.h>
#endif
#ifndef _WIN32
#include <sys/time.h>
#endif
#endif

meUByte evalResult[meTOKENBUF_SIZE_MAX];    /* resulting string */
static meUByte machineName[]=meSYSTEM_NAME;    /* resulting string */

#if MEOPT_TIMSTMP
extern meUByte time_stamp[];   /* Time stamp string */
#endif

#if MEOPT_EXTENDED
static time_t timeOffset=0 ;            /* Time offset in seconds */

#ifdef _INSENSE_CASE
meNamesList buffNames={0,0,NULL,NULL,0} ;
meDirList  fileNames={0,0,NULL,NULL,0,NULL} ;
#else
meNamesList buffNames={1,0,NULL,NULL,0} ;
meDirList  fileNames={1,0,NULL,NULL,0,NULL,0} ;
#endif
meNamesList commNames={1,0,NULL,NULL,0} ;
meNamesList modeNames={0,MDNUMMODES,modeName,NULL,0} ;
meNamesList varbNames={1,0,NULL,NULL,0} ;
#endif

/* The following horrid global variable solves a horrid problem, consider:
   define-macro l3
      set-variable @1 @2
   !emacro
   define-macro l2
      l3 @1 @2
   !emacro
   define-macro l1
      l2 @1 @2
   !emacro
   define-macro Test
      l1 #l0 @#
      ml-write #l0
   !emacro
   500 Test
 * The above example should write out "500".
 * its not good enough for @1 in l3 to return #l0, it must return Test's #l0
 * and thats horrid, hence the variable. Similarly it should be set to Test's
 * @#, not l3's which will be 1.
 * The hopefully final saga to this is that the process of setting a variable
 * may not be instantaneous, e.g. when doing "set-variable #l1 @ml "get value""
 * it waits for the user to enter a value. During this time a callback macro
 * may occur which could (probably will) change the value of gmaLocalRegPtr
 * so the value must be cached as soon as the variable to be set is obtained.
 */
static meRegister *gmaLocalRegPtr=NULL ;

#if MEOPT_MAGIC
meRegex meRegexStrCmp ;
/* Quick and easy interface to regex compare
 * 
 * return -1 on error, 0 on no match 1 if matched
 */
int
regexStrCmp(meUByte *str, meUByte *reg, int flags)
{
    meRegex *rg ;
    int ii, rr ;
    
    if(flags & meRSTRCMP_USEMAIN)
    {
        rg = &mereRegex ;
        srchLastState = meFALSE ;
    }
    else
        rg = &meRegexStrCmp ;
    
    if(meRegexComp(rg,reg,(flags & meRSTRCMP_ICASE)) != meREGEX_OKAY)
        return -1 ;
    
    ii = meStrlen(str) ;
    rr = meRegexMatch(rg,str,ii,0,ii,(flags & meRSTRCMP_WHOLE)) ;
    if((flags & meRSTRCMP_USEMAIN) && (rr > 0))
    {
        if(ii >= mereNewlBufSz)
        {
            if((mereNewlBuf = meRealloc(mereNewlBuf,ii+128)) == NULL)
            {
                mereNewlBufSz = 0 ;
                return meFALSE ;
            }
            mereNewlBufSz = ii+127 ;
        }
        meStrcpy(mereNewlBuf,str) ;
        srchLastMatch = mereNewlBuf ;
        srchLastState = meTRUE ;
        /* or in the STRCMP bit so that hunt-forw/backward can still determine
         * whether the last main search was magic or not & @s1 also work correctly */ 
        srchLastMagic |= meSEARCH_LAST_MAGIC_STRCMP ;
    }
    return rr ;
}
#endif

/* meItoa
 * integer to ascii string..........
 * This is too inconsistant to use the system's */
#define INTWIDTH (sizeof(int)*4+2)
meUByte *
meItoa(int i)
{
    register meUByte *sp;      /* pointer into result */
    register int sign;      /* sign of resulting number */
    
    if(i == 0)          /* eliminate the trivial 0  */
        return meLtoa(0) ;
    else if(i == 1)     /* and trivial 1            */
        return meLtoa(1) ;
    
    if((sign = (i < 0)))    /* record the sign...*/
        i = -i;
    
    /* and build the string (backwards!) */
    sp = &(evalResult[INTWIDTH]);
    *sp = 0;
    do
        *(--sp) = '0' + (i % 10);   /* Install the new digit */
    while (i /= 10);            /* Remove digit and test for 0 */
    
    if (sign != 0)          /* and fix the sign */
        *(--sp) = '-';          /* and install the minus sign */
    
    return sp ;
}


meVariable *
SetUsrLclCmdVar(meUByte *vname, meUByte *vvalue, register meVarList *varList)
{
    /* set a user variable */
    register meVariable *vptr, *vp ;
    register int ii, jj ;
    
    vptr = (meVariable *) varList ;
    ii = varList->count ;
    /* scan the list looking for the user var name */
    while(ii)
    {
        meUByte *s1, *s2, cc ;
        
        jj = (ii>>1)+1 ;
        vp = vptr ;
        while(--jj >= 0)
            vp = vp->next ;
        
        s1 = vp->name ;
        s2 = vname ;
        for( ; ((cc=*s1++) == *s2) ; s2++)
            if(cc == 0)
            {
                /* found it, replace value */
                meStrrep(&vp->value,vvalue) ;
                return vp ;
            }
        if(cc > *s2)
            ii = ii>>1 ;
        else
        {
            ii -= (ii>>1) + 1 ;
            vptr = vp ;
        }
    }
    
    /* Not found so create a new one */
    if((vp = (meVariable *) meMalloc(sizeof(meVariable)+meStrlen(vname))) != NULL)
    {
        meStrcpy(vp->name,vname) ;
        vp->next = vptr->next ;
        vptr->next = vp ;
        varList->count++ ;
        vp->value = meStrdup(vvalue) ;
    }
    return vp ;
}

int
setVar(meUByte *vname, meUByte *vvalue, meRegister *regs)
{
    register int status ;         /* status return */
    meUByte *nn ;
    
    /* check the legality and find the var */
    nn = vname+1 ;
    switch(getMacroTypeS(vname))
    {
    case TKREG:
        {
            meUByte cc ;
            if((cc=*nn) == 'g')
                regs = meRegHead ;
            else if(cc == 'p')
                regs = regs->prev ;
            cc = nn[1] - '0' ;
            if(cc >= ('0'+meREGISTER_MAX))
                return mlwrite(MWABORT,(meUByte *)"[No such register %s]",vname);
            meStrcpy(regs->reg[cc],vvalue) ;
            break ;
        }
#if MEOPT_EXTENDED
    case TKVAR:
        if(SetUsrLclCmdVar(nn,vvalue,&usrVarList) == NULL)
            return meFALSE ;
        break ;
    case TKLVR:
        {
            meBuffer *bp ;
            meUByte *ss ;
            if((ss=meStrrchr(nn,':')) != NULL)
            {
                *ss = '\0' ;
                bp = bfind(nn,0) ;
                *ss++ = ':' ;
                if(bp == NULL)
                    /* not a buffer - abort */
                    return mlwrite(MWABORT,(meUByte *)"[No such variable]");
                nn = ss ;
            }
            else
                bp = frameCur->bufferCur ;
            if(SetUsrLclCmdVar(nn,vvalue,&(bp->varList)) == NULL)
                return meFALSE ;
            break ;
        }
    case TKCVR:
        {
            meVarList *varList ;
            meUByte *ss ;
            if((ss=meStrrchr(nn,'.')) != NULL)
            {
                int ii ;
                
                *ss = '\0' ;
                ii = decode_fncname(nn,1) ;
                *ss++ = '.' ;
                if(ii < 0)
                    /* not a command - abort */
                    return mlwrite(MWABORT,(meUByte *)"[No such variable]");
                varList = &(cmdTable[ii]->varList) ;
                nn = ss ;
            }
            else if((varList = regs->varList) == NULL)
                return mlwrite(MWABORT,(meUByte *)"[No such variable]");
            if(SetUsrLclCmdVar(nn,vvalue,varList) == NULL)
                return meFALSE ;
            break ;
        }
    
    case TKARG:
        if(*nn == 'w')
        {
            if((status=bufferSetEdit()) <= 0)
                return status ;
            if(nn[1] == 'l')
            {
                int ll=0 ;
                
                if((frameCur->windowCur->dotLineNo == frameCur->bufferCur->lineCount) ||
                   (meStrchr(vvalue,meCHAR_NL) != NULL))
                    return ctrlg(0,1) ;
                
                frameCur->windowCur->dotOffset = 0 ;
                if((status = ldelete(meLineGetLength(frameCur->windowCur->dotLine),6)) > 0)
                {
                    ll = meStrlen(vvalue) ;
                    status = lineInsertString(ll,vvalue) ;
#if MEOPT_UNDO
                    if(status >= 0)
                        meUndoAddReplaceEnd(ll) ;
#endif
                }
                return status ;
            }
            else
            {
                if (frameCur->windowCur->dotOffset >= frameCur->windowCur->dotLine->length)
                    return mlwrite(MWABORT,(meUByte *)"[Cannot set @wc here]");
                
                lineSetChanged(WFMAIN);
#if MEOPT_UNDO
                meUndoAddRepChar() ;
#endif
                meLineSetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset,vvalue[0]) ;
            }
        }
        else if(*nn == 'c')
        {
            int ii ;
            if(nn[2] == 'k')
            {
                meUByte *saves, savcle ;
                          
                /* use the macro string to key evaluator to get the value,
                 * must set this up carefully and restore state */
                savcle = clexec;        /* save execution mode */
                clexec = meTRUE;      /* get the argument */
                saves = execstr ;
                execstr = vvalue ;
                ii = meGetKey(meGETKEY_SILENT) ;
                execstr = saves ;
                clexec = savcle;        /* restore execution mode */
                if(ii != 0)
                {
                    if(nn[1] == 'c')
                        thisCommand = ii ;
                    else
                        lastCommand = ii ;
                }
            }
            else
            {
                ii = decode_fncname(vvalue,1) ;
                if(nn[1] == 'l')
                {
                    switch(ii)
                    {
                    case CK_BAKLIN:
                    case CK_FORLIN:
                        thisflag = meCFCPCN;
                        break ;
                    case CK_DELWBAK:
                    case CK_CPYREG:
                    case CK_DELFWRD:
                    case CK_KILEOL:
#if MEOPT_WORDPRO
                    case CK_KILPAR:
#endif
                    case CK_KILREG:
                        thisflag = meCFKILL;
                        break ;
                    case CK_REYANK:
                        thisflag = meCFRYANK ;
                        break ;
                    case CK_YANK:
                        thisflag = meCFYANK ;
                        break ;
#if MEOPT_UNDO
                    case CK_UNDO:
                        thisflag = meCFUNDO ;
                        break ;
#endif
                    }
                    lastIndex = ii ;
                }
                else
                    thisIndex = ii ;
            }
        }
        else if(*nn == '?')
            meRegCurr->f = meAtoi(vvalue) ;
        else if(*nn == '#')
            meRegCurr->n = meAtoi(vvalue) ;
        else if(*nn == 'y')
        {
            int len = meStrlen(vvalue) ;
            killSave() ;
            if((nn = killAddNode(len)) == NULL)
                return meABORT ;
            memcpy(nn,vvalue,len) ;
            nn[len] = '\0' ;
        }
        else if(*nn == 'h')
        {
            int option ;
            if((nn[1] > '0') && (nn[1] <= '4'))
                option = 1 << (nn[1] - '1') ;
            else
                option = 0 ;
            addHistory(option,vvalue) ;
        }
        else
            return mlwrite(MWABORT,(meUByte *)"[Cannot set variable %s]",vname);
        break ;
#endif
    
    case TKENV:
        /* set an environment variable */
        status = biChopFindString(nn,14,envars,NEVARS) ;
        switch(status)
        {
        case EVAUTOTIME:
            autoTime = meAtoi(vvalue);
            break;
#if MEOPT_CALLBACK
        case EVIDLETIME:
            idleTime = (meUInt) meAtoi(vvalue);
            if (idleTime < 10)
                idleTime = 10;
            break;
#endif
#if MEOPT_MOUSE
        case EVDELAYTIME:
            delayTime = (meUInt) meAtoi(vvalue);
            if (delayTime < 10)
                delayTime = 10;
            break;
        case EVREPEATTIME:
            repeatTime = (meUInt) meAtoi(vvalue);
            if (repeatTime < 10)
                repeatTime = 10;
            break;
#endif
#if MEOPT_EXTENDED
            /* always allow the user to set the mouse position as termcap
             * does not support the mouse but context menus need to be positioned sensibly */
        case EVMOUSE:
            meMouseCfg = meAtoi(vvalue) ;
#if MEOPT_MOUSE
            TTinitMouse() ;
#endif
            break;
        case EVMOUSEPOS:
            mouse_pos = (meUByte) meAtoi(vvalue) ;
            break;
        case EVMOUSEX:
            mouse_X = (meShort) meAtoi(vvalue) ;
            break;
        case EVMOUSEY:
            mouse_Y = (meShort) meAtoi(vvalue) ;
            break;
        case EVPAUSETIME:
            pauseTime = (meShort) meAtoi(vvalue);
            break;
        case EVFSPROMPT:
            fileSizePrompt = (meUInt) meAtoi(vvalue);
            break;
        case EVKEPTVERS:
            if(!(meSystemCfg & meSYSTEM_DOSFNAMES) && 
               ((keptVersions = meAtoi(vvalue)) < 0))
                keptVersions = 0 ;
            break;
#endif
        case EVBOXCHRS:
            {
                meUByte cc ;
                int len;
                len = meStrlen(vvalue);
                if (len > BCLEN)
                    len = BCLEN;
                while(--len >= 0)
                {
                    cc = vvalue[len] ;
                    if(!isPokable(cc))
                        cc = '.' ;
                    boxChars[len] = cc ;
                }
            }
            break;
        case EVMODELINE:
            if(modeLineStr != orgModeLineStr)
                meFree(modeLineStr) ;
            modeLineStr = meStrdup(vvalue) ;
            modeLineFlags = assessModeLine(vvalue) ;
            frameAddModeToWindows(WFMODE) ;
            break ;
#if MEOPT_EXTENDED
        case EVBMDLINE:
            meStrrep(&frameCur->bufferCur->modeLineStr,vvalue) ;
            frameCur->bufferCur->modeLineFlags = assessModeLine(vvalue) ;
            frameAddModeToWindows(WFMODE) ;
            break ;
#endif
        case EVSYSTEM:
            {
                meSystemCfg = (meSystemCfg & ~meSYSTEM_MASK) | 
                          (meAtoi(vvalue)&meSYSTEM_MASK) ;
#if MEOPT_EXTENDED
                if(meSystemCfg & meSYSTEM_DOSFNAMES)
                    keptVersions = 0 ;
#endif
                if(meSystemCfg & meSYSTEM_SHOWWHITE)
                {
                    displayTab = windowChars[WCDISPTAB];
                    displayNewLine = windowChars[WCDISPCR];
                    displaySpace = windowChars[WCDISPSPACE];
                }
                else
                {
                    displayTab = ' ';
                    displayNewLine = ' ';
                    displaySpace = ' ';    
                }
#ifdef _XTERM
                /* on unix, if using x-window then can't set ANSI || XANSI bits */
                if(!(meSystemCfg & meSYSTEM_CONSOLE))
                    meSystemCfg &= ~(meSYSTEM_ANSICOLOR|meSYSTEM_XANSICOLOR) ;
#endif
#if MEOPT_CLIENTSERVER
                /* open or close the client server files */
                if(meSystemCfg & meSYSTEM_CLNTSRVR)
                    TTopenClientServer ();
                else
                    TTkillClientServer ();
#endif
                /* may changes may case a redraw so always redraw next time */
                sgarbf = meTRUE ;
                break;
            }
#if MEOPT_WORDPRO
        case EVBUFFILLCOL:
            {
                meInt col=meAtoi(vvalue);
                if(col <= 0)
                    frameCur->bufferCur->fillcol = 0 ;
                else
                    frameCur->bufferCur->fillcol = (meUShort) col ;
                break;
            }
        case EVBUFFILLMODE:
            frameCur->bufferCur->fillmode = *vvalue ;
            break;
        case EVFILLBULLET:
            meStrncpy(fillbullet,vvalue,15);
            fillbullet[15] = '\0' ;
            break;
        case EVFILLBULLETLEN:
            fillbulletlen = (meUByte) meAtoi (vvalue);
            break;
        case EVFILLCOL:
            {
                meInt col=meAtoi(vvalue);
                if(col <= 0)
                    fillcol = 0 ;
                else
                    fillcol = (meUShort) col ;
                break;
            }
            break;
        case EVFILLEOS:
            meStrncpy(filleos,vvalue,15);
            filleos[15] = '\0' ;
            break;
        case EVFILLEOSLEN:
            filleoslen = (meUByte) meAtoi (vvalue);
            break;
        case EVFILLIGNORE:
            meStrncpy(fillignore,vvalue,15);
            fillignore[15] = '\0' ;
            break;
        case EVFILLMODE:
            fillmode = *vvalue ;
            break;
#endif
#if MEOPT_EXTENDED
        case EVTIME:
            timeOffset = meAtoi(vvalue) ;
            break;
        case EVRANDOM:
            srand((unsigned int) time(NULL)) ;
            break ;
#endif
        case EVFRMDPTH:
            return frameChangeDepth(meTRUE,meAtoi(vvalue)-(frameCur->depth+1));
#if MEOPT_EXTENDED
        case EVFRMID:
            frameCur->id = meAtoi(vvalue) ;
            break;
        case EVFRMTITLE:
            if(frameTitle != NULL)
                meFree(frameTitle) ;
            frameTitle = meStrdup(vvalue) ;
            /* force a redraw */
            sgarbf = meTRUE ;
            break;
#endif
        case EVFRMWDTH:
            return frameChangeWidth(meTRUE,meAtoi(vvalue)-frameCur->width);
        case EVABSCOL:
            return setcwcol(meAtoi(vvalue));
#if MEOPT_EXTENDED
        case EVABSLINE:
            return windowGotoAbsLine(meAtoi(vvalue));
        case EVWMRKCOL:
            if((status=meAtoi(vvalue)) < 0)
            {
                frameCur->windowCur->markOffset = 0 ;
                return meFALSE ;
            }
            else if(status && ((frameCur->bufferCur->intFlag & BIFNACT) == 0) &&
                    (frameCur->windowCur->markLine != NULL) &&
                    (status > meLineGetLength(frameCur->windowCur->markLine)))
            {
                frameCur->windowCur->markOffset = meLineGetLength(frameCur->windowCur->markLine) ;
                return meFALSE ;
            }
            frameCur->windowCur->markOffset = status ;
            return meTRUE ;
        case EVWMRKLINE:
            if((status=meAtoi(vvalue)) < 0)
                return meFALSE ;
            frameCur->windowCur->markOffset = 0 ;
            if(status == 0)
            {
                frameCur->windowCur->markLine = NULL ;
                frameCur->windowCur->markLineNo = 0 ;
            }
            else if(frameCur->bufferCur->intFlag & BIFNACT)
                frameCur->windowCur->markLineNo = status ;
            else
            {
                meLine   *odotp ;
                meUShort  odoto ;
                meInt     lineno ;

                odotp = frameCur->windowCur->dotLine ;
                lineno = frameCur->windowCur->dotLineNo ;
                odoto = frameCur->windowCur->dotOffset ;
                if((status = windowGotoLine(meTRUE,status)) > 0)
                {
                    frameCur->windowCur->markLine = frameCur->windowCur->dotLine ;
                    frameCur->windowCur->markLineNo = frameCur->windowCur->dotLineNo ;
                }
                frameCur->windowCur->dotLine = odotp ;
                frameCur->windowCur->dotLineNo = lineno ;
                frameCur->windowCur->dotOffset = odoto ;
                return status ;
            }
            return meTRUE ;
#endif
        case EVCURCOL:
            if((status=meAtoi(vvalue)) < 0)
            {
                frameCur->windowCur->dotOffset = 0 ;
                return meFALSE ;
            }
            else if(status && ((frameCur->bufferCur->intFlag & BIFNACT) == 0) &&
                    (status > meLineGetLength(frameCur->windowCur->dotLine)))
            {
                frameCur->windowCur->dotOffset = meLineGetLength(frameCur->windowCur->dotLine) ;
                return meFALSE ;
            }
            frameCur->windowCur->dotOffset = status ;
            return meTRUE ;
        case EVCURLINE:
            if((status=meAtoi(vvalue)) <= 0)
                return meFALSE ;
            if(frameCur->bufferCur->intFlag & BIFNACT)
            {
                frameCur->windowCur->dotLineNo = status ;
                return meTRUE ;
            }
            return windowGotoLine(meTRUE,status) ;
        case EVWINCHRS:
            {
                meUByte cc ;
                int len;
                len = meStrlen(vvalue);
                if (len > WCLEN)
                    len = WCLEN;
                while(--len >= 0)
                {
                    cc = vvalue[len] ;
                    if(!isPokable(cc))
                        cc = '.' ;
                    windowChars[len] = cc ;
                }
                if(meSystemCfg & meSYSTEM_SHOWWHITE)
                {
                    displayTab = windowChars[WCDISPTAB];
                    displayNewLine = windowChars[WCDISPCR];
                    displaySpace = windowChars[WCDISPSPACE];
                }
            }
            break;
#if MEOPT_EXTENDED
        case EVWFLAGS:
            frameCur->windowCur->flags = (meUShort) meAtoi(vvalue) ;
            break;
        case EVWID:
            frameCur->windowCur->id = meAtoi(vvalue) ;
            break;
        case EVWXSCROLL:
            if((status=meAtoi(vvalue)) < 0)
                status = 0 ;
            if(frameCur->windowCur->horzScrollRest != status)
            {
                frameCur->windowCur->horzScrollRest = status ;
                frameCur->windowCur->updateFlags |= WFREDRAW ;        /* Force a screen update */
                updCursor(frameCur->windowCur) ;
            }
            return meTRUE ;
        case EVWXCLSCROLL:
            if((status=meAtoi(vvalue)) < 0)
                status = 0 ;
            else if(status >= meLineGetLength(frameCur->windowCur->dotLine))
                status = meLineGetLength(frameCur->windowCur->dotLine)-1 ;
            if(frameCur->windowCur->horzScroll != status)
            {
                frameCur->windowCur->horzScroll = status ;
                frameCur->windowCur->updateFlags |= WFREDRAW ;        /* Force a screen update */
                updCursor(frameCur->windowCur) ;
            }
            return meTRUE ;
        case EVWYSCROLL:
            if((status=meAtoi(vvalue)) < 0)
                status = 0 ;
            else if(frameCur->bufferCur->lineCount && (status >= frameCur->bufferCur->lineCount))
                status = frameCur->bufferCur->lineCount-1 ;
            if(frameCur->windowCur->vertScroll != status)
            {
                frameCur->windowCur->vertScroll = status ;
                frameCur->windowCur->updateFlags |= WFMAIN|WFSBOX|WFLOOKBK ;
                reframe(frameCur->windowCur) ;
            }
            return meTRUE ;
#endif
        case EVCBUFNAME:
            unlinkBuffer(frameCur->bufferCur) ;
            if((vvalue[0] == '\0') || (bfind(vvalue,0) != NULL) ||
               ((nn = meStrdup(vvalue)) == NULL))
            {
                /* if the name is used */
                linkBuffer(frameCur->bufferCur) ;
                return meABORT ;
            }
            meFree(frameCur->bufferCur->name) ;
            frameCur->bufferCur->name = nn ;
            frameAddModeToWindows(WFMODE) ;
            linkBuffer(frameCur->bufferCur) ;
            break;
        case EVBUFFMOD:
            frameCur->bufferCur->stats.stmode = (meUShort) meAtoi(vvalue) ;
            break ;
        case EVGLOBFMOD:
            meUmask = (meUShort) meAtoi(vvalue) ;
            break ;
        case EVCFNAME:
            meStrrep(&frameCur->bufferCur->fileName,vvalue);
            frameAddModeToWindows(WFMODE) ;
            break;
#if MEOPT_DEBUGM
        case EVDEBUG:
            macbug = (meByte) meAtoi(vvalue);
            break;
#endif
#if MEOPT_TIMSTMP
        case EVTIMSTMP:
            meStrncpy(&time_stamp[0], vvalue, meTIME_STAMP_SIZE_MAX-1);
            time_stamp[meTIME_STAMP_SIZE_MAX-1] = '\0';
            break;
#endif
        case EVINDWIDTH:
            if((indentWidth = (meUByte) meAtoi(vvalue)) <= 0)
                indentWidth = 1 ;
            break;
        case EVBUFINDWIDTH:
            if((frameCur->bufferCur->indentWidth = (meUByte) meAtoi(vvalue)) <= 0)
                frameCur->bufferCur->indentWidth = 1 ;
            break;
        case EVTABWIDTH:
            if((tabWidth = (meUByte) meAtoi(vvalue)) <= 0)
                tabWidth = 1 ;
            frameAddModeToWindows(WFRESIZE) ;
            break;
        case EVBUFTABWIDTH:
            if((frameCur->bufferCur->tabWidth = (meUByte) meAtoi(vvalue)) <= 0)
                frameCur->bufferCur->tabWidth = 1 ;
            frameAddModeToWindows(WFRESIZE) ;
            break;
        case EVSRCHPATH:
            meStrrep(&searchPath,vvalue) ;
            break;
        case EVHOMEDIR:
            fileNameSetHome(vvalue) ;
            break;
#if MEOPT_EXTENDED
        case EVSHWMDS:
            {
                meUByte cc ;
                int nn ;
                for(nn=0; nn < MDNUMMODES ; nn++) 
                {
                    if((cc=*vvalue++) == '\0')
                        break ;
                    else if(cc == '0')
                        meModeClear(modeLineDraw,nn) ;
                    else
                        meModeSet(modeLineDraw,nn) ;
                }
                frameAddModeToWindows(WFMODE) ;
                break ;
            }
        case EVSHWRGN:
            selhilight.uFlags = (meUShort) meAtoi(vvalue);
            break;
#endif
        case EVCURSORBLK:
            timerKill(CURSOR_TIMER_ID) ;
            if((cursorBlink = meAtoi(vvalue)) > 0)
                /* kick off the timer */
                TThandleBlink(1) ;
            break ;
#if MEOPT_COLOR
        case EVCURSORCOL:
            if((cursorColor = (meColor) meAtoi(vvalue)) >= noColors)
                cursorColor = meCOLOR_FDEFAULT ;
            break ;
        case EVMLSCHM:
            mlScheme = convertUserScheme(meAtoi(vvalue),mlScheme);
            break ;
        case EVMDLNSCHM:
            mdLnScheme = convertUserScheme(meAtoi(vvalue),mdLnScheme);
            frameAddModeToWindows(WFMODE) ;
            break ;
        case EVGLOBSCHM:
            globScheme = convertUserScheme(meAtoi(vvalue),globScheme);
            frameCur->video.lineArray[frameCur->depth].endp = frameCur->width ;
            sgarbf = meTRUE ;
            TTsetBgcol() ;
            break ;
        case EVTRNCSCHM:
            trncScheme = convertUserScheme(meAtoi(vvalue),trncScheme);
            sgarbf = meTRUE ;
            break ;
        case EVBUFSCHM:
            frameCur->bufferCur->scheme = convertUserScheme(meAtoi(vvalue),frameCur->bufferCur->scheme);
            meBufferAddModeToWindows(frameCur->bufferCur,WFRESIZE) ;
            break ;
        case EVSBARSCHM:
            sbarScheme = convertUserScheme(meAtoi(vvalue),sbarScheme);
            frameAddModeToWindows(WFRESIZE|WFSBAR) ;
            break;
#if MEOPT_OSD
        case EVOSDSCHM:
            osdScheme = convertUserScheme(meAtoi(vvalue),osdScheme);
            break ;
#endif
#endif
        case EVSBAR:    
            gsbarmode = meAtoi(vvalue) & WMUSER;
            meFrameResizeWindows(frameCur,0);
            break;
#if MEOPT_IPIPES
        case EVBUFIPIPE:
            frameCur->bufferCur->ipipeFunc = decode_fncname(vvalue,1) ;
            break ;
#endif
#if MEOPT_FILEHOOK
        case EVBUFBHK:
            frameCur->bufferCur->bhook = decode_fncname(vvalue,1) ;
            break ;
        case EVBUFDHK:
            frameCur->bufferCur->dhook = decode_fncname(vvalue,1) ;
            break ;
        case EVBUFEHK:
            frameCur->bufferCur->ehook = decode_fncname(vvalue,1) ;
            break ;
        case EVBUFFHK:    
            frameCur->bufferCur->fhook = decode_fncname(vvalue,1) ;
            break ;
#endif
#if MEOPT_EXTENDED
        case EVBUFINP:
            frameCur->bufferCur->inputFunc = decode_fncname(vvalue,1) ;
            break ;
        case EVLINEFLAGS:
            frameCur->windowCur->dotLine->flag = ((meLineFlag) (meAtoi(vvalue) & meLINE_SET_MASK)) |
                      (frameCur->windowCur->dotLine->flag & ~meLINE_SET_MASK) ;
            break ;
#endif
#if MEOPT_HILIGHT
        case EVLINESCHM:
            {
                register int ii ;
                meScheme schm ;
                
                ii = meAtoi(vvalue) ;
                if(ii >= 0)
                {
                    schm = convertUserScheme(ii,frameCur->bufferCur->scheme);
                    for(ii=1 ; ii<meLINE_SCHEME_MAX ; ii++)
                        if(frameCur->bufferCur->lscheme[ii] == schm)
                            break ;
                    if(ii == meLINE_SCHEME_MAX)
                    {
                        if((ii=frameCur->bufferCur->lschemeNext+1) == meLINE_SCHEME_MAX)
                            ii = 1 ;
                        frameCur->bufferCur->lscheme[ii] = schm ;
                        frameCur->bufferCur->lschemeNext = ii ;
                    }
                    frameCur->windowCur->dotLine->flag = (frameCur->windowCur->dotLine->flag & ~meLINE_SCHEME_MASK) | (ii << meLINE_SCHEME_SHIFT) ;
                }
                else if(meLineIsSchemeSet(frameCur->windowCur->dotLine))
                    frameCur->windowCur->dotLine->flag = (frameCur->windowCur->dotLine->flag & ~meLINE_SCHEME_MASK) ;
                lineSetChanged(WFMAIN) ;
                break ;
            }
        case EVBUFHIL:
            if(((frameCur->bufferCur->hilight = (meUByte) meAtoi(vvalue)) >= noHilights) ||
               (hilights[frameCur->bufferCur->hilight] == NULL))
                frameCur->bufferCur->hilight = 0 ;
            meBufferAddModeToWindows(frameCur->bufferCur,WFRESIZE) ;
            break ;
        case EVBUFIND:
            if(((frameCur->bufferCur->indent = (meUByte) meAtoi(vvalue)) >= noIndents) ||
               (indents[frameCur->bufferCur->indent] == NULL))
                frameCur->bufferCur->indent = 0 ;
            break ;
#endif
#if MEOPT_EXTENDED
        case EVBUFMASK:
            {
                int ii ;
                isWordMask = 0 ;
                for(ii=0 ; ii<8 ; ii++)
                    if(meStrchr(vvalue,charMaskFlags[ii]) != NULL)
                        isWordMask |= 1<<ii ;
                if(frameCur->bufferCur->isWordMask != isWordMask)
                {
                    frameCur->bufferCur->isWordMask = isWordMask ;
#if MEOPT_MAGIC
                    mereRegexClassChanged() ;
#endif
                }
                break ;
            }
#endif
#if MEOPT_FILENEXT
        case EVFILETEMP:
            meStrrep(&flNextFileTemp,vvalue) ;
            break ;
        case EVLINETEMP:
            meStrrep(&flNextLineTemp,vvalue) ;
            break ;
#endif
#if MEOPT_RCS
        case EVRCSFILE:
            meStrrep(&rcsFile,vvalue) ;
            break ;
        case EVRCSCICOM:
            meStrrep(&rcsCiStr,vvalue) ;
            break ;
        case EVRCSCIFCOM:
            meStrrep(&rcsCiFStr,vvalue) ;
            break ;
        case EVRCSCOCOM:
            meStrrep(&rcsCoStr,vvalue) ;
            break ;
        case EVRCSCOUCOM:
            meStrrep(&rcsCoUStr,vvalue) ;
            break ;
        case EVRCSUECOM:
            meStrrep(&rcsUeStr,vvalue) ;
            break ;
#endif
        case EVUSERPATH:
            meStrrep(&meUserPath,vvalue);
            break;
        case EVSCROLL:
            scrollFlag = (meUByte) meAtoi(vvalue) ;
            break ;
        case EVQUIET:
            quietMode = (meUByte) meAtoi(vvalue) ;
            break;
#if MEOPT_EXTENDED
        case EVFILEIGNORE:
            meStrrep(&fileIgnore,vvalue) ;
            break ;
        case EVBNAMES:
            buffNames.curr = 0 ;
            meNullFree(buffNames.list) ;
            buffNames.list = NULL ;
            meStrrep(&buffNames.mask,vvalue) ;
            if(buffNames.mask != NULL)
                buffNames.size = createBuffList(&buffNames.list,0) ;
            break ;
        case EVCNAMES:
            commNames.curr = 0 ;
            meNullFree(commNames.list) ;
            commNames.list = NULL ;
            meStrrep(&commNames.mask,vvalue) ;
            if(commNames.mask != NULL)
            {
                commNames.size = createCommList(&commNames.list,1) ;
                sortStrings(commNames.size,commNames.list,0,(meIFuncSS) strcmp) ;
            }
            break ;
        case EVFNAMES:
            {
                meUByte *mm, *ss, cc, dd ;
                
                ss = mm = vvalue ;
                while(((cc=*ss++) != '\0') && (cc != '[') && ((dd=*ss) != '\0') &&
                      ((cc != '\\') || ((dd != '(') && (dd != '|') && (dd != '<'))))
                {
#ifdef _DRV_CHAR
                    if((cc == '/') || ((cc == _DRV_CHAR) && (*ss != '/')))
#else
                    if(cc == '/')
#endif
                        mm = ss ;
                }
                meStrrep(&fileNames.mask,mm) ;
                if(fileNames.mask != NULL)
                {
                    meUByte buff[meBUF_SIZE_MAX], path[meBUF_SIZE_MAX] ;
#ifdef _DRV_CHAR
                    if((isAlpha(mm[0]) || (mm[0] == '.' )) && (mm[1] == _DRV_CHAR))
                        path[0] = '\0' ;
                    else
#endif
                    {
                        *mm = '\0' ;
                        getFilePath(frameCur->bufferCur->fileName,buff) ;
                        meStrcat(buff,vvalue) ;
                        pathNameCorrect(buff,PATHNAME_COMPLETE,path,NULL) ;
                    }
                    getDirectoryList(path,&fileNames) ;
                    meStrcpy(resultStr,path) ;
                }
                fileNames.curr = 0 ;
                break ;
            }
        case EVMNAMES:
            modeNames.curr = 0 ;
            meStrrep(&modeNames.mask,vvalue) ;
            break ;
        case EVVNAMES:
            varbNames.curr = 0 ;
            freeFileList(varbNames.size,varbNames.list) ;
            varbNames.list = NULL ;
            meStrrep(&varbNames.mask,vvalue) ;
            if(varbNames.mask != NULL)
                varbNames.size = createVarList(&varbNames.list) ;
            break ;
#endif
#if MEOPT_SPELL
        case EVFINDWORDS:
            findWordsInit(vvalue) ;
            break ;
#endif
#if MEOPT_EXTENDED
        case EVRECENTKEYS:
            if(vvalue[0] != '\0')
                return meFALSE ;
            TTkeyBuf[TTnextKeyIdx] = 0 ;
            break ;
#endif
        case EVRESULT:                      /* Result string - assignable */
            meStrcpy (resultStr, vvalue);
            break;
        case -1:
            {
                /* unknown so set an environment variable */
                meUByte buff[meBUF_SIZE_MAX+meSBUF_SIZE_MAX], *ss ;
                sprintf((char *) buff,"%s=%s",nn,vvalue) ;
                if((ss=meStrdup(buff)) != NULL)
                    mePutenv(ss) ;
                break ;
            }
        default:
            /* default includes EVUSERNAME EVCBUFBACKUP EVSTATUS EVMACHINE 
             * EVSYSRET EVUSEX, EVVERSION, EVWMDLINE, EVEOBLINE or system dependant vars
             * where this isn't the system (e.g. use-x) which cant be set
             */
            return meFALSE ;
        }
        break ;
    default:
        /* else its not legal....bitch */
        return mlwrite(MWABORT,(meUByte *)"[No such variable]");
    }
    return meTRUE ;
}

static meUByte *
gtenv(meUByte *vname)   /* vname   name of environment variable to retrieve */
{
    int ii ;
    register meUByte *ret ;
#if MEOPT_EXTENDED
    register meNamesList *mv ;
#endif
    
    /* scan the list, looking for the referenced name */
    ii = biChopFindString(vname,14,envars,NEVARS) ;
    switch(ii)
    {
        /* Fetch the appropriate value */
    case EVAUTOTIME:    return meItoa(autoTime);
#if MEOPT_MOUSE
    case EVDELAYTIME:   return meItoa(delayTime);
    case EVREPEATTIME:  return meItoa(repeatTime);
#endif
#if MEOPT_CALLBACK
    case EVIDLETIME:    return meItoa(idleTime);
#endif
    case EVBOXCHRS:     return boxChars;
    case EVMODELINE:    return modeLineStr ;
    case EVMACHINE:     return machineName ;
    case EVPROGNM:      return meProgName ;
    case EVCURSORX:     return meItoa(frameCur->mainColumn);
    case EVCURSORY:     return meItoa(frameCur->mainRow);
    case EVSYSTEM:      return meItoa(meSystemCfg);
#if MEOPT_WORDPRO
    case EVBUFFILLCOL:  return meItoa(frameCur->bufferCur->fillcol) ;
    case EVBUFFILLMODE:
        evalResult[0] = frameCur->bufferCur->fillmode ;
        evalResult[1] = '\0' ;
        return evalResult ;
    case EVFILLBULLET:  return fillbullet;
    case EVFILLBULLETLEN:return meItoa(fillbulletlen);
    case EVFILLCOL:     return meItoa(fillcol);
    case EVFILLEOS:     return filleos;
    case EVFILLEOSLEN:  return meItoa(filleoslen);
    case EVFILLIGNORE:  return fillignore;
    case EVFILLMODE:
        evalResult[0] = fillmode ;
        evalResult[1] = '\0' ;
        return evalResult ;
#endif
#if MEOPT_EXTENDED
    case EVPAUSETIME:   return meItoa(pauseTime);
    case EVFSPROMPT:    return meItoa(fileSizePrompt);
    case EVKEPTVERS:    return meItoa(keptVersions);
    case EVMOUSE:       return meItoa(meMouseCfg);
    case EVMOUSEPOS:    return meItoa(mouse_pos);
    case EVMOUSEX:      return meItoa(mouse_X);
    case EVMOUSEY:      return meItoa(mouse_Y);
    case EVBNAMES:
        mv = &buffNames ;
        goto handle_namesvar ;
    case EVCNAMES:
        mv = &commNames ;
        goto handle_namesvar ;
    case EVFNAMES:
        mv = (meNamesList *) &fileNames ;
        goto handle_namesvar ;
    case EVMNAMES:
        mv = &modeNames ;
        goto handle_namesvar ;
    case EVVNAMES:
        mv = &varbNames ;
handle_namesvar:
        if(mv->mask == NULL)
            return abortm ;
        while(mv->curr < mv->size)
        {
            meUByte *ss = mv->list[(mv->curr)++] ;
            if(regexStrCmp(ss,mv->mask,(mv->exact) ? meRSTRCMP_WHOLE:meRSTRCMP_ICASE|meRSTRCMP_WHOLE))
                return ss ;
        }
        return emptym ;
    case EVTEMPNAME:
        mkTempName (evalResult, NULL,NULL);
        return evalResult ;
#endif
    case EVVERSION:
        return (meUByte *) meVERSION_CODE ;
#if MEOPT_SPELL
    case EVFINDWORDS:   return findWordsNext() ;
#endif
#if MEOPT_EXTENDED
    case EVRECENTKEYS:
        {
            int ii, jj, kk ;
            meUShort cc ;
            for(ii=100,jj=TTnextKeyIdx,kk=0 ; --ii>0 ; )
            {
                if(((cc=TTkeyBuf[jj++]) == 0) ||
                   ((kk+=meGetStringFromChar(cc,evalResult+kk)) > meBUF_SIZE_MAX-20))
                    break ;
                evalResult[kk++] = ' ' ;
                if(jj == KEYBUFSIZ)
                    jj = 0 ;
            }
            evalResult[kk] = '\0' ;
            return evalResult;
        }
#endif
    case EVRESULT:      return resultStr;
#if MEOPT_EXTENDED
    case EVTIME:
        {
            struct meTimeval tp ;           /* Time interval for current time */
            struct tm *time_ptr;            /* Pointer to time frame. */
            time_t clock;                   /* Time in machine format. */
            
            /* Format the result. Format is:-
             * 
             *    YYYYCCCMMDDWhhmmssSSS
             * 
             * Where:
             *     YYYY - Year i.e. 1997
             *     CCC  - Day of the week since January 1st (0-365).
             *     MM   - Month of year 1-12
             *     DD   - Day of the month 1-31
             *     W    - Weekday since sunday 0-6 (sunday == 0)
             *     hh   - Hours 0-23
             *     mm   - Minutes 0-59
             *     ss   - Seconds 0-59
             *     SSS  - Milliseconds 0-999
             */
            gettimeofday(&tp,NULL) ;
            clock = tp.tv_sec + timeOffset ;              /* Get system time */
            time_ptr = (struct tm *) localtime (&clock);  /* Get time frame */
            /* SWP - Took out the 0 padding because of macro use.
             *       ME interprets numbers of the form 0## as octagon based numbers
             *       so in september the number is 09 which is illegal etc.
             */
            sprintf ((char *)evalResult, "%4d%3d%2d%2d%1d%2d%2d%2d%3d",
                     time_ptr->tm_year+1900,/* Year - 2000 should be ok !! */
                     time_ptr->tm_yday,     /* Year day */
                     time_ptr->tm_mon+1,    /* Month */
                     time_ptr->tm_mday,     /* Day of the month */
                     time_ptr->tm_wday,     /* Day of the week */
                     time_ptr->tm_hour,     /* Hours */
                     time_ptr->tm_min,      /* Minutes */
                     time_ptr->tm_sec,      /* Seconds */
                     (int) (tp.tv_usec/1000));/* Milliseconds */
            return evalResult;
        }
    case EVRANDOM:      return meItoa(rand());
#endif
    case EVFRMDPTH:     return meItoa(frameCur->depth + 1);
#if MEOPT_EXTENDED
    case EVFRMID:       return meItoa(frameCur->id);
#endif
    case EVFRMWDTH:     return meItoa(frameCur->width);
    case EVABSCOL:      return meItoa(getcwcol());
    case EVCURCOL:      return meItoa(frameCur->windowCur->dotOffset);
    case EVCURLINE:     return meItoa(frameCur->windowCur->dotLineNo+1);
#if MEOPT_EXTENDED
    case EVABSLINE:     return meItoa(windowGotoAbsLine(-1)+1);
    case EVEOBLINE:     return meItoa(frameCur->bufferCur->lineCount+1);
    case EVWMRKCOL:     return meItoa(frameCur->windowCur->markOffset);
    case EVWMRKLINE:    return meItoa((frameCur->windowCur->markLine == NULL) ? 0:frameCur->windowCur->markLineNo+1);
    case EVBMDLINE:
        if(frameCur->bufferCur->modeLineStr == NULL)
            return modeLineStr ;
        return frameCur->bufferCur->modeLineStr ;
    case EVWXSCROLL:    return meItoa(frameCur->windowCur->horzScrollRest);
    case EVWXCLSCROLL:  return meItoa(frameCur->windowCur->horzScroll);
    case EVWYSCROLL:    return meItoa(frameCur->windowCur->vertScroll);
    case EVWMDLINE:     return meItoa(frameCur->windowCur->frameRow+frameCur->windowCur->textDepth);
    case EVWSBAR:       return meItoa(frameCur->windowCur->frameColumn+frameCur->windowCur->textWidth);
    case EVWFLAGS:      return meItoa(frameCur->windowCur->flags);
    case EVWID:         return meItoa(frameCur->windowCur->id);
    case EVWDEPTH:      return meItoa(frameCur->windowCur->textDepth);
    case EVWWIDTH:      return meItoa(frameCur->windowCur->textWidth);
#endif
    case EVWINCHRS:     return windowChars;
    case EVCBUFBACKUP:
        if((frameCur->bufferCur->fileName == NULL) || createBackupName(evalResult,frameCur->bufferCur->fileName,'~',0))
            return emptym ;
#if MEOPT_EXTENDED
#ifndef _DOS
        if(!(meSystemCfg & meSYSTEM_DOSFNAMES) && (keptVersions > 0))
            meStrcpy(evalResult+meStrlen(evalResult)-1,".~0~") ;
#endif
#endif
        return evalResult ;
    case EVCBUFNAME:    return frameCur->bufferCur->name ;
    case EVBUFFMOD:     return meItoa((((meInt) frameCur->bufferCur->fileFlag) << 16) | ((meInt) frameCur->bufferCur->stats.stmode));
    case EVGLOBFMOD:    return meItoa(meUmask);
    case EVCFNAME:      return mePtos(frameCur->bufferCur->fileName) ;
#if MEOPT_DEBUGM
    case EVDEBUG:       return meItoa(macbug);
#endif
    case EVSTATUS:      return meLtoa(cmdstatus);
#if MEOPT_TIMSTMP
    case EVTIMSTMP:     return time_stamp ;
#endif
    case EVINDWIDTH:    return meItoa(indentWidth);
    case EVBUFINDWIDTH: return meItoa(frameCur->bufferCur->indentWidth);
    case EVTABWIDTH:    return meItoa(tabWidth);
    case EVBUFTABWIDTH: return meItoa(frameCur->bufferCur->tabWidth);
    case EVSRCHPATH:    return mePtos(searchPath) ;
    case EVHOMEDIR:     return mePtos(homedir) ;
#if MEOPT_EXTENDED
    case EVMODECHRS:    return modeCode ;
    case EVSHWMDS:
        {
            ret = evalResult;
            for (ii=0; ii < MDNUMMODES ; ii++) 
                *ret++ = (meModeTest(modeLineDraw,ii)) ? '1':'0' ;
            *ret = '\0' ;
            return evalResult;
        }
    case EVSHWRGN:      return meItoa(selhilight.uFlags);
#endif
    case EVCURSORBLK:   return meItoa(cursorBlink);
#if MEOPT_COLOR
    case EVCURSORCOL:   return meItoa(cursorColor);
    case EVMLSCHM:      return meItoa(mlScheme/meSCHEME_STYLES);
    case EVMDLNSCHM:    return meItoa(mdLnScheme/meSCHEME_STYLES);
    case EVSBARSCHM:    return meItoa(sbarScheme/meSCHEME_STYLES);
    case EVGLOBSCHM:    return meItoa(globScheme/meSCHEME_STYLES);
    case EVTRNCSCHM:    return meItoa(trncScheme/meSCHEME_STYLES);
    case EVBUFSCHM:     return meItoa(frameCur->bufferCur->scheme/meSCHEME_STYLES);
    case EVSBAR:        return meItoa(gsbarmode);
#if MEOPT_OSD
    case EVOSDSCHM:     return meItoa(osdScheme/meSCHEME_STYLES);
#endif
#endif
#if MEOPT_IPIPES
    case EVBUFIPIPE:
        ii = frameCur->bufferCur->ipipeFunc ;
        goto hook_jump ;
#endif
#if MEOPT_FILEHOOK
    case EVBUFBHK:
        ii = frameCur->bufferCur->bhook ;
        goto hook_jump ;
    case EVBUFDHK:
        ii = frameCur->bufferCur->dhook ;
        goto hook_jump ;
    case EVBUFEHK:
        ii = frameCur->bufferCur->ehook ;
        goto hook_jump ;
    case EVBUFFHK:    
        ii = frameCur->bufferCur->fhook ;
        goto hook_jump ;
#endif
#if MEOPT_EXTENDED
    case EVBUFINP:
        ii = frameCur->bufferCur->inputFunc ;
hook_jump:
        if(ii < 0)
            evalResult[0] = '\0';
        else
            meStrcpy(evalResult,getCommandName(ii)) ;
        return evalResult ;
    case EVLINEFLAGS:   return meItoa(frameCur->windowCur->dotLine->flag) ;
    case EVBUFMASK:
        {
            meUByte *ss=evalResult ;
            int   ii ;
            for(ii=0 ; ii<8 ; ii++)
                if(isWordMask & (1<<ii))
                    *ss++ = charMaskFlags[ii] ;
            *ss = '\0' ;
            return evalResult ;
        }
    case EVFILEIGNORE:  return mePtos(fileIgnore) ;
#endif
#if MEOPT_HILIGHT
    case EVLINESCHM:    return meItoa(meLineIsSchemeSet(frameCur->windowCur->dotLine) ? 
                                      (frameCur->bufferCur->lscheme[meLineGetSchemeIndex(frameCur->windowCur->dotLine)]/meSCHEME_STYLES):-1) ;
    case EVBUFHIL:      return meItoa(frameCur->bufferCur->hilight);
    case EVBUFIND:      return meItoa(frameCur->bufferCur->indent);
#endif
#if MEOPT_FILENEXT
    case EVFILETEMP:    return mePtos(flNextFileTemp) ;
    case EVLINETEMP:    return mePtos(flNextLineTemp) ;
#endif
#if MEOPT_RCS
    case EVRCSFILE:     return mePtos(rcsFile) ;
    case EVRCSCOCOM:    return mePtos(rcsCoStr) ;
    case EVRCSCOUCOM:   return mePtos(rcsCoUStr) ;
    case EVRCSCICOM:    return mePtos(rcsCiStr) ;
    case EVRCSCIFCOM:   return mePtos(rcsCiFStr) ;
    case EVRCSUECOM:    return mePtos(rcsUeStr) ;
#endif
    case EVUSERNAME:    return mePtos(meUserName) ;
    case EVUSERPATH:    return mePtos(meUserPath) ;
    case EVSCROLL:      return meItoa(scrollFlag) ;
    case EVQUIET:       return meItoa(quietMode);
    }
    if((ret = meGetenv(vname)) == NULL)
        ret = errorm ; /* errorm on bad reference */
    
    return ret ;
}

/* look up a user var's value */
/* vname - name of user variable to fetch */
meUByte *
getUsrLclCmdVar(meUByte *vname, register meVarList *varList)
{
    register meVariable *vptr, *vp ;
    register int ii, jj ;
    
    vptr = varList->head ;
    ii = varList->count ;
    /* scan the list looking for the user var name */
    while(ii)
    {
        meByte *s1, *s2, ss ;
        
        jj = ii>>1 ;
        vp = vptr ;
        while(--jj >= 0)
            vp = vp->next ;
        
        s1 = (meByte *) vp->name ;
        s2 = (meByte *) vname ;
        for( ; ((ss=*s1++) == *s2) ; s2++)
            if(ss == 0)
                return(vp->value);
        if(ss > *s2)
            ii = ii>>1 ;
        else
        {
            ii -= (ii>>1) + 1 ;
            vptr = vp->next ;
        }
    }
    return errorm ;     /* return errorm on a bad reference */
}


static meUByte *
getMacroArg(int index)
{
    meRegister *crp ;
    meUByte *oldestr, *ss ;
    
    /* move the register pointer to the parent as any # reference
     * will be w.r.t the parent
     */
    crp = meRegCurr ;
    if((ss=crp->execstr) == NULL)
        return NULL ;
    oldestr = execstr ;
    execstr = ss ;
    meRegCurr = crp->prev ;
    if(alarmState & meALARM_VARIABLE)
        gmaLocalRegPtr = meRegCurr ;
    for( ; index>0 ; index--)
    {
        execstr = token(execstr, evalResult) ;
        ss = getval(evalResult) ;
    }
    execstr   = oldestr ;
    meRegCurr = crp ;
    return ss ;
}


meUByte *
getval(meUByte *tkn)   /* find the value of a token */
{
    switch (getMacroTypeS(tkn)) 
    {
    case TKARG:
        if(tkn[1] == 'w')
        {
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            if(tkn[2] == 'l')
            {
                register int blen ;
                meUByte *ss, *dd ;
                /* Current buffer line fetch */
                blen = frameCur->windowCur->dotLine->length;
                if (blen >= meBUF_SIZE_MAX)
                    blen = meBUF_SIZE_MAX-1 ;
                ss = frameCur->windowCur->dotLine->text ;
                dd = evalResult ;
                while(--blen >= 0)
                    *dd++ = *ss++ ;
                *dd = '\0' ;
            }
            else
            {
                /* Current Buffer character fetch */
                evalResult[0] = meWindowGetChar(frameCur->windowCur) ;
                evalResult[1] = '\0';
            }
        }
        else if(tkn[1] == '#')
        {
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            return meItoa(meRegCurr->n) ;
        }
        else if(tkn[1] == '?')
        {
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            return meItoa(meRegCurr->f) ;
        }
        else if(isDigit(tkn[1]))
        {
            int index ;
            meUByte *ss ;
            
            if((index = meAtoi(tkn+1)) == 0)
            {
                ss = meRegCurr->commandName ;
                if(ss == NULL)
                    return (meUByte *) "" ;
            }
            else if((ss = getMacroArg(index)) == NULL)
                return abortm ;
            return ss ;
        }
#if MEOPT_EXTENDED
        else if((tkn[1] == 's') && isDigit(tkn[2]))
        {
            int ii ;
            
            if(srchLastState != meTRUE)
                return abortm ;
            if((ii=tkn[2]-'0') == 0)
                return srchLastMatch ;
#if MEOPT_MAGIC
            if(srchLastMagic && (ii <= mereRegexGroupNo()))
            {
                int jj ;
                if((jj=mereRegexGroupStart(ii)) >= 0)
                {
                    ii=mereRegexGroupEnd(ii) - jj ;
                    if(ii >= meBUF_SIZE_MAX)
                        ii = meBUF_SIZE_MAX-1 ;
                    memcpy(evalResult,srchLastMatch+jj,ii) ;
                }
                else
                    ii = 0 ;
                evalResult[ii] = '\0';
                return evalResult ;
            }
#endif
            return abortm ;
        }
        else if(tkn[1] == 'h')
        {
            meUByte buff[meBUF_SIZE_MAX] ;
            int ht, hn ;
            
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            
            /* get the history type */
            if((tkn[2] > '0') && (tkn[2] <= '4'))
                ht = tkn[2]-'0' ;
            else
                ht = 0 ;
            
            /* Get and evaluate the next argument - this is the history number */
            if(meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0)
                return abortm ;
            hn = meAtoi(buff) ;
            
            return mePtos(strHist[(ht*meHISTORY_SIZE)+hn]) ;
        }
        else if(tkn[1] == 'y')
        {
            meUByte *ss, *dd, cc ;
            meKillNode *killp;
            meKill *kl ;
            int ii ;
            
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            
            if(isDigit(tkn[2]))
            {
                /* Don't use the X or windows clipboard in this case */
                ii = meAtoi(tkn+2) ;
                kl = klhead ;
                while((kl != NULL) && (--ii >= 0))
                    kl = kl->next ;
            }
            else
            {
#ifdef _CLIPBRD
                TTgetClipboard() ;
#endif
                kl = klhead ;
            }
            if(kl == (meKill*) NULL)
                return abortm ;
            dd = evalResult ;
            killp = kl->kill ;
            ii = meBUF_SIZE_MAX-1 ;
            while(ii && (killp != NULL))
            {
                ss = killp->data ;
                while(((cc=*ss++) != '\0') && (--ii >= 0))
                    *dd++ = cc ;
                killp = killp->next;
            }
            *dd = '\0' ;
        }
        else if(tkn[1] == 'c')
        {
            meUShort key ;
            int kk, index ;
                        
            kk = (tkn[3] == 'k') ;
            if(tkn[2] == 'c')
            {
                if(alarmState & meALARM_VARIABLE)
                    return tkn ;
                key = (meUShort) thisCommand ;
                index = thisIndex ;
            }
            else if(tkn[2] == 'l')
            {
                if(alarmState & meALARM_VARIABLE)
                    return tkn ;
                key = (meUShort) lastCommand ;
                index = lastIndex ;
            }
            else
            {
                if(tkn[2] == 'q')
                {
                    /* intercative single char read which will be quoted */
                    int cc ;
                    key = meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT|meGETKEY_SINGLE) ;
                    if((cc=quoteKeyToChar(key)) > 0)
                    {
                        if(tkn[3] == 'k')
                        {
                            evalResult[0] = cc ;
                            evalResult[1] = '\0';
                            return evalResult ;
                        }
                        key = cc ;
                    }
                }
                else
                    key = meGetKeyFromUser(meFALSE, 1, 0) ;
                if(!kk)
                {
                    meUInt arg ;
                    index = decode_key(key,&arg) ;
                }
            }
            if(kk)
                meGetStringFromKey(key,evalResult) ;
            else if(index < 0)
            {
                if(key > 0xff)
                    return errorm ;
                evalResult[0] = (meUByte) key ;
                evalResult[1] = '\0';
            }
            else
                meStrcpy(evalResult,getCommandName(index)) ;
        }
        else if(tkn[1] == 'm')
        {
            if(tkn[2] == 'l')
            {
                /* interactive argument */
                /* note: the result buffer cant be used directly as it is used
                 *       in modeline which is is by update which can be used
                 *       by meGetStringFromUser
                 */
                static meUByte **strList=NULL ;
                static int strListSize=0 ;
                meUByte cc, *ss, buff[meBUF_SIZE_MAX] ;
                meUByte comp[meBUF_SIZE_MAX], *tt, divChr ;
                meUByte prompt[meBUF_SIZE_MAX] ;
                int option=0 ;
                int ret, flag, defH ;
                
                if((flag=tkn[3]) != '\0')
                {
                    flag = hexToNum(flag) ;
                    if(((cc=tkn[4]) > '0') && (cc<='9'))
                        cc -= '0' ;
                    else if(cc == 'a')
                        cc = 10 ;
                    else
                        cc = 11 ;
                    if(cc == 1)
                        option = MLFILE ;
                    else
                    {
                        if(cc == 7)
                        {
                            mlgsStrList = modeName ;
                            mlgsStrListSize = MDNUMMODES ;
                        }
                        else if(cc == 9)
                        {
                            flag |= 0x100 ;
                            cc = 7 ;
                        }
                        else if(cc == 10)
                        {
                            flag |= 0x300 ;
                            cc = 7 ;
                        }
                        if(cc < 11)
                            option = 1 << (cc-2) ;
                    }
#if (MLFILECASE != MLFILE)
                    if(option & MLFILE)
                        option |= MLFILECASE ;
#if (MLBUFFERCASE != MLBUFFER)
                    else if(option & MLBUFFER)
                        option |= MLBUFFERCASE ;
#endif
#endif
                }
                else
                    cc = 0 ;
                
                /* Get and evaluate the next argument - this is the prompt */
                if(meGetString(NULL,0,0,prompt,meBUF_SIZE_MAX) <= 0)
                    return abortm ;
                if(flag & 0x01)
                {
                    /* Get and evaluate the next argument - this is the default
                     * value */
                    if(meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0)
                        return abortm ;
                    addHistory(option,buff) ;
                    defH = 1 ;
                }
                else
                    defH = 0 ;
                if(flag & 0x02)
                {
                    /* Get and evaluate the next argument - this is the ml
                     * line init value */
                    if(meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0)
                        return abortm ;
                    option |= MLNORESET ;
                }
                if(flag & 0x04)
                    /* set the first key to 'tab' to expand the input according 
                     * to the completion list */
                    meGetKeyFirst = ME_SPECIAL|SKEY_tab ;
                if(flag & 0x08)
                    option |= MLNOHIST|MLHIDEVAL ;
                
                if(flag & 0x100)
                {
                    /* Get and evaluate the next argument - this is the
                     * completion list */
                    if(meGetString(NULL,0,0,comp,meBUF_SIZE_MAX) <= 0)
                        return abortm ;
                    if(flag & 0x200)
                    {
                        meBuffer *bp ;
                        meLine *lp ;
                        flag &= ~0x100 ;
                        if((bp = bfind(comp,0)) == NULL)
                            return abortm ;
                        if(bp->lineCount > strListSize)
                        {
                            strListSize = bp->lineCount ;
                            if((strList = meRealloc(strList,strListSize*sizeof(meUByte *))) == NULL)
                            {
                                strListSize = 0 ;
                                return abortm ;
                            }
                        }
                        lp = bp->baseLine ;
                        mlgsStrListSize = 0 ;
                        while((lp=meLineGetNext(lp)) != bp->baseLine)
                            strList[mlgsStrListSize++] = lp->text ;
                    }
                    else
                    {
                        ss = comp ;
                        divChr = *ss++ ;
                        mlgsStrListSize = 0 ;
                        while((tt = meStrchr(ss,divChr)) != NULL)
                        {
                            *tt++ = '\0' ;
                            if(mlgsStrListSize == strListSize)
                            {
                                if((strList = meRealloc(strList,(strListSize+8)*sizeof(meUByte *))) == NULL)
                                {
                                    strListSize = 0 ;
                                    return abortm ;
                                }
                                strListSize += 8 ;
                            }
                            strList[mlgsStrListSize++] = ss ;
                            ss = tt ;
                        }
                    }
                    mlgsStrList = strList ;
                }
                if(cc == 1)
                {
                    if(!(option & MLNORESET))
                        getFilePath(frameCur->bufferCur->fileName,buff) ;
                    option &= MLHIDEVAL ;
                    option |= MLFILECASE|MLNORESET|MLMACNORT ;

                    if((ret = meGetStringFromUser(prompt,option,0,buff,meBUF_SIZE_MAX)) > 0)
                        fileNameCorrect(buff,evalResult,NULL) ;
                }
                else
                {
                    /* Note that buffer result cannot be used directly as in the process
                     * of updating the screen the result buffer can be used by meItoa
                     * so use a temp buffer and copy across. note that inputFileName
                     * above doesn't suffer from this as the function uses a temp buffer
                     */
                    ret = meGetStringFromUser(prompt,option,defH,buff,meBUF_SIZE_MAX) ;
                    meStrncpy(evalResult,buff,meBUF_SIZE_MAX) ;
                    evalResult[meBUF_SIZE_MAX-1] = '\0' ;
                }
                if(ret < 0)
                    return abortm ;
            }
            else if(tkn[2] == 'c')
            {
                meUByte *ss, *helpStr, buff[meBUF_SIZE_MAX] ;
                meUByte prompt[meBUF_SIZE_MAX], helpBuff[meBUF_SIZE_MAX] ;
                int ret ;
                
                if((ret=tkn[3]) != '\0')
                    ret -= '0' ;
                
                /* Get and evaluate the next argument - this is the prompt */
                if(meGetString(NULL,0,0,prompt,meBUF_SIZE_MAX) <= 0)
                    return abortm ;
                if(ret & 0x01)
                {
                    /* Get and evaluate the next argument - this is valid
                     * values list */
                    if(meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0)
                        return abortm ;
                    ss = buff ;
                }
                else
                    ss = NULL ;
                if(ret & 0x04)
                {
                    /* Get and evaluate the next argument - this is the help string */
                    if(meGetString(NULL,0,0,helpBuff,meBUF_SIZE_MAX) <= 0)
                        return abortm ;
                    helpStr = helpBuff ;
                }
                else
                    helpStr = NULL ;
                ret = (ret & 0x02) ? mlCR_QUOTE_CHAR:0 ;
                
                clexec = meFALSE ;
                ret = mlCharReply(prompt,ret,ss,helpStr) ;
                clexec = meTRUE ;
                if(ret < 0)
                    return abortm ;
                evalResult[0] = ret ;
                evalResult[1] = 0 ;
            }
            else
                return abortm ;
                    
        }
        else if(tkn[1] == 'p')
        {
            /* parent command name */
            if(meRegCurr->prev->commandName == NULL)
                return (meUByte *) "" ;
            return meRegCurr->prev->commandName ;
        }
        else if((tkn[1] == 'f') && (tkn[2] == 's'))
        {
            /* frame store @fs <row> <col> */
            meUByte buff[meBUF_SIZE_MAX] ;
            int row, col ;
            
            /* Get and evaluate the arguments */
            if((meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0) ||
               ((row = meAtoi(buff)) < 0) || (row > frameCur->depth) ||
               (meGetString(NULL,0,0,buff,meBUF_SIZE_MAX) <= 0) ||
               ((col = meAtoi(buff)) < 0) || (col >= frameCur->width))
                evalResult[0] = 0 ;
            else if(tkn[3] == 's')
                return meItoa((frameCur->store[row].scheme[col] & meSCHEME_STYLE)/meSCHEME_STYLES) ;
            else
            {
                evalResult[0] = frameCur->store[row].text[col] ;
                evalResult[1] = 0 ;
            }
        }
        else
#endif
        {
            mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unknown argument %s]",tkn);
            return abortm ;
        }
        return evalResult ;
        
    case TKREG:
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        {
            meRegister *rp ;
            meUByte cc ;
            
            if((cc=tkn[1]) == 'l')
                rp = meRegCurr ;
            else if(cc == 'p')
                rp = meRegCurr->prev ;
            else
                rp = meRegHead ;
            cc = tkn[2] - '0' ;
            if(cc < ('0'+meREGISTER_MAX))
                return rp->reg[cc] ;
            break ;
        }
    
#if MEOPT_EXTENDED
    case TKVAR:
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        return getUsrVar(tkn+1) ;
    
    case TKLVR:
        {
            meUByte *ss, *tt ;
            meBuffer *bp ;
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            tt = tkn+1 ;
            if((ss=meStrrchr(tt,':')) != NULL)
            {
                *ss = '\0' ;
                bp = bfind(tt,0) ;
                *ss++ = ':' ;
                if(bp == NULL)
                    /* not a buffer - abort */
                    return errorm ;
                tt = ss ;
            }
            else
                bp = frameCur->bufferCur ;
            return getUsrLclCmdVar(tt,&(bp->varList)) ;
        }
        
    case TKCVR:
        {
            meUByte *ss, *tt ;
            if(alarmState & meALARM_VARIABLE)
                return tkn ;
            tt = tkn+1 ;
            if((ss=meStrrchr(tt,'.')) != NULL)
            {
                int ii ;
                *ss = '\0' ;
                ii = decode_fncname(tt,1) ;
                *ss++ = '.' ;
                if(ii < 0)
                    /* not a command - error */
                    return errorm ;
                return getUsrLclCmdVar(ss,&(cmdTable[ii]->varList)) ;
            }
            if(meRegCurr->varList == NULL)
                return abortm ;
            return getUsrLclCmdVar(tt,meRegCurr->varList) ;
        }
#endif
    case TKENV:
        if(alarmState & meALARM_VARIABLE)
            return tkn ;
        return gtenv(tkn+1) ;
    
    case TKFUN:
        return gtfun(tkn+1) ;
    
    case TKSTR:
        tkn++ ;
    case TKCMD:
    case TKLIT:
        return tkn ;
    }
    /* If here then it is either TKDIR, TKLBL or TKNUL. Doesn't matter which
     * cause something has gone wrong and we return abortm
     */
    return abortm ;
}

static meUByte *
queryMode(meUByte *name, meMode mode)
{
    int ii ;
    
    for (ii=0; ii < MDNUMMODES ; ii++) 
        if(meStrcmp(name,modeName[ii]) == 0) 
            return meLtoa(meModeTest(mode,ii)) ;
    return abortm ;
}

#if (defined _DOS) || (defined _WIN32)
meInt
meGetDayOfYear(meInt year, meInt month, meInt day)
{
    static int dom[11]={31,28,31,30,31,30,31,31,30,31,30} ;
    
    if(month > 2)
    {
        if(((year & 0x03) == 0) &&
           (((year % 100) != 0) || ((year % 400) == 0)))
            day++ ;
    }
    month-- ;
    while(--month >= 0)
        day += dom[month] ;
    return day ;
}
#endif

meUByte *
gtfun(meUByte *fname)  /* evaluate a function given name of function */
{
#if MEOPT_EXTENDED
    meRegister *regs ;     /* pointer to relevant regs if setting var */
#endif
    register int fnum;      /* index to function to eval */
    meUByte arg1[meBUF_SIZE_MAX];      /* value of first argument */
    meUByte arg2[meBUF_SIZE_MAX];      /* value of second argument */
    meUByte arg3[meBUF_SIZE_MAX];      /* value of third argument */
    meUByte *varVal ;
    
    /* look the function up in the function table */
    if((fnum = biChopFindString(fname,3,funcNames,NFUNCS)) < 0)
    {
        mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unknown function &%s]",fname);
        return abortm ;
    }
#if MEOPT_EXTENDED
    if((fnum == UFCBIND) || (fnum == UFNBIND))
    {
        meUInt arg ;
        meUShort ii ;
        int jj ;
        
        ii = meGetKey(meGETKEY_SILENT) ;
        if((jj = decode_key(ii,&arg)) < 0)
            return errorm ;
        if(fnum == UFCBIND)
            meStrcpy(evalResult,getCommandName(jj)) ;
        else if(arg == 0)
            evalResult[0] = '\0' ;
        else
            return meItoa((int) (arg + 0x80000000)) ;
        return evalResult ;
    }
#endif
    /* retrieve the arguments */
    {
        meUByte alarmStateStr=alarmState ;
        int ii ;
        
#if MEOPT_EXTENDED
        if(funcTypes[fnum] & FUN_SETVAR)
        {
            /* horrid global variable, see notes at definition */
            gmaLocalRegPtr = meRegCurr ;
            alarmState |= meALARM_VARIABLE ;
            ii = macarg(arg1) ;
            alarmState &= ~meALARM_VARIABLE ;
            regs = gmaLocalRegPtr ;
            if((ii > 0) && (funcTypes[fnum] & FUN_GETVAR) &&
               ((varVal = getval(arg1)) == abortm))
                ii = meFALSE ;
        }
        else
#endif
        {
            alarmState &= ~meALARM_VARIABLE ;
            ii = macarg(arg1) ;
        }
        if((ii <= 0) ||
           ((funcTypes[fnum] & FUN_ARG2) &&
            ((macarg(arg2) <= 0) ||
             ((funcTypes[fnum] & FUN_ARG3) &&
              (macarg(arg3) <= 0)))))
        {
            mlwrite(MWABORT|MWWAIT,(meUByte *)"[Failed to get argument for function &%s]",fname);
            return abortm ;
        }
        alarmState = alarmStateStr ;
    }
    
    /* and now evaluate it! */
    switch(fnum)
    {
    case UFIND:
        /* one gotcha here is that if we are getting a variable name
         * getval can return arg1! in this case we must copy arg1 to
         * evalResult */
        if((varVal=getval(arg1)) != arg1)
            return varVal ;
        meStrcpy (evalResult, arg1);
        return evalResult;
#if MEOPT_EXTENDED
    case UFEXIST:
        switch (getMacroTypeS(arg1)) 
        {
        case TKARG:
        case TKREG:
        case TKVAR:
        case TKLVR:
        case TKCVR:
        case TKENV:
            /* all variable types */
            varVal=getval(arg1) ;
            return (meLtoa((varVal != abortm) && (varVal != errorm))) ;
            
        case TKCMD:
            return (meLtoa(decode_fncname(arg1,1) >= 0)) ;
            
        default:
            /* If here then it is either TKARG, TKFUN, TKDIR, TKSTR, TKLIT, TKLBL or TKNUL.
             * Doesn't matter which cause &exist doesn't work on these, return NO
             */
            return meLtoa(0) ;
        }
    case UFKBIND:
        {
            meUInt narg ;
            int ii, idx ;
            meUShort code=ME_INVALID_KEY ;
            
            if((idx = decode_fncname(arg2,0)) < 0)
                return abortm ;
            if(arg1[0] == '\0')
                narg = 0 ;
            else
                narg = (meUInt) (0x80000000+meAtoi(arg1)) ;
#if MEOPT_LOCALBIND
            for(ii=0 ; ii<frameCur->bufferCur->bindCount ; ii++)
            {
                if((frameCur->bufferCur->bindList[ii].index == idx) &&
                   (frameCur->bufferCur->bindList[ii].arg == narg))
                {
                    code = frameCur->bufferCur->bindList[ii].code ;
                    break ;
                }
            }
            if(code == ME_INVALID_KEY)
#endif
            {
                for(ii=0 ; ii<keyTableSize ; ii++)
                    if((keytab[ii].index == idx) &&
                       (keytab[ii].arg == narg))
                    {
                        code = keytab[ii].code ;
                        break ;
                    }
            }
            if(code != ME_INVALID_KEY)
                meGetStringFromKey(code,evalResult) ;
            else
                evalResult[0] = '\0' ;
            return evalResult ;
        }
#endif
    case UFABS:        return meItoa(abs(meAtoi(arg1)));
    case UFADD:        return meItoa(meAtoi(arg1) + meAtoi(arg2));
    case UFSUB:        return meItoa(meAtoi(arg1) - meAtoi(arg2));
    case UFMUL:        return meItoa(meAtoi(arg1) * meAtoi(arg2));
    case UFDIV:
    case UFMOD:
        {
            int ii, jj ;
            
            /* Check for divide/mod by zero, this causes an exception if
             * encountered so we must abort if detected. */
            if((jj = meAtoi(arg2)) == 0)
            {
                mlwrite(MWABORT|MWWAIT,(meUByte *) 
                        ((fnum == UFDIV) ? "[Divide by zero &div]":"[Modulus of zero &mod]")) ;
                return abortm ;
            }
            ii = meAtoi(arg1) ;
            if(fnum == UFDIV)
                ii /= jj ;
            else
                ii %= jj ;
            return meItoa(ii) ;
        }
    case UFNEG:        return meItoa(0-meAtoi(arg1));
    case UFOPT:
        arg1[3] = '\0' ;
        return meLtoa(meStrstr(meOptionList,arg1) != NULL) ;
        
#if MEOPT_EXTENDED
    case UFATOI:       return meItoa(arg1[0]) ;
    case UFITOA:
        {
            int ii = meAtoi(arg1) ;
            evalResult[0] = (meUByte) ii ;
            if(ii == meCHAR_LEADER)
            {
                evalResult[1] = meCHAR_TRAIL_LEADER ;
                evalResult[2] = '\0' ;
            }
            else
                evalResult[1] = '\0' ;
            return evalResult ;
        }
#endif
    case UFCAT:
        {
            meUByte *dd, *ss ;
            int ii = meBUF_SIZE_MAX-1 ;
            
            /* first string can be copied, second we must check the left */
            dd = evalResult ;
            ss = arg1 ;
            while((*dd++ = *ss++))
                ii-- ;
            dd-- ;
            ss = arg2 ;
            do {
                if(--ii <= 0)
                {
                    *dd = '\0' ;
                    break ;
                }
            } while((*dd++ = *ss++)) ;
            return evalResult ;
        }
    case UFLEFT:
        {
            meUByte *dd, *ss, cc ;
            int ii=meAtoi(arg2) ;
            ss = arg1 ;
            if(ii < 0)
                ii = meStrlen(ss) + ii ;
            dd = evalResult ;
            while(--ii >= 0)
            {
                if((cc=*ss++) == meCHAR_LEADER)
                {
                    *dd++ = cc ;
                    cc = *ss++ ;
                }
                if(cc == '\0')
                    break ;
                *dd++ = cc ;
            }
            *dd = '\0' ;
            return evalResult ;
        }
    case UFRIGHT:
        {
            meUByte *dd, *ss, cc ;
            int ii=meAtoi(arg2) ;
            ss = arg1 ;
            if(ii < 0)
                ii = meStrlen(ss) + ii ;
            while(--ii >= 0)
            {
                if((cc=*ss++) == meCHAR_LEADER)
                    cc = *ss++ ;
                if(cc == '\0')
                {
                    ss-- ;
                    break ;
                }
            }
            dd = evalResult ;
            while((*dd++ = *ss++) != '\0')
                ;
            return evalResult ;
        }
    case UFMID:
        {
            meUByte *dd, *ss, cc ;
            int ii, ll ;
            ss = arg1 ;
            
            ll = meAtoi(arg3) ;
            ii = meAtoi(arg2) ;
            if(ii < 0)
                ii = meStrlen(ss) + ii ;
            if(ii > 0)
            {
                do
                {
                    if((cc=*ss++) == meCHAR_LEADER)
                        cc = *ss++ ;
                    if(cc == '\0')
                    {
                        ss-- ;
                        break ;
                    }
                } while(--ii > 0) ;
            }
            else if(ll >= 0)
                ll += ii ;
            if(ll < 0)
                ll = meStrlen(ss) + ll ;
            dd = evalResult ;
            while(--ll >= 0)
            {
                if((cc=*ss++) == meCHAR_LEADER)
                {
                    *dd++ = cc ;
                    cc = *ss++ ;
                }
                if(cc == '\0')
                    break ;
                *dd++ = cc ;
            }
            *dd = '\0' ;
            return evalResult ;
        }
    case UFLEN:
        {
            meUByte *ss, cc ;
            int ii=0 ;
            ss = arg1 ;
            for(;;)
            {
                if((cc=*ss++) == meCHAR_LEADER)
                    cc=*ss++ ;
                if(cc == '\0')
                    break ;
                ii++ ;
            }
            return meItoa(ii) ;
        }
#if MEOPT_EXTENDED
    case UFSLOWER:
        {
            meUByte cc, *dd, *ss ;
            dd = evalResult ;
            ss = arg1 ;
            do {
                cc = *ss++ ;
                *dd++ = toLower(cc) ;
            } while(cc) ;
            return evalResult ;
        }
    case UFSUPPER:
        {
            meUByte cc, *dd, *ss ;
            dd = evalResult ;
            ss = arg1 ;
            do {
                cc = *ss++ ;
                *dd++ = toUpper(cc) ;
            } while(cc) ;
            return evalResult ;
        }
    case UFSIN:
    case UFRSIN:
    case UFISIN:
    case UFRISIN:
        {
            meIFuncSSI cmpIFunc ;
            meUByte cc, *ss=arg2, *lss=NULL ;
            int len, off=0 ;
            len = meStrlen(arg1) ;
            if((fnum == UFSIN) || (fnum == UFRSIN))
                cmpIFunc = (meIFuncSSI) strncmp ;
            else
                cmpIFunc = meStrnicmp ;
            
            do
            {
                if(!cmpIFunc(arg1,ss,len))
                {
                    lss = ss ;
                    if((fnum == UFSIN) || (fnum == UFISIN))
                        break ;
                }
                if((cc=*ss++) == meCHAR_LEADER)
                {
                    cc = *ss++ ;
                    off++ ;
                }
            } while(cc != '\0') ;
            
            if(lss != NULL)
                return meItoa(lss-arg2-off+1);
            return meLtoa(0) ;
        }
    case UFREP:
    case UFIREP:
        {
            meIFuncSSI cmpIFunc ;
            meUByte cc, *ss=arg1 ;
            int mlen, dlen=0, rlen, ii ;
            mlen = meStrlen(arg2) ;
            rlen = meStrlen(arg3) ;
            if(fnum == UFREP)
                cmpIFunc = (meIFuncSSI) strncmp ;
            else
                cmpIFunc = meStrnicmp ;
            
            for(;;)
            {
                ii = cmpIFunc(arg2,ss,mlen) ;
                if(!ii)
                {
                    if((dlen+rlen) >= meBUF_SIZE_MAX)
                        break ;
                    meStrcpy(evalResult+dlen,arg3) ;
                    dlen += rlen ;
                    ss += mlen ;
                }
                if((cc=*ss) == '\0')
                    break ;
                if(ii || (mlen == 0))
                {
                    if(dlen >= meBUF_SIZE_MAX-2)
                        break ;
                    ss++ ;
                    if(cc == meCHAR_LEADER)
                    {
                        evalResult[dlen++] = meCHAR_LEADER ;
                        cc = *ss++ ;
                    }
                    evalResult[dlen++] = cc ;
                }
            }
            evalResult[dlen] = '\0' ;
            return evalResult ;
        }
    case UFXREP:
    case UFXIREP:
        {
            meUByte cc, *rr ;
            int slen, soff=0, mlen, dlen=0, ii ;
            
            if(meRegexComp(&meRegexStrCmp,arg2,(fnum == UFXREP) ? 0:meREGEX_ICASE) != meREGEX_OKAY)
                return abortm ;
    
            slen = meStrlen(arg1) ;
            for(;;)
            {
                ii = meRegexMatch(&meRegexStrCmp,arg1,slen,soff,slen,(meREGEX_BEGBUFF|meREGEX_ENDBUFF)) ;
                if(ii)
                {
                    mlen = meRegexStrCmp.group[0].start - soff ;
                    if(dlen+mlen >= meBUF_SIZE_MAX)
                        break ;
                    meStrncpy(evalResult+dlen,arg1+soff,mlen) ;
                    dlen += mlen ;
                    rr = arg3 ;
                    while((cc=*rr++) != '\0')
                    {
                        if(dlen >= meBUF_SIZE_MAX-1)
                            break ;
                        if((cc == '\\') && (*rr != '\0'))
                        {
                            int changeCase=0 ;
                            cc = *rr++ ;
                            if(((cc == 'l') || (cc == 'u') || (cc == 'c')) && (*rr != '\0'))
                            {
                                changeCase = (cc == 'l') ? -1:cc ;
                                cc = *rr++ ;
                            }
                            if((cc == '&') || ((cc >= '0') && (cc <= '9') && ((((int) cc) - '0') <= meRegexStrCmp.groupNo)))
                            {
                                cc = (cc == '&') ? 0:(cc-'0') ;
                                /* if start offset is < 0, it was a ? auto repeat which was not found,
                                   therefore replace str == "" */ 
                                if((soff=meRegexStrCmp.group[cc].start) >= 0)
                                {
                                    mlen = meRegexStrCmp.group[cc].end - soff ;
                                    if(dlen+mlen >= meBUF_SIZE_MAX)
                                        break ;
                                    if(cc)
                                        soff += meRegexStrCmp.group[0].start ;
                                    if(changeCase)
                                    {
                                        while(--mlen >= 0)
                                        {
                                            cc = arg1[soff++] ;
                                            if(changeCase > 0)
                                            {
                                                evalResult[dlen++] = toUpper(cc) ;
                                                if(changeCase == 'c')
                                                    changeCase = -1 ;
                                            }
                                            else
                                                evalResult[dlen++] = toLower(cc) ;
                                        } 
                                    }
                                    else
                                    {
                                        meStrncpy(evalResult+dlen,arg1+soff,mlen) ;
                                        dlen += mlen ;
                                    }
                                }
                            }
                            else
                            {
                                if(cc == 'n')
                                    cc = '\n' ;
                                else if(cc == 'r')
                                    cc = '\r' ;
                                else if(cc == 't')
                                    cc = '\t' ;
                                evalResult[dlen++] = cc ;
                            }
                        }
                        else
                            evalResult[dlen++] = cc ;
                    }
                    mlen = meRegexStrCmp.group[0].end - meRegexStrCmp.group[0].start ;
                    soff = meRegexStrCmp.group[0].end ;
                }
                if((cc=arg1[soff]) == '\0')
                    break ;
                if(!ii || (mlen == 0))
                {
                    if(dlen >= meBUF_SIZE_MAX-2)
                        break ;
                    soff++ ;
                    if(cc == meCHAR_LEADER)
                    {
                        evalResult[dlen++] = meCHAR_LEADER ;
                        cc = arg1[soff++] ;
                    }
                    evalResult[dlen++] = cc ;
                }
            }
            evalResult[dlen] = '\0' ;
            return evalResult ;
        }
    case UFNBMODE:
        {
            meBuffer *bp ;
            if((bp = bfind(arg1,0)) == NULL)
                return abortm ;
            return queryMode(arg2,bp->mode) ;
        }
    case UFINWORD:     return(meLtoa(isWord(arg1[0])));
#endif
    case UFBMODE:      return queryMode(arg1,frameCur->bufferCur->mode) ;
    case UFGMODE:      return queryMode(arg1,globMode) ;
    case UFBAND:       return(meItoa(meAtoi(arg1) & meAtoi(arg2)));
    case UFBNOT:       return(meItoa(~meAtoi(arg1))) ;
    case UFBOR:        return(meItoa(meAtoi(arg1) |  meAtoi(arg2)));
    case UFBXOR:       return(meItoa(meAtoi(arg1) ^  meAtoi(arg2)));
    case UFNOT:        return(meLtoa(meAtol(arg1) == meFALSE));
    case UFAND:        return(meLtoa(meAtol(arg1) && meAtol(arg2)));
    case UFOR:         return(meLtoa(meAtol(arg1) || meAtol(arg2)));
    case UFEQUAL:      return(meLtoa(meAtoi(arg1) == meAtoi(arg2)));
    case UFLESS:       return(meLtoa(meAtoi(arg1) <  meAtoi(arg2)));
    case UFGREATER:    return(meLtoa(meAtoi(arg1) >  meAtoi(arg2)));
    case UFSEQUAL:     return(meLtoa(meStrcmp(arg1,arg2) == 0));
    case UFSLESS:      return(meLtoa(meStrcmp(arg1,arg2) < 0));
    case UFSGREAT:     return(meLtoa(meStrcmp(arg1,arg2) > 0));
    case UFISEQUAL:    return(meLtoa(meStricmp(arg1,arg2) == 0));
#if MEOPT_EXTENDED
    case UFXSEQ:       return(meLtoa(regexStrCmp(arg1,arg2,meRSTRCMP_WHOLE|meRSTRCMP_USEMAIN) == 1));
    case UFXISEQ:      return(meLtoa(regexStrCmp(arg1,arg2,meRSTRCMP_ICASE|meRSTRCMP_WHOLE|meRSTRCMP_USEMAIN) == 1));
    case UFLDEL:
        {
            int  index=meAtoi(arg2) ;
            meUByte cc, *s1, *s2 ;
            if(index > 0)
            {
                s2 = arg1 ;
                cc = *s2 ;
                do {
                    s1 = s2+1 ;
                    if((s2 = meStrchr(s1,cc)) == NULL)
                        break ;
                } while(--index > 0) ;
            }
            else
                s2 = NULL ;
            if(s2 == NULL)
                meStrcpy(evalResult,arg1) ;
            else
            {
                index = (int) (s1 - arg1) ;
                meStrncpy(evalResult,arg1,index) ;
                meStrcpy(evalResult+index,s2+1) ;
            }
            return evalResult ;
        }
    case UFLFIND:
        {
            int  index ;
            meUByte cc, *s1, *s2 ;
            s2 = arg1 ;
            cc = *s2 ;
            for(index=1 ; ; index++)
            {
                s1 = s2+1 ;
                if((s2 = meStrchr(s1,cc)) == NULL)
                    return meLtoa(0) ;
                *s2 = '\0' ;
                if(!meStrcmp(s1,arg2))
                    return meItoa(index) ;
            }
        }
    case UFLLEN:
        {
            int  index ;
            meUByte cc, *s1, *s2 ;
            s2 = arg1 ;
            cc = *s2 ;
            for(index=1 ; ; index++)
            {
                s1 = s2+1 ;
                if((s2 = meStrchr(s1,cc)) == NULL)
                    return meItoa(index-1) ;
                *s2 = '\0' ;
            }
        }
    case UFLGET:
        {
            int  index ;
            meUByte cc, *s1, *s2 ;
            if((index=meAtoi(arg2)) <= 0)
                return emptym ;
            s2 = arg1 ;
            cc = *s2 ;
            do {
                s1 = s2+1 ;
                if((s2 = meStrchr(s1,cc)) == NULL)
                    return emptym ;
            } while(--index > 0) ;
            index = (int) (s2 - s1) ;
            meStrncpy(evalResult,s1,index) ;
            evalResult[index] = '\0' ;
            return evalResult ;
        }
    case UFLINS:
    case UFLSET:
        {
            int  index=meAtoi(arg2), ii ;
            meUByte cc, *s1, *s2 ;
            s2 = arg1 ;
            cc = *s2 ;
            if(index <= 0)
            {
                if((fnum == UFLINS) && (index < -1))
                {
                    do
                    {
                        s1 = s2+1 ;
                        if((s2 = meStrchr(s1,cc)) == NULL)
                        {
                            s2 = s1-1 ;
                            break ;
                        }
                        *s2 = '\0' ;
                        if(index == -2)
                            ii = (meStrcmp(s1,arg3) < 0) ;
                        else
                            ii = (meStricmp(s1,arg3) < 0) ;
                        *s2 = cc ;
                    } while(ii) ;
                    s2 = s1-1 ;
                }
                else
                {
                    if(index < 0)
                        s2 += meStrlen(s2)-1 ;
                    s1 = s2+1 ;
                }
            }
            else
            {
                do {
                    s1 = s2+1 ;
                    if((s2 = meStrchr(s1,cc)) == NULL)
                    {
                        s2 = s1-1 ;
                        break ;
                    }
                } while(--index > 0) ;
            }
            index = (int) (s1 - arg1) ;
            meStrncpy(evalResult,arg1,index) ;
            ii = meStrlen(arg3) ;
            if(ii+index < meBUF_SIZE_MAX)
            {
                meStrncpy(evalResult+index,arg3,ii) ;
                index += ii ;
                if(fnum == UFLINS)
                    s2 = s1-1 ;
                ii = meStrlen(s2) ;
                if(ii+index < meBUF_SIZE_MAX)
                {
                    meStrcpy(evalResult+index,s2) ;
                    index += ii ;
                }
            }
            evalResult[index] = '\0' ;
            return evalResult ;
        }
    case UFFIND:
        {
            if(!fileLookup(arg1,arg2,meFL_CHECKDOT|meFL_USESRCHPATH,evalResult))
                return errorm ;
            return evalResult ;
        }
    case UFWHICH:
        {
            if(!executableLookup(arg1,evalResult))
                return errorm ;
            return evalResult ;
        }
    case UFCOND:
        {
            if(meAtol(arg1))
                meStrcpy(evalResult,arg2) ;
            else
                meStrcpy(evalResult,arg3) ;
            return evalResult ;
        }
    case UFTRIMB:
        {
            /* Trim both */
            meUByte *ss;
            meUByte cc;
            int ii;
            
            /* Trim left */
            for (ss = arg1; (((cc = *ss) != '\0') && isSpace(cc)) ; ss++)
                ;
            /* Trim right */
            ii = meStrlen (ss);
            while (ii > 0)
            {
                cc = ss[ii-1];
                if (isSpace(cc))
                    ii--;
                else
                    break;
            }
            meStrncpy(evalResult, ss, ii);
            evalResult[ii] = '\0';
            return evalResult;
        }
    case UFTRIML:
        {
            /* Trim left */
            meUByte *ss;
            meUByte cc;
            for (ss = arg1; (((cc = *ss) != '\0') && isSpace(cc)) ; ss++)
                ;
            meStrcpy(evalResult, ss);
            return evalResult;
        }
    case UFTRIMR:
        {
            /* Trim right */
            int ii;
            ii = meStrlen (arg1);
            while (ii > 0)
            {
                meUByte cc;
                cc = arg1[ii-1];
                if (isSpace(cc))
                    ii--;
                else
                    break;
            }
            meStrncpy (evalResult, arg1, ii);
            evalResult[ii] = '\0';
            return evalResult;
        }
    case UFSET:
        {
            if(setVar(arg1,arg2,regs) <= 0)
                return abortm ;
            meStrcpy (evalResult, arg2);
            return evalResult;
        }
    case UFDEC:
        {
            varVal = meItoa(meAtoi(varVal) - meAtoi(arg2));
            if(setVar(arg1,varVal,regs) <= 0)
                return abortm ;
            return varVal ;
        }
    case UFINC:
        {
            varVal = meItoa(meAtoi(varVal) + meAtoi(arg2));
            if(setVar(arg1,varVal,regs) <= 0)
                return abortm ;
            return varVal ;
        }
    case UFPDEC:
        {
            /* cannot copy into evalResult here cos meItoa uses it */
            meStrcpy(arg3,varVal) ;
            varVal = meItoa(meAtoi(varVal) - meAtoi(arg2));
            if(setVar(arg1,varVal,regs) <= 0)
                return abortm ;
            meStrcpy(evalResult,arg3) ;
            return evalResult ;
        }
    case UFPINC:
        {
            /* cannot copy into evalResult here cos meItoa uses it */
            meStrcpy(arg3,varVal) ;
            varVal = meItoa(meAtoi(varVal) + meAtoi(arg2));
            if(setVar(arg1,varVal,regs) <= 0)
                return abortm ;
            meStrcpy(evalResult,arg3) ;
            return evalResult ;
        }
#endif
#if MEOPT_REGISTRY
    case UFREGISTRY:
        {
            meRegNode *reg;
            meUByte *p = arg2;
            /*
             * Read a value from the registry.
             * 
             * Arg1 is the path,
             * Arg2 is the subkey
             * Arg3 is the default value
             * 
             * If the node cannot be found then return the result.
             */
            if((reg = regFind (NULL, arg1)) != NULL)
                p = regGetString (reg, p) ;
            /* Set up the result string */
            if (p == NULL)
                evalResult [0] = '\0';
            else
                meStrcpy (evalResult, p);
            return evalResult;
        }
#endif
#if MEOPT_EXTENDED
    case UFSPRINT:                         /* Monamic function !! */
        {
            /*
             * Do a sprintf type of function. 
             * &sprintf "This is a %s %s." "arg1" "arg2"
             *
             * Note that we build the result in a buffer (arg3) on the stack
             * which allows us to effectivelly do recursive invocations 
             * of &sprintf.
             * 
             * Note that this is a monamic function, the first argument is the
             * format string. The rest of the arguments are dynamic and are
             * feteched as required.
             * 
             * Note - need to sort the '"' character. Not sure how macro process
             * it when specified in string.
             * 
             * Syntax defined as follows:-
             * 
             * sprintf "%s" arg
             * sprintf "%n" count arg
             * sprintf "%d" 10
             * 
             * e.g.
             *     sprintf "Foo [%s%s]" "a" "b"
             * generates:-
             *     "Foo [ab]"
             * 
             *     sprintf "Foo [%n%s]" 10 "a" "b"
             * generates:-
             *     "Foo [aaaaaaaaaab]"
             */
            
            meUByte *p;                            /* Temp char pointer */
            meUByte  c;                            /* Current char */
            meUByte *s = arg1;                     /* String pointer */
            int   count, nn, index=0 ;          /* Count of repeat */
            meUByte alarmStateStr=alarmState ;
            
            alarmState &= ~meALARM_VARIABLE ;
            
            while ((c = *s++) != '\0') 
            {
                if (c == '%')                   /* Check for control */
                {
                    p = s-1 ;
                    count = 1 ;
get_flag:
                    switch((c = *s++))
                    {
                    case 'n':                   /* Replicate string */
                        if (macarg (arg2) <= 0) 
                            return abortm;
                        count *= meAtoi(arg2);
                    case 's':                   /* String */
                        if (macarg (arg2) <= 0)
                            return abortm;
                        nn = meStrlen(arg2) ;
                        while(--count >= 0)
                        {
                            if(index+nn >= meBUF_SIZE_MAX)
                                /* break if we dont have enough space */
                                break ;
                            meStrcpy(arg3+index,arg2) ;
                            index += nn ;
                        }
                        break;
                    case 'X':                   /* Hexadecimal */
                    case 'x':                   /* Hexadecimal */
                    case 'd':                   /* Decimal */
                    case 'o':                   /* Octal */
                        if (macarg (arg2) <= 0) 
                            return abortm;
                        nn = meAtoi(arg2) ;
                        c = *s ;
                        *s = '\0' ;
                        if(index < meBUF_SIZE_MAX-16)
                            /* Only do it if we have enough space */
                            index += sprintf((char *)arg3+index,(char *)p,nn) ;
                        *s = c ;
                        break;
                    default:
                        if(isDigit(c))
                        {
                            count = c - '0' ;
                            while((c = *s++),isDigit(c)) 
                                count = (count*10) + (c-'0') ;
                            s-- ;
                            goto get_flag ;
                        }
                        else if(index >= meBUF_SIZE_MAX)
                            break ;
                        else
                            arg3[index++] = c;  /* Just a literal char - pass in */
                    }
                }
                else if(index >= meBUF_SIZE_MAX)
                    break ;
                else
                    arg3[index++] = c;          /* Just a literal char - pass in */
            }
            arg3[index] = '\0';                 /* make sure sting terminated */
            meStrcpy(evalResult,arg3) ;
            alarmState = alarmStateStr ;
            return evalResult ;
        }
    case UFSTAT:
        {
            static meUByte typeRet[] = "NRDXHF" ;
            meStat stats ;
            int    ftype ;
            /*
             * 0 -> N - If file is nasty
             * 1 -> R - If file is regular
             * 2 -> D - If file is a directory
             * 3 -> X - If file doesn't exist
             * 4 -> H - If file is a HTTP url
             * 5 -> F - If file is a FTP url
             */
            ftype = getFileStats(arg2,1,&stats,evalResult) ;
            switch(arg1[0])
            {
            case 'a':
                /* absolute name - check for symbolic link */
                if(*evalResult == '\0')
                    fileNameCorrect(arg2,evalResult,NULL) ;
                /* if its a directory check there's an ending '/' */
                if(ftype == meFILETYPE_DIRECTORY)
                {
                    meUByte *ss=evalResult+meStrlen(evalResult) ;
                    if(ss[-1] != DIR_CHAR)
                    {
                        ss[0] = DIR_CHAR ;
                        ss[1] = '\0' ;
                    }
                }
                return evalResult ;
                
            case 'd':
                /* file date stamp */
                ftype = meFiletimeToInt(stats.stmtime) ;
                break ;
                
            case 'i':
                {
                    /* file information in list form as follows:
                     *  |<url-type>|<file-type>|<link-file-type>|<attrib>|<size-upper>|<size-lower>|
                     * Note:
                     *   - this version differs from 't' as links have a file
                     *     type of 'L' and ftp files return the file type not 'F'
                     *   - The fourth value is the file attributes (-1 == not available)
                     *   - The size has an upper int and lower int value to handle
                     *     large files
                     *   - The file modification time will be the 7th value (n.y.i.)
                     */ 
                    meUByte v1, v2, v3='\0', v5=0, *dd, v7[16] ;
                    meUInt v51, v52 ;
                    int v4=-1 ;
                    
                    v7[0] = '\0' ;
                    if(ftype == meFILETYPE_HTTP)
                        v1 = v2 = 'H' ;
                    else if(ftype == meFILETYPE_FTP)
                    {
                        v1 = 'F' ;
                        if(ffFileOp(arg2,NULL,meRWFLAG_STAT|meRWFLAG_SILENT,-1) > 0)
                        {
                            v2 = evalResult[0] ;
                            dd = evalResult + 1 ;
                            if(*dd != '|')
                            {
                                v5 = 1 ;
                                v51 = 0 ;
                                v52 = meAtoi(dd) ;
                                dd = meStrchr(dd,'|') ;
                            }
                            dd++ ;
                            if(*dd != '\0')
                            {
                                meStrncpy(v7,dd,15) ;
                                v7[15] = '\0' ;
                            }
                        }
                        else
                            v2 = 'N' ;
                    }
                    else
                    {
#ifdef _UNIX
                        struct tm *tmp;            /* Pointer to time frame. */
                        if ((tmp = localtime(&stats.stmtime)) != NULL)
                            sprintf((char *)v7, "%4d%2d%2d%2d%2d%2d",
                                    tmp->tm_year+1900,tmp->tm_mon+1,tmp->tm_mday,
                                    tmp->tm_hour,tmp->tm_min,tmp->tm_sec);
#endif
#ifdef _DOS
                        if((stats.stmtime & 0x0ffff) != 0x7fff)
                            sprintf((char *)v7,"%4d%2d%2d%2d%2d%2d",
                                    (int) ((stats.stmtime >> 25) & 0x007f)+1980,
                                    (int) ((stats.stmtime >> 21) & 0x000f),
                                    (int) ((stats.stmtime >> 16) & 0x001f),
                                    (int) ((stats.stmtime >> 11) & 0x001f),
                                    (int) ((stats.stmtime >>  5) & 0x003f),
                                    (int) ((stats.stmtime & 0x001f)  << 1)) ;
#endif
#ifdef _WIN32
                        SYSTEMTIME tmp;
                        FILETIME ftmp;
                    
                        if(FileTimeToLocalFileTime(&stats.stmtime,&ftmp) && FileTimeToSystemTime(&ftmp,&tmp))
                            sprintf((char *)v7,"%4d%2d%2d%2d%2d%2d",
                                    tmp.wYear,tmp.wMonth,tmp.wDay,
                                    tmp.wHour,tmp.wMinute,tmp.wSecond) ;
#endif
                        v1 = 'L' ;
                        if(evalResult[0] != '\0')
                        {
                            meStat lstats ;
                            v2 = 'L' ;
                            v3 = typeRet[getFileStats(evalResult,0,&lstats,NULL)] ;
                        }
                        else
                            v2 =  typeRet[ftype] ;
                        if(ftype != meFILETYPE_NOTEXIST)
                        {
                            v4 = meFileGetAttributes(arg2) ;
                            v5 = 1 ;
                            v51 = stats.stsizeHigh ;
                            v52 = stats.stsizeLow ;
                        }
                    }
                    dd = evalResult ;
                    *dd++ = '\b' ;
                    *dd++ = v1 ;
                    *dd++ = '\b' ;
                    *dd++ = v2 ;
                    *dd++ = '\b' ;
                    if(v3 != '\0')
                        *dd++ = v3 ;
                    *dd++ = '\b' ;
                    if(v4 >= 0) 
                        dd += sprintf((char *)dd,"%d",v4) ;
                    *dd++ = '\b' ;
                    if(v5)
                        dd += sprintf((char *)dd,"0x%x\b0x%x",v51,v52) ;
                    else
                        *dd++ = '\b' ;
                    *dd++ = '\b' ;
                    if(v7[0] != '\0')
                    {
                        meStrcpy(dd,v7) ;
                        dd += meStrlen(dd) ;
                    }
                    *dd++ = '\b' ;
                    *dd = '\0' ;
                    return evalResult;
                }
            case 'm':
                {
                    /* file modified date stamp in a useable form */
                    /* Format same as $time variable */
#ifdef _UNIX
                    struct tm *tmp;            /* Pointer to time frame. */
                    if ((tmp = localtime(&stats.stmtime)) != NULL)
                        sprintf((char *)evalResult, "%4d%3d%2d%2d%1d%2d%2d%2d  -",
                                tmp->tm_year+1900,tmp->tm_yday,tmp->tm_mon+1,tmp->tm_mday,
                                tmp->tm_wday,tmp->tm_hour,tmp->tm_min,tmp->tm_sec);
                    else
#endif
#ifdef _DOS
                    if((stats.stmtime & 0x0ffff) != 0x7fff)
                    {
                        meInt year, month, day, doy ;
                        
                        year = ((stats.stmtime >> 25) & 0x007f)+1980 ;
                        month = ((stats.stmtime >> 21) & 0x000f) ;
                        day = ((stats.stmtime >> 16) & 0x001f) ;
                        doy = meGetDayOfYear(year,month,day) ;
                        sprintf((char *)evalResult,"%4d%3d%2d%2d-%2d%2d%2d  -",
                                (int) year,(int) doy,(int) month,(int) day,
                                (int) ((stats.stmtime >> 11) & 0x001f),
                                (int) ((stats.stmtime >>  5) & 0x003f),
                                (int) ((stats.stmtime & 0x001f)  << 1)) ;
                    }
                    else
#endif
#ifdef _WIN32
                    SYSTEMTIME tmp;
                    FILETIME ftmp;
                    meInt doy ;
                    
                    if(FileTimeToLocalFileTime(&stats.stmtime,&ftmp) && FileTimeToSystemTime(&ftmp,&tmp))
                    {
                        doy = meGetDayOfYear(tmp.wYear,tmp.wMonth,tmp.wDay) ;
                        sprintf((char *)evalResult, "%4d%3d%2d%2d%1d%2d%2d%2d%3d",
                                tmp.wYear,(int) doy,tmp.wMonth,tmp.wDay,tmp.wDayOfWeek,tmp.wHour,
                                tmp.wMinute,tmp.wSecond,tmp.wMilliseconds) ;
                    }
                    else
#endif
                        sprintf((char *)evalResult, "   -  - - -- - - -  -") ;
                    return evalResult;
                }
            
            case 'r':
                /* File read permission */
                /* If its a nasty or it doesn't exist - no */
                if((ftype == meFILETYPE_NASTY) || (ftype == meFILETYPE_NOTEXIST))
                    return meLtoa(0) ;
                /* If its a url then do we support url ? */
                if((ftype == meFILETYPE_HTTP) || (ftype == meFILETYPE_FTP))
#if MEOPT_SOCKET
                    return meLtoa(1) ;
#else
                    return meLtoa(0) ;
#endif
                /* If its a DOS/WIN directory - yes */
#if (defined _DOS) || (defined _WIN32)
                if(ftype == meFILETYPE_DIRECTORY)
                    return meLtoa(1) ;
#endif
                /* normal test */
                return meLtoa(meStatTestRead(stats)) ;
                
            case 's':
                /* File size - return -1 if not a regular file */
                if(ftype != meFILETYPE_REGULAR)
                    ftype = -1 ;
                else if((stats.stsizeHigh > 0) || (stats.stsizeLow & 0x80000000))
                    ftype = 0x7fffffff ;
                else
                    ftype = stats.stsizeLow ;
                break ;
            case 't':
                /* File type - use look up table, see first comment */
                evalResult[0] = typeRet[ftype] ;
                evalResult[1] = '\0' ;
                return evalResult ;
            
            case 'w':
                /* File write permission */
                /* If nasty or url - no */
                if((ftype == meFILETYPE_NASTY) || (ftype == meFILETYPE_HTTP) ||
                   (ftype == meFILETYPE_FTP))
                    return meLtoa(0) ;
                
                /* if it doesnt exist or its an DOS/WIN directory - yes */
                if((ftype == meFILETYPE_NOTEXIST)
#if (defined _DOS) || (defined _WIN32)
                   || (ftype == meFILETYPE_DIRECTORY)
#endif
                   )
                    return meLtoa(1) ;
                return meLtoa(meStatTestWrite(stats)) ;
            
            case 'x':
                /* File execute permission */
                /* If nasty or doesnt exist, or url then no */
                if((ftype == meFILETYPE_NASTY) || (ftype == meFILETYPE_NOTEXIST) ||
                   (ftype == meFILETYPE_HTTP) || (ftype == meFILETYPE_FTP))
                    return meLtoa(0) ;
#if (defined _DOS) || (defined _WIN32)
                /* If directory, simulate unix style execute flag and return yes */
                if(ftype == meFILETYPE_DIRECTORY)
                    return meLtoa(1) ;
                /* Must test for .exe, .com, .bat, .btm extensions etc */
                return meLtoa(meTestExecutable(arg2)) ;
#else
                /* Test the unix execute flag */
                return meLtoa((((stats.stuid == meUid) && (stats.stmode & S_IXUSR)) ||
                               ((stats.stgid == meGid) && (stats.stmode & S_IXGRP)) ||
                               (stats.stmode & S_IXOTH))) ;
#endif
            default:
                return abortm ;
            }                
            return meItoa(ftype) ;
        }
    case UFBSTAT:
        {
            int ret ;
            switch(arg1[0])
            {
            case 'o':
                ret = bufferOutOfDate(frameCur->bufferCur) ;
                break ;
            default:
                return abortm ;
            }                
            return meItoa(ret) ;
        }
#endif
    }
    return abortm ;
}


#if MEOPT_EXTENDED
/* numeric arg can overide prompted value */
int
unsetVariable(int f, int n)     /* Delete a variable */
{
    register meVarList  *varList ;      /* User variable pointer */
    register meVariable *vptr, *prev;   /* User variable pointer */
    register int   vnum;                /* ordinal number of var refrenced */
    meRegister *regs ;
    meUByte var[meSBUF_SIZE_MAX] ;                 /* name of variable to fetch */
    meUByte *vv ;
    
    /* First get the variable to set.. */
    /* set this flag to indicate that the variable name is required, NOT
     * its value */
    alarmState |= meALARM_VARIABLE ;
    /* horrid global variable, see notes at definition */
    gmaLocalRegPtr = meRegCurr ;
    vnum = meGetString((meUByte *)"Unset variable", MLVARBL, 0, var, meSBUF_SIZE_MAX) ;
    regs = gmaLocalRegPtr ;
    alarmState &= ~meALARM_VARIABLE ;
    if(vnum <= 0)
        return vnum ;
    
    /* Check the legality and find the var */
    vv = var+1 ;
    vnum = getMacroTypeS(var) ; 
    if(vnum == TKVAR) 
        varList = &usrVarList ;
    else if(vnum == TKLVR)
    {        
        meUByte *ss ;
        meBuffer *bp ;
        
        if((ss=meStrrchr(vv,':')) != NULL)
        {
            *ss = '\0' ;
            bp = bfind(vv,0) ;
            *ss++ = ':' ;
            if(bp == NULL)
                /* not a command - abort */
                return mlwrite(MWABORT,(meUByte *)"[No such variable]");
            vv = ss ;
        }
        else
            bp = frameCur->bufferCur ;
        varList = &(bp->varList) ;
    }
    else if(vnum == TKCVR)
    {
        meUByte *ss ;
        if((ss=meStrrchr(vv,'.')) != NULL)
        {
            int idx ;
            
            *ss = '\0' ;
            idx = decode_fncname(var+1,0) ;
            *ss++ = '.' ;
            if(idx < 0)
                return mlwrite(MWABORT,(meUByte *)"[No such variable]");
            varList = &(cmdTable[idx]->varList) ;
            vv = ss ;
        }
        else if((varList = regs->varList) == NULL)
            return mlwrite(MWABORT,(meUByte *)"[No such variable]");
    }
    else
        return mlwrite(MWABORT,(meUByte *)"[User variable required]");
    
    /*---   Check for existing legal user variable */
    prev = NULL ;
    vptr = varList->head ;
    while(vptr != NULL)
    {
        if(!(vnum=meStrcmp(vptr->name,vv)))
        {
            /* link out the variable to unset and free it */
            if(prev == NULL)
                varList->head = vptr->next ;
            else
                prev->next = vptr->next ;
            varList->count-- ;
            meNullFree(vptr->value) ;
            meFree(vptr) ;
            return meTRUE ;           /* True exit */
        }
        if(vnum > 0)
            break ;
        prev = vptr ;
        vptr = vptr->next ;
    }
    
    /* If its not legal....bitch */
    return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[No such variable]") ;
}
#endif

int
setVariable(int f, int n)       /* set a variable */
{
    register int   status ;         /* status return */
    meUByte var[meSBUF_SIZE_MAX] ;            /* name of variable to fetch */
    meUByte value[meBUF_SIZE_MAX] ;           /* value to set variable to */
    meRegister *regs ;
    
    /* horrid global variable, see notes at definition */
    gmaLocalRegPtr = meRegCurr ;
    /* set this flag to indicate that the variable name is required, NOT
     * its value */
    alarmState |= meALARM_VARIABLE ;
    status = meGetString((meUByte *)"Set variable", MLVARBL, 0, var,meSBUF_SIZE_MAX) ;
    alarmState &= ~meALARM_VARIABLE ;
    regs = gmaLocalRegPtr ;
    if(status <= 0)
        return status ;
    /* get the value for that variable */
    if(f == meTRUE)
        meStrcpy(value, meItoa(n));
    else if((status = meGetString((meUByte *)"Value", MLFFZERO, 0, value,meBUF_SIZE_MAX)) <= 0)
        return status ;
    
    return setVar(var,value,regs) ;
}

int
descVariable(int f, int n)      /* describe a variable */
{
    meUByte  var[meSBUF_SIZE_MAX]; /* name of variable to fetch */
    meUByte *ss ;
    int ii ;
    
    /* first get the variable to describe */
    if((ii = meGetString((meUByte *)"Show variable",MLVARBL,0,var,meSBUF_SIZE_MAX)) <= 0)
        return ii ;
    if((((ii=getMacroTypeS(var)) != TKREG) && (ii != TKVAR) && (ii != TKENV) && (ii != TKLVR) && (ii != TKCVR)) ||
       ((ss = getval(var)) == NULL))
        return mlwrite(MWABORT,(meUByte *)"Unknown variable type") ;
    mlwrite(0,(meUByte *)"Current setting is \"%s\"", ss) ;
    return meTRUE ;
} 

#if MEOPT_EXTENDED
/*
 * showVariable
 * Format a variable for display. This is a helper function for 
 * "listVariables()" used to format and display each of the values.
 * 
 * Arguments:-
 *    bp     - The buffer to be rendered into.
 *    prefix - The prefix character(s) of the variable
 *    name   - The name of the variable.
 *    value  - The value assigned to variable.
 * 
 * Produce a line in the form:-
 * 
 *    name......... "value"
 */
static void
showVariable(meBuffer *bp, meUByte prefix, meUByte *name, meUByte *value)
{
    meUByte buf[meBUF_SIZE_MAX+meSBUF_SIZE_MAX+16] ;
    int len;
    
    len = sprintf ((char *) buf, "    %c%s ", prefix, name);
    while (len < 35)
        buf[len++] = '.';
    buf[len++] = ' ';
    buf[len++] = '"';
    len = expandexp(-1,value,meBUF_SIZE_MAX+meSBUF_SIZE_MAX+16-2,len,buf,-1,NULL,meEXPAND_BACKSLASH|meEXPAND_FFZERO) ;
    buf[len++] = '"';
    buf[len] = '\0';
    addLineToEob(bp,buf);
}

/*
 * listVariables
 * List ALL of the variables held in the system.
 */

int
listVariables (int f, int n)
{
    meVariable *tv ;
    meWindow *wp ;
    meBuffer *bp ;
    meUByte   buf[meBUF_SIZE_MAX] ;
    int     ii ;
    
    if((wp = meWindowPopup(BvariablesN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR),NULL)) == NULL)
        return meFALSE ;
    bp = wp->buffer ;
    
    addLineToEob(bp,(meUByte *)"Register variables:\n");
    
    buf[0] = 'g';
    buf[2] = '\0';
    for(ii = 0; ii<meREGISTER_MAX ; ii++)
    {
        buf[1] = (int)('0') + ii;
        showVariable(bp,'#',buf,meRegHead->reg[ii]);
    }
    addLineToEob(bp,(meUByte *)"\nBuffer variables:\n") ;
    tv = frameCur->bufferCur->varList.head ;
    while(tv != NULL)
    {
        showVariable(bp, ':', tv->name,tv->value);
        tv = tv->next ;
    }
    
    addLineToEob(bp,(meUByte *)"\nSystem variables:\n");
    for (ii = 0; ii < NEVARS; ii++)
        showVariable(bp,'$',envars[ii],gtenv(envars[ii]));
    
    addLineToEob(bp,(meUByte *)"\nGlobal variables:\n");
    for (tv=usrVarList.head ; tv != NULL ; tv = tv->next)
        showVariable(bp,'%',tv->name,tv->value);
    
    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotOffset = 0 ;
    bp->dotLineNo = 0 ;
    meModeClear(bp->mode,MDEDIT) ;    /* don't flag this as a change */
    meModeSet(bp->mode,MDVIEW) ;      /* put this buffer view mode */
    resetBufferWindows(bp) ;            /* Update the window */
    mlerase(MWCLEXEC);                  /* clear the mode line */
    return meTRUE ;
}
#endif
