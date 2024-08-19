/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * exec.c - Command execution.
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
 * Synopsis:    Command execution.
 * Authors:     Daniel Lawrence, Jon Green & Steven Phillips
 * Description:
 *     This file is for functions dealing with execution of
 *     commands, command lines, buffers, files and startup files
 */

#define __EXECC                 /* Define program name */

#define KEY_TEST 0

#include "emain.h"
#include "efunc.h"
#include "eskeys.h"
#include "evar.h"
#include "evers.h"


#define DRTESTFAIL   0x80

#define DRUNTILF     (DRUNTIL|DRTESTFAIL)

#define DRRCONTIN    0x60
#define DRRDONE      0x61
#define DRRELIF      0xc8
#define DRRELSE      0xc9
#define DRRENDIF     0x08
#define DRRGOTO      0x62
#define DRRIF        0x38
#define DRRREPEAT    0x20
#define DRRRETURN    0x05
#define DRRUNTIL     0x31
#define DRRUNTILF    0xb3
#define DRRWHILE     0x37
#define DRRWHILEF    0xb9

#define DRRJUMP      0x80

int relJumpTo ;

int
biChopFindString(register meUByte *ss, register int len, register meUByte **tbl,
                 register int size)
{
    register int hi, low, mid, ll ;
    
    hi  = size-1;               /* Set hi water to end of table */
    low = 0;                    /* Set low water to start of table */
    
    do
    {
        meUByte *s1, *s2, cc ;
        mid = (low + hi) >> 1;          /* Get mid value. */
        
        ll = len ;
        s1 = tbl[mid] ;
        s2 = ss ;
        for( ; ((cc=*s1++) == *s2) ; s2++)
            if(!--ll || (cc == 0))
                return mid ;
        if(cc > *s2)
            hi = mid - 1;               /* Discard bottom half */
        else
            low = mid + 1;              /* Discard top half */
    } while(low <= hi);                 /* Until converges */
    
    return -1 ;
}

/* token:       chop a token off a string
 * return a pointer past the token
 *
 * src - source string,
 * tok - destination token string (must be meTOKENBUF_SIZE_MAX in size, 
 * returning a string no bigger than meBUF_SIZE_MAX with the \0)
 */
meUByte *
token(meUByte *src, meUByte *tok)
{
    register meUByte cc, *ss, *dd, *tokEnd ;
    meUShort key ;
    
    ss = src ;
    dd = tok ;
    /* note tokEnd is set to tok+meBUF_SIZE_MAX-1 to leave room for the terminating \0 */
    tokEnd = dd + meBUF_SIZE_MAX-1 ;
    
    /*---       First scan past any whitespace in the source string */
    
    while(((cc = *ss++) == ' ') || (cc == '\t'))
        ;
    
    /*---       Scan through the source string */
    if(cc == '"')
    {
        *dd++ = '"' ;
        while((cc=*ss++) != '"')
        {
            if(cc == '\0') 
            {
                ss-- ;
                break ;
            }
            else if(cc == '\\')
            {
                /* Process special characters */
                switch ((cc = *ss++))
                {
                case '\0':
                    /* this probably should be an error */
                    *dd = 0;
                    ss-- ;
                    return ss ;
                    
                case 'B':
                    /* backspace key - replace with backspace */
                    key = ME_SPECIAL|SKEY_backspace ;
                    goto quote_spec_key1 ;
                case 'C':
                    /* Control key - \C? */
                    if((*dd++ = *ss++ - '@') == meCHAR_LEADER)
                        *dd++ = meCHAR_TRAIL_LEADER ;
                    break;
                case 'D':
                    /* Delete key - replace with delete */
                    key = ME_SPECIAL|SKEY_delete ;
                    goto quote_spec_key1 ;
                case 'E':
                    /* Escape key - replace with esc */
                    key = ME_SPECIAL|SKEY_esc ;
                    goto quote_spec_key1 ;
                case 'H':
                    /* OSD hotkey */
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_HOTKEY ;  
                    break;
                case 'I':
                    /* backward-delete-tab - replace with S-tab */
                    key = ME_SPECIAL|ME_SHIFT|SKEY_tab ;
                    goto quote_spec_key1 ;
                case 'L':
                    /* Exec-line special - replace with x-line key */
                    key = ME_SPECIAL|SKEY_x_line ;
                    goto quote_spec_key1 ;
                case 'N':
                    /* Return key - replace with return */
                    key = ME_SPECIAL|SKEY_return ;
                    goto quote_spec_key1 ;
                case 'P':
                    /* Go to set position, defined by \p - replace with \CXAP */
                    {
                        int ll = meStrlen(__cmdArray[CK_GOAMRK].name) ;
                        if((size_t) (dd+ll+9) <= (size_t) tokEnd)
                        {
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = meCHAR_TRAIL_SPECIAL ;
                            *dd++ = (ME_SPECIAL|SKEY_x_command) >> 8 ;
                            *dd++ = (ME_SPECIAL|SKEY_x_command) & 0xff ;
                            meStrcpy(dd,__cmdArray[CK_GOAMRK].name) ;
                            dd += ll ;
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = meCHAR_TRAIL_SPECIAL ;
                            *dd++ = (ME_SPECIAL|SKEY_return) >> 8 ;
                            *dd++ = (ME_SPECIAL|SKEY_return) & 0xff ;
                            *dd++ = meANCHOR_EXSTRPOS ;
                        }
                        break;
                    }
                case 'T':
                    /* Tab key - replace with tab */
                    key = ME_SPECIAL|SKEY_tab ;
                    goto quote_spec_key1 ;
                case 'X':
                    /* Exec-command special - replace with x-command key */
                    key = ME_SPECIAL|SKEY_x_command ;
quote_spec_key1:
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_SPECIAL ;
                    *dd++ = key >> 8 ;
                    *dd++ = key & 0xff ;
                    break;
                case 'b':   *dd++ = 0x08; break;
                case 'd':   *dd++ = 0x7f; break;
                case 'e':   *dd++ = 0x1b; break;
                case 'f':   *dd++ = 0x0c; break;
                case 'g':   *dd++ = 0x07; break;
                case 'i':
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_SPECIAL ;  
                    *dd++ = '@';
                    *dd++ = 'I' - '@';
                    break;
                case 'n':   *dd++ = 0x0a; break;
                case 'p':
                    {
                        int ll = meStrlen(__cmdArray[CK_SETAMRK].name) ;
                        if((size_t) (dd+ll+9) <= (size_t) tokEnd)
                        {
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = meCHAR_TRAIL_SPECIAL ;
                            *dd++ = (ME_SPECIAL|SKEY_x_command) >> 8 ;
                            *dd++ = (ME_SPECIAL|SKEY_x_command) & 0xff ;
                            meStrcpy(dd,__cmdArray[CK_SETAMRK].name) ;
                            dd += ll ;
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = meCHAR_TRAIL_SPECIAL ;
                            *dd++ = (ME_SPECIAL|SKEY_return) >> 8 ;
                            *dd++ = (ME_SPECIAL|SKEY_return) & 0xff ;
                            *dd++ = meANCHOR_EXSTRPOS ;
                        }
                        break;
                    }
                case 'r':   *dd++ = 0x0d; break;
                case 's':   
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_SPECIAL ;  
                    break;
                case 't':   *dd++ = 0x09; break;
                case 'v':   *dd++ = 0x0b; break;
                case 'x':
                    cc = *ss ;
                    if(isXDigit(cc))
                    {
                        register meUByte c1 ;
                        c1 = *++ss ;
                        if(isXDigit(c1))
                        {
                            cc = (hexToNum(cc) << 4) | hexToNum(c1) ;
                            ss++ ;
                        }
                        else
                            cc = hexToNum(cc) ;
                        if(cc == 0)
                        {
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = 0x01 ;
                        }
                        else if(cc == meCHAR_LEADER)
                        {
                            *dd++ = meCHAR_LEADER ;
                            *dd++ = meCHAR_TRAIL_LEADER ;
                        }
                        else
                            *dd++ = cc ;
                    }
                    break;
                case '{':
                    /* OSD start hilight */
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_HILSTART ;  
                    break;
                case '}':
                    /* OSD stop hilight */
                    *dd++ = meCHAR_LEADER ;
                    *dd++ = meCHAR_TRAIL_HILSTOP ;  
                    break;
                    
                default:
                    *dd++ = cc;
                }
            }
            else if(cc == meCHAR_LEADER)
            {
                *dd++ = meCHAR_LEADER ;
                *dd++ = meCHAR_TRAIL_LEADER ;
            }
            else
                *dd++ = cc;
            if((size_t) dd > (size_t) tokEnd)
                /* reset dd into safe area and keep going cos we must
                 * finish parsing this token */
                dd = tokEnd ;
        }
    }
    else if((cc == '\0') || (cc == ';'))
        ss-- ;
    else
    {
        do
        {
            *dd++ = cc;
            if((cc=*ss++) == '\0')
            {
                ss-- ;
                break ;
            }
        } while((cc != ' ') && (cc != '\t')) ;
    }
    *dd = 0 ;
    return ss ;
}

