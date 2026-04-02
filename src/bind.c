/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * bind.c - Binding functions.
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
 * Created:     11-feb-86
 * Synopsis:    Binding functions.
 * Authors:     Daniel Lawrence, Jon Green & Steven Phillips
 * Description:
 *     This file is for functions having to do with key bindings,
 *     descriptions, help commands and startup file.
 */

#define	__BINDC				/* Define file name */

#include "emain.h"
#ifdef _UNIX
#include <sys/stat.h>
#endif
#include "efunc.h"
#include "esearch.h"
#include "eskeys.h"

meUShort
meGetKeyFromString(meUByte **tp)
{
    meUByte *ss=*tp, cc, dd ;
    meUShort ii=0 ;
    
    for(;;)
    {
        if(((cc=*ss++) == '\0') ||
           ((dd=*ss++) != '-'))
            break ;
        if     (cc == 'C')
            ii |= ME_CONTROL ;
        else if(cc == 'S')
            ii |= ME_SHIFT ;
        else if(cc == 'A')
            ii |= ME_ALT ;
    }
    if(cc != '\0')
    {
        if(!isSpace(dd))
        {
            meUByte *s1=ss ;
            int skey ;
            
            while(((dd=*s1) != '\0') && (dd != ' '))
                s1++ ;
            *s1 = '\0' ;
            skey = biChopFindString(ss-2,13,specKeyNames,SKEY_MAX) ;
            *s1 = dd ;
            
            if(skey >= 0)
            {
                if(skey == SKEY_space)
                    ii |= 32 ;
                else
                    ii |= (skey|ME_SPECIAL) ;
                *tp = s1 ;
                return ii ;
            }
            /* duff key string - fail */
            return 0 ;
        }
        if(ii)
        {
            cc = toUpper(cc) ;
            if((ii == ME_CONTROL) && (cc >= 'A') && (cc <= '_'))
                ii = cc - '@' ;
            else
            {
                cc = toLower(cc) ;
                ii |= cc ;
            }
        }
        else
            ii |= cc ;
    }
    *tp = ss-1 ;
    return ii ;
}

/* meGetKey
 * 
 * get a key sequence from either the current macro execute line, a keyboard
 * macro or the keyboard
 * 
 *     flag - meGETKEY flags
 */
meUShort
meGetKey(int flag)
{
    register meUShort cc ;	/* character fetched */
    
    /* check to see if we are executing a command line */
    if(clexec == meTRUE)
    {
        meUByte *tp;		/* pointer into the token */
        meUByte tok[meBUF_SIZE_MAX];	/* command incoming */
        
        tp = execstr ;
        execstr = token(tp,tok) ;
        if((tok[0] != '@') || (tok[1] != 'm') || (tok[2] != 'n'))
        {
            tp = getval(tok) ;
            if((tp == abortm) || ((cc = meGetKeyFromString(&tp)) == 0))
                return 0 ;
            
            if(!(flag & meGETKEY_SINGLE))
            {
                int ii=ME_PREFIX_NUM+1 ;
                
                while(--ii > 0)
                    if(cc == prefixc[ii])
                    {
                        cc = ii << ME_PREFIX_BIT ;
                        break ;
                    }
                if(cc & ME_PREFIX_MASK)
                {
                    meUShort ee ;	/* character fetched */
                    meUByte  dd ;
                    while(((dd=*tp) != '\0') && isSpace(dd))
                        tp++ ;
                    
                    if((ee = meGetKeyFromString(&tp)) == 0)
                        return 0 ;
                    
                    if((ee >= 'A') && (ee <= 'Z'))
                        /* with a prefix make a letter lower case */
                        ee ^= 0x20 ;
                    cc |= ee ;
                }
            }
            /* check there are no superfluous chars in the string, fail if found as
             * this is probably a bad bind string */
            if(*tp != '\0')
                return 0 ;
            return cc ;
        }
        /* if @mna (get all input from user) then rewind the execstr */
        if(tok[3] == 'a')
            execstr = tp ;
        /* Force an update of the screen to to ensure that the user
         * can see the information in the correct location */
        update (meTRUE);
    }
    /* or the normal way */
    cc = meGetKeyFromUser(meFALSE,0,flag) ;
    
    return cc ;
}

