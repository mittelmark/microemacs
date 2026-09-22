/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * eopt.h - &opt optional functionality labels.
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
 * Created:     Sat Jan 19 2002
 * Synopsis:    &opt optional functionality labels.
 * Authors:     Steve Phillips
 * Description:
 *      Creates an meOptionList string defining all optional components
 *      enabled in the current build.
 */

#ifndef __EOPT_H
#define __EOPT_H

/* List of available options	*/
meUByte meOptionList[]=
#if MEOPT_ABBREV
    "abb\n"
#endif
#if MEOPT_CALLBACK
    "cal\n"
#endif
#if MEOPT_CLIENTSERVER
    "cli\n"
#endif
#if MEOPT_COLOR
    "col\n"
#endif
#if MEOPT_CRYPT
    "cry\n"
#endif
#if MEOPT_DEBUGM
    "deb\n"
#endif
#if MEOPT_DIRLIST
    "dir\n"
#endif
#if MEOPT_EXTENDED
    "ext\n"
#endif
#if MEOPT_FENCE
    "fen\n"
#endif
#if MEOPT_FILEHOOK
    "fho\n"
#endif
#if MEOPT_FRAME
    "fra\n"
#endif
#if MEOPT_CMDHASH
    "has\n"
#endif
#if MEOPT_HILIGHT
    "hil\n"
#endif
#if MEOPT_HSPLIT
    "hsp\n"
#endif
#if MEOPT_IPIPES
    "ipi\n"
#endif
#if MEOPT_ISEARCH
    "ise\n"
#endif
#if MEOPT_LOCALBIND
    "lbi\n"
#endif
#if MEOPT_MAGIC
    "mag\n"
#endif
#if MEOPT_MOUSE
    "mou\n"
#endif
#if MEOPT_MWFRAME
    "mwf\n"
#endif
#if MEOPT_NARROW
    "nar\n"
#endif
#if MEOPT_FILENEXT
    "nex\n"
#endif
#if MEOPT_OSD
    "osd\n"
#endif
#if MEOPT_POSITION
    "pos\n"
#endif
#if MEOPT_PRINT
    "pri\n"
#endif
#if MEOPT_RCS
    "rcs\n"
#endif
#if MEOPT_REGISTRY
    "reg\n"
#endif
#if MEOPT_SCROLL
    "scr\n"
#endif
#if MEOPT_SOCKET
    "soc\n"
#endif
#if MEOPT_SPAWN
    "spa\n"
#endif
#if MEOPT_SPELL
    "spe\n"
#endif
#if MEOPT_TAGS
    "tag\n"
#endif
#if MEOPT_TIMSTMP
    "tim\n"
#endif
#if MEOPT_TYPEAH
    "typ\n"
#endif
#if MEOPT_UNDO
    "und\n"
#endif
#if MEOPT_WORDPRO
    "wor\n"
#endif
#if MEOPT_XFT
    "xft\n"
#endif
;

#endif /* __EOPT_H */