/* meGetString
 * Get a string input, either from the macro line or from the user
 * prompt  - prompt to use if we must be interactive
 * option  - options for getstring
 * defnum  - Default history number
 * buffer  - buffer to put token into
 * size    - size of the buffer */
int
meGetString(meUByte *prompt, int option, int defnum, meUByte *buffer, int size)
{
    /* if we are not interactive, go get it! */
    if(clexec == meTRUE)
    {
        meUByte buff[meTOKENBUF_SIZE_MAX], *res, *ss, cc ;
        
        /* grab token and advance past */
        ss = execstr ;
        execstr = token(ss, buff);
        
        if((buff[0] == '@') && (buff[1] == 'm') && (buff[2] == 'x'))
        {
            cc = buff[3] ;
            execstr = token(execstr, buff);
            if(cc == 'a')
                execstr = ss ;
            if(prompt == NULL)
                resultStr[0] = '\0' ;
            else
                meStrcpy(resultStr,prompt) ;
            if(lineExec (0, 1, buff) <= 0)
                return meABORT ;
            meStrncpy(buffer,resultStr,size-1) ;
            buffer[size-1] = '\0' ;
            return meTRUE ;
        }
        else if((buff[0] != '@') || (buff[1] != 'm') || (buff[2] != 'n'))
        {
            /* evaluate it */
            res = getval(buff) ;
            if(res != (meUByte *) abortm)
            {
                ss = buffer ;
                if(option & MLMACNORT)
                {
                    while((*ss))
                    {
                        ss++ ;
                        size-- ;
                    }
                }
                if(option & MLFFZERO)
                {
                    while((--size > 0) && ((cc = *res++) != '\0'))
                        *ss++ = cc ;
                }
                else
                {
                    for(; ((--size) > 0) && ((cc = *res++) != '\0') ; )
                    {
                        if((cc == meCHAR_LEADER) && ((cc = *res++) != meCHAR_TRAIL_LEADER))
                            break ;
                        *ss++ = cc ;
                    }
                }
                *ss = '\0' ;
                return meTRUE ;
            }
            if(((buff[0] != '\0') && (buff[0] != ';')) || (option & MLEXECNOUSER))
            {
                *buffer = '\0' ;
                return meFALSE ;
            }
        }
        /* if @mna (get all input from user) then rewind the execstr */
        else if(buff[3] == 'a')
            execstr = ss ;
    }
    if(prompt == NULL)
    {
        *buffer = '\0' ;
        return ctrlg(meFALSE,1) ;
    }
    return meGetStringFromUser(prompt, option, defnum, buffer, size) ;
}

int
macarg(meUByte *tok)               /* get a macro line argument */
{
    int savcle;                 /* buffer to store original clexec */
    int status;
    
    savcle = clexec;            /* save execution mode */
    clexec = meTRUE;              /* get the argument */
    status = meGetString(NULL,MLNOHIST|MLFFZERO,0,tok,meBUF_SIZE_MAX) ;
    clexec = savcle;            /* restore execution mode */
    
    return status ;
}

#if KEY_TEST
/*---   Little function to test the  alphabetic state of the table.  Invoked
   via "!test" primative.  */