int
meGetStringFromChar(meUShort cc, meUByte *d)
{
    register int  doff=0, spec ;
    register meUByte *ss ;
    
    if(cc & ME_ALT)
    {
        d[doff++] = 'A' ;
        d[doff++] = '-' ;
    }        
    if(cc & ME_CONTROL)
    {
        d[doff++] = 'C' ;
        d[doff++] = '-' ;
    }        
    if(cc & ME_SHIFT)
    {
        d[doff++] = 'S' ;
        d[doff++] = '-' ;
    }
    spec = (cc & ME_SPECIAL) ;
    cc &= 0x0ff ;
    
    if(spec)
    {
        register meUByte ch ;
        
        ss = specKeyNames[cc] ;
name_copy:
        while((ch = *ss++) != '\0')
            d[doff++] = ch ;
        return doff ;
    }
    
    if((cc > 0) && (cc <= 0x20))	/* control character */
    {
        if(cc == 0x20)
        {
            ss = specKeyNames[SKEY_space] ;
            goto name_copy ;
        }
        cc |= 0x40 ;
        cc = toLower(cc) ;
        d[doff++] = 'C' ;
        d[doff++] = '-' ;
        d[doff++] = (meUByte) cc ;
    }
    else if((cc == 0) || (cc >= 0x80))
    {
        d[doff++] = '\\';
        d[doff++] = 'x';
        d[doff++] = hexdigits[cc >> 4] ;
        d[doff++] = hexdigits[cc & 0x0f] ;
    }
    else if((cc == '\\') || (cc == '"'))
    {
        d[doff++] = '\\';
        d[doff++] = (meUByte) cc ;
    }
    else			/* any other character */
        d[doff++] = (meUByte) cc ;
    
    return doff ;
}

/* change a key command to a string we can print out */
/* c   - 	sequence to translate */
/* seq - 	destination string for sequence */
void
meGetStringFromKey(meUShort cc, register meUByte * seq)
{
    /* apply prefix sequence if needed */
    if(cc & ME_PREFIX_MASK)
    {
        seq += meGetStringFromChar(prefixc[(cc&ME_PREFIX_MASK)>>ME_PREFIX_BIT],seq);
        *seq++ = ' ' ;
    }
    seq += meGetStringFromChar(cc,seq);
    *seq = 0;	/* terminate the string */
}

int
descKey(int f, int n)	/* describe the command for a certain key */
{
    static   meUByte  notBound[] = "Not Bound" ;
    static   meUByte  dskyPrompt[]="Show binding: " ;
    register meUShort cc;	/* command character to describe */
    register meUByte *ptr; 	/* string pointer to scan output strings */
    register int    found;	/* matched command flag */
    meUInt          arg ;	/* argument */
    meUByte           outseq[40];	/* output buffer for command sequence */
    meUByte           argStr[20];	/* argument string */
    
    /* prompt the user to type us a key to describe */
    mlwrite(MWCURSOR,dskyPrompt);
    
    /* get the command sequence to describe */
    if((cc = meGetKey(meGETKEY_SILENT)) == 0)	/* get a silent command sequence */
        return ctrlg(0,1) ;
    
    /* change it to something we can print as well */
    meGetStringFromKey(cc, outseq);
    
    /* find the right function */
    if ((found = decode_key(cc,&arg)) < 0)
    {
        ptr = notBound ;
        argStr[0] = 0 ;
    }
    else
    {
        ptr = getCommandName(found) ;
        if(arg)
            sprintf((char *) argStr,"%d ",(int) (arg + 0x80000000)) ;
        else
            argStr[0] = 0 ;
    }

    /* output the resultant string */
    mlwrite(MWCURSOR,(meUByte *)"%s\"%s\" %s%s",dskyPrompt,outseq,argStr,ptr);
    
    return meTRUE ;
}

