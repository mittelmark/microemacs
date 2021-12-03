# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32v2.mak - Make file for Windows using Microsoft MSVC v2.0 development kit.
#
# Copyright (C) 2001-2009 JASSPA (www.jasspa.com)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Mass Ave, Cambridge, MA 02139, USA.
#
##############################################################################
#
# Created:     Sat Jan 24 1998
# Synopsis:    Make file for Windows using Microsoft MSVC v2.0 development kit.
# Notes:
#       Build from the command line using nmake.
#
#	Run "c:\msvc20\bin\nmake -f win32v2.mak"      for optimised build produces ./me32.exe
#	Run "c:\msvc20\bin\nmake -f win32v2.mak mec"  for console support          ./mec32.exe
#	Run "c:\msvc20\bin\nmake -f win32v2.mak med"  for debug build produces     ./med32.exe
#
#	Run "c:\msvc20\bin\nmake -f win32v2.mak clean"      to clean source directory
#	Run "c:\msvc20\bin\nmake -f win32v2.mak spotless"   to clean source directory even more
#
#       Note that the explicit cl/rc/link paths have been used on this makefile
#       this is not really necessary, it is used here to allow the V2.0 compiler
#       to co-exist with later versions of the compiler.
#
##############################################################################
#
# Microsoft MSCV 2.0 install directory
TOOLSDIR      = c:\msvc20
LIB	      = $(TOOLSDIR)\lib
#
# Installation Directory
INSTDIR	      = c:\emacs
INSTPROGFLAGS =
#
# Local Definitions
RM            = - erase
RC            =	$(TOOLSDIR)\bin\rc
CC            =	$(TOOLSDIR)\bin\cl
LD            =	$(TOOLSDIR)\bin\link
INSTALL       =	copy
CDEBUG        =	-nologo -G5 -W3 -GX -Z7 -YX -Yd -Od
COPTIMISE     =	-nologo -G5 -YX -GX -O2 -DNDEBUG=1
CPROFILE      =	-nologo -G5 -YX -GX
CDEFS         = -D_WIN32 -I. "-I$(TOOLSDIR)\include"
LDFLAGS       = /NOLOGO /INCREMENTAL:no /MACHINE:IX86 /PDB:NONE
LIBS          = $(TOOLSDIR)\lib\libc.lib \
		$(TOOLSDIR)\lib\oldnames.lib \
		$(TOOLSDIR)\lib\user32.lib \
		$(TOOLSDIR)\lib\gdi32.lib \
		$(TOOLSDIR)\lib\winspool.lib \
		$(TOOLSDIR)\lib\comdlg32.lib \
		$(TOOLSDIR)\lib\advapi32.lib \
		$(TOOLSDIR)\lib\shell32.lib \
		$(TOOLSDIR)\lib\kernel32.lib
#
# Rules
.SUFFIXES: .c .o .od .on .ou .onu .rc .res

.c.o:
	$(CC) $(COPTIMISE) $(CDEFS) -c $< -Fo$*.o
.c.on:
	$(CC) $(COPTIMISE) $(CDEFS) -D_WINCON -c $< -Fo$*.on
.c.ou:
	$(CC) $(COPTIMISE) $(CDEFS) -D_SOCKET -c $< -Fo$*.ou
.c.onu:
	$(CC) $(COPTIMISE) $(CDEFS) -D_WINCON -D_SOCKET -c $< -Fo$*.onu
.c.od:
	$(CC) $(CDEBUG) $(CDEFS) -c $< -Fo$*.od

.rc.res:
	$(RC) -v -i . -i "$(TOOLSDIR)\include" -i "$(TOOLSDIR)\mfc\include" -fo $@ $*.rc
#
# Source files
STDHDR	= ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h \
	  esearch.h eskeys.h estruct.h eterm.h evar.h evers.h eopt.h \
	  ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def