int
fnctest(void)
{
    register meBind *ktp;                       /* Keyboard character array */
    register meCommand *cmd ;                       /* Names pointer */
    meUInt key ;
    meUByte outseq[12];
    int count=0, ii;                            /* Counter of errors */
    
    /* test the command hash table */
    for (ii=0; ii < CK_MAX; ii++)
    {
        key = cmdHashFunc(getCommandName(ii)) ;
        cmd = cmdHash[key] ;
        while((cmd != NULL) && (cmd != cmdTable[ii]))
            cmd = cmd->hnext ;
        if(cmd == NULL)
        {
            count++;
            mlwrite(MWWAIT,(meUByte *) "cmdHash Error: [%s] should be in position %d", 
                    getCommandName(ii),key) ;
        }
    }
    
    /* test the command alphabetically ordered list */
    cmd = cmdHead ;
    while(cmd->anext != NULL)
    {
        if(meStrcmp(cmd->anext->name,cmd->name) < 0)
        {
            count++;
            mlwrite(MWWAIT,(meUByte *) "cmdHead Error: [%s] should be before [%s]", 
                    cmd->anext->name,cmd->name) ;
        }
        cmd = cmd->anext ;
    }
    /* Scan through the key table looking for the character code.  If found
     * then return the index into the names table.  */
    
    for (ktp = &keytab[0]; ktp[1].code != 0; ktp++)
        if (ktp->code > ktp[1].code)
        {
            count++;
            meGetStringFromKey(ktp->code, outseq);
            mlwrite(MWWAIT,(meUByte *) "[%s] key out of place",outseq);
        }
    
    if(count)
        mlwrite(MWWAIT,(meUByte *) "!test: %d Error(s) Detected",count);
    return (count);
}
#endif

static int
domstore(meUByte *cline)
{
    register meUByte cc ;          /* Character */
    
    if(mcStore == 2)
    {
#if MEOPT_EXTENDED
        /* check to see if this line ends definition */
        if(meStrncmp(cline, "!ehe",4) == 0)
        {
            helpBufferReset(lpStoreBp) ;
            mcStore = 0 ;
            lpStore = NULL;
            return meTRUE ;
        }
#endif
        if(addLine(lpStore,cline) <= 0)
            return meFALSE ;
        lpStoreBp->lineCount++ ;
        return meTRUE ;
    }
    cc = *cline ;
    /* eat leading spaces */
    while((cc == ' ') || (cc == '\t'))
        cc = *++cline ;
    
    /* dump comments and empties here */
    if((cc == ';') || (cc == '\0'))
        return meTRUE ;
    /* check to see if this line turns macro storage off */
    if((cc == '!') && !meStrncmp(cline+1, "ema", 3) && !execlevel--)
    {
        mcStore = meFALSE;
        lpStore = NULL;
        execlevel = 0 ;
        return meTRUE ;
    }
    else
    {
        meUByte *ss = cline ;
        if(isDigit(cc))
        {
            do
                cc = *++ss ;
            while(isDigit(cc)) ;
            while((cc == ' ') || (cc == '\t'))
                cc = *++ss ;
        }
        if((cc == 'd') && !meStrncmp(ss, getCommandName(CK_DEFMAC),12) &&
           (((cc=ss[12]) == ' ') || (cc == '\t') || (cc == ';') || (cc == '\0')))
            execlevel++ ;
    }
    return addLine(lpStore,cline) ;
}


/*
   docmd - take a passed string as a command line and translate
   it to be executed as a command. This function will be
   used by execute-line and by all source and
   startup files.
   
   format of the command line is:
   
   {# arg} <command-name> {<argument string(s)>}
   
   Directives start with a "!" and include:
   
   !endm           End a macro
   !if (cond)      conditional execution
   !else
   !endif
   !return         Return (terminating current macro)
   !goto <label>   Jump to a label in the current macro
   
   Line Labels begin with a "*" in column 1, like:
   
 *LBL01
 */
