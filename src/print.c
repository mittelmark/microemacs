/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * print.c - Buffer printing routines.
 *
 * Copyright (C) 1996-2001 Jon Green & Steven Phillips    
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
 * Created:     1996
 * Synopsis:    Buffer printing routines.
 * Authors:     Steven Phillips & Jon Green
 * Description:
 *     This file contains routines to format and print buffers
 */

#include "emain.h"

#if MEOPT_PRINT

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <sys/types.h>
#include <time.h>
#endif
#include "eprint.h"

#define TESTPRINT  0                    /* Enable printer testing */

meUByte *printNames [] =
{
#define DEFPRINT(s,t,v) (meUByte *)s,
#include "eprint.def"
    NULL
#undef DEFPRINT
};

meUByte printTypes[] =
{
#define DEFPRINT(s,t,v) t,
#include "eprint.def"
    0
#undef DEFPRINT
};

PRINTER printer;
static meBuffer *gbp;                     /* Source of print buffer */
static meBuffer *gpbp;                    /* Composition print buffer */

#if MEOPT_HILIGHT
#define mePRINT_FONT      0x00ff
#define mePRINT_COLOR     0xff00

#define mePRINTOPTION_FONT    0x001
#define mePRINTOPTION_FCOLOR  0x002
#define mePRINTOPTION_BCOLOR  0x004

#define mePRINTSTYLE_FCOLOR (meFONT_COUNT)
#define mePRINTSTYLE_BCOLOR (meFONT_COUNT+1)
#define mePRINTSTYLE_SIZE   (meFONT_COUNT+2)

static int     fontCount ;
static meStyle fontMasks[2][mePRINTSTYLE_SIZE] ;
static meShort   fontStrlen[2][mePRINTSTYLE_SIZE] ;
static meUByte  *fontStrings[2][mePRINTSTYLE_SIZE] ;
static meUByte  *fontPaths[2] = {(meUByte *)"start",(meUByte *)"end"} ;
static meUByte  *fontOrder = (meUByte *)"biu" ;

/* The hilighting characters */
static meUByte hilchars[mePRINTSTYLE_SIZE] = {'b', 'i', 'l', 'r', 'u', 'F', 'B'};
meScheme printTextScheme=meSCHEME_INVALID ;
meScheme printCurrScheme=meSCHEME_INVALID ;

#endif

#ifdef _ME_FREE_ALL_MEMORY
void
printFreeMemory(void)
{
    meNullFree(printer.filter) ;
    meNullFree(printer.color) ;
    meNullFree(printer.scheme) ;
}
#endif

/*
 * getTranslationLen
 * Compute the length of the string with characters translated
 */
static int
getTranslationLen (meUByte *str, int n)
{
    int len = 0;                        /* Length of the string */
    int cc;                             /* Character pointer */
    meUByte *p;

    while (((cc = *str++) != '\0') && (--n >= 0))
    {
        if ((p = printer.filter[cc]) == NULL)
            len++;
        else
            len += meStrlen (p);
    }
    return (len);
}

/*
 * doTranslation
 * Translate the characters in the string ready for the printer.
 */
static int
doTranslation (meUByte *dest, meUByte *str, int n)
{
    int len = 0;                        /* Length of the string */
    int cc;                             /* Character pointer */
    meUByte *p;                   /* Character pointer */

    while (((cc = *str++) != '\0') && (--n >= 0))
    {
        /* Convert special characters i.e. box chars to an ASCII equivelent */
        if ((meSystemCfg & meSYSTEM_FONTFIX) && (cc < TTSPECCHARS))
            cc = ttSpeChars [cc];
        
        /* Process the character through the filter */
        if ((p = (meUByte *) printer.filter[cc]) == NULL)
            dest [len++] = cc;
        else
        {

            while ((dest[len] = *p++) != '\0')
                len++;
        }
    }
    return len;
}

int
printColor (int f, int n)
{
    meUByte buf[20];              /* Local character buffer */
    int colNo;                  /* color number to add */
    meInt color;                /* the color */
    
    if(n == 0)
    {
        /* reset the color table */
        meNullFree(printer.color) ;
        printer.color = NULL ;
        printer.colorNum = 0;
        return meTRUE ;
    }
        
    /* Get the color number and color */
    color = 0 ;
    if ((meGetString((meUByte *)"Number", 0, 0, buf, 20) == meABORT) ||
        ((colNo = meAtoi(buf)) < 0) || (colNo > 255) ||
        (meGetString((meUByte *)"Red",0,0,buf,20) == meABORT) ||
        ((color = mePrintColorSetRed(color,meAtoi(buf))),
         (meGetString((meUByte *)"Green",0,0,buf,20) == meABORT)) ||
        ((color = mePrintColorSetGreen(color,meAtoi(buf))),
         (meGetString((meUByte *)"Blue",0,0,buf,20) == meABORT)))
        return meFALSE;
    color = mePrintColorSetBlue(color,meAtoi(buf)) ;

    /* Add a new entry to the printer color table */
    if (printer.colorNum <= colNo)
    {
        printer.color = meRealloc (printer.color, sizeof(meUInt) * (colNo+1));
        if (printer.color == NULL)
        {
            /* Make safe */
            printer.colorNum = 0;
            return meABORT ;
        }
        /* Fill the memory with zero's */
        memset (&printer.color[printer.colorNum], 0,
                ((colNo+1) - printer.colorNum) * sizeof(meUInt));
        printer.colorNum = colNo+1;
    }
    printer.color[colNo] = color ;
    return meTRUE ;
}
int
printScheme (int f, int n)
{
    meUByte buf[20];              /* Local character buffer */
    int schmNo;                 /* color number to add */
    meUByte fc, bc, ff ;          /* temporary fore, back & font */
    meStyle scheme;             /* the scheme */
    
    if(n == 0)
    {
        /* reset the color table */
        meNullFree(printer.scheme) ;
        printer.scheme = NULL ;
        printer.schemeNum = 0;
        return meTRUE ;
    }
        
    /* Get the scheme number and scheme */
    scheme = 0 ;
    if ((meGetString((meUByte *)"Number", 0, 0, buf, 10) == meABORT) ||
        ((schmNo = meAtoi(buf)) < 0) || (schmNo > 255) ||
        (meGetString((meUByte *)"Fore-col",0,0,buf,meBUF_SIZE_MAX) == meABORT) ||
        ((fc = (meUByte) meAtoi(buf)),
         (meGetString((meUByte *)"Back-col",0,0,buf,meBUF_SIZE_MAX) == meABORT)) ||
        ((bc = (meUByte) meAtoi(buf)),
         (meGetString((meUByte *)"Font",0,0,buf,meBUF_SIZE_MAX) == meABORT)))
        return meFALSE;
    ff = (meUByte) meAtoi(buf) ;
    
    /* create the scheme */
    meStyleSet(scheme,fc,bc,ff) ;

    /* Add a new entry to the printer color table */
    if (printer.schemeNum <= schmNo)
    {
        printer.scheme = meRealloc (printer.scheme, sizeof(meStyle) * (schmNo+1));
        if (printer.scheme == NULL)
        {
            /* Make safe */
            printer.schemeNum = 0;
            return meABORT ;
        }
        /* Fill the memory with zero's */
        memset (&printer.scheme[printer.schemeNum], 0,
                ((schmNo+1) - printer.schemeNum) * sizeof(meStyle));
        printer.schemeNum = schmNo+1;
    }
    printer.scheme[schmNo] = scheme ;
    return meTRUE ;
}

