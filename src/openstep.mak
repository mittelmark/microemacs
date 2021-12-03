# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# openstep.mak - Make file for Openstep 4.2 on NeXT Motorola 040
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
# Synopsis:    Make file for Openstep 4.2 on NeXT Motorola 040
# Notes:
#       ME's XTerm support has not yet been ported to NeXT so there is no
#       'Window' builds.
# 
#	Run "make -f openstep.mak"      for optimised build produces ./me
#	Run "make -f openstep.mak med"  for debug build produces     ./med
#
#	Run "make -f openstep.mak clean"      to clean source directory
#	Run "make -f openstep.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Installation Directory
INSTDIR	      = /usr/local/bin
INSTPROGFLAGS = -s -o root -g root -m 0775
#
# Local Definitions
CP            = cp
RM            = rm -f
CC            = cc 
# -posix
LD            = $(CC)
STRIP         =	strip
INSTALL       =	install
CDEBUG        =	-g -Wall
COPTIMISE     =	-O
CDEFS         = -D_NEXT -I.
CONSOLE_DEFS  = -D_ME_CONSOLE
NANOEMACS_DEFS= -D_NANOEMACS
LDDEBUG       =
LDOPTIMISE    =
LDFLAGS       =
LIBS          =
CONSOLE_LIBS  = -ltermcap
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
PLTSRC  = unixterm.c

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
	$(RM) core me mec mew mecw ne nec new necw med medc medw medcw ned nedc nedw nedcw
	$(RM) *.oc *.ow *.ob *.on *.ov *.oe
	$(RM) *.odc *.odw *.odb *.odn *.odv *.ode

spotless: clean
	$(RM) tags *~

mec:	$(OBJ_C)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o $@ $(OBJ_C) $(CONSOLE_LIBS) $(LIBS)
	$(STRIP) $@

me:	mec
	$(CP) mec $@

nec:	$(OBJ_N)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o $@ $(OBJ_N) $(CONSOLE_LIBS) $(LIBS)
	$(STRIP) $@

ne:	nec
	$(CP) nec $@

# Debug Builds
medc:	$(OBJ_DC)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDDEBUG) -o $@ $(OBJ_DC) $(CONSOLE_LIBS) $(LIBS)

med:	medc
	$(CP) medc $@

nedc:	$(OBJ_DN)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDDEBUG) -o $@ $(OBJ_DN) $(CONSOLE_LIBS) $(LIBS)

ned:	nedc
	$(CP) nedc $@
#
# Dependancies
$(OBJ_C): $(HEADERS)
$(OBJ_N): $(HEADERS)

# Debug Builds
$(OBJ_DC): $(HEADERS)
$(OBJ_DN): $(HEADERS)