STDSRC	= abbrev.c basic.c bind.c buffer.c crypt.c dirlist.c display.c \
	  eval.c exec.c file.c fileio.c frame.c hilight.c history.c input.c \
	  isearch.c key.c line.c macro.c main.c narrow.c next.c osd.c \
	  print.c random.c regex.c region.c registry.c search.c spawn.c \
	  spell.c tag.c termio.c time.c undo.c window.c word.c

PLTHDR  = wintermr.h
PLTSRC  = winterm.c winprint.c
PLTRC   = memsvc20.rc
#
# Object files
STDOBJ	= $(STDSRC:.c=.o)
PLTOBJ  = $(PLTSRC:.c=.o)
OBJ	= $(STDOBJ) $(PLTOBJ)

NSTDOBJ	= $(STDSRC:.c=.on)
NPLTOBJ = $(PLTSRC:.c=.on)
NOBJ	= $(NSTDOBJ) $(NPLTOBJ)

USTDOBJ	= $(STDSRC:.c=.ou)
UPLTOBJ = $(PLTSRC:.c=.ou)
UOBJ	= $(USTDOBJ) $(UPLTOBJ)

NUSTDOBJ = $(STDSRC:.c=.onu)
NUPLTOBJ = $(PLTSRC:.c=.onu)
NUOBJ	= $(NUSTDOBJ) $(NUPLTOBJ)

DSTDOBJ	= $(STDSRC:.c=.od)
DPLTOBJ = $(PLTSRC:.c=.od)
DOBJ	= $(DSTDOBJ) $(DPLTOBJ)

PLTRES  = $(PLTRC:.rc=.res)
#
# Targets
all: me

install: me
	$(INSTALL) $(INSTPROGFLAGS) me $(INSTDIR)
	@echo "install done"

clean:
	$(RM) me32.exe
	$(RM) med32.exe
	$(RM) mec32.exe
	$(RM) meu32.exe
	$(RM) mecu32.exe
	$(RM) *.o
	$(RM) *.od
	$(RM) *.on
	$(RM) *.ou
	$(RM) *.onu
	$(RM) *.res
	$(RM) me.aps
	$(RM) me.map
	$(RM) $(PLTRES)

spotless: clean
	$(RM) *.opt
	$(RM) *.pch
	$(RM) tags
	$(RM) *~

me:	me32.exe
me32.exe: $(OBJ) $(PLTRES)
	$(LD) $(LDFLAGS) /SUBSYSTEM:windows /out:$@ $(OBJ) $(PLTRES) $(LIBS)

mec:	mec32.exe
mec32.exe: $(NOBJ) $(PLTRES)
	$(LD) $(LDFLAGS) /SUBSYSTEM:console /out:$@ $(NOBJ) $(PLTRES) $(LIBS)

meu:	meu32.exe
meu32.exe: $(UOBJ) $(PLTRES)
	$(LD) $(LDFLAGS) /SUBSYSTEM:windows /out:$@ $(UOBJ) $(PLTRES) $(TOOLSDIR)\lib\wsock32.lib $(LIBS)

mecu:	mecu32.exe
mecu32.exe: $(NUOBJ) $(PLTRES)
	$(LD) $(LDFLAGS) /SUBSYSTEM:console /out:$@ $(NUOBJ) $(PLTRES) $(TOOLSDIR)\lib\wsock32.lib $(LIBS)

med:	med32.exe
med32.exe: $(DOBJ) $(PLTRES)
	$(LD) $(LDFLAGS) /DEBUG /SUBSYSTEM:windows /out:$@ $(DOBJ) $(PLTRES) $(LIBS)
#
# Dependancies
$(STDOBJ): $(STDHDR)
$(PLTOBJ): $(STDHDR) $(PLTHDR)

$(NSTDOBJ): $(STDHDR)
$(NPLTOBJ): $(STDHDR) $(PLTHDR)

$(USTDOBJ): $(STDHDR)
$(UPLTOBJ): $(STDHDR) $(PLTHDR)

$(NUSTDOBJ): $(STDHDR)
$(NUPLTOBJ): $(STDHDR) $(PLTHDR)

$(DSTDOBJ): $(STDHDR)
$(DPLTOBJ): $(STDHDR) $(PLTHDR)
