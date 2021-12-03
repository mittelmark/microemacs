# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# dosdj1.mak - Make file for Dos using djgpp v1.0
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
# Synopsis:    Make file for Dos using djgpp v1.0
# Notes:
#       No 'Window' support on dos.
#	DJDIR must be setup to point to your DJGPP installation (normally
#		done in djgpp.env)
#
#	Run "make -f dosdj1.mak"      for optimised build produces ./me.exe
#	Run "make -f dosdj1.mak med"  for debug build produces     ./med.386
#
#	Run "make -f dosdj1.mak clean"      to clean source directory
#	Run "make -f dosdj1.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Installation Directory
INSTDIR		= c:\emacs
INSTPROGFLAGS	= 
#
# Local Definitions
CP            = copy
RM            = del
CC            = gcc
LD            = $(CC)
STRIP         =	strip
GO32          = go32.exe
COFF          = coff2exe
INSTALL       =	copy
CDEBUG        =	-g -Wall
COPTIMISE     =	-O2 -DNDEBUG=1 -Wall -Wno-uninitialized
CDEFS         = -D_DOS -I.
CONSOLE_DEFS  = -D_ME_CONSOLE
NANOEMACS_DEFS= -D_NANOEMACS
LDDEBUG       =
LDOPTIMISE    =	
LDFLAGS       = 
LIBS          = -lpc 
CONSOLE_LIBS  =
#
# Rules
.SUFFIXES: .c .oc .on .odc .odn

.c.oc:
	$(CC) $(COPTIMISE) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o $@ -c $<

.c.on:
	$(CC) $(COPTIMISE) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o $@ -c $<

# Debug Builds
.c.odc:	
	$(CC) $(CDEBUG) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o $@ -c $<

.c.odn:	
	$(CC) $(CDEBUG) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o $@ -c $<
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

PLTHDR  = 
PLTSRC  = dosterm.c

HEADERS = $(STDHDR) $(PLTHDR)
SRC     = $(STDSRC) $(PLTSRC)
#
# Object files
OBJ_C    = $(SRC:.c=.oc)
OBJ_N    = $(SRC:.c=.on)

# Debug Builds
OBJ_DC   = $(SRC:.c=.odc)
OBJ_DN   = $(SRC:.c=.odn)
#
# Targets
all: me

install: me
	$(INSTALL) $(INSTPROGFLAGS) me $(INSTDIR)
	@echo "install done"

clean:
	$(RM) me.exe
	$(RM) mec.exe
	$(RM) mec.386
	$(RM) ne.exe
	$(RM) nec.exe
	$(RM) nec.386
	$(RM) med.exe
	$(RM) medc.exe
	$(RM) medc.386
	$(RM) ned.exe
	$(RM) nedc.exe
	$(RM) nedc.386
	$(RM) *.oc
	$(RM) *.on
	$(RM) *.odc
	$(RM) *.odn

spotless: clean
	$(RM) tags
	$(RM) *~

mec: mec.exe
mec.exe: $(OBJ_C)
	$(RM) mec.386
	$(RM) mec.exe
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o mec.386 $(OBJ_C) $(CONSOLE_LIBS) $(LIBS)
	$(STRIP) mec.386
	$(COFF) -s $(DJDIR)/bin/$(GO32) mec.386

me: me.exe
me.exe:	mec.exe
	$(CP) mec.exe $@

nec:	nec.exe
nec.exe: $(OBJ_N)
	$(RM) nec.386
	$(RM) nec.exe
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o nec.386 $(OBJ_N) $(CONSOLE_LIBS) $(LIBS)
	$(STRIP) nec.386
	$(COFF) -s $(DJDIR)/bin/$(GO32) nec.386

ne: ne.exe
ne.exe:	nec.exe
	$(CP) nec.exe $@
#
# Debug Builds
medc: medc.exe
medc.exe: $(OBJ_DC)
	$(RM) medc.386
	$(RM) medc.exe
	$(LD) $(LDFLAGS) $(LDDEBUG) -o medc.386 $(OBJ_DC) $(CONSOLE_LIBS) $(LIBS)
	$(COFF) -s $(DJDIR)/bin/$(GO32) medc.386

med: med.exe
med.exe: medc.exe
	$(CP) medc.exe $@

nedc: nedc.exe
nedc.exe: $(OBJ_DN)
	$(RM) nedc.386
	$(RM) nedc.exe
	$(LD) $(LDFLAGS) $(LDDEBUG) -o nedc.386 $(OBJ_DN) $(CONSOLE_LIBS) $(LIBS)
	$(STRIP) nedc.386
	$(COFF) -s $(DJDIR)/bin/$(GO32) nedc.386

ned: ned.exe
ned.exe: nedc.exe
	$(CP) nedc.exe $@

#
# Dependancies
$(OBJ_C): $(HEADERS)
$(OBJ_N): $(HEADERS)

# Debug Builds
$(OBJ_DC): $(HEADERS)
$(OBJ_DN): $(HEADERS)