#if MEOPT_EXTENDED
void
charMaskTblInit(void)
{
    int ii ;
    for(ii=0 ; ii<128 ; ii++)
    {
        charLatinUserTbl[ii] = ii ;
        charUserLatinTbl[ii] = ii ;
        charMaskTbl2[ii] &= ~(CHRMSK_LOWER|CHRMSK_UPPER|CHRMSK_HEXDIGIT|CHRMSK_SPLLEXT) ;
        charCaseTbl[ii] = ii ;
    }
    for( ; ii<256 ; ii++)
    {
        charLatinUserTbl[ii] = ii ;
        charUserLatinTbl[ii] = ii ;
        charMaskTbl2[ii] = 0x00 ;
        charCaseTbl[ii] = ii ;
    }
#ifdef MEOPT_UTF8
    for(ii=128 ; ii<256 ; ii++)
        charMaskTbl1[ii] = 0x7A ;
#elif defined(_UNIX)
    /* 0xA0 is the No-Break SPace char - nothing is drawn! */
    for(ii=128 ; ii<161 ; ii++)
        charMaskTbl1[ii] = 0x0A ;
#else
    for(ii=128 ; ii<160 ; ii++)
        charMaskTbl1[ii] = 0x3A ;
    for( ; ii<256 ; ii++)
        charMaskTbl1[ii] = 0x7A ;
#endif
    for(ii='0' ; ii<='9' ; ii++)
        charMaskTbl2[ii] |= CHRMSK_HEXDIGIT ;
    for(ii='A' ; ii<='F' ; ii++)
    {
        charMaskTbl2[ii] |= (CHRMSK_UPPER|CHRMSK_HEXDIGIT) ;
        charCaseTbl[ii] = (ii^0x20) ;
    }
    for( ; ii<='Z' ; ii++)
    {
        charMaskTbl2[ii] |= CHRMSK_UPPER ;
        charCaseTbl[ii] = (ii^0x20) ;
    }
    for(ii='a' ; ii<='f' ; ii++)
    {
        charMaskTbl2[ii] |= (CHRMSK_LOWER|CHRMSK_HEXDIGIT) ; ;
        charCaseTbl[ii] = (ii^0x20) ;
    }
    for( ; ii<='z' ; ii++)
    {
        charMaskTbl2[ii] |= CHRMSK_LOWER ;
        charCaseTbl[ii] = (ii^0x20) ;
    }
    charMaskTbl2[0x27] |= CHRMSK_SPLLEXT ;
    charMaskTbl2[0x2d] |= CHRMSK_SPLLEXT ;
    charMaskTbl2[0x2e] |= CHRMSK_SPLLEXT ;
    charMaskTbl2[0x5f] |= CHRMSK_USER1 ;
}


