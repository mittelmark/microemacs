# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32bc.mak - Make file for Windows using Borland C++ Builder 3
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
# Synopsis:    Make file for Windows using Borland C++ Builder 3
# Notes:
#	Run "make -f win32bc.mak"      for optimised build produces ./me32.exe
#	Run "make -f win32bc.mak mec"  for console support          ./mec32.exe
#	Run "make -f win32bc.mak med"  for debug build produces     ./med32.exe
#
#	Run "make -f win32bc.mak clean"      to clean source directory
#	Run "make -f win32bc.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Borland C++ install directory (progra~1 == "Program Files")
TOOLSDIR      = c:\progra~1\borland\cbuild~1
#
# Installation Directory
INSTDIR	      = c:\emacs
INSTPROGFLAGS = 
#
# Local Definitions
CP            = copy
RM            = - erase
RC            =	brc32
CC            =	bcc32
LD            =	tlink32
INSTALL       =	copy
CDEBUG        =	-N -v
COPTIMISE     =	-N -O2 -DNDEBUG=1
CDEFS         = -D_WIN32 -w-par -I. -I$(TOOLSDIR)\include
CONSOLE_DEFS  = -D_ME_CONSOLE
WINDOW_DEFS   = -D_ME_WINDOW
MICROEMACS_DEFS= -D_SOCKET
NANOEMACS_DEFS= -D_NANOEMACS
LDDEBUG       =	
LDOPTIMISE    =	
LDFLAGS       = -v -L$(TOOLSDIR)\lib -Tpe -x -v
LIBS          = import32.lib cw32mt.lib
CONSOLE_LIBS  = 
WINDOW_LIBS   = 
#
# Rules
.SUFFIXES: .rc .res .c .oc .ow .ob .on .ov .oe .odc .odw .odb .odn .odv .ode

.rc.res:	
	$(RC) -v -i . -i "$(TOOLSDIR)\include" -i "$(TOOLSDIR)\include\mfc" -fo $@ $< 

.c.oc:
	$(CC) $(COPTIMISE) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o$@ -c $<

.c.ow:	
	$(CC) $(COPTIMISE) $(CDEFS) $(MICROEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o$@ -c $<

.c.ob:	
	$(CC) $(COPTIMISE) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o$@ -c $<

.c.on:
	$(CC) $(COPTIMISE) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o$@ -c $<

.c.ov:	
	$(CC) $(COPTIMISE) $(CDEFS) $(NANOEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o$@ -c $<

.c.oe:	
	$(CC) $(COPTIMISE) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o$@ -c $<

# Debug Builds
.c.odc:
	$(CC) $(CDEBUG) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o$@ -c $<

.c.odw:	
	$(CC) $(CDEBUG) $(CDEFS) $(MICROEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o$@ -c $<

.c.odb:	
	$(CC) $(CDEBUG) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o$@ -c $<

.c.odn:
	$(CC) $(CDEBUG) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o$@ -c $<

.c.odv:	
	$(CC) $(CDEBUG) $(CDEFS) $(NANOEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o$@ -c $<

.c.ode:	
	$(CC) $(CDEBUG) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o$@ -c $<
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
	$(RM) tags
	$(RM) *~

mec:	mec32.exe
mec32.exe: $(OBJ_C) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -ap "$(TOOLSDIR)\lib\c0x32.obj" $(OBJ_C), $@,, $(CONSOLE_LIBS) $(LIBS),, $(PLTMRES)

mew:	mew32.exe
mew32.exe: $(OBJ_W) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -aa "$(TOOLSDIR)\lib\c0w32.obj" $(OBJ_W), $@,, $(WINDOW_LIBS) $(LIBS),, $(PLTMRES)

mecw:	mecw32.exe
mecw32.exe: $(OBJ_B) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -ap "$(TOOLSDIR)\lib\c0x32.obj" $(OBJ_B), $@,, $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS),, $(PLTMRES)

me:	me32.exe
me32.exe: mew32.exe
	$(CP) mew32.exe $@

nec:	nec32.exe
nec32.exe: $(OBJ_N) $(PLTNCRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -ap "$(TOOLSDIR)\lib\c0x32.obj" $(OBJ_N), $@,, $(CONSOLE_LIBS) $(LIBS),, $(PLTNCRES)

new:	new32.exe
new32.exe: $(OBJ_V) $(PLTNRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -aa "$(TOOLSDIR)\lib\c0w32.obj" $(OBJ_V), $@,, $(WINDOW_LIBS) $(LIBS),, $(PLTNRES)

necw:	necw32.exe
necw32.exe: $(OBJ_E) $(PLTNRES)
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -ap "$(TOOLSDIR)\lib\c0x32.obj" $(OBJ_E), $@,, $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS),, $(PLTNRES)

ne:	ne32.exe
ne32.exe: nec32.exe
	$(CP) nec32.exe $@
#
# Debug Builds
medc:	medc32.exe
medc32.exe: $(OBJ_DC) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) -ap "$(TOOLSDIR)\lib\c0x32.obj" $(OBJ_DC), $@,, $(CONSOLE_LIBS) $(LIBS),, $(PLTMRES)

medw:	medw32.exe
medw32.exe: $(OBJ_DW) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) -aa "$(TOOLSDIR)\lib\c0w32.obj" $(OBJ_DW), $@,, $(WINDOW_LIBS) $(LIBS),, $(PLTMRES)

medcw:	medcw32.exe
medcw32.exe: $(OBJ_DB) $(PLTMRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) -ap "$(TOOLSDIR)\lib\c0x32.obj" $(OBJ_DB), $@,, $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS),, $(PLTMRES)

med:	med32.exe
med32.exe: medw32.exe
	$(CP) medw32.exe $@

nedc:	nedc32.exe
nedc32.exe: $(OBJ_DN) $(PLTNCRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) -ap "$(TOOLSDIR)\lib\c0x32.obj" $(OBJ_DN), $@,, $(CONSOLE_LIBS) $(LIBS),, $(PLTNCRES)

nedw:	nedw32.exe
nedw32.exe: $(OBJ_DV) $(PLTNRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) -aa "$(TOOLSDIR)\lib\c0w32.obj" $(OBJ_DV), $@,, $(WINDOW_LIBS) $(LIBS),, $(PLTNRES)

nedcw:	nedcw32.exe
nedcw32.exe: $(OBJ_DE) $(PLTNRES)
	$(LD) $(LDFLAGS) $(LDDEBUG) -ap "$(TOOLSDIR)\lib\c0x32.obj" $(OBJ_DE), $@,, $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS),, $(PLTNRES)

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
