# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32v5.mak - Make file for Windows using Microsoft MSVC v5.0 development kit.
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
# Synopsis:    Make file for Windows using Microsoft MSVC v5.0 development kit.
# Notes:
#       Build from the command line using nmake. 
#
#	Run "nmake -f win32v5.mak"      for optimised build produces ./me32.exe
#	Run "nmake -f win32v5.mak mec"  for console support          ./mec32.exe
#	Run "nmake -f win32v5.mak med"  for debug build produces     ./med32.exe
#
#	Run "nmake -f win32v5.mak clean"      to clean source directory
#	Run "nmake -f win32v5.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Microsoft MSCV 5.0 install directory
TOOLSDIR      = c:\Program Files\DevStudio\vc
#
# Installation Directory
INSTDIR	      = c:\emacs
INSTPROGFLAGS = 
#
# Local Definitions
CP            = copy
RM            = - erase
RC            =	rc
CC            =	cl
LD            =	link
INSTALL       =	copy
CDEBUG        =	-nologo -G5 -W3 -GX -Z7 -YX -Yd -Od -MLd -D_DEBUG
COPTIMISE     =	-nologo -G5 -YX -GX -O2 -DNDEBUG=1
CDEFS         = -D_WIN32 -I. "-I$(TOOLSDIR)\include"
CONSOLE_DEFS  = -D_ME_CONSOLE
WINDOW_DEFS   = -D_ME_WINDOW
MICROEMACS_DEFS= -D_SOCKET
NANOEMACS_DEFS= -D_NANOEMACS
LDDEBUG       =	/DEBUG
LDOPTIMISE    =	
LDFLAGS       = /NOLOGO /INCREMENTAL:no /MACHINE:IX86 /PDB:NONE "/LIBPATH:$(TOOLSDIR)\lib"
LIBS          = shell32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
CONSOLE_LIBS  = 
WINDOW_LIBS   = 
#
# Rules
.SUFFIXES: .rc .res .c .oc .ow .ob .on .ov .oe .odc .odw .odb .odn .odv .ode 

.rc.res:	
	$(RC) -v -i . -i "$(TOOLSDIR)\include" -i "$(TOOLSDIR)\mfc\include" -fo $@ $*.rc 

.c.oc:
	$(CC) $(COPTIMISE) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -c $< -Fo$@

.c.ow:	
	$(CC) $(COPTIMISE) $(CDEFS) $(MICROEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -c $< -Fo$@

.c.ob:	
	$(CC) $(COPTIMISE) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -c $< -Fo$@

.c.on:
	$(CC) $(COPTIMISE) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -c $< -Fo$@

.c.ov:	
	$(CC) $(COPTIMISE) $(CDEFS) $(NANOEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -c $< -Fo$@

.c.oe:	
	$(CC) $(COPTIMISE) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -c $< -Fo$@

# Debug Builds
.c.odc:
	$(CC) $(CDEBUG) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -c $< -Fo$@

.c.odw:	
	$(CC) $(CDEBUG) $(CDEFS) $(MICROEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -c $< -Fo$@

.c.odb:	
	$(CC) $(CDEBUG) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -c $< -Fo$@

.c.odn:
	$(CC) $(CDEBUG) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -c $< -Fo$@

.c.odv:	
	$(CC) $(CDEBUG) $(CDEFS) $(NANOEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -c $< -Fo$@

.c.ode:	
	$(CC) $(CDEBUG) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -c $< -Fo$@
#
# Source files
PLTMRC  = me.rc
PLTNRC  = ne.rc
PLTNCRC = nec.rc
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

HEADERS = $(STDHDR) $(PLTHDR)
SRC     = $(STDSRC) $(PLTSRC)
#
# Object files
PLTMRES  = $(PLTMRC:.rc=.res)
PLTNRES  = $(PLTNRC:.rc=.res)
PLTNCRES = $(PLTNCRC:.rc=.res)

OBJ_C    = $(SRC:.c=.oc)
OBJ_W    = $(SRC:.c=.ow)
OBJ_B    = $(SRC:.c=.ob)
OBJ_N    = $(SRC:.c=.on)
OBJ_V    = $(SRC:.c=.ov)
OBJ_E    = $(SRC:.c=.oe)

# Debug Builds
OBJ_DC   = $(SRC:.c=.odc)
OBJ_DW   = $(SRC:.c=.odw)
OBJ_DB   = $(SRC:.c=.odb)
OBJ_DN   = $(SRC:.c=.odn)
OBJ_DV   = $(SRC:.c=.odv)
OBJ_DE   = $(SRC:.c=.ode)
#
# Targets
all: me

install: me
	$(INSTALL) $(INSTPROGFLAGS) me $(INSTDIR)
	@echo "install done"

clean:
	$(RM) me.aps
	$(RM) me.map
	$(RM) $(PLTMRES)
	$(RM) $(PLTNRES)
	$(RM) $(PLTNCRES)
	$(RM) me32.exe
	$(RM) mec32.exe
	$(RM) mew32.exe
	$(RM) mecw32.exe
	$(RM) ne32.exe
	$(RM) nec32.exe
	$(RM) new32.exe
	$(RM) necw32.exe
	$(RM) med32.exe
	$(RM) medc32.exe
	$(RM) medw32.exe
	$(RM) medcw32.exe
	$(RM) ned32.exe
	$(RM) nedc32.exe
	$(RM) nedw32.exe
	$(RM) nedcw32.exe
	$(RM) *.oc
	$(RM) *.ow
	$(RM) *.ob
	$(RM) *.on
	$(RM) *.ov
	$(RM) *.oe
	$(RM) *.odc
	$(RM) *.odw
	$(RM) *.odb
	$(RM) *.odn
	$(RM) *.odv
	$(RM) *.ode

spotless: clean
	$(RM) *.opt
	$(RM) *.pch
	$(RM) tags
	$(RM) *~

mec:	mec32.exe
mec32.exe: $(OBJ_C) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) /SUBSYSTEM:console /out:$@ $(OBJ_C) $(PLTMRES) $(CONSOLE_LIBS) $(LIBS)

mew:	mew32.exe
mew32.exe: $(OBJ_W) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) /SUBSYSTEM:windows /out:$@ $(OBJ_W) $(PLTMRES) $(WINDOW_LIBS) $(LIBS)

mecw:	mecw32.exe
mecw32.exe: $(OBJ_B) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) /SUBSYSTEM:console /out:$@ $(OBJ_B) $(PLTMRES) $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS)