/* cline        command line to execute */
static int
docmd(meUByte *cline, register meUByte *tkn)
{
    int  f ;           /* default argument flag */
    int  n ;           /* numeric repeat value */
    meUByte cc ;         /* Character */
    int  status ;      /* return status of function */
    int  nmacro ;      /* run as it not a macro? */
    
    cc = *cline ;
    /* eat leading spaces */
    while((cc == ' ') || (cc == '\t'))
        cc = *++cline ;
    
    /* dump comments, empties and labels here */
    if((cc == ';') || (cc == '\0') || (cc == '*'))
        return meTRUE ;
    
    nmacro = meFALSE;
    execstr = cline;    /* and set this one as current */
    meRegCurr->force = 0 ;
    
    /* process argument */
try_again:
    execstr = token(execstr, tkn);
    if((status=getMacroTypeS(tkn)) == TKDIR)
    {
        int dirType ;
        
        /* SWP 2003-12-22 Used to use the biChopFindString to look the
         * derivative up in a name table, but after profiling found this to be
         * the second biggest consumer of cpu and the function was 3rd (token
         * was the worst at 42%, the call to look-up the derivative was second
         * taking over 9% and docmd took 7%).
         * 
         * By changing this to an optimised set of if statements embedded in
         * docmd the time usage in my tests dropped from 4.52 sec to 2.73, i.e.
         * 40% time saving.
         * 
         * So yes this does look horrid but its fast!
         */
        status = -1 ;
        
        cc = tkn[1] ;
        if(cc > 'e')
        {
            if(cc < 'r')
            {
                if(cc < 'i')
                {
                    if(cc == 'f')
                    {
                        if((tkn[2] == 'o') && (tkn[3] == 'r'))
                            status = DRFORCE ;
                    }
                    else if(cc == 'g')
                    {
                        if((tkn[2] == 'o') && (tkn[3] == 't'))
                            status = DRGOTO ;
                    }
                }
                else if(cc == 'i')
                {
                    if((tkn[2] == 'f') && (tkn[3] == '\0'))
                        status = DRIF ;
                    if((tkn[2] == 'i') && (tkn[3] == 'f'))
                        status = DRIIF ;
                }
                else if(cc == 'j')
                {
                    if((tkn[2] == 'u') && (tkn[3] == 'm'))
                        status = DRJUMP ;
                }
                else if(cc == 'n')
                {
                    if((tkn[2] == 'm') && (tkn[3] == 'a'))
                        status = DRNMACRO ;
                }
            }
            else if(cc == 'r')
            {
                if(tkn[2] == 'e')
                {
                    if(tkn[3] == 'p')
                        status = DRREPEAT ;
                    else if(tkn[3] == 't')
                        status = DRRETURN ;
                }
            }
            else if(cc > 't')
            {
                if(cc == 'u')
                {
                    if((tkn[2] == 'n') && (tkn[3] == 't'))
                        status = DRUNTIL ;
                }
                else if(cc == 'w')
                {
                    if((tkn[2] == 'h') && (tkn[3] == 'i'))
                        status = DRWHILE ;
                }
            }
            else if(cc == 't')
            {
                if((tkn[2] == 'g') && (tkn[3] == 'o'))
                    status = DRTGOTO ;
                if((tkn[2] == 'j') && (tkn[3] == 'u'))
                    status = DRTJUMP ;
#if KEY_TEST
                else if((tkn[2] == 'e') && (tkn[3] == 's'))
                    status = DRTEST ;
#endif
            }
        }
        else if(cc == 'e')
        {
            cc = tkn[2] ;
            if(cc > 'l')
            {
                if((cc == 'n') && (tkn[3] == 'd'))
                    status = DRENDIF ;
                else if((cc == 'm') && (tkn[3] == 'a'))
                    status = DREMACRO ;
            }
            else if(cc == 'l')
            {
                if(tkn[3] == 'i')
                    status = DRELIF ;
                else if(tkn[3] == 's')
                    status = DRELSE ;
            }
        }
        else if(cc > 'b')
        {
            if((tkn[2] == 'o') && (tkn[3] == 'n'))
                status = (cc == 'c') ? DRCONTIN:DRDONE ;
        }
        else if(cc == 'b')
        {
            if((tkn[2] == 'e') && (tkn[3] == 'l'))
                status = DRBELL ;
        }
        else if((cc == 'a') && (tkn[2] == 'b') && (tkn[3] == 'o'))
            status = DRABORT ;
        
        if(status < 0)
            return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unknown directive %s]",tkn);
        
        dirType = dirTypes[status] ;
        
        if(execlevel == 0)
        {
elif_jump:
            if(dirType & DRFLAG_TEST)
            {
                if(macarg(tkn) <= 0)
                    return meFALSE ;
                if(!meAtol(tkn))
                {
                    /* if DRTGOTO or DRTJUMP and the test failed, we dont
                     * need the argument and if DRIIF we don't want the switch */
                    dirType &= ~(DRFLAG_ARG|DRFLAG_SWITCH);
                    execlevel += (dirType & DRFLAG_AMSKEXECLVL);
                    status |= DRTESTFAIL ;
                }
            }
            else if(dirType & DRFLAG_ASGLEXECLVL)
                /* DRELIF DRELSE */
                execlevel += 2 ;
            if(dirType & DRFLAG_ARG)
            {
                if(meGetString(NULL,MLNOHIST|MLFFZERO|MLEXECNOUSER,0,tkn,meBUF_SIZE_MAX) <= 0)
                {
                    if(!(dirType & DRFLAG_OPTARG))
                        return meFALSE ;
                    f = meFALSE;
                    n = 1 ;
                }
                else if(dirType & DRFLAG_NARG)
                {
                    f = meTRUE;
                    n = meAtoi(tkn) ;
                    if(dirType & DRFLAG_JUMP)
                        relJumpTo = n ;
                }
            }
            if(!(dirType & DRFLAG_SWITCH))
                return status ;
            
            
            /* DRTEST, DRFORCE, DRNMACRO, DRABORT, DRBELL, DRIIF, DRRETURN */
            switch(status)
            {
#if KEY_TEST
            case DRTEST:
                /* Test directive. */
                return ((fnctest() == 0));
#endif
            case DREMACRO:
                return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unexpected !emacro]");
            case DRFORCE:
                (meRegCurr->force)++ ;
                goto try_again;
            case DRIIF:
                while(((cc=*execstr) == ' ') || (cc == '\t'))
                    execstr++;
                goto try_again;
            case DRNMACRO:
                nmacro = meTRUE;
                goto try_again;
            case DRABORT:
                if(f)
                    TTdoBell(n) ;
                return meFALSE ;
            case DRBELL:
                TTdoBell(n) ;
                return meTRUE ;
            case DRRETURN:
                /* Stop the debugger kicking in on a !return, a macro doing !return 0 is okay */
                meRegCurr->force = 1 ;
                return (n) ? DRRETURN:meFALSE ;
            }
        }
        
        if(dirType & DRFLAG_SMSKEXECLVL)
        {
            if(execlevel == 1)
            {
                execlevel = 0 ;
                if(status == DRELIF)
                {
                    dirType = DRFLAG_ASGLEXECLVL|DRFLAG_TEST ;
                    goto elif_jump ;
                }
            }
            else if(dirType & DRFLAG_SDBLEXECLVL)
                execlevel -= 2 ;
        }
        else if(dirType & DRFLAG_AMSKEXECLVL)
            execlevel += 2 ;
        
        return meTRUE ;
    }
    /* if not a directive and execlevel > 0 ignore it */
    if(execlevel)
        return meTRUE ;
    
    /* first set up the default command values */
    f = meFALSE;
    n = 1;
    
    if(status != TKCMD)
    {
        meUByte *tmp ;
        
        if((tmp = getval(tkn)) == abortm)
            return meFALSE ;
        n = meAtoi(tmp) ;
        f = meTRUE;
        
        execstr = token(execstr, tkn);
        if(getMacroTypeS(tkn) != TKCMD)
            return meFALSE ;
    }
    
    {
        register int idx ; /* index to function to execute */
        
        if(execlevel)
            return meTRUE ;
        
        /* and match the token to see if it exists */
        if ((idx = decode_fncname(tkn,0)) < 0)
            return meFALSE ;
        if(nmacro)
            clexec = meFALSE ;
        cmdstatus = (execFunc(idx,f,n) > 0) ;       /* call the function */
        clexec = meTRUE ;
        if(cmdstatus)
            status = meTRUE ;
        else if(TTbreakFlag)
        {
            if(meRegCurr->force > 1)
            {
                /* A double !force - macro says \CG is okay, 
                 * remove all input (must remove the \CG) and rest flag
                 */
                TTinflush() ;
                TTbreakFlag = 0 ;
                status = meTRUE ;
            }
            else
                status = meFALSE ;
        }
        else if(meRegCurr->force)
            status = meTRUE ;
        else
            status = meFALSE ;
        return status ;
    }
}

/*      dobuf:  execute the contents of the buffer pointed to
   by the passed BP                                */
/* bp - buffer to execute */
static meLine *errorLine=NULL ;

static long
macroPrintError(meInt type, meLine *hlp, meUByte *macName)
{
    meLine *lp ;
    long  ln ;
    
    if(errorLine == NULL)
        /* Quit if there is no error line */
        return -1 ;
    
    lp=meLineGetNext(hlp) ;
    ln=0 ;
    while(lp != errorLine)
    {
        lp = meLineGetNext(lp) ;
        ln++ ;
    }
    mlwrite(MWABORT|MWWAIT,(meUByte *)"[%s:: %s executing %s, line %d]",errorLine->text,(type) ? "Halt":"Error",macName,ln+1) ;
    return ln ;
}

