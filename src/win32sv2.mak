# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32sv2.mak - Make file for Windows using Microsoft MSVC v2.0 development kit.
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
#	Run "c:\msvc20\bin\nmake -f win32sv2.mak"      for optimised build produces ./me32.exe
#	Run "c:\msvc20\bin\nmake -f win32sv2.mak mec"  for console support          ./mec32.exe
#	Run "c:\msvc20\bin\nmake -f win32sv2.mak med"  for debug build produces     ./med32.exe
#
#	Run "c:\msvc20\bin\nmake -f win32sv2.mak clean"      to clean source directory
#	Run "c:\msvc20\bin\nmake -f win32sv2.mak spotless"   to clean source directory even more
#
#       Note that the explicit cl/rc/link paths have been used on this makefile
#       this is not really necessary, it is used here to allow the V2.0 compiler
#       to co-exist with later versions of the compiler.
#
##############################################################################
#
# Microsoft MSCV 5.0 install directory
TOOLSDIR      = c:\msvc20
LIB	      = $(TOOLSDIR)\lib
#
# Installation Directory
INSTDIR	      = c:\emacs
INSTPROGFLAGS =
#
# Local Definitions
RM            = erase
RC            =	$(TOOLSDIR)\bin\rc
CC            =	$(TOOLSDIR)\bin\cl
LD            =	$(TOOLSDIR)\bin\link
INSTALL       =	copy
CDEBUG        =	-nologo -G5 -W3 -GX -Z7 -YX -Yd -Od
COPTIMISE     =	-nologo -G5 -YX -GX -O2 -DNDEBUG=1
CPROFILE      =	-nologo -G5 -YX -GX
CDEFS         = -D_WIN32s -I. "-I$(TOOLSDIR)\include"
LDFLAGS       = /NOLOGO /INCREMENTAL:no /MACHINE:IX86 /PDB:NONE
LIBS          = $(TOOLSDIR)\lib\libc.lib \
		$(TOOLSDIR)\lib\user32.lib \
		$(TOOLSDIR)\lib\gdi32.lib \
		$(TOOLSDIR)\lib\winspool.lib \
		$(TOOLSDIR)\lib\comdlg32.lib \
		$(TOOLSDIR)\lib\advapi32.lib \
		$(TOOLSDIR)\lib\shell32.lib \
		$(TOOLSDIR)\lib\oldnames.lib \
		$(TOOLSDIR)\lib\kernel32.lib \
		win32s\w32sut32.lib
#
# Rules
.SUFFIXES: .c .o .od .on .ou .onu .rc .res .on .ov .oe

.c.o:
	$(CC) $(COPTIMISE) $(CDEFS) -D_ME_WINDOW -c $< -Fo$*.o
.c.oc:
	$(CC) $(COPTIMISE) $(CDEFS) -D_ME_CONSOLE -c $< -Fo$*.oc
.c.ou:
	$(CC) $(COPTIMISE) $(CDEFS) -D_ME_WINDOW -D_SOCKET -c $< -Fo$*.ou
.c.oa:
	$(CC) $(COPTIMISE) $(CDEFS) -D_ME_CONSOLE -D_SOCKET -c $< -Fo$*.oa
.c.od:
	$(CC) $(CDEBUG) $(CDEFS) -D_ME_WINDOW -c $< -Fo$*.od
.c.on:
	$(CC) $(COPTIMISE) $(CDEFS) -D_NANOEMACS -D_ME_CONSOLE -c $< -Fo$*.on
.c.ov:	
	$(CC) $(COPTIMISE) $(CDEFS) -D_NANOEMACS -D_ME_WINDOW -c $< -Fo$*.ov
.c.oe:	
	$(CC) $(COPTIMISE) $(CDEFS) -D_NANOEMACS -D_ME_WINDOW -D_ME_CONSOLE -c $< -Fo$*.oe
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
PLTNRC  = nemsvc20.rc
PLTNCRC = ncmsvc20.rc
#
# Object files
STDOBJ	= $(STDSRC:.c=.o)
PLTOBJ  = $(PLTSRC:.c=.o)
OBJ	= $(STDOBJ) $(PLTOBJ)

CSTDOBJ	= $(STDSRC:.c=.oc)
CPLTOBJ = $(PLTSRC:.c=.oc)
COBJ	= $(CSTDOBJ) $(CPLTOBJ)

USTDOBJ	= $(STDSRC:.c=.ou)
UPLTOBJ = $(PLTSRC:.c=.ou)
UOBJ	= $(USTDOBJ) $(UPLTOBJ)

ASTDOBJ = $(STDSRC:.c=.oa)
APLTOBJ = $(PLTSRC:.c=.oa)
AOBJ	= $(ASTDOBJ) $(APLTOBJ)

