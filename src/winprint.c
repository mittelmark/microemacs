/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * winprint.c - Windows Print handler.
 *
 * Copyright (c) 1998-2001 Jon Green
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
 * Created:     28/02/1998
 * Synopsis:    Windows Print handler.
 * Authors:     Jon Green & Steven Phillips
 * Description:
 *     Printer driver routines for the WIN32 build for Microsoft windows
 *     environments.
 */

#define _WIN32_FULL_INC                 /* Include ALL of the windows stuff !! */

#include "emain.h"

#if MEOPT_PRINT

#include "evers.h"                      /* Version information */
#include "eskeys.h"                     /* Emacs special keys */
#include "wintermr.h"                   /* Windows resource file */
#include "winterm.h"                    /* Windows terminal definition file */
#include "eprint.h"                     /* Printer definitions */

#ifndef BST_CHECKED
#define BST_UNCHECKED 0x0000
#define BST_CHECKED   0x0001
#endif

/* Page sizes - Determine user requirements for sizing */
#define PAGSIZ_DONTCARE          0      /* Do not care what size is */
#define PAGSIZ_EXACT             1      /* Require exact page size (if possible) */
#define PAGSIZ_MIN               2      /* This is a minimum page size */

/* Printer font definitions. */
#define PFONT_COUNT                 3   /* Number of fonts */
#define PFONT_BOLD               0x01   /* Bold font */
#define PFONT_ITALIC             0x02   /* Italic font */
#define PFONT_UNDER              0x04   /* Underline font */
#define PFONT_MAX                0x08   /* Maximum mask for fonts */

/* Set this flag to use the new way that we should enumerate fonts. This only
 * operates for NT 4.0 and Win 95. We typeically use the existing Font
 * enumeration that is good for win32s */
#define USE_EnumFontFamiliesEx    0     /* 1 = use Ex rather then old function */

/* Pain the special characters */
typedef struct
{
    int paperx;                         /* Page width in charss */
    int papery;                         /* Page depth in chars */
    int upaperx;                        /* Used paper width */
    int upapery;                        /* Used paper depth */
    CharMetrics cell;                   /* Metrics of a character cell */
    int pointy;                         /* Point height of a character * 10 */
    int upagex;                         /* Used page width */
    int upagey;                         /* Used page depth */
    char fontName [32];                 /* Name of the font */
} PAGEDESC;

static PAGEDESC pd;

#define PRINT_SPOOLING         0x01     /* Printer is spooling. */
#define PRINT_DIALOGUE         0x02     /* Printer dialogue is constucted */

static int printStatus = 0;             /* Status of print dialogue */
static char *printJob = NULL;           /* Name of current print job */
static HWND hDlgCancel;                 /* Cancel Dialogue */
PRINTDLG printDlg;                      /* Print Dialogue */
static int bPrint;

extern HANDLE ttInstance;

/*
 * Exactly strings
 * Used in the dialogue.
 */

static const char *pcdExactlyStrings[] =
{
    "Don't Care",
    "Exactly",
    "Minimum"
};

static char *regPrintName = "/print" ;
static meRegNode *regPrint;

/*
 * printFont
 * Initialise the generic printer font
 */
static void
printFont (LOGFONT *plf, char *fontName)
{
    plf->lfWeight = FW_NORMAL;
    plf->lfItalic = meFALSE;
    plf->lfUnderline = meFALSE;
    plf->lfCharSet = ttlogfont.lfCharSet;
    plf->lfWidth = 0;
    plf->lfHeight = 0;
    plf->lfEscapement = 0;
    plf->lfOrientation = 0;
    plf->lfStrikeOut = meFALSE;
    plf->lfOutPrecision = OUT_DEVICE_PRECIS/*|OUT_TT_PRECIS*/;
    plf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    plf->lfQuality = DEFAULT_QUALITY;
    plf->lfPitchAndFamily = FIXED_PITCH|FF_DONTCARE;
    strcpy (plf->lfFaceName, fontName);
}

/*
 * printMakeDevNames
 * Construct a new device mode from the names
 */
DEVNAMES *
printMakeDevNames (char *driver, char *printer, char *port, int defaultp)
{
    HANDLE hDevNames = NULL;           /* Global, moveable handle */
    if ((driver != NULL) && (printer != NULL))
    {
        DEVNAMES *hDev;
        char *dicp;
        int jj;

        if (port == NULL)
            port = "";

        /* Get the size of the structure */
        jj = ((strlen (printer) + 1) +
              (strlen (driver) + 1) +
              (strlen (port) + 1) +
               sizeof (DEVNAMES));

        /* Build movable global object */
        if ((hDevNames = GlobalAlloc (GMEM_MOVEABLE|GMEM_ZEROINIT, jj)) != NULL)
        {

            hDev = GlobalLock (hDevNames);
            jj = sizeof (DEVNAMES);
            dicp = (char *)(hDev);

            /* Copy in driver details */
            hDev->wDriverOffset = jj;
            jj += sprintf (&dicp[jj], "%s", driver) + 1;
            hDev->wDeviceOffset = jj;
            jj += sprintf (&dicp[jj], "%s", printer) + 1;
            hDev->wOutputOffset = jj;
            sprintf (&dicp[jj], "%s", port);
            hDev->wDefault = defaultp;

            /* Relinquich object */
            GlobalUnlock (hDevNames);
        }
    }
    /* Return the global memory handle to the object */
    return hDevNames;
}

/*
 * getPrinterInfo
 * Determine the available printers in the system. Enumerate
 * the printer devices to locate what is in the system.
 *
 * The index is 0..NumDevices
 * An index of -1 is the default printer.
 */