static int
dobuf(meLine *hlp)
{
#if MEOPT_DEBUGM
    static meUByte __dobufStr1[]="(C)ontinue, (S)tep, (V)ariable, <any>=next, (^G)Abort, (^L)redraw ? " ;
    meByte debug=0;
#endif
    meUByte *tline;               /* Temp line */
    int status;                 /* status return */
    meLine *lp;                   /* pointer to line to execute */
    meLine *wlp;                  /* line to while */
    meLine *rlp;                  /* line to repeat */
    
    clexec = meTRUE;                      /* in cline execution */
    execstr = NULL ;
    execlevel = 0;                      /* Reset execution level */
    
    /* starting at the beginning of the buffer */
    wlp = NULL;
    rlp = NULL;
    lp = hlp->next;
    while (lp != hlp)
    {
        tline = lp->text;
#if MEOPT_DEBUGM
        /* if $debug == meTRUE, every line to execute
           gets echoed and a key needs to be pressed to continue
           ^G will abort the command */
        
        if((macbug & 0x02) && (!execlevel || (macbug & 0x04) || ((execlevel == 1) && !meStrncmp(tline,"!el",3))))
        {
            meUByte dd, outline[meBUF_SIZE_MAX];   /* string to hold debug line text */
            meLine *tlp=hlp ;
            meUShort cc ;
            int lno=0 ;
            
            dd = macbug & ~0x80 ;
            /* force debugging off while we are getting input from the user,
             * if we don't and any macro is executed (idle-pick macro) then
             * ME will spin and crash!! */
            macbug = 0 ;
            
            /*---   Generate the debugging line for the 'programmer' !! */
            do 
                lno++ ;
            while ((tlp=meLineGetNext(tlp)) != lp)
                ;
            sprintf((char *)outline,"%s:%d:%d [%s] ? ",meRegCurr->commandName,lno,execlevel,tline) ;
loop_round2:
            mlwrite(MWSPEC,outline);        /* Write out the debug line */
            /* Cannot do update as if this calls a macro then
             * we will end up in an infinite loop, the best we can do is
             * call the screenUpdate function
             */
            screenUpdate(meTRUE,2-sgarbf) ;
            /* reset garbled status */
            sgarbf = meFALSE ;
            if((dd & 0x08) == 0)
            {
loop_round:
                /* and get the keystroke */
                if(((cc = meGetKeyFromUser(meFALSE,0,meGETKEY_SILENT|meGETKEY_SINGLE)) & 0xff00) == 0)
                    cc = toLower(cc) ;
                switch(cc)
                {
                case '?':
                    mlwrite(MWSPEC,__dobufStr1) ;       /* Write out the debug line */
                    goto loop_round ;
                case 'L'-'@':
                    goto loop_round2 ;
                case 'v':
                    {
                        meUByte mlStatusStore ;
                        mlStatusStore = frameCur->mlStatus ;
                        frameCur->mlStatus = 0 ;
                        clexec = meFALSE ;
                        descVariable(meFALSE,1) ;
                        clexec = meTRUE ;
                        frameCur->mlStatus = mlStatusStore ;
                        goto loop_round ;
                    }
                default:
                    if(cc == breakc)
                    {
                        /* Abort - Must exit the usual way so that macro
                         * storing and execlevel are corrected */
                        ctrlg(meFALSE,1) ;
                        status = meABORT ;
                        errorLine = lp ;
                        macbug = dd ;
                        goto dobuf_exit ;
                    }
                    debug = dd ;
                case 'c':
                    dd &= ~0x02 ;
                case 's':
                    break;
                }
            }
            macbug = dd ;
        }
#endif
        if(TTbreakTest(0))
        {
            mcStore = meFALSE;
            lpStore = NULL;
            execlevel = 0 ;
            status = ctrlg(meFALSE,1) ;
        }
        else
        {
            meUByte tkn[meTOKENBUF_SIZE_MAX] ;
            
            if(mcStore)
                status = domstore(tline) ;
            else
                status = docmd(tline,tkn) ;
#if     MEOPT_DEBUGM
            if(debug)
            {
                macbug = debug ;
                debug = 0 ;
            }
#endif
            switch(status)
            {
            case meABORT:
            case meFALSE:
            case meTRUE:
                break ;
                
            case DRGOTO:
            case DRTGOTO:
                {
                    register int linlen;        /* length of line to execute */
                    register meLine *glp;         /* line to goto */
                    
                    linlen = meStrlen(tkn) ;
                    glp = hlp->next;
                    while (glp != hlp)
                    {
                        if (*glp->text == '*' &&
                            (meStrncmp(glp->text+1,tkn,linlen) == 0) &&
                            isSpace(glp->text[linlen+1]))
                        {
                            lp = glp;
                            status = meTRUE;
                            break ;
                        }
                        glp = glp->next;
                    }
                    
                    if (status == DRRGOTO)
                        status = mlwrite(MWABORT|MWWAIT,(meUByte *)"No such label");
                    break;
                }
            case DRREPEAT:                      /* REPEAT */
                if (rlp == NULL)
                {
                    rlp = lp;               /* Save line */
                    status = meTRUE;
                }
                else
                    status = mlwrite(MWABORT|MWWAIT,(meUByte *)"Nested Repeat");
                break;
            case DRUNTIL:                       /* meTRUE UNTIL */
                if (rlp != NULL)
                {
                    rlp = NULL;
                    status = meTRUE;
                }
                else
                    status = mlwrite(MWABORT|MWWAIT,(meUByte *)"No repeat set");
                break;  
            case DRUNTILF:                      /* meFALSE UNTIL */
                if (rlp != NULL)
                {
                    lp = rlp;               /* Back to 'repeat' */
                    status = meTRUE;
                }
                else
                    status = mlwrite(MWABORT|MWWAIT,(meUByte *)"No repeat set");
                break; 
            case DRCONTIN:
                if (wlp != NULL)
                {
                    lp = wlp;
                    continue;
                }
                status = mlwrite(MWABORT|MWWAIT,(meUByte *)"No while");
                break;
                
            case DRDONE:        
                if (wlp != NULL)
                {
                    lp = wlp;
                    wlp = NULL;
                    continue;
                }
                status = mlwrite(MWABORT|MWWAIT,(meUByte *)"No while");
                break;
                
            case DRWHILE:                       
                wlp = lp;
                status = meTRUE;
                break;
                
            case DRJUMP:
            case DRTJUMP:
                if(relJumpTo < 0)
                    for( ; relJumpTo < 0 ; relJumpTo++)
                        lp = lp->prev ;
                else
                    for( ; relJumpTo > 0 ; relJumpTo--)
                        lp = lp->next ;
                continue;
                
            case DRRETURN:      /* if it is a !RETURN directive...do so */
                status = meTRUE ;
                goto dobuf_exit ;
            default:
                status = meTRUE ;
                
            }
        }
        /* check for a command error */
        if(status <= 0)
        {
            /* in any case set the buffer . */
            errorLine = lp;
#if     MEOPT_DEBUGM
            if(macbug & 0x01)
            {
                /* check if the failure is handled by a !force */
                meRegister *rr ;
                rr = meRegCurr ;
                for(;; rr = rr->prev)
                {
                    if(rr->force)
                        goto dobuf_exit ;
                    if(rr == meRegHead) 
                        break ;
                }
                macroPrintError(0,hlp,meRegCurr->commandName) ;
            }
#endif
            goto dobuf_exit ;
        }
#if     MEOPT_DEBUGM
        if(macbug & 0x80)
        {
            errorLine = lp;
            macroPrintError(1,hlp,meRegCurr->commandName) ;
            macbug &= ~0x80 ;
        }
#endif
        
        /* on to the next line */
        lp = lp->next;
    }
    if(mcStore)
        mlwrite(MWABORT|MWWAIT,(meUByte *)"[Missing !emacro termination]");
    else
        status = meTRUE ;
    
    /* exit the current function */
dobuf_exit:
    
    if(mcStore)
    {
        mcStore = 0 ;
        lpStore = NULL ;
        execlevel = 0 ;
        errorLine = hlp;
        status = meFALSE ;
    }
    
    return status ;
}