int
setCharMask(int f, int n)
{
    static meUByte tnkyPrompt[]="set-char-mask chars" ;
    meUByte buf1[meBUF_SIZE_MAX], *ss, *dd ;
    int	  c1, c2, flags, ii, jj ;
    meUByte mask1, mask2 ;
    
    meStrcpy(tnkyPrompt+14,"flags") ;
    if(meGetString(tnkyPrompt,0,0,buf1,20) <= 0)
        return meFALSE ;
    /* setup the masks */
    flags = 0 ;
    ss = buf1 ;
    while((c1 = *ss++) != '\0')
        if((dd = (meUByte *) strchr((char *) charMaskFlags,c1)) != NULL)
            flags |= 1 << ((int)(dd - charMaskFlags)) ;
    mask2 = (flags & 0x00ff) ;
    mask1 = (flags & 0x0f00) >> 4 ;
    
    if(n == 0)
    {
        ss = (meUByte *) resultStr ;
        if(flags & 0x20000)
        {
            if(charKeyboardMap != NULL)
            {
                for(c1=1 ; c1<256 ; c1++)
                {
                    if((c2 = charKeyboardMap[c1]) != '\0')
                    {
                        if(c1 == meCHAR_LEADER)
                            *ss++ = meCHAR_LEADER ;
                        *ss++ = c1 ;
                        if(c2 == meCHAR_LEADER)
                            *ss++ = meCHAR_LEADER ;
                        *ss++ = c2 ;
                    }
                }
            }
                
        }
        else if(flags & 0x1c000)
        {
            if(flags & 0x4000)
                flags |= 0x18000 ;
            for(c1=0 ; c1<256 ; c1++)
            {
                c2 = charLatinUserTbl[c1] ;
                if(c1 != c2)
                {
                    if(flags & 0x8000)
                    {
                        if(c1 == 0)
                        {
                            *ss++ = meCHAR_LEADER ;
                            c1 = meCHAR_TRAIL_NULL ;
                        }
                        else if(c1 == meCHAR_LEADER)
                            *ss++ = meCHAR_LEADER ;
                        *ss++ = c1 ;
                    }
                    if(flags & 0x10000)
                    {
                        if(c2 == 0)
                        {
                            *ss++ = meCHAR_LEADER ;
                            c2 = meCHAR_TRAIL_NULL ;
                        }
                        else if(c2 == meCHAR_LEADER)
                            *ss++ = meCHAR_LEADER ;
                        *ss++ = c2 ;
                    }
                }
            }
        }
        else
        {
            if(flags & 0x1000)
                mask2 |= CHRMSK_LOWER ;
            if(flags & 0x2000)
                mask2 |= (CHRMSK_LOWER|CHRMSK_HEXDIGIT) ;
            for(c1=0 ; c1<256 ; c1++)
                if((charMaskTbl1[c1] & mask1) ||
                   (charMaskTbl2[c1] & mask2))
                {
                    if((flags & 0x3000) &&
                       ((c2 = toggleCase(c1)) != c1))
                    {
                        if(c2 == 0)
                        {
                            *ss++ = meCHAR_LEADER ;
                            c2 = meCHAR_TRAIL_NULL ;
                        }
                        else if(c2 == meCHAR_LEADER)
                            *ss++ = meCHAR_LEADER ;
                        *ss++ = c2 ;
                    }
                    if(c1 == 0)
                    {
                        *ss++ = meCHAR_LEADER ;
                        c1 = meCHAR_TRAIL_NULL ;
                    }
                    else if(c1 == meCHAR_LEADER)
                        *ss++ = meCHAR_LEADER ;
                    *ss++ = c1 ;
                }
        }
        *ss = '\0' ;
    }
    else
    {
#if MEOPT_MAGIC
        /* reset the last magic search string to force a recompile as char groups may change */
        mereRegexInvalidate() ;
#endif
        if(flags & 0x1A003)
            return mlwrite(MWABORT,(meUByte *)"[Cannot set flags u, l, A, L or U]");
        meStrcpy(tnkyPrompt+14,"chars") ;
        if(meGetString(tnkyPrompt,MLFFZERO,0,buf1,meBUF_SIZE_MAX) <= 0)
            return meFALSE ;
        if(flags & 0x20000)
        {
            ss = buf1 ;
            if(*ss != '\0')
            {
                if((charKeyboardMap == NULL) &&
                   ((charKeyboardMap = (meUByte *) meMalloc(256)) == NULL))
                    return meABORT ;
                /* reset the map */
                memset(charKeyboardMap,0,256) ;
                while((c1=*ss++) != '\0')
                {
                    if((c1 == meCHAR_LEADER) && ((c1=*ss++) == meCHAR_TRAIL_NULL))
                        c1 = 0 ;
                    c1 = toUserFont(c1) ;
                    if((c2=*ss++) == '\0')
                        break ;
                    if((c2 == meCHAR_LEADER) && ((c2=*ss++) == meCHAR_TRAIL_NULL))
                        c2 = 0 ;
                    charKeyboardMap[c1] = c2 ;
                }
            }
            /* reset completely */
            else if(charKeyboardMap != NULL)
            {
                meFree(charKeyboardMap) ;
                charKeyboardMap = NULL ;
            }
            return meTRUE ;
        }
        if(flags & 0x4000)
        {
            /* reset the tables */
            charMaskTblInit() ;
            ss = buf1 ;
            while((c1=*ss++) != '\0')
            {
                if((c1 == meCHAR_LEADER) && ((c1=*ss++) == meCHAR_TRAIL_NULL))
                    c1 = 0 ;
                if((c2=*ss++) == '\0')
                    break ;
                if((c2 == meCHAR_LEADER) && ((c2=*ss++) == meCHAR_TRAIL_NULL))
                    c2 = 0 ;
                charLatinUserTbl[c1] = c2 ;
                charUserLatinTbl[c2] = c1 ;
            }
            return meTRUE ;
        }
        /* convert the string to user font and take out any LEADER padding, 0's are now 0's so need int count */
        ss = buf1 ;
        dd = buf1 ;
        ii = 0 ;
        while((c1=*ss++) != '\0')
        {
            if((c1 == meCHAR_LEADER) && ((c1=*ss++) == meCHAR_TRAIL_NULL))
                c1 = 0 ;
            *dd++ = toUserFont(c1) ;
            ii++ ;
        }
        if(n < 0)
        {
            if(flags & 0x1000)
                mask2 |= CHRMSK_ALPHA ;
            if(mask1 != 0)
            {
                mask1 ^= 0xff ;
                for(jj=0 ; jj<ii ; jj++)
                    charMaskTbl1[buf1[jj]] &= mask1 ;
            }
            if(mask2 != 0)
            {
                mask2 ^= 0xff ;
                for(jj=0 ; jj<ii ; jj++)
                    charMaskTbl2[buf1[jj]] &= mask2 ;
            }
        }
        else
        {
            if(flags & 0x1000)
            {
                for(jj=0 ; jj<ii ; )
                {
                    c1 = buf1[jj++] ;
                    c2 = buf1[jj++] ;
                    charMaskTbl2[c1] |= CHRMSK_UPPER ;
                    charMaskTbl2[c2] |= CHRMSK_LOWER ;
                    charCaseTbl[c1] = c2 ;
                    charCaseTbl[c2] = c1 ;
                }
            }
            if(mask1 != 0)
            {
                for(jj=0 ; jj<ii ; jj++)
                    charMaskTbl1[buf1[jj]] |= mask1 ;
            }
            if(mask2 != 0)
            {
                for(jj=0 ; jj<ii ; jj++)
                    charMaskTbl2[buf1[jj]] |= mask2 ;
            }
        }
    }
    return meTRUE ;
}
#endif