me:	me32.exe
me32.exe: mew32.exe
	$(CP) mew32.exe $@

nec:	nec32.exe
nec32.exe: $(OBJ_N) $(PLTNCRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) /SUBSYSTEM:console /out:$@ $(OBJ_N) $(PLTNCRES) $(CONSOLE_LIBS) $(LIBS)

new:	new32.exe
new32.exe: $(OBJ_V) $(PLTNRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) /SUBSYSTEM:windows /out:$@ $(OBJ_V) $(PLTNRES) $(WINDOW_LIBS) $(LIBS)

necw:	necw32.exe
necw32.exe: $(OBJ_E) $(PLTNRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) /SUBSYSTEM:console /out:$@ $(OBJ_E) $(PLTNRES) $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS)

ne:	ne32.exe
ne32.exe: nec32.exe
	$(CP) nec32.exe $@
#
# Debug Builds
medc:	medc32.exe
medc32.exe: $(OBJ_DC) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) /SUBSYSTEM:console /out:$@ $(OBJ_DC) $(PLTMRES) $(CONSOLE_LIBS) $(LIBS)

medw:	medw32.exe
medw32.exe: $(OBJ_DW) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) /SUBSYSTEM:windows /out:$@ $(OBJ_DW) $(PLTMRES) $(WINDOW_LIBS) $(LIBS)

medcw:	medcw32.exe
medcw32.exe: $(OBJ_DB) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) /SUBSYSTEM:console /out:$@ $(OBJ_DB) $(PLTMRES) $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS)

med:	med32.exe
med32.exe: medw32.exe
	$(CP) medw32.exe $@

nedc:	nedc32.exe
nedc32.exe: $(OBJ_DN) $(PLTNCRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) /SUBSYSTEM:console /out:$@ $(OBJ_DN) $(PLTNCRES) $(CONSOLE_LIBS) $(LIBS)

nedw:	nedw32.exe
nedw32.exe: $(OBJ_DV) $(PLTNRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) /SUBSYSTEM:windows /out:$@ $(OBJ_DV) $(PLTNRES) $(WINDOW_LIBS) $(LIBS)

nedcw:	nedcw32.exe
nedcw32.exe: $(OBJ_DE) $(PLTNRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) /SUBSYSTEM:console /out:$@ $(OBJ_DE) $(PLTNRES) $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS)

ned:	ned32.exe
ned32.exe: nedc32.exe
	$(CP) nedc32.exe $@
#
# Dependancies
$(OBJ_C): $(HEADERS)
$(OBJ_W): $(HEADERS)
$(OBJ_B): $(HEADERS)
$(OBJ_N): $(HEADERS)
$(OBJ_V): $(HEADERS)
$(OBJ_E): $(HEADERS)

# Debug Builds
$(OBJ_DC): $(HEADERS)
$(OBJ_DW): $(HEADERS)
$(OBJ_DB): $(HEADERS)
$(OBJ_DN): $(HEADERS)
$(OBJ_DV): $(HEADERS)
$(OBJ_DE): $(HEADERS)