static int
printGetParams (void)
{
    meRegNode *regPrint;              /* /print registry pointer */
    meRegNode *regData;               /* Temporary data */
    meUByte *order, *ss ;
    int ii, jj, kk, option;
#if TESTPRINT 
    FILE *fp;
#endif
    /* silent printing may be required, don't know yet - so don't mlwrite yet */
    /* mlwrite(0,(meUByte *)"[Configuring printer ...]");*/
	
    if(execFile((meUByte *)"print",0,1) <= 0)
        return meABORT ;
    
    /* Get the registry directory & the name of the driver out of the registry */
    if(((regPrint = regFind (NULL,(meUByte *)"/print")) == NULL) ||
       ((regData = regFind (regPrint,(meUByte *)"setup")) == NULL) ||
       ((ss = regGetString(regData,NULL)) == NULL) || !meAtoi(ss))
        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Printer driver not setup]");

#ifdef WIN32
    if(((regData = regFind (regPrint,(meUByte *)"internal")) != NULL) &&
       ((ss = regGetString(regData,NULL)) != NULL))
        printer.pInternal = meAtoi(ss) ;
    else
        printer.pInternal = 0 ;
#endif
        
    /* Get the output destination */
    if (((regData = regFind (regPrint,(meUByte *)"dest")) == NULL) ||
        ((ss = regGetString (regData,NULL)) == NULL) ||
        ((printer.pDestination=meAtoi(ss)) < 0) ||
        (printer.pDestination > PDEST_COMLINE))
        printer.pDestination = PDEST_BUFFER ;
#ifdef _WIN32
    if ((printer.pDestination == PDEST_INTERNAL) && !printer.pInternal)
        printer.pDestination = PDEST_BUFFER ;
    else if ((printer.pDestination == PDEST_COMLINE) && printer.pInternal)
        printer.pDestination = PDEST_FILE ;
#else 
    /* Only windows support an internal device */
    if (printer.pDestination == PDEST_INTERNAL)
        printer.pDestination = PDEST_BUFFER ;
#endif

#if TESTPRINT
    /*
     * Get all of the printer codes out of the registry.
     */
    fp = fopen ("print.cfg", "wb");
#endif
    
    for (ii = 0; ii < PRINT_MAX; ii++)
    {
        /* NOTE:- If any of these change print.emf must be up-dated also */
        /* Try /print/<id> */
        if ((regData = regFind (regPrint,printNames[ii])) == NULL)
        {
            if (printTypes[ii] & mePD_INT)
                printer.param[ii].l = 0;
            else
                printer.param[ii].p = NULL;
        }
        else if (printTypes[ii] & mePD_INT)
            printer.param[ii].l = regGetLong(regData, 0) ;
        else
            printer.param[ii].p = regGetString(regData, NULL);
#if TESTPRINT
        if (fp)
        {
            if (printTypes[ii] & mePD_INT)
                fprintf (fp, "Parameter %s = %ld\n", printNames [ii],
                         printer.param[ii].l);
            else
                fprintf (fp, "Parameter %s = %s\n", printNames [ii],
                         (printer.param[ii].p == NULL) ? "(nil)" : printer.param[ii].p);
        }
#endif
    }

    /*
     * Get the filter codes out of the registry
     */

    /* Allocate the filter table */
    if (printer.filter == NULL)
        printer.filter = meMalloc (sizeof (meUByte *) * 256);
    memset(printer.filter,0, sizeof(meUByte *) * 256) ;

    if ((regData = regFind (regPrint,(meUByte *)"filter")) != NULL)
    {
        meUByte *nodeName;
        regData = regGetChild(regData) ;

        while (regData != NULL)
        {
            /* Get the node name out */
            nodeName = regGetName(regData) ;
            if (nodeName[0] != '\0')
            {
                if (nodeName[1] == '\0')
                    ii = nodeName[0];
                else
                    ii = meAtoi (nodeName);
                if ((ii >= 0) && (ii <= 255))
                    printer.filter [ii] = regGetString (regData, NULL);
            }
            regData = regGetNext (regData) ;
        }
    }
    /* get the font and color settings */
    option = 0 ;
    if(((regData = regFind (regPrint,(meUByte *)"scheme-flags")) != NULL) &&
       ((ss = regGetString (regData, NULL)) != NULL))
    {
        ii = meAtoi(ss) ;
        if(ii & mePRINTOPTION_FONT)
            option |= meFONT_MASK ;
        if(ii & mePRINTOPTION_FCOLOR)
            option |= 1 << mePRINTSTYLE_FCOLOR ;
        if(ii & mePRINTOPTION_BCOLOR)
            option |= 1 << mePRINTSTYLE_BCOLOR ;
    }
    if(!(option & (1<<mePRINTSTYLE_BCOLOR)))
        printer.param[mePS_BGCOL].p = NULL ;
        
    regData = regFind(regPrint,(meUByte *)"scheme-order") ;
    order = (regData == NULL) ? fontOrder : regGetString (regData, NULL) ;
        
    for(ii=0 ; ii<2 ; ii++)
    {
        /* Search in /print/<section>/order */
        for(jj=0 ; (jj<mePRINTSTYLE_SIZE) && (order[jj] != '\0') ; jj++)
        {
            fontStrings[ii][jj] = NULL ;
            fontStrlen[ii][jj] = 0 ;
            fontMasks[ii][jj] = 0 ;
            for(kk=0 ; kk < mePRINTSTYLE_SIZE ; kk++)
            {
                if(hilchars[kk] == order[jj])
                {
                    if(option & (1 << kk))
                    {
                        meUByte subkey[2] ;
                        subkey[0] = order[jj] ;
                        subkey[1] = '\0' ;
                        if ((regData = vregFind (regPrint,(meUByte *)"%s/%s", fontPaths[ii],subkey)) != NULL)
                        {
                            fontStrings[ii][jj] = regGetString (regData, NULL) ;
                            if(kk == mePRINTSTYLE_FCOLOR)
                            {
                                fontStrlen[ii][jj] = -1 ;
                                fontMasks[ii][jj] = meSTYLE_FCOLOR ;
                            }
                            else if(kk == mePRINTSTYLE_BCOLOR)
                            {
                                fontStrlen[ii][jj] = -1 ;
                                fontMasks[ii][jj] = meSTYLE_BCOLOR ;
                            }
                            else
                            {
                                fontStrlen[ii][jj] = (meUShort) meStrlen(fontStrings[ii][jj]) ;
                                fontMasks[ii][jj] = 1 << (kk+16) ;
                            }
#if TESTPRINT
                            fprintf (fp, "Scheme %s - %d set to %c %d [%s]\n",fontPaths[ii],jj,order[jj],fontStrlen[ii][jj],fontStrings[ii][jj]);
#endif
                        }
                    }
                    break ;
                }
            }
        }
    }
    fontCount = jj ;