#if MEOPT_CMDHASH
meUInt
cmdHashFunc (register meUByte *cmdName)
{
    meUInt ii = 0;
    meUInt jj;
    meUByte cc;
    
    while ((cc = *cmdName++) != '\0')
    {
        ii = (ii << 4) + cc;
        if ((jj = ii & 0xf0000000) != 0)
        {
            ii ^= (jj >> 24);
            ii ^= jj;
        }
    }
    return ii % cmdHashSize ;
}
#endif
     

/* match fname to a function in the names table
   and return index into table or -1 if none	
   fname;	name to attempt to match */
int
decode_fncname(register meUByte *fname, int silent)
{
    meCommand *cmd ;
#if MEOPT_CMDHASH
    meUInt key ;
    
    key = cmdHashFunc(fname) ;
    for (cmd=cmdHash[key] ; cmd != NULL; cmd = cmd->hnext)
        if (meStrcmp (fname, cmd->name) == 0)
            return cmd->id ;
#else
    int ii ;
    cmd = cmdHead;
    do
    {
        if((ii=meStrcmp(fname, cmd->name)) == 0)
            return cmd->id ;
    } while((ii > 0) &&  ((cmd=cmd->anext) != NULL)) ;
#endif    
       
    if(!silent)
        mlwrite(MWABORT,(meUByte *)"[No such command \"%s\"]", fname);
    
    return -1 ;				/* Not found - error. */
}

/* bindtokey:	add a new key to the key binding table
*/

int
bindkey(meUByte *prom, int f, int n, meUShort *lclNoBinds, meBind **lclBinds)
{
    register int	ss;
    register meUShort   cc;	/* command key to bind */
    register meIFuncII	kfunc;	/* ptr to the requexted function to bind to */
    register meBind    *ktp;	/* pointer into the command table */
    meUByte  outseq[40];        /* output buffer for keystroke sequence */
    int	     namidx;		/* Name index */
    meUByte  buf[meBUF_SIZE_MAX] ;
    meUInt   arg ;
    meUShort ssc ;
    
    /*---	Get the function name to bind it to */
    
    if(meGetString(prom, MLCOMMAND, 0, buf, meBUF_SIZE_MAX) <= 0)
        return meFALSE ;
    if((namidx = decode_fncname(buf,0)) < 0)
        return meFALSE ;
    kfunc = getCommandFunc(namidx) ;

    /*---	Prompt the user to type in a key to bind */
    /*---	Get the command sequence to bind */
    
    if((ss=(kfunc==(meIFuncII) prefixFunc)))
    {
        if((n < 1) || (n > ME_PREFIX_NUM))
            return mlwrite(MWABORT,(meUByte *)"[Prefix number is out of range]") ;
        ssc = prefixc[n] ;
    }
    else if((ss=(kfunc==(meIFuncII) ctrlg)))
        ssc = breakc ;
    else if((ss=(kfunc==(meIFuncII) uniArgument)))
        ssc = reptc ;

#if MEOPT_LOCALBIND
    if((lclNoBinds != NULL) && ss)
        return mlwrite(MWABORT,(meUByte *)"Can not locally bind [%s]",buf) ;
#endif              
    mlwrite(MWCURSOR|MWCLEXEC,(meUByte *)"%s [%s] to: ",prom,buf) ;
    if((cc = meGetKey((ss) ? meGETKEY_SILENT|meGETKEY_SINGLE:meGETKEY_SILENT)) == 0)
        return ctrlg(0,1) ;
    
    /*---   Change it to something we can print as well */
    
    if (!clexec)
    {
        meGetStringFromKey(cc,outseq);
        mlwrite(0,(meUByte *)"%s [%s] to: %s",prom,buf,outseq);
    }
    
    /*---   Calculate the argument */
    if(f)
        arg = (meUInt) (n+0x80000000) ;
    else
        arg = 0 ;
    
    /*---	If the function is a prefix key */
    
    if(ss)
    {
	/*---	Search for an existing binding for the prefix key */
	if(ssc != ME_INVALID_KEY)
            delete_key(ssc);
        
	/*---	Reset the appropriate global prefix variable */
        if(kfunc == (meIFuncII) prefixFunc)
            prefixc[n] = cc ;
        else if(kfunc ==(meIFuncII) ctrlg)
            breakc = cc ;
        else
            reptc = cc ;
    }
    
    /*---	Search the table to see if it exists */
#if MEOPT_LOCALBIND
    if(lclNoBinds != NULL)
    {
        register meBind *kk=NULL ;
        
        ktp = *lclBinds ;
        for (ss=0 ; ss < *lclNoBinds ; ss++)
        {
            if(ktp[ss].code == cc)
            {
                ktp[ss].index = (meUShort) namidx;
                ktp[ss].arg   = arg ;
                return meTRUE ;
            }
            if(ktp[ss].code == 0)
                kk = ktp+ss ;
        }
        if(kk != NULL)
        {
            kk->code  = cc ;
            kk->index = (meUShort) namidx;
            kk->arg   = arg ;
        }
        else
        {
            if((ktp = *lclBinds = 
                meRealloc(ktp,(++(*lclNoBinds))*sizeof(meBind))) == NULL)
                return meFALSE ;
            ktp[ss].code  = cc ;
            ktp[ss].index = (meUShort) namidx ;
            ktp[ss].arg   = arg ;
        }
    }
    else
#endif
    {
        for (ktp = &keytab[0]; ktp->code != ME_INVALID_KEY; ktp++)
            if (ktp->code == cc)
            {
                ktp->index = (meUShort) namidx;
                ktp->arg   = arg ;
                return meTRUE ;
            }
        
        /*---	Otherwise we  need to  add it  to the  end, if  we run  out of
           binding room, bitch */
        
        if(insert_key(cc, (meUShort) namidx, arg) <= 0)
            return mlwrite(MWABORT,(meUByte *)"Binding table FULL!");
    }
    return(meTRUE);
}