static int
donbuf(meLine *hlp, meVarList *varList, meUByte *commandName, int f, int n)
{
    meRegister  rp ;
    meUByte oldcle ;
    int oldexec, status ;
    
    if(meRegCurr->depth >= meMACRO_DEPTH_MAX)
    {
        /* macro recursion gone too deep, bail out.
         * attempt to force the break out by pretending C-g was pressed */
        TTbreakFlag = 1 ;
        return mlwrite(MWABORT|MWWAIT,(meUByte *)"[Macro recursion gone too deep]") ;
    }
    /* save the arguments */
    oldcle = clexec;
    oldexec = execlevel ;
    /* Setup these argument and move the register stack */
    rp.prev = meRegCurr ;
    rp.commandName = commandName ;
    rp.execstr = execstr ;
#if MEOPT_EXTENDED
    rp.varList = varList ;
#endif
    rp.f = f ;
    rp.n = n ;
    rp.depth = meRegCurr->depth + 1 ;
    meRegCurr = &rp ;
    
    status = dobuf(hlp) ;
    
    /* restore old arguments & stack */
    meRegCurr = rp.prev ;
    execstr = rp.execstr ;
    execlevel = oldexec ;
    clexec = oldcle;
    
    cmdstatus = status ;
    return status ;
}

int
execFunc(int index, int f, int n)
{
    register int status;
    
    if(index < CK_MAX)
    {
        thisflag = 0 ;
        status = cmdTable[index]->func(f,n) ;
        if(isComIgnore(index))
        {
            thisflag = lastflag ;
            return status ;
        }
        lastflag = thisflag;
        if(selhilight.uFlags)
        {
            if((status > 0) || isComSelIgnFail(index))
            {
                if(isComSelKill(index))
                {
                    selhilight.bp = NULL ;
                    selhilight.flags = 0 ;
                }
                else
                {
                    if(isComSelStart(index))
                    {
                        selhilight.bp = frameCur->bufferCur;                  /* Select the current buffer */
                        selhilight.flags = SELHIL_ACTIVE|SELHIL_CHANGED ;
                    }
                    if(selhilight.flags & SELHIL_FIXED)
                    {
                        if(!(selhilight.flags & SELHIL_KEEP) && isComSelDelFix(index))
                            selhilight.flags &= ~SELHIL_ACTIVE ;
                    }
                    else if(selhilight.flags & SELHIL_ACTIVE)
                    {
                        if(isComSelSetMark(index) &&
                           ((selhilight.markOffset != frameCur->windowCur->markOffset) ||
                            (selhilight.markLineNo != frameCur->windowCur->markLineNo)))
                        {
                            selhilight.flags |= SELHIL_CHANGED ;
                            selhilight.markOffset = frameCur->windowCur->markOffset;   /* Current mark offset */
                            selhilight.markLineNo = frameCur->windowCur->markLineNo;    /* Current mark line number */
                        }
                        if(isComSelSetDot(index) &&
                           ((selhilight.dotOffset != frameCur->windowCur->dotOffset) ||
                            (selhilight.dotLineNo != frameCur->windowCur->dotLineNo)))
                        {
                            selhilight.flags |= SELHIL_CHANGED ;
                            selhilight.dotOffset = frameCur->windowCur->dotOffset;   /* Current mark offset */
                            selhilight.dotLineNo = frameCur->windowCur->dotLineNo;    /* Current mark line number */
                        }
                        if(isComSelSetFix(index))
                        {
                            selhilight.flags |= SELHIL_FIXED ;
                            if(selhilight.uFlags & SELHILU_KEEP)
                                selhilight.flags |= SELHIL_KEEP ;
                        }
                        if(isComSelStop(index))
                            selhilight.flags &= ~SELHIL_ACTIVE ;
                        if(selhilight.flags & SELHIL_CHANGED)
                            meBufferAddModeToWindows(selhilight.bp, WFSELHIL);
                    }
                }
            }
            else if(!meRegCurr->force && ((selhilight.flags & (SELHIL_ACTIVE|SELHIL_KEEP)) == SELHIL_ACTIVE))
                selhilight.flags &= ~SELHIL_ACTIVE ;
        }
    }
    else
    {
        meMacro *mac = getMacro(index) ;
#if MEOPT_EXTENDED
        meUByte firstExec ;                    /* set if this is first */
        meLine *hlp ;
        
        hlp = mac->hlp ;
        if(!(hlp->flag & meMACRO_EXEC))
        {
            if(hlp->flag & meMACRO_FILE)
            {
                if(mac->fname != NULL)
                    execFile(mac->fname,0,1) ;
                else
                    execFile(mac->name,0,1) ;
                hlp = mac->hlp ;
                if(hlp->flag & meMACRO_FILE)
                    return mlwrite(MWABORT,(meUByte *)"[file-macro %s not defined]",mac->name);
            }
            hlp->flag |= meMACRO_EXEC ;
            firstExec = 1 ;
        }
        else
            firstExec = 0 ;
        
        status = donbuf(hlp,&mac->varList,mac->name,f,n) ;
        
        if(firstExec)
        {
            if(mac->hlp != hlp)
                meLineLoopFree(hlp,1) ;
            else
                hlp->flag &= ~meMACRO_EXEC ;
        }
#else
        status = donbuf(mac->hlp,NULL,mac->name,f,n) ;
#endif
    }
    
    return status ;
}