static HANDLE
getPrinterInfo (int index)
{
    HANDLE *hDevNames = NULL;           /* Global, moveable handle */
    DWORD dwNeeded;                     /* Bytes needed for devices */
    DWORD dwReturned;                   /* Bytes returned for devices */
    int best=-1 ;
    int any=-1;                         /* Any printer */

    /* Figure out how much memory is needed and allocate it. */
    EnumPrinters (PRINTER_ENUM_LOCAL|PRINTER_ENUM_FAVORITE,
                  NULL, 2,
                  NULL, 0,
                  &dwNeeded,
                  &dwReturned);

    /* Watch for the case of no default printer */
    if (dwNeeded != 0)
    {
        LPPRINTER_INFO_2 pi2;           /* Line printer information */

        pi2 = (LPPRINTER_INFO_2) LocalAlloc (LPTR, dwNeeded + 4);

        /* Get the information about the printer */
        if (EnumPrinters (PRINTER_ENUM_FAVORITE|PRINTER_ENUM_LOCAL,
                          NULL, 2,
                          (LPBYTE) pi2, dwNeeded,
                          &dwNeeded, &dwReturned))
        {
            int ii ;                   /* Loop counter */

            for (ii = 0; ii < (int)(dwReturned); ii++)
            {
                /* Skip any hidden printers */
                if (pi2[ii].Attributes & PRINTER_ATTRIBUTE_HIDDEN)
                    continue;

                /* Find the named printer from our parameters this is always the best. */
                if ((index == -1) &&
                    (/*((printer.param [mePI_WINDRIVER].p != NULL) &&
                      (pi2[ii].pDriverName != NULL) &&
                      (strcmp (printer.param [mePI_WINDRIVER].p, pi2[ii].pDriverName) == 0)) &&*/
                     ((printer.param [mePI_WINDEVICE].p != NULL) &&
                      (pi2[ii].pPrinterName != NULL) &&
                      (strcmp (printer.param [mePI_WINDEVICE].p, pi2[ii].pPrinterName) == 0)) &&
                     ((printer.param [mePI_WINPORT].p != NULL) &&
                      (pi2[ii].pPortName != NULL) &&
                      (strcmp (printer.param [mePI_WINPORT].p, pi2[ii].pPortName) == 0))))
                {
                    /* Set this to our index. */
                    index = ii;
                }

                if ((pi2[ii].Attributes & PRINTER_ATTRIBUTE_DEFAULT) && (best < 0))
                {
                    best = ii;
                    continue;
                }

                /* See if this is the printer we want. */
                if (((index == -1) && (pi2[ii].Attributes & PRINTER_ATTRIBUTE_DEFAULT)) ||
                    (index == ii))
                {
                    DEVNAMES *hDev;
                    char *dicp;
                    int jj;

                    /* Get the size of the structure */
                    jj = ((strlen (pi2[ii].pPrinterName) + 1) +
                          (strlen (pi2[ii].pDriverName) + 1) +
                          (strlen (pi2[ii].pPortName + 1) +
                           sizeof (DEVNAMES)));

                    /* Build movable global object */
                    if ((hDevNames = GlobalAlloc (GMEM_MOVEABLE|GMEM_ZEROINIT, jj)) == NULL)
                    {
                        mlwrite (MWABORT|MWPAUSE, "Cannot allocate device memory");
                        break;
                    }

                    hDev = GlobalLock (hDevNames);
                    jj = sizeof (DEVNAMES);
                    dicp = (char *)(hDev);

                    /* Copy in driver details */
                    hDev->wDriverOffset = jj;
                    jj += sprintf (&dicp[jj], "%s", pi2[ii].pDriverName) + 1;
                    hDev->wDeviceOffset = jj;
                    jj += sprintf (&dicp[jj], "%s", pi2[ii].pPrinterName) + 1;
                    hDev->wOutputOffset = jj;
                    sprintf (&dicp[jj], "%s", pi2[ii].pPortName);

                    if (pi2[ii].Attributes & PRINTER_ATTRIBUTE_DEFAULT)
                        hDev->wDefault = 1;

                    /* Relinquich object */
                    GlobalUnlock (hDevNames);
                    break;              /* Quit - we have found it */
                }
                else if((any < 0) && (ii != index))
                    any = ii ;
            }
        }
        /* Free off the memory */
        LocalFree (LocalHandle (pi2));
    }
    if((hDevNames == NULL) && ((best >= 0) || (any >= 0)))
    {
        if (best < 0)
            best = any;
        hDevNames = getPrinterInfo (best) ;
    }
    return hDevNames;
}

/*
 * printGetDC
 * Get the device context
 */
static HDC
printGetDC (void)
{
    DEVNAMES *hDev;

    if ((printStatus & PRINT_DIALOGUE) && (printDlg.hDC != NULL))
        return printDlg.hDC;

    /* Get the printer device if we do not already have it */
    if ((printDlg.hDevNames == NULL) &&
        ((printDlg.hDevNames = getPrinterInfo (-1)) == NULL))
        return NULL;

    /* Construct the device context for the print job */
    hDev = GlobalLock (printDlg.hDevNames);
    printDlg.hDC = CreateDC (&((char *)(hDev))[hDev->wDriverOffset],
                             &((char *)(hDev))[hDev->wDeviceOffset],
                             NULL, /*&((char *)(hDev))[hDev->wOutputOffset]*/
                             NULL);
    GlobalUnlock (printDlg.hDevNames);
    return (printDlg.hDC);
}

/*
 * buildFontTable
 * Build up a table with all of the text metrics of the font types that
 * we might need. We should really do this on demand, but it is easier
 * to do it all up-front.
 */
static int
constructFontTable (HDC hDC, HFONT *fontTable, int charx, int chary, char *fontName)
{
    LOGFONT logfont;                /* Logical font structure */
    int ii;

    /* Set up the fonts. Build a font table for each of the font types */
    printFont (&logfont, fontName);
    logfont.lfWidth = charx;
    logfont.lfHeight = chary;

    for (ii = 0; ii < PFONT_MAX; ii++)
    {
        logfont.lfWeight = (ii & PFONT_BOLD) ? FW_BOLD : FW_NORMAL;
        logfont.lfItalic = (ii & PFONT_ITALIC) ? meTRUE : meFALSE;
        logfont.lfUnderline = (ii & PFONT_UNDER) ? meTRUE : meFALSE;

        if (ii == 0)
        {
            /* Create the font */
            if ((fontTable [ii] = CreateFontIndirect (&logfont)) == 0)
                return meFALSE;

            /* Get the width and height of the font. This is a mono font so all
             * fonts should be the same size */
            SelectObject (hDC, fontTable[0]);
        }
        else if ((fontTable[ii] = CreateFontIndirect (&logfont)) == NULL)
        {
            int jj;

            /* Font failed - Find the nearest matching font */
            for (jj = PFONT_MAX; jj != 0; jj >>= 1)
            {
                if (ii & jj)
                {
                    fontTable[ii] = fontTable[ii & ~jj];
                    break;
                }
            }
        }
    }
    return meTRUE;
}

/*
 * Destruct the font table
 */
static void
destructFontTable (HFONT *fontTable)
{
    int ii;

    for (ii = 0; ii < PFONT_MAX; ii++)
    {
        if (fontTable [ii] != NULL)
        {
            DeleteObject (fontTable [ii]);
            fontTable [ii] = NULL;
        }
    }
}

/***********************************************************************
 *
 * Abort: Abort Dialogue.
 *
 * The following functions manage the abort dialogue. This provides a
 * mechanism for the user to abort the print job midway through printing.
 *
 ***********************************************************************/

/*
 * AbortProc
 * Printer abort procedure
 */
BOOL CALLBACK
AbortProc(HDC hdc, int nCode)
{
    MSG msg;

    /*
     * Retrieve and remove messages from the thread's message
     * queue.
     */
    while (PeekMessage((LPMSG) &msg, (HWND) NULL, 0, 0, PM_REMOVE))
    {
        /* Process any messages for the Cancel dialog box. */
        if (!IsDialogMessage (hDlgCancel, (LPMSG) &msg))
        {
            TranslateMessage ((LPMSG) &msg);
            DispatchMessage (&msg); /* Unhandled */
        }
    }

    /*
     * Return the global bPrint flag (which is set to meFALSE
     * if the user presses the Cancel button).
     */
     return /*(printStatus & PRINT_SPOOLING) ? meTRUE : meFALSE;*/ bPrint;
}

