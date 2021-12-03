/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * methnk16.c - Windows 16-bit thunking code.
 *
 * Copyright (C) 1999-2002 JASSPA (www.jasspa.com)
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
 * Created:     Sun Jan 3 15:09:43 1999
 * Synopsis:    16-bit thunking code - code for 16-bit side of thunk.
 * Authors:     Jon Green
 * Description:
 *     We have to make an explicit call to  change directory on the 16-bit
 *     side as it would appear that the 16 and 32-bit sides have a different
 *     concept of the current working directory. This is a bloody mess really
 *     - I personally blame Microsoft.
 *
 *     Requires linking with TOOLHELP.LIB, for ModuleFindHandle().
 */

#ifndef APIENTRY
#define APIENTRY
#endif

#define SYNCHSPAWN     1                /* Our command number */
#define W32SUT_16      1                /* Needed for w32sut.h in 16-bit code */

#include <windows.h>
#include <toolhelp.h>
/*#include <malloc.h>*/
/*#include <stdlib.h>*/
/*#include <stdio.h>*/
/*#include <ctype.h>*/
/*#include <string.h>*/
/*#include <direct.h>*/
#include "w32sut.h"
/*#include "methunk.h"*/

UT16CBPROC glpfnUT16CallBack;

/**************************************************************************
* Function: LRESULT CALLBACK LibMain(HANDLE, WORD, WORD, LPSTR)           *
*                                                                         *
* Purpose: DLL entry point                                                *
**************************************************************************/

int FAR PASCAL
LibMain (HANDLE hLibInst, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{
    return (1);
} /* LibMain() */

/**************************************************************************
* Function: DWORD FAR PASCAL UTInit(UT16CBPROC, LPVOID)                   *
*                                                                         *
* Purpose: Universal Thunk initialization procedure                       *
**************************************************************************/
DWORD FAR PASCAL
UTInit (UT16CBPROC lpfnUT16CallBack, LPVOID lpBuf )
{
   glpfnUT16CallBack = lpfnUT16CallBack;
   return(1);   /* Return Success */
} /* UTInit() */

/**************************************************************************
* Function: DWORD FAR PASCAL UTProc(LPVOID, DWORD)                        *
*                                                                         *
* Purpose: Dispatch routine called by 32-bit UT DLL                       *
**************************************************************************/
DWORD FAR PASCAL
UTProc (LPVOID lpBuf, DWORD dwFunc)
{
   switch (dwFunc)
   {
      case SYNCHSPAWN:
      {
          UINT hInst;
          LPCSTR lpszCmdLine;
          UINT nCmdShow;

          /* Retrieve the command line arguments stored in buffer */
          lpszCmdLine = (LPSTR) ((LPDWORD)lpBuf)[0];
          nCmdShow = (UINT) ((LPDWORD)lpBuf)[1];

          /* Start the application with WinExec(). Note that there is a massive bug
           * with WIN32s in that the environment is not inherited by the command shell.
           * hence all work with win32s is done through a BAT file. To add to our wows
           * CreateProcess() in the win32 environment cannot determine when the spawned
             process has finished. Hence this 16-bit thunk. */

          hInst = WinExec (lpszCmdLine, nCmdShow);
          if( hInst < 32 )
             return 0;                  /* Error - cannot spawn process */
          else
          {
              TASKENTRY te;
              int found = 0;
              te.dwSize = sizeof (TASKENTRY);

              /* Iterate until the process has finished */
              while (found == 0)
              {
                  found = 1;
                  /* Give processing resouce back to windows */
                  Yield ();

                  /* See if we can find the process. If not then we have finished */
                  if (TaskFirst (&te) == FALSE)
                      break;
                  do
                  {
                      /* Found the task; back to the top */
                      if (te.hInst == hInst)
                      {
                          found = 0;
                          break;
                      }
                  }
                  while (TaskNext (&te));
              }
          }
          return 1;                     /* OK status */
      }
   } /* switch (dwFunc) */

   return( (DWORD)-1L ); /* We should never get here. */
} /* UTProc()  */

/**************************************************************************
* Function: int FAR PASCAL _WEP(int)                                      *
*                                                                         *
* Purpose: Windows exit procedure                                         *
**************************************************************************/
int FAR PASCAL
WEP (int bSystemExit)
{
    return (1);
} /* WEP() */