int
globalBindKey(int f, int n)
{
    return bindkey((meUByte *)"global bind", f, n, NULL, NULL) ;
}

#if MEOPT_LOCALBIND
int
bufferBindKey(int f, int n)
{
    return bindkey((meUByte *)"local bind", f, n, &(frameCur->bufferCur->bindCount), &(frameCur->bufferCur->bindList)) ;
}
int
mlBind(int f, int n)
{
    return bindkey((meUByte *)"ml bind", f, n, &mlNoBinds, &mlBinds) ;
}
#endif

/* unbindkey:	delete a key from the key binding table */

int
unbindkey(meUByte *prom, int n, meUShort *lclNoBinds, meBind **lclBinds)
{
    register meUShort cc;  /* command key to unbind */
    meUByte outseq[40];	   /* output buffer for keystroke sequence */
    int  rr ;
    
    if(n < 0)
    {
        sprintf((char *)outseq,"Remove all %s binds",prom);
        if(mlyesno(outseq) <= 0)
            return ctrlg(meFALSE,1) ;
        
#if MEOPT_LOCALBIND
        if(lclNoBinds != NULL)
        {
            if(meNullFree(*lclBinds))
            {
                *lclBinds = NULL ;
                *lclNoBinds = 0 ;
            }
        }
        else
#endif
        {
            keyTableSize = 0 ;
            keytab[0].code = ME_INVALID_KEY ;
            
            /* remove all the prefixes */
            rr = ME_PREFIX_NUM+1;
            while(--rr > 0)
                prefixc[rr] = ME_INVALID_KEY ;
            breakc = ME_INVALID_KEY ;
            reptc = ME_INVALID_KEY ;
#if MEOPT_LOCALBIND
            {
                meBuffer *bp ;
                
                /* loop through removing all buffer bindings */
                bp = bheadp ;
                while(bp != NULL)
                {
                    if(meNullFree(bp->bindList))
                    {
                        bp->bindList = NULL ;
                        bp->bindCount = 0 ;
                    }
                    bp = bp->next ;
                }
                /* remove all ml bindings */
                if(meNullFree(mlBinds))
                {
                    mlBinds = NULL ;
                    mlNoBinds = 0 ;
                }
            }
#endif
        }
        return meTRUE ;
    }
        
    /*---	Prompt the user to type in a key to unbind */
    
    /* if interactive then print prompt */
    mlwrite(MWCURSOR|MWCLEXEC,(meUByte *)"%s unbind: ",prom);
    
    /*---	Get the command sequence to unbind */
    
    /* get a command sequence */
    if((cc = meGetKey((n==0) ? meGETKEY_SILENT|meGETKEY_SINGLE:meGETKEY_SILENT)) == 0)
        return ctrlg(0,1) ;
    
    /* If interactive then there is no really need to print the string because
     * you will probably not see it !! The TTputc() is dangerous and squirts
     * characters over the screen if the previous mlwrite() determined that
     * we were in a macro and has not moved the cursor to the message line.
     * Hence, safer bet is to re-do the mlwrite(); my preference would be
     * to remove the keyboard echo !! - Jon Green 13/03/97 */
    meGetStringFromKey(cc, outseq);		/* change to something printable */
    mlwrite(MWCURSOR|MWCLEXEC,(meUByte *)"%s unbind: %s", prom, outseq);
#if MEOPT_LOCALBIND
    if(lclNoBinds != NULL)
    {
        register meBind *ktp ;
        register int	 ss;
        
        rr = meFALSE ;
        ktp = *lclBinds ;
        ss = *lclNoBinds ;
        while(--ss >= 0)
        {
            if(ktp[ss].code == cc)
            {
                ktp[ss].code = ME_INVALID_KEY ;
                rr = meTRUE ;
            }
        }
    }
    else
#endif
    {
        if((rr = delete_key(cc)) > 0)
        {
            if(n == 0)
            {
                register meUShort mask=ME_PREFIX_NUM+1;
                
                while(--mask > 0)
                    if(cc == prefixc[mask])
                        break ;
                if(mask)
                {
                    int ii= (int) keyTableSize ;
                    
                    prefixc[mask] = ME_INVALID_KEY ;
                    mask <<= ME_PREFIX_BIT ;
                    while(--ii>=0)
                        if((keytab[ii].code & ME_PREFIX_MASK) == mask)
                            delete_key(keytab[ii].code) ;
#if MEOPT_LOCALBIND
                    {
                        register int ss ;
                        meBuffer *bp ;
                        
                        /* loop through all buffer bindings using prefix */
                        bp = bheadp ;
                        while(bp != NULL)
                        {
                            ss = bp->bindCount ;
                            while(--ss >= 0)
                                if((bp->bindList[ss].code & ME_PREFIX_MASK) == mask)
                                    bp->bindList[ss].code = ME_INVALID_KEY ;
                            bp = bp->next ;
                        }
                        /* loop through all ml bindings using prefix */
                        ss = mlNoBinds ;
                        while(--ss >= 0)
                            if((mlBinds[ss].code & ME_PREFIX_MASK) == mask)
                                mlBinds[ss].code = ME_INVALID_KEY ;
                    }
#endif
                }
            }
            if (breakc == cc)
                breakc = ME_INVALID_KEY ;
            if (reptc == cc)
                reptc = ME_INVALID_KEY ;
        }
    }
    if(rr == meFALSE)	/* if it isn't bound, bitch */
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s Key \"%s\" not bound]", prom,outseq);
    return meTRUE ;
}