LRESULT CALLBACK
AbortPrintJob (HWND hwndDlg,     /* window handle of dialog box     */
               UINT message,     /* type of message                 */
               WPARAM wParam,    /* message-specific information    */
               LPARAM lParam)    /* message-specific information    */
{
    switch (message)
    {
    case WM_INITDIALOG:  /* message: initialize dialog box  */

        /* Initialize the static text control. */
        if (printJob == NULL)
            SetDlgItemText(hwndDlg, IDC_FILE, "**NO INFORMATION**");
        else
            SetDlgItemText(hwndDlg, IDC_FILE, printJob);
        return meTRUE;

    case WM_COMMAND:     /* message: received a command */
        /* User pressed "Cancel" button--stop print job. */
        if ((LOWORD (wParam)) == IDABORT)
        {
            /*        printStatus &= ~PRINT_SPOOLING;*/
            bPrint = meFALSE;
        }
        return meTRUE;
    }
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(message);
    return meFALSE;     /* didn't process a message   */
}

/*************************************************************************
 *
 * PPS: Printer Configuration Dialogue.
 *
 * The following functions manage the printer configuration dialogue
 * and capture information from the user about the printer. The dialogue
 * determines the printer attached to the system and computes the page
 * sizing information using information supplied by the user.
 *
 ***********************************************************************/

/*
 * pcdComputeFont
 * Calculate the size of the font from the information provided in the
 * dialogue.
 */
static int
pcdComputeFont (PAGEDESC *pd, int pagex, int selx, int pagey, int sely, int pointy)
{
    HFONT   hFont;
    LOGFONT logfont;
    TEXTMETRIC textmetric;
    HDC hDC;
    int retries = 0;
    int paperx;
    int papery;
    int xtraWidth;
    int xtraDepth;

    /* Sort out the parameters */
    if (printer.param[mePI_COLS].l < 1)
        printer.param[mePI_COLS].l = 1;
    if (printer.param[mePI_ROWS].l < 1)
        printer.param[mePI_ROWS].l = 1;

    /* Compute the additional horizontal spacing requirements */
    xtraWidth = printer.param[mePI_MLEFT].l + printer.param[mePI_MRIGHT].l ;
    xtraWidth += (printer.param[mePI_COLS].l-1) * printer.param[mePI_WSEP].l;
    if (printer.pLineNumDigits > 0)
        xtraWidth += printer.param[mePI_COLS].l * printer.pLineNumDigits ;

    /* Compute the additional vertical spacing requirements */
    xtraDepth = (printer.param[mePI_ROWS].l-1) ;
    xtraDepth += printer.param[mePI_MTOP].l + printer.param[mePI_MBOTTOM].l ;
    xtraDepth += printer.pNoHeaderLines + printer.pNoFooterLines ;

    /* The paper size is undefined, determine the paper size */
    paperx = pagex * printer.param[mePI_COLS].l + xtraWidth;
    papery = pagey * printer.param[mePI_ROWS].l + xtraDepth;

    hDC = printGetDC();

    /* Set into text mode. */
    SetMapMode (hDC, MM_TEXT);

    /* Select the font into the system */
    printFont (&logfont, printer.param [mePI_FONTFACE].p);
    strncpy (pd->fontName, printer.param [mePI_FONTFACE].p, 32);
    pd->fontName[31] = '\0';

    /* Quick check on the selection */
    if (paperx <= 0)
        selx = PAGSIZ_DONTCARE;
    if (papery <= 0)
        sely = PAGSIZ_DONTCARE;

    /* See if we are processing a point sized image */
    if ((selx == PAGSIZ_DONTCARE) && (sely == PAGSIZ_DONTCARE))
    {
        if ((pointy <= 0) || (pointy > 72))  /* Points defined ?? */
            pointy = 10;                     /* No default to 10 point */
        logfont.lfHeight = MulDiv(pointy,
                                  GetDeviceCaps(hDC, LOGPIXELSY),
                                  72);
    }
    /* Match vertically */
    else if (sely != PAGSIZ_DONTCARE)
        logfont.lfHeight = GetDeviceCaps(hDC, VERTRES)/papery;
    /* Match horizontally. Choose a low number for the vertical
     * scale - will correct later. */
    else
        logfont.lfHeight = GetDeviceCaps(hDC, VERTRES)/30;

    /* Attempt twice to get the font correct */
    do
    {
        int ii;
        /* Create the font */
        if ((hFont = CreateFontIndirect (&logfont)) == 0)
            return mlwrite (MWABORT|MWPAUSE, "Cannot create font");

        /* A Font has been created, now determine what the size of the page is. */
        SelectObject (hDC, hFont);
        GetTextMetrics(hDC, &textmetric);
        DeleteObject (hFont);

        pd->cell.sizeY = textmetric.tmHeight;
        pd->cell.sizeX = textmetric.tmAveCharWidth;

        /* Allocate on font size */
        if (pd->cell.sizeY > 0)
            pd->papery = GetDeviceCaps(hDC, VERTRES) / pd->cell.sizeY;
        else
            pd->papery = 1;
        if (pd->cell.sizeX > 0)
            pd->paperx = GetDeviceCaps(hDC, HORZRES) / pd->cell.sizeX;
        else
            pd->papery = 1;

        /* Determine point size */
        if ((ii = GetDeviceCaps(hDC, LOGPIXELSY)) > 0)
            pd->pointy = (pd->cell.sizeY * 72 * 10) / ii;
        else
            pd->pointy = 10;

        /* Handle point size immediatly. */
        if ((selx == PAGSIZ_DONTCARE) && (sely == PAGSIZ_DONTCARE))
            goto done;

        /* Handle the vertical scaling */
        if (sely != PAGSIZ_DONTCARE)
        {
            if (papery > pd->papery)
                mlwrite (0, "Cannot get veritical font scale");

            /* If we do not care about the width, or we have acheived the
             * requested width we have finished. */
            else if ((selx == PAGSIZ_DONTCARE) || (paperx <= pd->paperx))
                goto done;

            /* Font is too wide, try to reduce the font to fit horizontally. */
            logfont.lfHeight = (pd->cell.sizeY * pd->paperx) / paperx;
            logfont.lfWidth = 0;
        }
        else if (selx != PAGSIZ_DONTCARE)
        {
            if (paperx > pd->paperx)
                mlwrite (0, "Cannot get horizontal font scale");
            else if ((sely == PAGSIZ_DONTCARE) || (papery <= pd->papery))
                goto done;

            /* Font is too wide, try to reduce the font to fit horizontally. */
            logfont.lfHeight = GetDeviceCaps(hDC, VERTRES)/papery;
            logfont.lfWidth = 0;
        }
    }
    while (++retries < 2);
    mlwrite (0, "Cannot find page size");
done:
    /* Back compute the settings according to the user request
     * Sort out the horizontal sizing first. */
    if ((selx == PAGSIZ_EXACT) && (paperx <= pd->paperx))
        pd->upagex = pagex;
    else
        /* Compute the new active page size from the logical paper
         * size */
        pd->upagex = (pd->paperx - xtraWidth) / printer.param[mePI_COLS].l;
    pd->upaperx = (pd->upagex * printer.param[mePI_COLS].l) + xtraWidth;

    /* Sort out the vertical sizing */
    if ((sely == PAGSIZ_EXACT) && (papery <= pd->papery))
        pd->upagey = pagey;
    else
        pd->upagey = (pd->papery - xtraDepth) / printer.param [mePI_ROWS].l;
    pd->upapery = (pd->upagey * printer.param[mePI_ROWS].l) + xtraDepth;
    return meTRUE;
}