#if TESTPRINT
    if (fp)
        fclose (fp);
#endif
    if((printer.param[mePI_FLAGS].l & PFLAG_SILENT) == 0)
        mlwrite(0,(meUByte *)"[Printer read the registry ...]");
    
    /* Add a few defaults */
    if (printer.param[mePI_COLS].l < 1)
        printer.param[mePI_COLS].l = 1;
    if (printer.param[mePI_ROWS].l < 1)
        printer.param[mePI_ROWS].l = 1;
    if (printer.param[mePS_SCONT].p == NULL)
        printer.param[mePS_SCONT].p = (meUByte *)" ";
    if (printer.param[mePS_ECONT].p == NULL)
        printer.param[mePS_ECONT].p = (meUByte *)" ";
    /* sort out the header and footer */
    if((printer.param [mePI_FLAGS].l & PFLAG_ENBLHEADER) &&
       ((ss=printer.param [mePS_HEADER].p) != NULL))
    {
        printer.pNoHeaderLines = 1 ;
        while(*ss != '\0')
            if(*ss++ == '\n')
                printer.pNoHeaderLines++ ;
    }
    else
        printer.pNoHeaderLines = 0 ;
    if((printer.param [mePI_FLAGS].l & PFLAG_ENBLFOOTER) &&
       ((ss=printer.param [mePS_HEADER].p) != NULL))
    {
        printer.pNoFooterLines = 1 ;
        while(*ss != '\0')
            if(*ss++ == '\n')
                printer.pNoFooterLines++ ;
    }
    else
        printer.pNoFooterLines = 0 ;

    /* Default the output states */
    if (printer.param[mePS_BUFFER].p == NULL)
        printer.param[mePS_BUFFER].p = (meUByte *)"*printer*";
    return meTRUE;
}

/*
 * printComputePageSetup
 * Compute the page setup for the printer
 */
static int
printComputePageSetup (int f)
{
    int xtraWidth;
    int xtraDepth;
    /* Sort out the parameters */
    if (printer.param[mePI_COLS].l < 1)
        printer.param[mePI_COLS].l = 1;
    if (printer.param[mePI_ROWS].l < 1)
        printer.param[mePI_ROWS].l = 1;
    /* allow for left/right margin, column dividers and line numbers */
    xtraWidth = printer.param[mePI_MLEFT].l + printer.param[mePI_MRIGHT].l ;
    xtraWidth += (printer.param[mePI_COLS].l-1) * printer.param[mePI_WSEP].l;
    if (printer.pLineNumDigits > 0)
        xtraWidth += printer.param[mePI_COLS].l * printer.pLineNumDigits ;
    
    /* allow for the row dividers, top/bottom margins and header/footer depths */
    xtraDepth = (printer.param[mePI_ROWS].l-1) ;
    xtraDepth += printer.param[mePI_MTOP].l + printer.param[mePI_MBOTTOM].l ;
    xtraDepth += printer.pNoHeaderLines + printer.pNoFooterLines ;

    /* Automatically fill some of the entries. */
    if ((printer.param[mePI_PAGEX].l == 0) && (printer.param[mePI_PAGEY].l == 0))
    {
        /* The page size is undefined, generate the page size from the
         * paper size */
        printer.param[mePI_PAGEX].l = (printer.param[mePI_PAPERX].l - xtraWidth) /
                  printer.param[mePI_COLS].l;
        printer.param[mePI_PAGEY].l = (printer.param[mePI_PAPERY].l - xtraDepth) /
                  printer.param[mePI_ROWS].l;
    }
    else if ((printer.param[mePI_PAPERX].l == 0) && (printer.param[mePI_PAPERY].l == 0))
    {
        /* The paper size is undefined, determine the paper size */
        printer.param[mePI_PAPERX].l = printer.param[mePI_PAGEX].l * printer.param[mePI_COLS].l + xtraWidth;
        printer.param[mePI_PAPERY].l = printer.param[mePI_PAGEY].l * printer.param[mePI_ROWS].l + xtraDepth;
    }

    /* Check to ensure that the page request is legal. */
    if ((printer.param[mePI_PAPERX].l < (printer.param[mePI_PAGEX].l * printer.param[mePI_COLS].l + xtraWidth)) ||
        (printer.param[mePI_PAPERY].l < (printer.param[mePI_PAGEY].l * printer.param[mePI_ROWS].l + xtraDepth)))
        return mlwrite(MWABORT|MWPAUSE,(meUByte *)"Invalid paper size %d x %d. Page requirements %d x %d",
                       printer.param[mePI_PAPERX].l,
                       printer.param[mePI_PAPERY].l,
                       printer.param[mePI_PAGEX].l * printer.param[mePI_COLS].l + xtraWidth,
                       (printer.param[mePI_PAGEY].l * printer.param[mePI_ROWS].l + xtraDepth));
    else if (!f && ((printer.param[mePI_FLAGS].l & PFLAG_SILENT) == 0))
        mlwrite(0,(meUByte *)"Valid paper size %d x %d. Page requirements %d x %d of %d x %d",
                printer.param[mePI_PAPERX].l,
                printer.param[mePI_PAPERY].l,
                printer.param[mePI_PAGEX].l,
                printer.param[mePI_PAGEY].l,
                printer.param[mePI_COLS],
                printer.param[mePI_ROWS]);
    return meTRUE;
}

/*
 * Initialise the driver ready for a print.
 * Recompute the print metrics ready for the page.
 */
static int
printInit (int f, int n)
{
#ifdef _WIN32
    extern int printSetup(int n);
#endif
    /* Get the parameters out of the registry. */
    if (printGetParams () <= 0)
        return meFALSE;
    
    /* if the line numbers aren't required then set printer.pLineNumDigits to zero
     * so we only have to check that value */
    if (!(printer.param [mePI_FLAGS].l & PFLAG_ENBLLINENO))
        printer.pLineNumDigits = 0 ;

#ifdef _WIN32
    /* On windows call printSetup to handle the windows printer initialization
     * and the setup dialog if required.
     */
    if (printer.pInternal && (printSetup(n) <= 0))
        return meFALSE;
#endif
    
    if (printComputePageSetup (f) <= 0)
        return meFALSE;
    
    if(n < 0)
    {
        meRegNode *regPrint;  
        
        regPrint = regFind (NULL,(meUByte *)"/print") ;
        regSet(regPrint, printNames[mePI_PAPERX], meItoa(printer.param[mePI_PAPERX].l));
        regSet(regPrint, printNames[mePI_PAPERY], meItoa(printer.param[mePI_PAPERY].l));
        regSet(regPrint, printNames[mePI_PAGEX], meItoa(printer.param[mePI_PAGEX].l));
        regSet(regPrint, printNames[mePI_PAGEY], meItoa(printer.param[mePI_PAGEY].l));
    }
    return meTRUE;
}

/*
 * addComposedLine
 * Add a composed line to the lpage line list
 */