/* Execute the given command, but in a hidden way, i.e.
 * the 'key' is not recorded
 */
void
execFuncHidden(int keyCode, int index, meUInt arg)
{
    meUByte tf, lf, cs;
    int f, n, tc, ti, lc, li, sv ;
#if MEOPT_EXTENDED
    int ii ;
#endif
    
    cs = cmdstatus ;
    lc = lastCommand ;
    li = lastIndex ;
    lf = lastflag ;
    tc = thisCommand ;
    ti = thisIndex ;
    tf = thisflag ;
    if((sv=(alarmState & meALARM_VARIABLE)))
        alarmState &= ~meALARM_VARIABLE ;
    lastCommand = tc ;
    lastIndex = ti ;
    thisCommand = keyCode ;
    thisIndex = index ;
    if(arg != 0)
    {
        f = 1 ;
        n = (int) (arg + 0x80000000) ;
    }
    else
    {
        f = 0 ;
        n = 1 ;
    }
#if MEOPT_EXTENDED
    if((ii=frameCur->bufferCur->inputFunc) >= 0)
    {
        meUByte *ss, ff ;
        /* set a force value for the execution as the macro is allowed to
         * fail and we don't want to kick in the macro debugging */
        ff = meRegHead->force ;
        meRegHead->force = 1 ;
        if((execFunc(ii,f,n) > 0) ||
           ((ss=getUsrLclCmdVar((meUByte *)"status",&(cmdTable[ii]->varList))) == errorm) || meAtoi(ss))
            index = -1 ;
        meRegHead->force = ff ;
    }
    if(index >= 0)
#endif
        execFunc(index,f,n);
    if(sv)
        alarmState |= meALARM_VARIABLE ;
    thisflag = tf;
    thisIndex = ti ;
    thisCommand = tc ;
    lastflag = lf;
    lastIndex = li ;
    lastCommand = lc ;
    cmdstatus = cs ;
}

int
execBufferFunc(meBuffer *bp, int index, int flags, int n)
{
    meSelection sh ;
    meUByte tf, lf, cs;
    int cg, tc, ti, ret ;
    
    cs = cmdstatus ;
    tc = thisCommand ;
    ti = thisIndex ;
    tf = thisflag ;
    lf = lastflag ;
    thisIndex = index ;
    if(flags & meEBF_USE_B_DOT)
    {
        cg = curgoal ;    
        memcpy(&sh,&selhilight,sizeof(meSelection)) ;
        if(bp == frameCur->bufferCur)
        {
            /* drop an anchor and return back to it after */
            meAnchorSet(frameCur->bufferCur,meANCHOR_EXEC_BUFFER,frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset,1) ;
            frameCur->windowCur->dotLine = frameCur->bufferCur->dotLine ;
            frameCur->windowCur->dotOffset = frameCur->bufferCur->dotOffset ;
            frameCur->windowCur->dotLineNo = frameCur->bufferCur->dotLineNo ;
        }
    }
    if(bp != frameCur->bufferCur)
    {
        /* swap this buffer into the current window, execute the function
         * and then swap back out */
        register meBuffer *tbp = frameCur->bufferCur ;
        meUInt flag;
        meInt vertScroll ;
        
        storeWindBSet(tbp,frameCur->windowCur) ;
        flag = frameCur->windowCur->updateFlags ;
        vertScroll = frameCur->windowCur->vertScroll ;
        
        tbp->windowCount-- ;
        frameCur->bufferCur = frameCur->windowCur->buffer = bp ;
        restoreWindBSet(frameCur->windowCur,bp) ;
#if MEOPT_EXTENDED
        isWordMask = bp->isWordMask ;
#endif
        bp->windowCount++ ;
        
        ret = execFunc(index,(flags & meEBF_ARG_GIVEN),n) ;
        
        frameCur->bufferCur->windowCount-- ;
        storeWindBSet(frameCur->bufferCur,frameCur->windowCur) ;
        frameCur->bufferCur = frameCur->windowCur->buffer = tbp ;
        tbp->windowCount++ ;
        frameCur->windowCur->vertScroll = vertScroll ;
        /* force a check on update, just incase the Func has done something behind our back */
        frameCur->windowCur->updateFlags |= (flag|WFMOVEL|WFMAIN) ;
        restoreWindBSet(frameCur->windowCur,tbp) ;
#if MEOPT_EXTENDED
        isWordMask = tbp->isWordMask ;
#endif
    }
    else
        ret = execFunc(index,(flags & meEBF_ARG_GIVEN),n) ;
    if(flags & meEBF_USE_B_DOT)
    {
        if(bp == frameCur->bufferCur)
        {
            /* return back to the anchor */
            if(meAnchorGet(frameCur->bufferCur,meANCHOR_EXEC_BUFFER) > 0)
            {
                frameCur->windowCur->dotLine = frameCur->bufferCur->dotLine ;
                frameCur->windowCur->dotOffset = frameCur->bufferCur->dotOffset ;
                frameCur->windowCur->dotLineNo = frameCur->bufferCur->dotLineNo ;
                frameCur->windowCur->updateFlags |= WFMOVEL ;
            }
            meAnchorDelete(frameCur->bufferCur,meANCHOR_EXEC_BUFFER) ;
        }
        selhilight.bp = NULL ;
        selhilight.flags = 0 ;
        if(sh.flags != 0)
        {
            meBuffer *bp = bheadp ;
            while((bp != NULL) && (bp != sh.bp))
                bp = bp->next;
            if((bp != NULL) && (sh.dotLineNo < bp->lineCount) && (sh.markLineNo < bp->lineCount))
                memcpy(&selhilight,&sh,sizeof(meSelection)) ;
        }
        curgoal = cg ;    
    }
    thisflag = tf;
    lastflag = lf;
    thisIndex = ti ;
    thisCommand = tc ;
    cmdstatus = cs ;
    return ret ;
}