/*
 * pcdEnumFontFamiliesCallback
 * Enumerates the font families and sets up the font combo box with
 * the list of fonts located in the system.
 */
#if USE_EnumFontFamiliesEx
int CALLBACK
pcdEnumFontFamiliesCallback (ENUMLOGFONTEX *lpelf, NEWTEXTMETRIC *lpntm, int fontType, LPARAM lParam)
#else
int CALLBACK
pcdEnumFontFamiliesCallback (ENUMLOGFONT *lpelf, NEWTEXTMETRIC *lpntm, int fontType, LPARAM lParam)
#endif
{
    /* If this is a fixed pitch font then add it to the dialogue */
    if ((fontType & (DEVICE_FONTTYPE|TRUETYPE_FONTTYPE)) &&
        (lpelf->elfLogFont.lfPitchAndFamily & FIXED_PITCH))
    {
        DWORD index;

        index = SendMessage ((HWND) lParam, CB_ADDSTRING, 0, (LPARAM) lpelf->elfLogFont.lfFaceName);
        if ((index >= 0) &&
            (printer.param [mePI_FONTFACE].p != NULL) &&
            (strcmp (printer.param [mePI_FONTFACE].p,  lpelf->elfLogFont.lfFaceName) == 0))
            SendMessage ((HWND) lParam, CB_SETCURSEL, index, 0);
    }
    return meTRUE;
}

/*
 * pcdChange
 * Interrogate the printer dialogue and determine what has changed.
 * This is run from a timer so that we allow multiple character
 * inputs to be performed within a certain time frame before updating
 * the statistics.
 */
static void
pcdChange (HWND hWnd)
{
    int buttons[] = {IDC_HEADER, IDC_FOOTER, IDC_LINE_NOS};
    int masks[] = {PFLAG_ENBLHEADER, PFLAG_ENBLFOOTER, PFLAG_ENBLLINENO};
    int combos[] = {IDC_HEIGHT,IDC_WIDTH,IDC_COLUMNS,IDC_ROWS};
    int cindicies[] = {mePI_PAGEY, mePI_PAGEX, mePI_COLS, mePI_ROWS};
    int ecombos[] = {IDC_WIDTH_EXACTLY, IDC_HEIGHT_EXACTLY};
    int ecindicies[] = {mePI_SPECX, mePI_SPECY, mePI_FONTFACE};
    int ii;
    char buf [500];
    HWND dWnd;
    DWORD value;

    /* Collect the arguments from the dialogue */
    for (ii = 0; ii < (sizeof (buttons)/sizeof (buttons[0])); ii++)
    {
        switch (IsDlgButtonChecked (hWnd, buttons [ii]))
        {
        case BST_CHECKED:
            printer.param [mePI_FLAGS].l |= masks[ii];
            break;
        case BST_UNCHECKED:
            printer.param [mePI_FLAGS].l &= ~masks[ii];
            break;
        }
    }

    /* Get the combo box states */
    for (ii = 0; ii < (sizeof (combos)/sizeof (combos[0])); ii++)
    {
        char *spos, *epos;              /* Start and end text positions */

        /* Get the current dialogue value */
        dWnd = GetDlgItem (hWnd, combos[ii]);
        if (SendMessage (dWnd, WM_GETTEXT, 30, (LPARAM)(buf)) == CB_ERR)
            buf [0] = '\0';
        for (value = 0, spos = (epos = buf); *spos != '\0'; spos++)
        {
            if ((*spos >= '0') && (*spos <= '9'))
            {
                if (epos != spos)
                    *epos = *spos;
                value = (value * 10) + (*epos++ - '0');
            }
        }
        /* Re-fill the dialogue if it is invalid */
        if (spos != epos)
        {
            *epos = '\0';
            SendMessage (dWnd, WM_SETTEXT, 0, (LPARAM) buf);
        }

        /* Fill the the value for the message type */
        printer.param [cindicies[ii]].l = value;
    }

    /* Get the exactly strings out of the dialogue */
    for (ii = 0; ii < (sizeof (ecombos)/sizeof (ecombos[0])); ii++)
    {
        /* Get the current dialogue value */
        dWnd = GetDlgItem (hWnd, ecombos[ii]);
        value = (DWORD) SendMessage (dWnd, CB_GETCURSEL, 0, 0);
        /* If there is an error then set to the exact size */
        if (value == CB_ERR)
            SendMessage (dWnd, CB_SETCURSEL, value = 1, 0);
        printer.param [ecindicies[ii]].l = value;
    }

    /* Get the font out of the dialogue */
    dWnd = GetDlgItem (hWnd, IDC_FONTFACE);
    value = (DWORD) SendMessage (dWnd, CB_GETCURSEL, 0, 0);
    if (value == CB_ERR)
        SendMessage (dWnd, CB_SETCURSEL, 0, 0);
    SendMessage (dWnd, WM_GETTEXT, 30, (LPARAM)(buf));

    /* Copy the new font into the registry immediatly */
    printer.param[mePI_FONTFACE].p =
              regGetString (regSet (regPrint, printNames[mePI_FONTFACE], buf),(meUByte *) "Courier New");

    /* Compute the size of the font */
    pcdComputeFont (&pd,
                    printer.param [mePI_PAGEX].l, printer.param[mePI_SPECX].l,
                    printer.param [mePI_PAGEY].l, printer.param[mePI_SPECY].l,
                    10);

    /* Put out the new page statistics */
    sprintf (buf,
             "Font:\t\t%s - %d.%d point\r\n"
             "Physical Page Size:\t%d x %d\r\n"
             "Used Page Size:\t%d x %d\r\n"
             "Unused Page Size:\t%d x %d\r\n"
             "Virtual Page Size\t%d x %d\r\n",
             printer.param [mePI_FONTFACE].p /* pd.fontName*/,
             pd.pointy/10, pd.pointy%10,
             pd.paperx, pd.papery,
             pd.upaperx, pd.upapery,
             pd.paperx - pd.upaperx, pd.papery - pd.upapery,
             pd.upagex, pd.upagey);
    SetDlgItemText (hWnd, IDC_PAGE_PROPS, buf);
}

/*
 * pcdTimerCallback
 * The dialogue timer callback function. This is invoked directly
 * from the timer and does not pass through the message handling functions.
 */
static VOID CALLBACK
pcdTimerCallback (HWND hWnd, UINT uMsg, UINT idEvent, DWORD time)
{
    KillTimer (hWnd, 10);
    pcdChange(hWnd);
}

/*
 * pcdDeviceName
 * Retrieves the name of the device and the landscape mode from the
 * locked resource data structures.
 */