static void
addComposedLine (meLine **head, meLine **tail, meUByte *buf, int len)
{
    meLine *nline;                        /* New line */

    nline = meLineMalloc(len,0);               /* Get the new line */
    if (len > 0)                        /* Copy in the data */
        memcpy (nline->text, buf, len);
    meLineGetNext (nline) = NULL;
    if ((meLineGetPrev (nline) = *tail) == NULL)
        *tail = (*head = nline);
    else
    {
        meLineGetNext (*tail) = nline;
        *tail = nline;
    }
}

/*
 * expandText
 * Expand control codes in the escape sequence
 */
static int
expandText (meUByte *buf, meUByte *strp, int level)
{
    meUByte cc;
    meUByte *bbuf = buf;
    meUByte *ss;

    if (strp == NULL)
        return 0;

    while ((cc = *strp++) != '\0')
    {
        /* Check out the escape characters */
        if (cc == '%')
        {
            switch (cc = *strp)
            {
            case 'c':
                if(((cc = *++strp) == meCHAR_LEADER) && ((cc=*++strp) == meCHAR_TRAIL_NULL))
                    *buf++ = '\0';
                else
                    *buf++ = cc;
                break;
            case 'h':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_hour);
                break;
            case 'm':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_min);
                break;
            case 's':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_sec);
                break;
            case 'D':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_mday);
                break;
            case 'M':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_mon+1);
                break;
            case 'Y':
                buf += sprintf ((char *) buf, "%02d", printer.ptime->tm_year % 100);
                break;
            case 'y':
                buf += sprintf ((char *) buf, "%d", printer.ptime->tm_year + 1900);
                break;
            case 'f':
                if ((gpbp->name[0] == '*') || (gpbp->fileName == NULL))
                    ss = (meUByte *) gpbp->name;
                else
                    ss = (meUByte *) gpbp->fileName;
                goto cat_string;
            case 'b':
                ss = (meUByte *) gbp->name;
cat_string:
                if (ss != NULL)
                    buf += sprintf ((char *) buf, "%s", ss);
                break;
            case 'n':
                buf += sprintf ((char *) buf, "%d", printer.pPageNo);
                break;
            case '\0':
                break;
            default:
                *buf++ = cc;
                break;
            }
            if (*strp != '\0')
                strp++;
        }
        else if (cc == meCHAR_NL)
        {
            if (level == 0)
                buf += expandText (buf, printer.param [mePS_EOL].p, 1);
            else
                *buf++ = meCHAR_NL;
        }
        else if (cc == '\r')
        {
            if (level != 0)
                *buf++ = '\r';
        }
        else if (cc == '\t')
            *buf++ = ' ';
        else
        {
            if(cc == meCHAR_LEADER)
            {
                if((cc = *strp++) == '\0')
                    break ;
                if(cc == meCHAR_TRAIL_NULL) 
                    cc = '\0' ;
            }
            *buf++ = cc;
        }
    }
    /* Return the delta length of the string */
    return buf - bbuf;
}
/*
 * expandTitleText
 * Expand the title text into the appropriate form
 */
static void
addFormatedLine (meLine **head, meLine **tail,
                 meUByte *buf, int noLines, meUByte *strp, int filter)
{
    meUByte *bhead = buf;
    meUByte *p = strp;

    while (--noLines >= 0)
    {
        int ii;
        meUByte cc;
        meUByte tbuf [meBUF_SIZE_MAX];

        /* Get the new lines out */
        if (strp != NULL)
        {
            ii = 0 ;
            while(ii < printer.param [mePI_MLEFT].l)
                tbuf[ii++] = ' ' ;
            for (; (cc = *p) != '\0'; /* NULL */)
            {
                p++;
                if (cc == meCHAR_NL)
                    break;
                /* Filter the character if required. */
                if (filter == 0)
                    tbuf [ii++] = cc;
                else
                {
                    /* doTranslation reads one past its last character (its
                     * loop test decrements n after the read), so hand it a
                     * NUL-terminated 2-byte buffer rather than &cc, which is
                     * a lone byte and would trigger an out-of-bounds read. */
                    meUByte ccBuf [2] = { cc, 0 };
                    ii += doTranslation (&tbuf[ii], ccBuf, 1);
                }
            }
        }
        else
            ii = 0;
        tbuf [ii] = '\0';

        /* Add the start of line code */
        buf = bhead + expandText (bhead, printer.param [mePS_SOL].p, 1);
        /* Expand the header text and append to the composition buffer */
        buf = buf + expandText (buf, tbuf, 0);
        /* Add the end of line code */
        buf = buf + expandText (buf, printer.param [mePS_EOL].p, 1);
        addComposedLine (head, tail, bhead, buf - bhead);
    }
}

#if MEOPT_HILIGHT
static int
printSetScheme(meScheme col, meUByte *buff)
{
    meScheme ts ;
    meStyle ss, cs ;
    int sore, ii, fstDiff, len=0 ;
    
    /* If the scheme is not registered then there is no colour change
     * to perform */
    ts = col / meSCHEME_STYLES ;
    /* Get the scheme out of print table */
    ss = (ts >= printer.schemeNum) ? 0 : printer.scheme[ts];
    if(printCurrScheme != meSCHEME_INVALID)
    {
        ts = printCurrScheme / meSCHEME_STYLES ;
        cs = (ts >= printer.schemeNum) ? 0 : printer.scheme[ts];
        /* loop through finding the first componant thats different */
        for(fstDiff=0 ; fstDiff<fontCount ; fstDiff++)
        {
            if(((ss & fontMasks[0][fstDiff]) != (cs & fontMasks[0][fstDiff])) ||
               ((ss & fontMasks[1][fstDiff]) != (cs & fontMasks[1][fstDiff])))
                break ;
        }
        sore = 1 ;
    }
    else
    {
        sore = 0 ;
        fstDiff = 0 ;
    }
    printCurrScheme = col ;
    
    if(fstDiff == fontCount)
        /* same characteristics, nothing to change */
        return 0 ;
    
    for(;;sore--)
    {
        ii = (sore) ? fontCount-1:fstDiff ;
        for(;;)
        {
            /* Always want to set the fore and back-ground colors (col 0 may not be black!) */
            if(fontStrlen[sore][ii] < 0)
            {
                meInt cno, cc, rr, gg, bb ;
                char *str ;
                if(fontMasks[sore][ii] & meSTYLE_FCOLOR)
                    cno = meStyleGetFColor(ss) ;
                else
                    cno = meStyleGetBColor(ss) ;
                cc = printer.color[cno] ;
                rr = mePrintColorGetRed(cc) ;
                gg = mePrintColorGetGreen(cc) ;
                bb = mePrintColorGetBlue(cc) ;
                str = meTParm((char *)fontStrings[sore][ii],cno,rr,gg,bb) ;
                if(buff != NULL)
                    meStrcpy(buff+len,str) ;
                len += strlen(str) ;
            }
            else if(((sore == 1) && (cs & fontMasks[sore][ii])) ||
                    ((sore == 0) && (ss & fontMasks[sore][ii])) )
            {
                if(buff != NULL)
                    meStrcpy(buff+len,fontStrings[sore][ii]) ;
                len += fontStrlen[sore][ii] ;
            }
            if(sore)
            {
                if(ii == fstDiff)
                    break ;
                ii-- ;
            }
            else if(++ii == fontCount)
                return len ;
        }
    }
}
#endif