int
globalUnbindKey(int f, int n)
{
    return unbindkey((meUByte *)"Global",n, NULL, NULL) ;
}

#if MEOPT_LOCALBIND
int
bufferUnbindKey(int f, int n)
{
    return unbindkey((meUByte *)"Local",n, &(frameCur->bufferCur->bindCount), &(frameCur->bufferCur->bindList)) ;
}
int
mlUnbind(int f, int n)
{
    return unbindkey((meUByte *)"ML",n, &mlNoBinds, &mlBinds) ;
}
#endif

#if MEOPT_EXTENDED
/* build a binding list (limited or full) */
/* type  	true = full list,   false = partial list */
/* mstring	match string if a partial list */
static int
buildlist(int n, meUByte *mstring)
{
    meCommand    *cmd ;
    meBind   *ktp;	        /* pointer into the command table */
    meWindow   *wp;
    meBuffer   *bp;	        /* buffer to put binding list into */
    int       cpos;   		/* current position to use in outseq */
    meUByte     _outseq[meBUF_SIZE_MAX]; 	/* output buffer for keystroke sequence */
    meUByte    *outseq=_outseq+4;	/* output buffer for keystroke sequence */
    int       curidx;           /* Name integer index. */
    char      op_char;		/* Output character */
    
    /* and get a buffer for it */
    if((wp = meWindowPopup(BcommandsN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR),NULL)) == NULL)
        return mlwrite(MWABORT,(meUByte *)"[Failed to create list]") ;
    bp = wp->buffer ;
    
    _outseq[0] = ' ' ;
    _outseq[1] = ' ' ;
    _outseq[2] = ' ' ;
    _outseq[3] = ' ' ;
    wp->markLine = NULL;
    wp->markOffset = 0;
    
    /* build the contents of this window, inserting it line by line */
    for(cmd=cmdHead ; cmd != NULL ; cmd=cmd->anext)
    {
        /* Skip command if its a hidden macro (and bit 1 of arg n is set) or
         * we are executing an apropos command and current string doesn't
         * include the search string */
        if(((n & 0x01) && (cmd->id >= CK_MAX) && (((meMacro *) cmd)->hlp->flag & meMACRO_HIDE)) ||
           ((mstring != NULL) && !regexStrCmp(cmd->name,mstring,meRSTRCMP_WHOLE)))
            continue;		/* Do next loop */
        /* Add in the command name */
        meStrcpy(outseq, cmd->name);
        curidx = cmd->id ;
        cpos = meStrlen(outseq);
        
        /* Search down any keys bound to this */
        
        for (ktp = &keytab[0] ; ktp->code != ME_INVALID_KEY ; ktp++)
        {
            if (ktp->index == curidx)
            {
                /* Padd out some spaces. If we are on a line with a string
                 * then pad with '.' else use space's */
                op_char = ((cpos) ? '.' : ' ');
                outseq[cpos++] = ' ';
                while (cpos < 31)
                    outseq[cpos++] = op_char;
                outseq[cpos++] = ' ';
                outseq[cpos++] = '"';
                /* Add in the command sequence */
                meGetStringFromKey(ktp->code, &outseq[cpos]);
                meStrcat(outseq+cpos,"\"");
                
                /* and add it as a line into the buffer */
                addLineToEob(bp,_outseq) ;
                cpos = 0;	/* and clear the line */
            }
        }
        
        /* if no key was bound, we need to dump it anyway */
        if (cpos > 0)
            addLineToEob(bp,_outseq) ;
    }
    meModeClear(bp->mode,MDEDIT) ;    /* don't flag this as a change */
    meModeSet(bp->mode,MDVIEW) ;      /* put this buffer view mode */
    wp->dotLine = meLineGetNext(bp->baseLine);    /* back to the beginning */
    wp->dotOffset = 0;
    mlerase(MWCLEXEC);          	/* clear the mode line */
    return(meTRUE);
}