static void
pcdDeviceName (HWND hWnd)
{
    char buf [32+32+2+1];               /* Local character buffer */
    int landscape = BST_UNCHECKED;      /* Landscape mode */

    /* Get the printer name out */
    if (printDlg.hDevNames/*Mode*/ != NULL)
    {
        DEVNAMES *hDev = GlobalLock (printDlg.hDevNames);
        sprintf (buf,
                 "%s on %s",
                 &((char *)(hDev))[hDev->wDeviceOffset],
                 &((char *)(hDev))[hDev->wOutputOffset]);

        /* Write the names into the registry */
        regSet (regPrint, printNames[mePI_WINDRIVER], &((char *)(hDev))[hDev->wDriverOffset]);
        regSet (regPrint, printNames[mePI_WINDEVICE], &((char *)(hDev))[hDev->wDeviceOffset]);
        regSet (regPrint, printNames[mePI_WINPORT], &((char *)(hDev))[hDev->wOutputOffset]);
        regSet (regPrint, printNames[mePI_WINDEFAULT], (hDev->wDefault == 0) ? "0" : "1");
        GlobalUnlock (printDlg.hDevNames);
    }
    else
        strcpy (buf, "** Unknown Device **");

    SetDlgItemText (hWnd, IDC_PC_PRINTER, buf);

    /* Get the paper orientation out */
    if (printDlg.hDevMode != NULL)
    {
        DEVMODE *hDev = GlobalLock (printDlg.hDevMode);
        if (hDev->dmOrientation == DMORIENT_LANDSCAPE)
            landscape = BST_CHECKED;
        GlobalUnlock (printDlg.hDevMode);
    }
    CheckDlgButton (hWnd, IDC_LANDSCAPE, landscape);
}

/*
 * pcdDialogue
 * Handle the printer dialogue.
 */
static BOOL CALLBACK
pcdDialogue (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND dWnd;                          /* Dialogue window handle */
    int ii;                             /* Local loop counter */
    char buf [40];                      /* Local character buffer */

    switch (message)
    {
    case WM_INITDIALOG:
        /***********************************************************
         *
         * Initialise the dialogue.
         *
         **********************************************************/
        /* Fill the combo list for the width characteristics */
        dWnd = GetDlgItem (hWnd, IDC_WIDTH_EXACTLY);
        SendMessage (dWnd, CB_RESETCONTENT, 0, 0);  /* Reset contents */
        for (ii = 0; ii < 3; ii++)
            SendMessage (dWnd, CB_INSERTSTRING, ii, (LPARAM) pcdExactlyStrings[ii]);
        /* Re-fill the dialogue with a default value */
        SendMessage (dWnd, CB_SETCURSEL, (WPARAM) printer.param[mePI_SPECX].l, 0);

        /* Fill the combo list for the height characteristic */
        dWnd = GetDlgItem (hWnd, IDC_HEIGHT_EXACTLY);
        SendMessage (dWnd, CB_RESETCONTENT, 0, 0);  /* Reset contents */
        for (ii = 0; ii < 3; ii++)
            SendMessage (dWnd, CB_INSERTSTRING, ii, (LPARAM) pcdExactlyStrings[ii]);
        SendMessage (dWnd, CB_SETCURSEL, (WPARAM) printer.param[mePI_SPECY].l, 0);

        /* Fill in the number of rows. */
        buf [1] = '\0';
        dWnd = GetDlgItem (hWnd, IDC_ROWS);
        SendMessage (dWnd, CB_RESETCONTENT, 0, 0);  /* Reset contents */
        for (ii = 0; ii < 6; ii++)
            SendMessage (dWnd, CB_INSERTSTRING, ii, (LPARAM) meItoa(ii+1));
        SendMessage (dWnd, WM_SETTEXT, 0, (LPARAM) meItoa(printer.param[mePI_ROWS].l));

        /* Fill in the number of columns. */
        buf [1] = '\0';
        dWnd = GetDlgItem (hWnd, IDC_COLUMNS);
        SendMessage (dWnd, CB_RESETCONTENT, 0, 0);  /* Reset contents */
        for (ii = 0; ii < 6; ii++)
            SendMessage (dWnd, CB_INSERTSTRING, ii, (LPARAM) meItoa(ii+1));
        SendMessage (dWnd, WM_SETTEXT, 0, (LPARAM) meItoa(printer.param[mePI_COLS].l));

        /* Fill in the page width */
        dWnd = GetDlgItem (hWnd, IDC_WIDTH);
        SendMessage (dWnd, CB_RESETCONTENT, 0, 0);  /* Reset contents */
        for (ii = 0; ii <= 3; ii++)
            SendMessage (dWnd, CB_INSERTSTRING, ii, (LPARAM) meItoa (ii * 80));
        SendMessage (dWnd, WM_SETTEXT, 0, (LPARAM) meItoa(printer.param[mePI_PAGEX].l));

        /* Fill in the page height  */
        dWnd = GetDlgItem (hWnd, IDC_HEIGHT);
        SendMessage (dWnd, CB_RESETCONTENT, 0, 0);  /* Reset contents */
        for (ii = 0; ii <= 3; ii++)
            SendMessage (dWnd, CB_INSERTSTRING, ii, (LPARAM) meItoa(ii*66));
        SendMessage (dWnd, WM_SETTEXT, 0, (LPARAM) meItoa(printer.param[mePI_PAGEY].l));

        /* Set up the header and footer checkboxes. */
        CheckDlgButton (hWnd, IDC_HEADER,
                        (((printer.param [mePI_FLAGS].l & PFLAG_ENBLHEADER) == 0) ? BST_UNCHECKED : BST_CHECKED));
        CheckDlgButton (hWnd, IDC_FOOTER,
                        (((printer.param [mePI_FLAGS].l & PFLAG_ENBLFOOTER) == 0) ? BST_UNCHECKED : BST_CHECKED));
        CheckDlgButton (hWnd, IDC_LINE_NOS,
                        (((printer.param [mePI_FLAGS].l & PFLAG_ENBLLINENO) == 0) ? BST_UNCHECKED : BST_CHECKED));
        /* Set up the font families */
        dWnd = GetDlgItem (hWnd, IDC_FONTFACE);
        SendMessage (dWnd, CB_RESETCONTENT, 0, 0);

        /* Fill in the printer dialogue */
        if ((printStatus & PRINT_DIALOGUE) == 0)
        {
            memset (&printDlg, 0, sizeof (printDlg));
            printDlg.lStructSize = sizeof (printDlg);
            printDlg.hwndOwner = hWnd;
            printDlg.hDevMode = NULL;
            printDlg.hDevNames = printMakeDevNames (printer.param[mePI_WINDRIVER].p,
                                                    printer.param[mePI_WINDEVICE].p,
                                                    printer.param[mePI_WINPORT].p,
                                                    printer.param[mePI_WINDEFAULT].l);
            printDlg.Flags = (PD_DISABLEPRINTTOFILE|
                              PD_HIDEPRINTTOFILE|
                              PD_NOPAGENUMS|
                              PD_PRINTSETUP|
                              PD_RETURNDC|
                              PD_RETURNDEFAULT);
            if (PrintDlg (&printDlg) == 0)
            {
                if (printDlg.hDevNames != NULL)
                {
                    GlobalFree (printDlg.hDevNames);
                    printDlg.hDevNames = NULL;
                }
                PrintDlg (&printDlg);
            }
            printDlg.Flags &= ~PD_RETURNDEFAULT;
            printStatus |= PRINT_DIALOGUE;
        }
        else
            printDlg.hwndOwner = hWnd;

        pcdDeviceName(hWnd);
#if USE_EnumFontFamiliesEx
        /* This is the new way that we should enumerate fonts. This only
         * operates for NT 4.0 and Win 95. We use the existing Font
         * enumeration that is good for win32s */
        {
            LOGFONT logfont;

            /* Set up to default values */
            memset (&logfont, 0, sizeof (LOGFONT));
            logfont.lfCharSet = ttlogfont.lfCharSet;

            EnumFontFamiliesEx (printGetDC(), &logfont, (FONTENUMPROC) pcdEnumFontFamiliesCallback, (LPARAM)dWnd, 0);
        }
#else
        EnumFontFamilies (printGetDC(), NULL, (FONTENUMPROC) pcdEnumFontFamiliesCallback, (LPARAM)dWnd);
#endif
        /* Set up the rows & columns */
        SetTimer (hWnd, PRINT_TIMER_ID, PRINT_TIMER_RESPONSE, (TIMERPROC) pcdTimerCallback);
        return meTRUE;

    case WM_COMMAND:
        switch (LOWORD (wParam))
        {
        case IDC_PC_PRINTER_SETUP:
            printDlg.Flags &= ~PD_RETURNDEFAULT;
            PrintDlg (&printDlg);
            /* Install the new device */
            pcdDeviceName(hWnd);
            break;
        case IDC_PC_PRINT:
            {
                KillTimer (hWnd, 10);
                pcdChange (hWnd);
                /* Save all of the settings back to the registry so that we
                 * use them next time.  First copy the computed values out of
                 * the printer descriptor. */
                printer.param [mePI_PAPERX].l = pd.paperx;
                printer.param [mePI_PAPERY].l = pd.papery;
                printer.param [mePI_PAGEX].l = pd.upagex;
                printer.param [mePI_PAGEY].l = pd.upagey;

                for (ii = 0; ii < PRINT_MAX; ii++)
                {
                    if ((printTypes [ii] & mePD_WIN) == 0)
                        continue;
                    if (printTypes [ii] & mePD_INT)
                        regSet (regPrint, printNames [ii], meItoa (printer.param [ii].l));
                    else
                        regSet (regPrint, printNames [ii], printer.param [ii].p);
                }
            }
            EndDialog (hWnd, meTRUE);
            return meTRUE;

        case IDCANCEL:
        case IDC_PC_CANCEL:
            //            EnableWindow(meFrameGetWinHandle(frameCur), meTRUE);
            KillTimer (hWnd, 10);
            EndDialog (hWnd, meFALSE);
            return meTRUE;

            /* Buttons */
        case IDC_HEADER:
        case IDC_FOOTER:
        case IDC_LINE_NOS:
            if (HIWORD(wParam) == BN_CLICKED)
                break;
            return meFALSE;

            /* Combo boxes */
        case IDC_ROWS:
        case IDC_COLUMNS:
        case IDC_HEIGHT:
        case IDC_WIDTH:
            /* User selection from the drop down menu */
            if (HIWORD(wParam) == CBN_CLOSEUP)
                break;
            /* User has edited the dialogue. */
            if (HIWORD(wParam) == CBN_EDITCHANGE)
            {
                char buf [30];
                char *spos;
                char *epos;

                /* Get the current dialogue value */
                SendMessage ((HWND) lParam, WM_GETTEXT, 30, (LPARAM)(buf));
                for (spos = (epos = buf); *spos != '\0'; spos++)
                {
                    if (isDigit (*spos))
                    {
                        if (epos != spos)
                            *epos = *spos;
                        epos++;
                    }
                }
                /* Re-fill the dialogue if it is invalid */
                if (spos != epos)
                {
                    *epos = '\0';
                    SendMessage ((HWND)lParam, WM_SETTEXT, 0, (LPARAM) buf);
                }
                break;
            }
            return meFALSE;
            /* The page size specifiers */
        case IDC_FONTFACE:
        case IDC_WIDTH_EXACTLY:
        case IDC_HEIGHT_EXACTLY:
            if ((HIWORD(wParam) == CBN_EDITCHANGE) || (HIWORD(wParam) == CBN_CLOSEUP))
                break;
            return meFALSE;
        default:
            return meFALSE;
        }

        /* Schedule an update of the rest of the dialogue. */
        SetTimer (hWnd, PRINT_TIMER_ID, PRINT_TIMER_RESPONSE, (TIMERPROC) pcdTimerCallback);
        return meTRUE;
    }
    return meFALSE;
}