/*
 * Render the page into a line store list
 */
static meLine *
composePage (int f)
{
    meLine *head = NULL;
    meLine *tail = NULL;
    meUByte buf [1024*5];                 /* Heafty line store !! */
    meUByte *p;                           /* Pointer to the buffer */
    int xx;                             /* Page column iterator */
    int yy;                             /* Page row iterator */
    int ll;                             /* Lines on page iterator */
    int ii;
    meLine *lp;                           /* Pointer to the line */

    /* Handle the end of file case */
    if (f)
    {
        if (f < 0)
        {
            /* End of file. */
            if (printer.param [mePS_EOF].p != NULL)
            {
                p = buf + expandText (buf, printer.param [mePS_EOF].p, 1);
                addComposedLine (&head, &tail, buf, p - buf);
            }
            return head;
        }
        else
        {
            /* Start of file */
            p = buf ;
            if (printer.param [mePS_SOF].p != NULL)
            {
                p += expandText (p, printer.param [mePS_SOF].p, 1);
            }
#if MEOPT_HILIGHT
            /* set the default bg-color */
            if(printer.param[mePS_BGCOL].p != NULL)
            {
                meInt cno, cc, rr, gg, bb ;
                char *str ;
                
                cno = printTextScheme / meSCHEME_STYLES ;
                cno = (cno >= printer.schemeNum) ? 0 : printer.scheme[cno];
                cno = meStyleGetBColor(cno) ;
                cc = printer.color[cno] ;
                rr = mePrintColorGetRed(cc) ;
                gg = mePrintColorGetGreen(cc) ;
                bb = mePrintColorGetBlue(cc) ;
                str = meTParm((char *)printer.param[mePS_BGCOL].p,cno,rr,gg,bb) ;
                meStrcpy(p,str) ;
                p += strlen(str) ;
            }
            /* set the scheme to the default text */
            p += printSetScheme(printTextScheme,p) ;
#endif
            if(p != buf)
                addComposedLine (&head, &tail, buf, p - buf);
            return head;
        }
    }

    /* Add the start of page marker */
    if (printer.param [mePS_SOP].p != NULL)
    {
        p = buf + expandText (buf, printer.param [mePS_SOP].p, 1);
        addComposedLine (&head, &tail, buf, p - buf);
    }

    /* Render the top margin and the header */
    if (printer.param [mePI_MTOP].l)
        addFormatedLine (&head, &tail, buf, printer.param [mePI_MTOP].l, NULL, 0);
    if (printer.pNoHeaderLines)
        addFormatedLine (&head, &tail, buf, printer.pNoHeaderLines, printer.param [mePS_HEADER].p, 1);

    /* Iterate over the number of page rows */
    for (yy = 0; yy < printer.param [mePI_ROWS].l; yy++)
    {
        /* Iterate over the lines on the page */
        for (ll = 0; ll < printer.param [mePI_PAGEY].l; ll++)
        {
            /* Add the start of line code */
            p = buf + expandText (buf, printer.param [mePS_SOL].p, 1);

            /* Add the leading margin */
            for (ii = printer.param [mePI_MLEFT].l; --ii >= 0; *p++ = ' ')
                /* NULL */;

            /* Iterate over the number of page columns */
            for (xx = 0;;)
            {
                /* Go to the start of the line in the buffer */
                ii = ((yy * printer.param [mePI_COLS].l + xx) * printer.param [mePI_PAGEY].l) + ll;
                lp = meLineGetNext (gbp->baseLine);
                while (--ii >= 0)
                {
                    if ((lp = meLineGetNext (lp)) == gbp->baseLine)
                    {
                        lp = NULL;
                        break;
                    }
                }

                /* Process the line */
                if (lp != NULL)
                {
                    /* Add the continuation marker - we have already done the line numbers */
                    if (((lp->flag & meLINE_CHANGED) == 0) && (printer.pLineNumDigits > 0))
                    {
                        for (ii = printer.pLineNumDigits-2; --ii >= 0; *p++ = ' ')
                            /* NULL */;
                        p += expandText (p, printer.param [mePS_SCONT].p, 1);
                        *p++ = ' ' ;
                    }

                    /* Concatinate the composed line information */
                    {
                        meUByte cc;
                        meUByte *str;

                        str = lp->text;
                        while ((cc = *str++) != '\0')
                        {
                            if(cc == meCHAR_LEADER)
                            {
                                if ((cc=*str++) == meCHAR_TRAIL_NULL)
                                    cc = 0;
                                else if (cc != meCHAR_TRAIL_LEADER)
                                    cc=*str++;
                            }
                            *p++ = cc;
                        }
                    }
                }
                else
                {
                    /* NULL line. Pad the line with spaces */
                    ii = printer.param [mePI_PAGEX].l;
                    if (printer.pLineNumDigits > 0)
                        ii += printer.pLineNumDigits ;
                    while (--ii >= 0)
                        *p++ = ' ';
                }

                /* End of the line reached ?? */
                if (++xx >= printer.param [mePI_COLS].l)
                    break;

                /* Add a new column */
                p += expandText (p, printer.param [mePS_VSEP].p, 1);
            }

            /* Add the end of line marker and push the buffer */
            if (printer.param [mePI_STRIP].l)
            {
                while ((p - buf) > 0)
                {
                    char cc;
                    if (((cc = *(p-1)) == ' ') || (cc == '\t'))
                        p--;
                    else
                        break;
                }
            }
            p += expandText (p, printer.param [mePS_EOL].p, 1);
            addComposedLine (&head, &tail, buf, p - buf);
        }

        /* Add the horizontal separator if required */
        if (yy+1 < printer.param [mePI_ROWS].l)
        {
            /* Add the start of line code */
            p = buf + expandText (buf, printer.param [mePS_SOL].p, 1);

            /* Add the leading margin */
            for (ii = printer.param [mePI_MLEFT].l; --ii >= 0; *p++ = ' ')
                /* NULL */;

            for (xx = printer.param [mePI_COLS].l; --xx >= 0; /* NULL */)
            {
                int nn;

                /* Work out the width - length of text plus line # width */
                nn = printer.param [mePI_PAGEX].l + printer.pLineNumDigits;
                while (--nn >= 0)
                    p += expandText (p, printer.param [mePS_HSEP].p, 1);

                /* If there is another column then add the vertical crossover
                 * separator. */
                if (xx > 0)
                    p += expandText (p, printer.param [mePS_XSEP].p, 1);
            }

            /* Add the end of line marker and push the buffer */
            p += expandText (p, printer.param [mePS_EOL].p, 1);
            addComposedLine (&head, &tail, buf, p - buf);
        }
    }

    /* Render the footer and the separator between the footer and the textual
     * page */
    if (printer.pNoFooterLines)
        addFormatedLine (&head, &tail, buf, printer.pNoFooterLines, printer.param [mePS_FOOTER].p, 1);
    
    /* Add the end of line marker and push the buffer */
    if (printer.param [mePS_EOP].p != NULL)
    {
        p = buf + expandText (buf, printer.param [mePS_EOP].p, 1);
        addComposedLine (&head, &tail, buf, p - buf);
    }
    return head;
}

