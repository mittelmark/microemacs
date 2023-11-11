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

/* THESE MAY ONLY BE MODIFIED BY JASSPA THEY REPRESENT THE CURRENT 
 * RELEASE OF THE BUILD AND ARE USED AS A REFERENCE POINT WHEN
 * PROBLEMS OCCUR. THESE VALUES ARE NOT INTENDED TO REPRESENT 
 * THE BUILD DATE OF THE SOFTWARE AND MUST NOT BE MODIFIED AS SUCH 
 * Groth: Since 2023 - they are not following the build date, but are
 *        a version indicator, they are not updated, to a new year 
 *        as we have now a version 2023 in preparation. This version will
 *        be called ME 2009 and stick with the year 2009  as the version.
 */
#define meCENTURY       "20"          /* Current century. Y2 complient :-) */
#define meYEAR          "09"          /* Current year */
#define meMONTH         "12"          /* Current month */
#define meDAY           "22"          /* Day of the month */

/* Version information - Date of build */
#define	meDATE          meYEAR "/" meMONTH "/" meDAY 
/* Version information - Major version number is the year of build */
#define	meVERSION       meYEAR 
/* Version information - Minor version number is the month of build */
#define meVERSION_MINOR meMONTH
/* Version information - Version as a numeric date code */
#define meVERSION_CODE  meCENTURY meYEAR meMONTH meDAY 

/* The program names - these values are also used in the rc files */
#define ME_MICROEMACS_FULLNAME  "MicroEmacs"
#define ME_MICROEMACS_SHORTNAME "me"
#define ME_NANOEMACS_FULLNAME   "NanoEmacs"
#define ME_NANOEMACS_SHORTNAME  "ne"

#define ME_COMPANY_NAME         "JASSPA"
#define ME_COMPANY_SITE         "www.jasspa.com"