void
WinPrintSetColor(HDC hDC, int colNo, int setBgCol)
{
    static COLORREF *pColorTbl=NULL;
    static HPALETTE pPal=NULL;                 /* Windows palette table */
    static int pPalSize=0;                     /* Size of the palette */

    if(colNo < 0)
    {
        /* free the pColorTbl */
        free(pColorTbl) ;
        pColorTbl = NULL ;
        if(pPalSize > 0)
        {
            DeleteObject(pPal) ;
            pPal = 0 ;
            pPalSize = 0 ;
        }
        return ;
    }
    if(pColorTbl == NULL)
    {
        /* first call, malloc the table */
        int ii ;
        pColorTbl = malloc(printer.colorNum * sizeof(COLORREF)) ;
        if(pColorTbl == NULL)
            return ;
        ii = printer.colorNum ;
        while(--ii >= 0)
           pColorTbl[ii] = CLR_INVALID ;
    }

    if(pColorTbl[colNo] == CLR_INVALID)
    {
        COLORREF cReq;                      /* Color requested */
        COLORREF cAsg;                      /* Color assigned */
        meUInt   cc ;
        BYTE rr, gg, bb ;

        cc = printer.color[colNo] ;
        rr = (BYTE) mePrintColorGetRed(cc) ;
        gg = (BYTE) mePrintColorGetGreen(cc) ;
        bb = (BYTE) mePrintColorGetBlue(cc) ;

        /* Determine if we are adding to the device context or the palette */
        if (GetDeviceCaps (hDC, RASTERCAPS) & RC_PALETTE)
        {
            int maxPaletteSize;             /* Maximum size of the palette */
            int closeIndex;                 /* A close existing palette index */
            PALETTEENTRY closeEntry;        /* A close palette entry */
            COLORREF closeColor;           /* A close existing palette colour */

            /* Construct the new colour reference */
            cReq = RGB(rr,gg,bb) ;
            maxPaletteSize = GetDeviceCaps(hDC, SIZEPALETTE);

            /* Create a new entry for the colour
             * Check the current palette for the colour. If we have it already we do
             * not need to create a new palette entry. */
            if (pPalSize > 0)
            {
                closeIndex = GetNearestPaletteIndex (pPal, cReq);
                if (closeIndex != CLR_INVALID)
                {
                    GetPaletteEntries (pPal, closeIndex, 1, &closeEntry);
                    closeColor = RGB (closeEntry.peRed, closeEntry.peGreen, closeEntry.peBlue);
                }

                /* Check for duplicates */
                if (cReq != closeColor)
                {
                    /* Fail if the colour map is full */
                    if (pPalSize == maxPaletteSize)
                        return ;

                    /* A  a new entry to the palette */
                    closeIndex = pPalSize;
                    pPalSize++;
                    ResizePalette(pPal,pPalSize);

                    closeEntry.peRed = rr;
                    closeEntry.peGreen = gg;
                    closeEntry.peBlue = bb;
                    closeEntry.peFlags = 0;
                    SetPaletteEntries(pPal, closeIndex, 1, &closeEntry);
                }
            }
            else
            {
                LPLOGPALETTE lPal;          /* Logical palette */

                /* No palette is defined. Allocate a new palette */
                lPal = HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY,
                                  sizeof (LOGPALETTE) + sizeof (PALETTEENTRY));
                lPal->palVersion = 0x300;   /* Windows magic number !! */
                lPal->palNumEntries = 1;    /* 2 entries in the palette */

                lPal->palPalEntry [0]. peRed = rr;
                lPal->palPalEntry [0]. peGreen = gg;
                lPal->palPalEntry [0]. peBlue = bb;
                lPal->palPalEntry [0]. peFlags = 0;

                /* Create the palette */
                pPal = CreatePalette (lPal);
                HeapFree (GetProcessHeap (), 0, lPal);  /* Dispose of the local palette */

                pPalSize = 1;
            }

            /* Save the assigned colour */
            cAsg = PALETTERGB (rr, gg, bb);
        }
        else
        {
            /* Determine what colour will actually be used on non-colour mapped systems */
            cAsg = GetNearestColor(hDC, PALETTERGB (rr, gg, bb));
        }
        pColorTbl[colNo] = cAsg;
    }
    if(setBgCol)
        SetBkColor(hDC,pColorTbl[colNo]) ;
    else
        SetTextColor(hDC,pColorTbl[colNo]) ;
}