/*
 * dumpToInternal
 * Write the line into the iternal line list
 */
static void
dumpToInternal (meLine **headp, meLine **tailp, meLine *lp)
{
    /* Check for NULL case - this should not happen */
    if (lp == NULL)
        return;

    /* Link to the head */
    if (*tailp == NULL)
        *headp = lp;
    else
    {
        meLineGetNext(*tailp) = lp;             /* Attach head to end of list */
        meLineGetPrev(lp) = *tailp;
    }

    /* Fix up the tail */
    while (meLineGetNext(lp) != NULL)
        lp = meLineGetNext(lp);
    *tailp = lp;
}

/*
 * dumpToFile
 * Write the composed lines into the file
 */
static void
dumpToFile (FILE *fp, meLine *lp)
{
    /* Dump the line to file */
    while (lp != NULL)
    {
        meLine *tlp;                      /* Temp line store */

        tlp = lp;
        lp = meLineGetNext (lp);

        fwrite (tlp->text, 1, (int)(tlp->length), fp);
#if TESTPRINT
        {
            FILE *fp2;
            if ((fp2 = fopen ("spool2.out","ab")) != NULL)
            {
                fwrite (tlp->text, 1, (int)(tlp->length), fp2);
                fclose (fp2);
            }
        }
#endif
        meFree (tlp);
    }
}

/*
 * dumpToBuffer
 * Write the composed lines into a buffer. We have to un-translate the
 * lines here. This is not particularly efficient since we have already
 * translated (nil) characters to '\0's, now we have to expand them again !!
 * However this is not really a problem, just a bit of a pain !!.
 */
static void
dumpToBuffer (meBuffer *bp, meLine *lp)
{
    int ii;
    char *p;

    while (lp != NULL)
    {
        meLine *tlp;                      /* Temp line store */
        char cc;
        int len;
        int jj, kk;

        /* Advance the line pointer */
        tlp = lp;
        lp = meLineGetNext (lp);

        /* Determine how many '\0's are in the string */
        jj = 0;
        while (jj != tlp->length)
        {
            meLine *nlp;
            char *q;
            
            for (kk = jj, ii = tlp->length - jj, len = 0, p = (char *) &tlp->text [jj]; --ii >= 0; /* NULL */)
            {
                jj++;
                if ((cc = *p++) == '\0')
                    len += 2;
                else if (cc == meCHAR_NL)
                    break;
                else 
                    len++;
            }
            
            /* Construct a new line for the buffer and translate out any 0's */
            nlp = meLineMalloc(len,0);
            for (p = (char *) &tlp->text[kk], q = (char *) nlp->text; --len >= 0; /* NULL */)
            {
                if ((cc = *p++) == '\0')
                {
                    *q++ = (char) meCHAR_LEADER ;
                    *q++ = (char) meCHAR_TRAIL_NULL ;
                }
                else if (cc == meCHAR_NL)
                    break;
                else
                    *q++ = cc;
            }
                
            /* Link the new line to the end of the buffer. */
            meLineGetNext(meLineGetPrev(bp->baseLine)) = nlp;
            meLineGetPrev(nlp) = meLineGetPrev(bp->baseLine);
            meLineGetPrev(bp->baseLine) = nlp;
            meLineGetNext(nlp) = bp->baseLine;
            bp->lineCount += 1;
        }
        /* Free off the old line and assign the converted line. */
        meFree (tlp);
    }
}

static void
printLinkLine (meBuffer *bp, meLine *nlp, meUShort lno)
{
    bp->baseLine->prev->next = nlp;
    nlp->prev = bp->baseLine->prev;
    bp->baseLine->prev = nlp;
    nlp->next = bp->baseLine;
    if (lno)
        nlp->flag &= ~meLINE_CHANGED;
    else
        printer.pLineNo++;              /* Next line */
}

#define PRTSIZ 2048
#if MEOPT_HILIGHT
static int
printAddLine (meBuffer *bp, meLine *lp, meVideoLine *vps)
#else
static int
printAddLine (meBuffer *bp, meLine *lp)
#endif
{
#if MEOPT_HILIGHT
    meScheme  scheme ;
#endif
    meUShort    noColChng ;
    meSchemeSet *blkp;
    meLine     *nlp;
    meUShort    len, wid, ii, jj, kk, ll;

    /* Render the line to get the colour information */
#if MEOPT_HILIGHT
    if (vps[0].hilno)
    {
        noColChng = hilightLine(vps,0);
        blkp = hilBlock + 1;
        wid = blkp[noColChng-1].column;
    }
    else
#endif
    {
        wid = renderLine (lp->text,lp->length,0,bp);
        noColChng = 0;
    }

    /* Commence rendering the line */
    len = 0;
    ii  = 0;
    for(;;)
    {
        /* Get the length of the text */
        ll = (meUShort) printer.param[mePI_PAGEX].l;
        kk = wid-len ;
        if ((printer.param [mePI_FLAGS].l & PFLAG_ENBLTRUNCC))
        {
            if ((kk = wid-len) >= ll)
                kk = ll - 1 ;    /* Normalise to print length */
        }
        else if (kk > ll)
            kk = ll ;   /* Normalise to print length */

        /* Determine the length of all of the colour change strings. For
         * all of the colour changes to the end of the line, add the length
         * of storage of each to our line total (ll) */
        for (jj=0; jj<noColChng; jj++)
        {
#if MEOPT_HILIGHT
            scheme = blkp[jj].scheme ;
            ll += printSetScheme(scheme,NULL) ;
#endif
            if (blkp[jj].column > len+kk)
                break;
        }
#if MEOPT_HILIGHT
        /* add the reseting of the current scheme */
        ll += printSetScheme(printTextScheme,NULL) ;
#endif
        /* Insert line numbers if required. We insert line numbers at the
         * initial expansion so that we do not need to remember where we
         * are later. However we do not add the start of line continuation
         * markers until later (i.e. a line that continues on the next line)
         * This basically saves some space and is easy to deal with later */
        if ((ii == 0) && (printer.pLineNumDigits > 0))
            ll += (meUShort) printer.pLineNumDigits ;
        ll += getTranslationLen (disLineBuff+len,kk) - kk;

        /* Construct a new composition line */
        if ((nlp=meLineMalloc(ll,0)) == NULL)
            return (meFALSE);
        printLinkLine (bp,nlp,ii++);

        /* Write the line numbers into the buffer if required */
        if ((nlp->flag & meLINE_CHANGED) && (printer.pLineNumDigits > 0))
        {
            char lnobuf [40];
            ll = (meUShort) printer.pLineNumDigits ;
            sprintf (lnobuf, "% 20d ", printer.pLineNo);
            memcpy (nlp->text, &lnobuf [21-printer.pLineNumDigits], ll);
        }
        else
            ll = 0;

        /* Copy the line into the buffer */
        if (noColChng == 0)
        {
            /* No colour changes - simply copy the line */
            ll += doTranslation (nlp->text+ll, disLineBuff+len, kk);
        }
        else
        {
            /* Propogate the colour change strings into the print buffer
             * whenever the colour changes. */
            for (jj = 0; jj < kk; /* NULL*/)
            {
#if MEOPT_HILIGHT
                /* Get the colour change string */
                ll += printSetScheme(blkp->scheme,nlp->text+ll) ;
#endif
                if (blkp->column > len+kk)
                {
                    /* Off the end of the line !! */
                    ll += doTranslation (nlp->text+ll,
                                         disLineBuff+len+jj,
                                         kk-jj);
                    jj = kk;
                }
                else
                {
                    /* All fits !! */
                    ll += doTranslation (nlp->text+ll,
                                         disLineBuff+len+jj,
                                         blkp->column-len-jj);
                    jj = blkp->column-len;
                    blkp++;
                }
            }
#if MEOPT_HILIGHT
            /* add the reseting of the current scheme */
            ll += printSetScheme(printTextScheme,nlp->text+ll) ;
#endif
        }
        len += kk ;
        /* Handle the end of the line */
        if (len >= wid)
        {
            /* Pad the end of the line with spaces. */
            len = ((meUShort) printer.param[mePI_PAGEX].l) - kk ;
            memset(nlp->text+ll,' ',len);
            nlp->text[ll+len] = '\0';
            break ;
        }
        if ((printer.param [mePI_FLAGS].l & PFLAG_ENBLTRUNCC))
            /* Line continues - add the line continuation character */
            ll += expandText (nlp->text+ll, printer.param [mePS_ECONT].p, 1);
        nlp->text[ll] = '\0';
    }
    printer.pNoLines += ii;
    return (meTRUE);
}

