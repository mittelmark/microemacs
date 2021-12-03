# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32sv4.mak - Make file for MicroEmacs win32s using Microsoft MSVC v4.0 development kit.
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
# Synopsis:    Make file for MicroEmacs win32s using Microsoft MSVC v4.0 development kit.
# Notes:
#       Build from the command line using nmake. 
#
#	Run "nmake -f win32sv4.mak"      for optimised build produces ./me32s.exe
#
#	Run "nmake -f win32sv4.mak clean"      to clean source directory
#	Run "nmake -f win32sv4.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Microsoft MSCV 4.0 install directory
TOOLSDIR=       c:\progra~1\DevStudio\vc
INCLUDE=	$(TOOLSDIR)\include
LIBRARY=	$(TOOLSDIR)\lib
TOOLSBIN=	$(TOOLSDIR)\bin

#
# Destination directory.
#
BINDIR	=	c:\emacs

#
# Standard Compilation tools and flags
#
CC	=	$(TOOLSBIN)\cl
RC	=	rc
RM	=	erase
LD	=	$(TOOLSBIN)\link
INSTALL	=	copy
MKDIR	=	mkdir

# Standard File Extensions
I	=	-I
O	=	.o
EXE	=	.exe

# Extended build rules.
.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $< -Fo$*.o

#
# Standard include stuff
#
CDEBUGFLAGS	=	-nologo -W3 -G3 -Z7 -YX -Yd -Od
CPROFILEFLAGS	=	-nologo -YX -G3
COPTIMISEFLAGS	=	-nologo -YX -G3 -O2 -DNDEBUG=1
CNONOPTIMISEFLAGS=	-nologo -YX -G3

LDDEBUGFLAGS	=	/DEBUG
LDOPTIMISEFLAGS	=	/DEBUG

CINCLUDES	=	$(I). $(I)$(INCLUDE)
#CDEFS		=	$(CDEBUGFLAGS)
CDEFS		=	$(COPTIMISEFLAGS)
CFLAGS		=	$(CINCLUDES) $(CDEFS) -D_WIN32s
#LDFLAGS	=	$(LDDEBUGFLAGS) /NOLOGO /SUBSYSTEM:windows /INCREMENTAL:no /MACHINE:IX86 /PDB:NONE
LDFLAGS		=	$(LDOPTIMISEFLAGS) /NOLOGO /SUBSYSTEM:windows /INCREMENTAL:no /MACHINE:IX86 /PDB:NONE

SYSLIBS         =	$(LIBRARY)\kernel32.lib	\
			$(LIBRARY)\wsock32.lib	\
			$(LIBRARY)\libc.lib	\
			$(LIBRARY)\user32.lib	\
			$(LIBRARY)\gdi32.lib	\
			$(LIBRARY)\winmm.lib	\
                        $(LIBRARY)\wsock32.lib	\
                        $(LIBRARY)\winspool.lib	\
                        $(LIBRARY)\comdlg32.lib	\
                        $(LIBRARY)\advapi32.lib	\
                        $(LIBRARY)\shell32.lib	\
                        $(LIBRARY)\ole32.lib	\
                        $(LIBRARY)\oleaut32.lib	\
                        $(LIBRARY)\uuid.lib	\
                        $(LIBRARY)\msvcrt.lib	\
                        $(LIBRARY)\oldnames.lib
                  
#
# Define the files involved in the build.
#
HEADERS = ebind.h edef.h eextrn.h efunc.h esearch.h estruct.h evar.h \
	  evers.h eopt.h eterm.h ebind.def efunc.def evar.def emain.h eprint.h
PLTHDR  = wintermr.h
STDSRC	= abbrev.c basic.c bind.c buffer.c crypt.c dirlist.c display.c \
	  eval.c exec.c file.c fileio.c frame.c hilight.c history.c input.c \
	  isearch.c key.c line.c macro.c main.c narrow.c next.c osd.c \
	  print.c registry.c random.c region.c search.c spawn.c spell.c \
	  tag.c termio.c time.c undo.c window.c word.c regex.c
PLTSRC  = winterm.c winprint.c

STDOBJ	= $(STDSRC:.c=.o)	
PLTOBJ  = $(PLTSRC:.c=.o)
OBJ	= $(STDOBJ) $(PLTOBJ)

#
# Standard build rules
#
#methnk32.dll
all::	me32s$(EXE) win32s/w32sut32.lib
me32s$(EXE):$(OBJ) me.res
	$(LD) $(LDFLAGS) /out:$@ $(OBJ) me.res $(SYSLIBS) win32s/w32sut32.lib

$(STDOBJ): $(HEADERS)
$(PLTOBJ): $(HEADERS) $(PLTHDR)

me.res:
	$(RC) -v -i . -i $(INCLUDE) -i $(TOOLSDIR)\mfc\include -fo $@ $*.rc 
relink::
	$(RM)	me32s$(EXE)
	$(MAKE) -f $(MAKEFILE) $(MFLAGS) me32s$(EXE)

install:: me32s$(EXE)
	$(INSTALL) me32s$(EXE) $(BINDIR)
	@echo Install done.

spotless:: clean
	$(RM) *.??~
clean::
	- $(RM) *.pch
	- $(RM) *.o
	- $(RM) *.exe
	- $(RM) *.res