/*
 * WinPrint
 * Print the specified file to the printer.
 *
 * Handle the print job. THIS IS A THREAD PROCESS.
 * All error reporting we do in here is through the windows
 * messagebox - DO NOT use any Microemacs error reporting
 * in here.
 *
 * NOTE: This is not a thread at the moment !!
 */
static void
WinPrintThread (LPVOID wParam)
{
    static const char title[] = ME_FULLNAME " " meVERSION " Print Spooler";

    HWND curHwnd ;
    DOCINFO di;                         /* Document information. */
    RECT paperArea;                     /* Paper area */
    HFONT fontTab [PFONT_MAX];          /* Font table */
    char nambuf [meBUF_SIZE_MAX];               /* Name buffer */
    char *docName;
    meLine *ihead;

#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
    if (meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
        curHwnd = meHWndNull ;
#ifdef _ME_WINDOW
    else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
        curHwnd = meFrameGetWinHandle(frameCur) ;
#endif /* _ME_WINDOW */

    /* Pick up the arguments */
    docName = ((char **)(wParam))[0];
    ihead = (meLine *)(((char **)(wParam)) [1]);

    /* Mark as spooling */
    printStatus |= PRINT_SPOOLING;

    /* Set up the printer */
    memset (fontTab, 0, sizeof (fontTab));

    if (ihead == NULL)
    {
        char msgbuf [meBUF_SIZE_MAX];           /* Message buffer */
        sprintf (msgbuf, "Nothing to spool \"%s\"", docName);
        MessageBox (curHwnd, msgbuf, title, MB_ICONEXCLAMATION|MB_OK);
        goto quick_exit;
    }
    bPrint = meTRUE;
    printDlg.hDC = NULL;
    printDlg.hDC = printGetDC();

    /* Get the printer information out */
/*    if (((printStatus & PRINT_DIALOGUE) == 0) || (printDlg.hDC == NULL))*/
    if (printDlg.hDC == NULL)
    {
        MessageBox (curHwnd, "Device context not configured", title, MB_ICONEXCLAMATION|MB_OK);
        goto quick_exit;
    }

    /* Put up an Abort and install the abort procedure */
    /* Register the application's AbortProc function with GDI. */
    printJob = docName;
    SetAbortProc(printDlg.hDC, AbortProc);

    /* Display the modeless Cancel dialog box and disable the application
       window. */
    hDlgCancel = CreateDialog (ttInstance,
                               MAKEINTRESOURCE (IDD_ABORT),
                               curHwnd,
                               (DLGPROC) AbortPrintJob);
/*    EnableWindow(curHwnd, meFALSE);*/

    /* Initialise the document indormation structure for the spool job */
    memset (&di, 0, sizeof (DOCINFO));
    di.cbSize = sizeof (DOCINFO);
    sprintf (nambuf, "MicroEmacs " meVERSION ": %s", docName);
    di.lpszDocName = nambuf;

    /* Set the font mapper into text mode */
    SetMapMode (printDlg.hDC, MM_TEXT);

    /* Get the size of the device */
    paperArea.left   = GetDeviceCaps(printDlg.hDC, LOGPIXELSX);   /* 1/4 inch */
    paperArea.right  = GetDeviceCaps(printDlg.hDC, HORZRES);
    paperArea.bottom = GetDeviceCaps(printDlg.hDC, VERTRES);      /* Vertical resolution */
    paperArea.top    = GetDeviceCaps(printDlg.hDC, LOGPIXELSY);   /* 1/4 inch */

    /* Set up the font size */
    if (constructFontTable (printDlg.hDC, fontTab, pd.cell.sizeX, pd.cell.sizeY, pd.fontName) == meFALSE)
        goto dlg_exit;

#define FlushBuffer() \
do { \
    if (bindex > 0) \
    { \
        TextOut (printDlg.hDC,colNo*pd.cell.sizeX,\
                 lineNo*pd.cell.sizeY, \
                 &buffer[0],bindex); \
        colNo += bindex; \
        bindex = 0; \
    } \
} while(0)

    /* Start the printing */
    /* Start the printing */
    if (StartDoc (printDlg.hDC, &di) != SP_ERROR)
    {
        char buffer [meBUF_SIZE_MAX];
        int bindex = 0;
        int charSet = 0;                /* The current character set */
        int lineNo = 0;
        int colNo = 0;
        unsigned char cc;
        int charIdx = 0;

        /* Read to the end of the file */
        for(;;)
        {
            /* Advance the line pointer if no characters left */
            while (charIdx >= ihead->length)
            {
                meLine *lp = ihead;
                /* Move to the next line and destruct. */
                charIdx = 0;
                ihead = meLineGetNext(ihead);
                meFree (lp);
                if (ihead == NULL)
                    break;
            }
            if(ihead == NULL)
                break ;

            /* Get the next character */
            cc = ihead->text [charIdx++];

            /* Handle any commands - Escape charcter */
            if ((cc == 0x1b) && (charIdx < ihead->length))
            {
                FlushBuffer();

                /* Get the command character */
                cc = ihead->text [charIdx++];

                /* Change Font or color */
                if((cc == 'c') || (cc == 'f'))
                {
                    int ll ;
                    for(;;)
                    {
                        ll = (cc == 'c') ? 3:1 ;
                        if((charIdx+ll) > ihead->length)
                        {
                            mlwrite (0, "[Illegal printer character Esc-%02x]", cc);
                            break ;
                        }
                        if(cc == 'c')
                        {
                            /* change the color */
                            cc = ihead->text [charIdx++];
                            ll = (hexToNum(cc)) << 4 ;
                            cc = ihead->text [charIdx++];
                            ll |= hexToNum(cc) ;
                            cc = ihead->text [charIdx++];
                            WinPrintSetColor(printDlg.hDC,ll,(cc == 'B')) ;
                        }
                        else
                        {
                            /* change the font */
                            cc = ihead->text [charIdx++];
                            if((cc >= 'A') && (cc < ('A'+PFONT_COUNT)))
                            {
                                charSet |= 1 << (cc - 'A') ;
                            }
                            else if((cc >= 'a') && (cc < ('a'+PFONT_COUNT)))
                            {
                                charSet &= ~(1 << (cc - 'a')) ;
                            }
                        }
                        if((charIdx+3 > ihead->length) ||
                           (ihead->text[charIdx] != 0x1b) ||
                           (((cc=ihead->text[charIdx+1]) != 'c') && (cc != 'f')))
                            break ;
                        charIdx += 2 ;
                    }
                    SelectObject (printDlg.hDC, fontTab [charSet]);
                }
                /* End of the page */
                else if (cc == 'e')
                {
                    FlushBuffer();
                    if (EndPage (printDlg.hDC) <= 0)
                        goto error_exit;
                    colNo = 0;
                }
                /* Start of new page */
                else if (cc == 's')
                {
                    /* Flush the buffer - nothing is valid */
                    colNo = 0;
                    bindex = 0;
                    if (StartPage (printDlg.hDC) <= 0)
                        goto error_exit;
                    /* Set the font mapper into text mode whenever we
                     * start a new page. */
                    SetMapMode (printDlg.hDC, MM_TEXT);
                    /* SWP - current font must be preserved across pages
                    SelectObject (printDlg.hDC, fontTab [0]); */
                    lineNo = 0;
                    colNo = 0;
                }
                else
                    mlwrite (0, "[Illegal printer character Esc-%02x]", cc);
                continue;
            }
            /* New line */
            else if (cc == meCHAR_NL)
            {
                FlushBuffer();
                lineNo++;
                colNo = 0;
            }
            /* Backspace */
            else if (cc == 0x08)
            {
                FlushBuffer();
                if (colNo > 0)
                    colNo = -1;
            }
            /* Tab */
            else if (cc == 0x9)
            {
                /* Assume 8 tab stops */
                int tablen;
                tablen = 8 - (colNo%8);
                while (--tablen >= 0)
                    buffer [bindex++] = ' ';
            }
            else
            {
                /* Handle the below 32 characters */
		if((meSystemCfg & meSYSTEM_FONTFIX) && ((cc & 0xe0) == 0))
		    cc = ttSpeChars [cc];

                /* SWP - removed OEM -> ANSI font printing fix */
                buffer [bindex++] = cc;
            }
        }

        /* The last command that we performed should have been an
         * end of page. Simply close the document. */

        if (EndDoc (printDlg.hDC) <= 0)
            goto error_exit;
        goto dlg_exit;
    }

error_exit:
    MessageBox (hDlgCancel, "Printer Job Unsucessful", title, MB_ICONEXCLAMATION|MB_OK);

dlg_exit:
    /* Abort dialogue is up, destruct the dialogue and enable main window. */
/*    EnableWindow (curHwnd, meTRUE);*/
    DestroyWindow (hDlgCancel);
    /* free of colorTbl etc */
    WinPrintSetColor(NULL,-1,0) ;

quick_exit:
    /* Close fdown the devide */
    if (printDlg.hDC != NULL)
    {
        DeleteDC (printDlg.hDC);
        printDlg.hDC = NULL;
    }

    /* Destruct the line list if it already exists. */
    while (ihead != NULL)
    {
        meLine *lp = ihead;
        ihead = meLineGetNext (ihead);
        meFree (lp);
    }

    /* Destruct the font table */
    destructFontTable (fontTab);

    /* Reset the spooler. */
    printStatus &= ~PRINT_SPOOLING;
    printJob = NULL;
}

/*
 * WinPrint
 * Print the specified file to the printer.
 *
 * Handle the print job. THIS IS A THREAD PROCESS.
 * All error reporting we do in here is through the windows
 * messagebox - DO NOT use any Microemacs error reporting
 * in here.
 *
 * NOTE: This is not a thread at the moment !!
 */
int
WinPrint (meUByte *docName, meLine *ihead)
{
    static char *args[2];
    DWORD threadId;

    /* Make sure we are not already running a print spool. */
    if (printStatus & PRINT_SPOOLING)
        return mlwrite (MWABORT, "Print spooler currently processing a job in the background");

    /* Create a thread to spool to the printer */
    args[0] = (char *)(docName);
    args[1] = (char *)(ihead);

    if (!CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) WinPrintThread,
                       (LPVOID) args, 0, &threadId))
        return mlwrite (MWABORT, "Cannot create printing thread");

    return meTRUE;
}