/*
 * printSection
 * Print a section of the buffer
 */

static int
printSection (meWindow *wp, long sLineNo, long numLines, meLine *sLine, meLine *eLine, int nn)
{
    time_t clock;                         /* Time in machine format. */
    meBuffer *bp;                         /* Composition buffer */
    meBuffer *dbp = NULL;                 /* Destination buffer */
    FILE *fp = NULL;                      /* Output file stream */
    meLine *lp, *nlp;                     /* Local line pointers */
    meLine *clp;                          /* Composed page list */
    meLine *ihead = NULL;                 /* Internal destination header */
    meLine *itail = NULL;                 /* Internal destination trailer */
    int status = meTRUE;                  /* Return status */
    meUByte fname [meBUF_SIZE_MAX];       /* File name buffer */
    int ii;                               /* Local loop counter. */

#if MEOPT_HILIGHT
    meVideoLine          vps[2];
#endif
    /* Work out the line number range. */
    for (numLines += sLineNo, printer.pLineNumDigits = 1; numLines > 0; printer.pLineNumDigits++)
        numLines /= 10;

    /* Initialise the printer. */
    if (printInit (meTRUE, nn) <= 0)
        return meABORT;
    if(nn < 0)
        return meTRUE ;
        
    /* Get the clock out and initialise */
    clock = time (0);	                 /* Get system time */
    printer.ptime = localtime (&clock);  /* Get time frame */
    printer.pLineNo = sLineNo;
    printer.pPageNo = 0;
    printer.pNoLines = 0;

    /* Determine the destination */
    switch (printer.pDestination)
    {
    case PDEST_INTERNAL:
        break;
    case PDEST_BUFFER:
#ifdef _INSENSE_CASE
        if(meStricmp(wp->buffer->name,printer.param[mePS_BUFFER].p) == 0)
#else
        if(meStrcmp(wp->buffer->name,printer.param[mePS_BUFFER].p) == 0)
#endif
            return mlwrite(MWABORT,(meUByte *)"[Cannot print \"%s\" buffer]",printer.param[mePS_BUFFER].p) ;
        if(((dbp=bfind(printer.param[mePS_BUFFER].p,BFND_CREAT|BFND_CLEAR)) == meFALSE) ||
           (((printer.param [mePI_FLAGS].l & PFLAG_SILENT) == 0) &&
            (meWindowPopup(dbp->name,WPOP_USESTR,NULL)) == NULL))
            return mlwrite(MWABORT,(meUByte *)"[Failed to create print buffer]") ;
        break;
    case PDEST_COMLINE:
    case PDEST_FILE:
        if ((printer.param [mePS_FILE].p == NULL) || (*printer.param [mePS_FILE].p == '\0'))
            mkTempName (fname, NULL, "prt");
        else
            meStrcpy (fname,printer.param [mePS_FILE].p);

        /* Endevour to open the file */
        if ((fp = fopen ((char *)fname, "wb")) == NULL)
            return mlwrite(MWABORT,(meUByte *)"Unable to open printer spool file %s", fname);
        break;
    }
    
    /* Shuffle the isDisplayable test so it actually tests isPrint */
    for(ii=0 ; ii<256 ; ii++)
        charMaskTbl1[ii] = (charMaskTbl1[ii] & ~(CHRMSK_DISPLAYABLE|CHRMSK_PRINTABLE)) |
              ((charMaskTbl1[ii] & CHRMSK_DISPLAYABLE) ? CHRMSK_PRINTABLE:0) |
              ((charMaskTbl1[ii] & CHRMSK_PRINTABLE) ? CHRMSK_DISPLAYABLE:0) ;
    /* Initialise the printer */
    if ((bp = createBuffer((meUByte *)"")) == NULL)
    {
        status = mlwrite(MWABORT,(meUByte *)"Unable to setup Printer");
        goto quitEarly;
    }
    /* Set up the buffer modes correctly so that we save the codes correctly */
    meModeSet(bp->mode,MDCR);
    meModeClear(bp->mode,MDLF);
    meModeClear(bp->mode,MDCTRLZ);
    
#if MEOPT_HILIGHT
    /* Clear the 2nd entry just in case the hilighting is not defined.
     * The 2nd entry is used as a continuation when hilighting is active. */
    vps[1].wind = vps[0].wind = wp ;
    vps[1].hilno = vps[0].hilno = wp->buffer->hilight ;
    vps[1].bracket = vps[0].bracket = NULL ;
    vps[1].flag = vps[0].flag = 0 ;
    /* get the default text scheme */
    if (vps[0].hilno)
        printTextScheme = hilights[vps[0].hilno]->scheme ;
    else
        printTextScheme = wp->buffer->scheme ;
    printCurrScheme = meSCHEME_INVALID ;
#endif
    printer.pLinesPerPage = (printer.param[mePI_PAGEY].l *
                             printer.param[mePI_ROWS].l *
                             printer.param[mePI_COLS].l);
    /* Construct the start of file */
    if ((clp = composePage (1)) != NULL)
    {
        if (fp)                         /* File device */
            dumpToFile (fp, clp);
        else if (dbp)                   /* Buffer device */
            dumpToBuffer (dbp, clp);
        else                            /* Internal device */
            dumpToInternal (&ihead, &itail, clp);
    }

    do {
        printer.pPageNo++;              /* Next page */
        
        if((printer.param [mePI_FLAGS].l & PFLAG_SILENT) == 0)
            mlwrite(0,(meUByte *)"Printing page %d ..", printer.pPageNo);

        /* Construcr a page worth of formatted data */
        while ((sLine != eLine) && (printer.pNoLines < printer.pLinesPerPage))
        {
#if MEOPT_HILIGHT
            vps[0].line = sLine;
            if (printAddLine (bp,sLine,vps) <= 0)
#else
            if (printAddLine (bp,sLine) <= 0)
#endif
            {
                status = mlwrite(MWABORT,(meUByte *)"Internal error: Cannot add new prin line");
                goto quitEarly;
            }
            sLine = meLineGetNext (sLine);
#if MEOPT_HILIGHT
            /* Propogate the next line information back into the
             * current line */
            vps[0].flag = vps[1].flag;
            vps[0].hilno = vps[1].hilno;
            vps[0].bracket = vps[1].bracket;
#endif
        }

        /* Compose the formatted data into a page */
        gbp = bp;
        gpbp = wp->buffer;
        if ((clp = composePage (0)) == NULL)
        {
            status = mlwrite(MWABORT,(meUByte *)"Internal error: Cannot compose printer page");
            goto quitEarly;
        }

        /* Output the formatted page to the approipriate destination */
        if (fp)                         /* File device */
            dumpToFile (fp, clp);
        else if (dbp)                   /* Buffer device */
            dumpToBuffer (dbp, clp);
        else                            /* Internal device */
            dumpToInternal (&ihead, &itail, clp);

        /* Delete the formatted page data */
        if(printer.pNoLines)
        {
            lp = meLineGetNext (bp->baseLine);
            for (ii = 0; ii < printer.pLinesPerPage; ii++)
            {
                nlp = meLineGetNext (lp);
                meFree (lp);
                lp = nlp;
                if (--printer.pNoLines == 0)
                    break;
            }
            /* Fix up the pointers */
            meLineGetNext (bp->baseLine) = lp;
            meLineGetPrev (lp) = bp->baseLine;
        }
    } while (sLine != eLine);

    /* Construct the end of file */
    if ((clp = composePage (-1)) != NULL)
    {
        if (fp)
            dumpToFile (fp, clp);
        else if (dbp)                   /* Buffer device */
            dumpToBuffer (dbp, clp);
        else                            /* Internal device */
            dumpToInternal (&ihead, &itail, clp);
    }

    /* Close the file descriptor */
    if (fp)
    {
        fclose (fp);
        fp = NULL;
    }
                  
    /* If this is a command line then print the file. */
    switch (printer.pDestination)
    {
    case PDEST_COMLINE:
        {
            meUByte cmdLine [meBUF_SIZE_MAX+20];
            meUByte *p, *q;
            meUByte cc;
            
            /* Expand the command line */
            p = printer.param [mePS_CMD].p;
            q = cmdLine;
            if ((p == NULL) || (*p == '\0'))
            {
                mlwrite(MWPAUSE|MWABORT,(meUByte *)"No command line specified. Print file %s", fname);
                goto quitEarly;
            }
            
            while ((cc = *p++) != '\0')
            {
                if (cc == '%')
                {
                    /* %f - insert the filename */
                    if ((cc = *p) == 'f')
                    {
                        meUByte *r = fname;
                        while ((*q = *r++) != '\0')
                            q++;
                        p++;                /* Skip the '%f' */
                    }
                    /* % at end of string, insert a % */
                    else if (cc == '\0')
                        *q++ = '%';
                    /* Otherwise insert the trailing character */
                    else
                    {
                        *q++ = cc;
                        p++;            /* Skip the character */
                    }
                }
                else
                    *q++ = cc;
            }
            *q = '\0';                  /* Terminate the string */
                    
            /* Call me's system to print the file. Me's version is
             * used cause its return value is correctly evaluated and
             * on unix a " </dev/null" safety arg is added to the cmdLine
             */
            if((status=doShellCommand(cmdLine,0)) > 0)
                status = (resultStr[0] == '0') ? meTRUE:meFALSE ;
            if(status <= 0)
                mlwrite(MWABORT,(meUByte *)"[Failed to print file %s]",fname);
            break;
        
        case PDEST_INTERNAL:
#ifdef _WIN32
            status = WinPrint ((frameCur->bufferCur->fileName != NULL) ? frameCur->bufferCur->fileName
                               : (frameCur->bufferCur->name != NULL) ? frameCur->bufferCur->name
                               :(meUByte *)"UNKNOWN source",
                               ihead);
            ihead = NULL;               /* Reset - never returned */
            break;
#endif
        case PDEST_BUFFER:
            dbp->dotLine = meLineGetNext (dbp->baseLine);
            dbp->dotOffset = 0;
            dbp->dotLineNo = 0;
            resetBufferWindows (dbp);
            break;
        }
    }
    
    /* Shut down the system */
quitEarly:
    /* Shut down the file */
    if (fp)
        fclose (fp);
    /* Kill off the buffer */
    if (bp)
        zotbuf (bp, meTRUE);
    /* Free off internal list */
    while (ihead != NULL)
    {
        itail = ihead;
        ihead = meLineGetNext (ihead);
        meFree (itail);
    }
    /* Reshuffle the isDisplayable test back again */
    for(ii=0 ; ii<256 ; ii++)
        charMaskTbl1[ii] = (charMaskTbl1[ii] & ~(CHRMSK_DISPLAYABLE|CHRMSK_PRINTABLE)) |
              ((charMaskTbl1[ii] & CHRMSK_DISPLAYABLE) ? CHRMSK_PRINTABLE:0) |
              ((charMaskTbl1[ii] & CHRMSK_PRINTABLE) ? CHRMSK_DISPLAYABLE:0) ;

    /* Put out end of file indicator. */
    if ((status > 0) &&
        ((printer.param [mePI_FLAGS].l & PFLAG_SILENT) == 0))
        mlwrite(0,(meUByte *)"Printing - Done. %d page(s).", printer.pPageNo);
    return status;


}

int
printRegion (int f, int n)
{
    meLine *startLine, *endLine;
    long startLineNo;
    long numLines;

    if (frameCur->windowCur->markLine == NULL)
        return noMarkSet ();

    if ((startLineNo=frameCur->windowCur->dotLineNo-frameCur->windowCur->markLineNo) == 0)
        return meTRUE;
    numLines = abs (startLineNo);
    if (startLineNo < 0)
    {
        startLineNo = frameCur->windowCur->dotLineNo;
        startLine = frameCur->windowCur->dotLine;
        endLine = frameCur->windowCur->markLine;
    }
    else
    {
        startLineNo = frameCur->windowCur->markLineNo;
        startLine = frameCur->windowCur->markLine;
        endLine = frameCur->windowCur->dotLine;
    }
    return printSection (frameCur->windowCur, startLineNo, numLines, startLine, endLine, n);
}

int
printBuffer (int f, int n)
{
    return printSection (frameCur->windowCur, 0, frameCur->bufferCur->lineCount,
                         meLineGetNext (frameCur->bufferCur->baseLine), frameCur->bufferCur->baseLine,n);
}

#endif