/* executeBuffer:     Execute the contents of a buffer of commands    */
int
executeBuffer(int f, int n)
{
    register meBuffer *bp;                /* ptr to buffer to execute */
    register int s;                     /* status return */
    meVarList varList={NULL,0} ;
    meUByte  bufn[meBUF_SIZE_MAX];                /* name of buffer to execute */
    meInt  ln ;
    
    /* find out what buffer the user wants to execute */
    if((s = getBufferName((meUByte *)"Execute buffer", 0, 1, bufn)) <= 0)
        return s ;
    
    /* find the pointer to that buffer */
    if ((bp=bfind(bufn, 0)) == NULL)
        return mlwrite(MWABORT,(meUByte *)"No such buffer");
    
    /* and now execute it as asked */
    if(((s = donbuf(bp->baseLine,&varList,bp->name,f,n)) <= 0) &&
       ((ln = macroPrintError(0,bp->baseLine,bufn)) >= 0))
    {
        /* the execution failed lets go to the line that caused the grief */
        bp->dotLine  = errorLine ;
        bp->dotLineNo = ln ;
        bp->dotOffset  = 0 ;
        /* look if buffer is showing */
        if(bp->windowCount)
        {
            register meWindow *wp;        /* ptr to windows to scan */
            
            meFrameLoopBegin() ;
            wp = loopFrame->windowList;
            while (wp != NULL)
            {
                if (wp->buffer == bp)
                {
                    /* and point it */
                    wp->dotLine = errorLine ;
                    wp->dotOffset = 0 ;
                    wp->dotLineNo = ln ;
                    wp->updateFlags |= WFMOVEL ;
                }
                wp = wp->next;
            }
            meFrameLoopEnd() ;
        }
    }
    /* free off any command variables */
    if(varList.head != NULL)
    {
        meVariable *cv=varList.head, *ncv ;
        while(cv != NULL)
        {
            ncv = cv->next ;
            meNullFree(cv->value) ;
            free(cv) ;
            cv = ncv ;
        }
    }
    return s ;
}

/* execFile: yank a file into a buffer and execute it
   if there are no errors, delete the buffer on exit */
/* Note for Dynamic file names
 * This ensures that nothing is done to fname
 */
int
execFile(meUByte *fname, int f, int n)
{
    meVarList     varList={NULL,0} ;
    meUByte       fn[meBUF_SIZE_MAX] ;
    meLine        hlp ;
    register int  status=0 ;
    
    if((fname == NULL) || (fname[0] == '\0'))
    {
        fname = (meUByte *) ME_SHORTNAME ;
#ifdef _NANOEMACS
        /* don't print-out the '[Failed to load file ne.emf]' error for ne - who cares */ 
        status = 1 ;
#endif
    }
    hlp.next = &hlp ;
    hlp.prev = &hlp ;
    /* use a new buffer to ensure it doesn't mess with any loaded files */
    if(!fileLookup(fname,(meUByte *)".emf",meFL_CHECKDOT|meFL_USESRCHPATH,fn) ||
       (ffReadFile(fn,meRWFLAG_SILENT,NULL,&hlp,0,0,0) == meABORT))
    {
        if(status)
            return meABORT ;
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Failed to load file %s]", fname);
    }
    /* go execute it! */
    if((status = donbuf(&hlp,&varList,fn,f,n)) <= 0)
        /* the execution failed lets go to the line that caused the grief */
        macroPrintError(0,&hlp,fn) ;
    meLineLoopFree(&hlp,0) ;
    /* free off any command variables */
    if(varList.head != NULL)
    {
        meVariable *cv=varList.head, *ncv ;
        while(cv != NULL)
        {
            ncv = cv->next ;
            meNullFree(cv->value) ;
            free(cv) ;
            cv = ncv ;
        }
    }
    
    return status ;
}

int
executeFile(int f, int n)  /* execute a series of commands in a file */
{
    meUByte filename[meBUF_SIZE_MAX];      /* Filename */
    int status;
    
    if ((status=meGetString((meUByte *)"Execute File",MLFILECASE, 0, filename, 
                            meBUF_SIZE_MAX)) <= 0)
        return status ;
    
    /* otherwise, execute it */
    return execFile(filename,f,n) ;
}

/* executeNamedCommand: execute a named command even if it is not bound */
int
executeNamedCommand(int f, int n)
{
    register int idx ;       /* index to the requested function */
    meUByte buf[meBUF_SIZE_MAX];       /* buffer to hold tentative command name */
    meUByte prm[30] ;
    
    /* setup prompt */
    if(f == meTRUE)
        sprintf((char *)prm,"Arg %d: Command",n) ;
    else
        meStrcpy(prm,"Command") ;
    
    /* if we are executing a command line get the next arg and match it */
    if((idx = meGetString(prm, MLCOMMAND, 1, buf, meBUF_SIZE_MAX)) <= 0)
        return idx ;
    
    /* decode the function name, if -ve then duff */
    if((idx = decode_fncname(buf,0)) < 0)
        return(meFALSE);
    
    /* and then execute the command */
    return execFunc(idx,f,n) ;
}


int
lineExec (int f, int n, meUByte *cmdstr)
{
    register int status;                /* status return */
    meUByte *oldestr;                     /* original exec string */
    meUByte  oldcle ;                     /* old contents of clexec flag */
    meUByte  tkn[meTOKENBUF_SIZE_MAX] ;
    int oldexec, oldF, oldN, oldForce, prevForce ;
    
    /* save the arguments */
    oldcle = clexec;
    oldexec = execlevel ;
    oldestr = execstr;
    oldF = meRegCurr->f ;
    oldN = meRegCurr->n ;
    oldForce = meRegCurr->force ;
    /* store the execute-line's !force count on the prev registry if greater,
     * the docmd resets the force count so a !force !force execute-line "..."
     * effectively loses the !force's during the execution which can allow the
     * debugger to kick in unexpectedly */
    if((prevForce=meRegCurr->prev->force) < oldForce)
        meRegCurr->prev->force = oldForce ;
    
    clexec = meTRUE;                      /* in cline execution */
    execstr = NULL ;
    execlevel = 0;                      /* Reset execution level */
    meRegCurr->f = f ;
    meRegCurr->n = n ;
    
    status = docmd(cmdstr,tkn) ;
    
    /* restore old arguments */
    execlevel = oldexec ;
    clexec = oldcle;
    execstr = oldestr ;
    meRegCurr->f = oldF ;
    meRegCurr->n = oldN ;
    meRegCurr->force = oldForce ;
    meRegCurr->prev->force = prevForce ;
    
    return status ;
}    


/* executeLine: Execute a command line command to be typed in by the user */
int
executeLine(int f, int n)
{
    register int status;                /* status return */
    meUByte cmdstr[meBUF_SIZE_MAX];               /* string holding command to execute */
    
    /* get the line wanted */
    if ((status = meGetString((meUByte *)":", 0, 0, cmdstr, meBUF_SIZE_MAX)) <= 0)
        return status ;
    
    return lineExec(f,n,cmdstr) ;
}

