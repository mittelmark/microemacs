/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * evers.h - The build version definition.
 *
 * Copyright (C) 1992-2009 JASSPA (www.jasspa.com)
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
 * Created:     1st May 1992
 * Synopsis:    The build version definition.
 * Authors:     Jon Green & Steven Phillips
 * Description:
 *     These are put in here so that you wont have to grub around in the depths
 *     of display.c & main.c etc every time you want to remind yourself that
 *     you've hacked emacs by updating the version number.
 *
 * Notes:
 *     The definitions have been moved out of estruct.h, because, if you change
 *     them and you have written your makefile properly, everything gets recompiled
 *     whereas putting them here only makes "make" recompile display.c
 * 
 *     The history was removed from this file as CVS and the help pages are used
 *     instead.
 */

/*
 * MicroEmacs 09 release information.
 *
 * meYEAR is the stable product line; meCALYEAR, meMONTH and meDAY form
 * the build date.  Stable builds use a day of 01.  Development/preview
 * builds use the actual preview day and set mePREVIEW.
 */
#define meCENTURY       "20"
#define meYEAR          "09"
#define meCALYEAR       "2026"
#define meMONTH         "11"
#define meDAY           "02"
#define mePREVIEW       "-preview1"
#define meGITHUB        "github.com/mittelmark/microemacs"
#define meAUTHOR        "D.Groth"
#define meRELEASE       "091102-preview1"
#define meDISPLAY_VERSION meCALYEAR "." meMONTH mePREVIEW
/* Version information - Date of build */
#define meDATE          meCALYEAR "/" meMONTH "/" meDAY mePREVIEW
/* Legacy major/minor values retained for existing macro and INI code */
#define meVERSION       meYEAR
#define meVERSION_MINOR meMONTH
/* Public macro version code */
#define meVERSION_CODE  meCALYEAR meMONTH meDAY mePREVIEW

/* The program names - these values are also used in the rc files */
#define ME_MICROEMACS_FULLNAME  "JASSPA MicroEmacs 09"
#define ME_MICROEMACS_SHORTNAME "me"
#define ME_NANOEMACS_FULLNAME   "NanoEmacs"
#define ME_NANOEMACS_SHORTNAME  "ne"

#define ME_COMPANY_NAME         "JASSPA"
#define ME_COMPANY_SITE         "www.jasspa.com"