/* list-functions
 * bring up a fake buffer and list the key bindings into it with view mode
 */
int
listCommands(int f, int n)
{
    return buildlist(n,NULL) ;
}

int
commandApropos(int f, int n)	/* Apropos (List functions that match a substring) */
{
    meUByte mstring[meSBUF_SIZE_MAX];	/* string to match cmd names to */
    int	  status;		/* status return */

    if ((status = meGetString((meUByte *)"Apropos string", MLCOMMAND, 0, 
                          mstring+2, meSBUF_SIZE_MAX-4)) <= 0)
        return status ;
    mstring[0] = '.' ;
    mstring[1] = '*' ;
    meStrcat(mstring,".*") ;
    return buildlist(n, mstring);
}
#endif

/*
 * listbindings
 * List ALL of the bindings held in the system.
 */
static void
showBinding(meBuffer *bp, meBind *ktp)
{
    meUByte *ss, buf[meBUF_SIZE_MAX], outseq[40];
    int len;
    
    /* If the keyboard code is zero then the command has been unbound.
     * Situation occurs when the buffer or ml binding list is traversed.
     * We test at point of print rather than at the caller so that we do
     * not repeat the test. Speed is not a crital issue here. */
    if (ktp->code == ME_INVALID_KEY)
        return;
    
    /* The key command is non-zero therefore it is legal - print it */
    meGetStringFromKey(ktp->code, outseq);
    len = sprintf((char *)buf, "    \"%s\" ", outseq);
    while (len < 35)
        buf[len++] = '.';
    ss = buf+len ;
    *ss++ = ' ';
    if(ktp->arg)
        sprintf((char *)ss,"%d ",(int) (ktp->arg + 0x80000000)) ;
    else
        *ss = 0 ;
    meStrcat(ss,getCommandName(ktp->index)) ;
    addLineToEob(bp,buf) ;
}

int
descBindings (int f, int n)
{
    meWindow *wp ;
    meBuffer *bp ;
    int     ii ;
    meBind *ktp;
    
    if((wp = meWindowPopup(BbindingsN,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR),NULL)) == NULL)
        return meFALSE ;
    bp = wp->buffer ;
    
#if MEOPT_LOCALBIND
    {
        addLineToEob(bp,(meUByte *)"Buffer bindings:\n");
        
        ii  = frameCur->bufferCur->bindCount ;
        ktp = frameCur->bufferCur->bindList ;
        while(ii--)
            showBinding(bp,ktp++) ;
        
        addLineToEob(bp,(meUByte *)"\nMl bindings:\n") ;
        
        ii  = mlNoBinds ;
        ktp = mlBinds ;
        while(ii--)
            showBinding(bp,ktp++) ;
        addLineToEob(bp,(meUByte *)"") ;
    }
#endif    
    
    addLineToEob(bp,(meUByte *)"Global bindings:\n");
    
    ii  = keyTableSize ;
    ktp = keytab ;
    while(ii--)
        showBinding(bp,ktp++) ;
    
    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotOffset = 0 ;
    bp->dotLineNo = 0 ;
    
    meModeClear(bp->mode,MDEDIT) ;    /* don't flag this as a change */
    meModeSet(bp->mode,MDVIEW) ;      /* put this buffer view mode */
    resetBufferWindows(bp) ;            /* Update the window */
    mlerase(MWCLEXEC);	                /* clear the mode line */
    return meTRUE ;
}
     