/*
 * printSetup.
 * Use a dialogue for the user to set up the printer.
 */
int
printSetup (int n)
{
    /* Construct the default registry entry for windows */
    if((regPrint = regFind (NULL,regPrintName)) == NULL)
        regPrint = regSet (NULL, regPrintName, NULL);
    if (regPrint == NULL)
        return mlwrite (MWABORT, "[Cannot locate print registry]");

    /* Make sure we are not already running a print spool. */
    if ((printStatus & PRINT_SPOOLING) && (n >= 0))
        return mlwrite (MWABORT, "Print spooler currently processing a job in the background");

    if ((n == -2) || (n == 0))
    {
        /* Set up the dialogue box and return the status.
         * Note that we explicitly test the return status for meTRUE which is
         * returned if the dialogue 'Print' button is pressed. Any other value
         * that is returned indicates that the 'Cancel' button was pressed or
         * the dialogue could not be created (-1). */
        HWND curHwnd ;

#ifdef _ME_CONSOLE
#ifdef _ME_WINDOW
        if (meSystemCfg & meSYSTEM_CONSOLE)
#endif /* _ME_WINDOW */
            curHwnd = meHWndNull ;
#ifdef _ME_WINDOW
        else
#endif /* _ME_WINDOW */
#endif /* _ME_CONSOLE */
#ifdef _ME_WINDOW
            curHwnd = meFrameGetWinHandle(frameCur) ;
#endif /* _ME_WINDOW */

        if (DialogBox (ttInstance,
                       MAKEINTRESOURCE (IDD_PRINTER_CONFIGURATION),
                       curHwnd,
                       (DLGPROC) pcdDialogue) != meTRUE)
            return meFALSE;
    }
    else if(n > 0)
    {
        /* Dialog box not required, simply setup the printer */
        /* Copy the new font into the registry immediatly */
        printer.param [mePI_FONTFACE].p = "Courier New" ;

        /* Compute the size of the font */
        pcdComputeFont (&pd,
                        printer.param [mePI_PAGEX].l, printer.param[mePI_SPECX].l,
                        printer.param [mePI_PAGEY].l, printer.param[mePI_SPECY].l,
                        10);
    }
    return meTRUE;
}

#endif