DSTDOBJ	= $(STDSRC:.c=.od)
DPLTOBJ = $(PLTSRC:.c=.od)
DOBJ	= $(DSTDOBJ) $(DPLTOBJ)

NSTDOBJ	= $(STDSRC:.c=.on)
NPLTOBJ = $(PLTSRC:.c=.on)
NOBJ	= $(NSTDOBJ) $(NPLTOBJ)

VSTDOBJ	= $(STDSRC:.c=.ov)
VPLTOBJ = $(PLTSRC:.c=.ov)
VOBJ	= $(VSTDOBJ) $(VPLTOBJ)

ESTDOBJ	= $(STDSRC:.c=.oe)
EPLTOBJ = $(PLTSRC:.c=.oe)
EOBJ	= $(ESTDOBJ) $(EPLTOBJ)

PLTRES  = $(PLTRC:.rc=.res)
PLTNCRES = $(PLTNCRC:.rc=.res)
PLTNRES  = $(PLTNRC:.rc=.res)
#
# Targets
all: me

install: me
	$(INSTALL) $(INSTPROGFLAGS) me $(INSTDIR)
	@echo "install done"

clean:
	- $(RM) me32.exe
	- $(RM) med32.exe
	- $(RM) mec32.exe
	- $(RM) meu32.exe
	- $(RM) mea32.exe
	- $(RM) new32.exe		
	- $(RM) nec32.exe		
	- $(RM) *.o
	- $(RM) *.ov
	- $(RM) *.od
	- $(RM) *.oe
	- $(RM) *.on
	- $(RM) *.ou
	- $(RM) *.onu
	- $(RM) *.res
	- $(RM) me.aps
	- $(RM) me.map
	- $(RM) $(PLTRES)

spotless: clean
	- $(RM) *.opt
	- $(RM) *.pch
	- $(RM) tags
	- $(RM) *~

me:	me32.exe
me32.exe: $(OBJ) $(PLTRES)
	$(LD) $(LDFLAGS) /SUBSYSTEM:windows /out:$@ $(OBJ) $(PLTRES) $(LIBS)

mec:	mec32.exe
mec32.exe: $(COBJ) $(PLTRES)
	$(LD) $(LDFLAGS) /SUBSYSTEM:console /out:$@ $(COBJ) $(PLTRES) $(LIBS)

meu:	meu32.exe
meu32.exe: $(UOBJ) $(PLTRES)
	$(LD) $(LDFLAGS) /SUBSYSTEM:windows /out:$@ $(UOBJ) $(PLTRES) $(TOOLSDIR)\lib\wsock32.lib $(LIBS)

mea:	mea32.exe
mea322.exe: $(AOBJ) $(PLTRES)
	$(LD) $(LDFLAGS) /SUBSYSTEM:console /out:$@ $(AOBJ) $(PLTRES) $(TOOLSDIR)\lib\wsock32.lib $(LIBS)

med:	med32.exe
med32.exe: $(DOBJ) $(PLTRES)
	$(LD) $(LDFLAGS) /DEBUG /SUBSYSTEM:windows /out:$@ $(DOBJ) $(PLTRES) $(LIBS)
nec:	nec32.exe
nec32.exe: $(NOBJ) $(PLTNCRES)
	$(LD) $(LDFLAGS) /SUBSYSTEM:console /out:$@ $(NOBJ) $(PLTNCRES) $(LIBS)

new:	new32.exe
new32.exe: $(VOBJ) $(PLTNRES)
	$(LD) $(LDFLAGS) /SUBSYSTEM:windows /out:$@ $(VOBJ) $(PLTNRES) $(LIBS)
#
# Dependancies
$(STDOBJ): $(STDHDR)
$(PLTOBJ): $(STDHDR) $(PLTHDR)

$(CSTDOBJ): $(STDHDR)
$(CPLTOBJ): $(STDHDR) $(PLTHDR)

$(USTDOBJ): $(STDHDR)
$(UPLTOBJ): $(STDHDR) $(PLTHDR)

$(ASTDOBJ): $(STDHDR)
$(APLTOBJ): $(STDHDR) $(PLTHDR)

$(DSTDOBJ): $(STDHDR)
$(DPLTOBJ): $(STDHDR) $(PLTHDR)

$(NSTDOBJ): $(STDHDR)
$(NPLTOBJ): $(STDHDR) $(PLTHDR)

$(ESTDOBJ): $(STDHDR)
$(EPLTOBJ): $(STDHDR) $(PLTHDR)

$(VSTDOBJ): $(STDHDR)
$(VPLTOBJ): $(STDHDR) $(PLTHDR)
