# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# freebsd.mak - Make file for FreeBSD using gcc
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
# Synopsis:    Make file for FreeBSD using gcc
# Notes:
#   Run "make -f freebsd.mak"            -> .freebsd-release-mecw/mecw
#   Run "make -f freebsd.mak BTYP=c"     -> .freebsd-release-mec/mec
#   Run "make -f freebsd.mak BTYP=w"     -> .freebsd-release-mew/mew
#   Run "make -f freebsd.mak BTYP=cw"    -> both (default)
#   Run "make -f freebsd.mak BCFG=debug" -> .freebsd-debug-*
#   Run "make -f freebsd.mak BCOR=ne"    -> NanoEmacs
#   Run "make -f freebsd.mak XFT=0|1"    -> force/disable libXft
#                                           (auto-detect when unset)
#
#   Legacy targets still work: mec, mew, mecw, med, medc, etc. (sub-make)
#
#   Run "make -f freebsd.mak clean"      -> removes .freebsd-* dirs
#   Run "make -f freebsd.mak spotless"   -> clean even more
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
RMDIR         = rm -r -f
CC            = gcc
LD            = $(CC)
STRIP         =	strip
INSTALL       =	install

CDEBUG        =	-Wall -g
COPTIMISE     =	-Wall -O3 -DNDEBUG=1 -Wno-uninitialized -Wno-unused-result
CCDEFS        = -D_FREEBSD -D_LINUX26 -I. -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DMEOPT_BINFS -D_64BIT
CONSOLE_DEFS  = -D_ME_CONSOLE
CONSOLE_LIBS  = -lncurses
NANOEMACS_DEFS= -D_NANOEMACS
LDDEBUG       =
LDOPTIMISE    =
LDFLAGS       =
LIBS          = -lz
WINDOW_X11LIB = $(MAKEWINLIBS) -L/usr/local/lib -lX11

BTYP          ?= cw
BCFG          ?= release
BCOR          ?= me

# Normalise BTYP before OUTDIR is computed
.if ${BTYP} != c && ${BTYP} != w
BTYP = cw
.endif

#
# libXft TrueType support - auto-detect when XFT is unset.
# Requires pkgconf + libXft, e.g.: pkg install pkgconf libXft
# Override: XFT=0 (core fonts) or XFT=1 (force Xft), or set
# XFT_DEFS/XFT_LIBS manually as before.
#
PKG_CONFIG   ?= pkg-config
.if !defined(XFT) && !defined(XFT_DEFS)
XFT_OK != if ${PKG_CONFIG} --exists xft >/dev/null 2>&1; then echo 1; else echo 0; fi
.if ${XFT_OK} == 1
XFT = 1
.else
XFT = 0
.endif
.endif
# Xft compile/link flags only when window code is part of this build
.if ${BTYP} != c && defined(XFT) && ${XFT} == 1 && !defined(XFT_DEFS)
XFT_CFLAGS != ${PKG_CONFIG} --cflags xft
XFT_LIBS   != ${PKG_CONFIG} --libs xft
XFT_DEFS    = -DMEOPT_XFT=1 ${XFT_CFLAGS}
.endif
WINDOW_DEFS   = $(MAKEWINDEFS) $(XFT_DEFS) -D_ME_WINDOW -I/usr/local/include
WINDOW_LIBS   = $(WINDOW_X11LIB) $(XFT_LIBS)

.if ${BTYP} == c
BTYP_CDF = $(CONSOLE_DEFS)
BTYP_LIB = $(CONSOLE_LIBS)
.elif ${BTYP} == w
BTYP_CDF = $(WINDOW_DEFS)
BTYP_LIB = $(WINDOW_LIBS)
.else
BTYP_CDF = $(CONSOLE_DEFS) $(WINDOW_DEFS)
BTYP_LIB = $(CONSOLE_LIBS) $(WINDOW_LIBS)
.endif

.if ${BCOR} == ne
BCOR_CDF = $(NANOEMACS_DEFS)
.else
BCOR = me
BCOR_CDF =
.endif

.if ${BCFG} == debug
BOUTDIR  = .freebsd-debug
CCFLAGS  = $(CDEBUG)
LDFLAGS  = $(LDDEBUG)
STRIP    = - echo No strip - debug
.else
BOUTDIR  = .freebsd-release
CCFLAGS  = $(COPTIMISE)
.endif

# Separate object dir for Xft - make does not track flag changes
.if ${BTYP} != c && defined(XFT) && ${XFT} == 1
XFT_OUTDIR = -xft
.else
XFT_OUTDIR =
.endif

OUTDIR   = $(BOUTDIR)-$(BCOR)$(BTYP)$(XFT_OUTDIR)
PRGNAME  = $(BCOR)$(BTYP)
PRGFILE  = $(PRGNAME)
PRGHDRS  = ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h \
	   esearch.h eskeys.h estruct.h eterm.h evar.h evers.h eopt.h \
	   ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def bfs.h

STDSRC	= abbrev.c basic.c bind.c buffer.c crypt.c dirlist.c display.c \
	  encoding.c eval.c exec.c file.c fileio.c frame.c hilight.c history.c input.c \
	  isearch.c key.c line.c macro.c main.c narrow.c next.c osd.c \
	  print.c random.c regex.c region.c registry.c search.c spawn.c \
	  spell.c tag.c termio.c time.c undo.c window.c word.c bfs.c
PLTSRC  = unixterm.c
SRC     = $(STDSRC) $(PLTSRC)

#
# Default goal must be declared before the .for rules below so that a bare
# "make -f freebsd.mak" builds the combined binary, not the first object.
#
all: $(OUTDIR)/$(PRGFILE)

.for _src in ${SRC:S/.c//}
OBJS += ${OUTDIR}/${_src}.o
.endfor

.for _src in ${SRC:S/.c//}
${OUTDIR}/${_src}.o: ${_src}.c ${PRGHDRS} freebsd.mak
	@mkdir -p $(OUTDIR)
	$(CC) $(CCDEFS) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) -o $@ -c ${_src}.c
.endfor

$(OUTDIR)/$(PRGFILE): $(OBJS)
	@mkdir -p $(OUTDIR)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o $@ $(OBJS) $(PRGLIBS) $(BTYP_LIB) $(LIBS)
	$(STRIP) $@

install: $(OUTDIR)/$(PRGFILE)
	$(INSTALL) $(INSTPROGFLAGS) $(OUTDIR)/$(PRGFILE) $(INSTDIR)/me
	@echo "install done"

clean:
	$(RMDIR) .freebsd-release-* .freebsd-debug-*
	$(RM) core me mec mew mecw ne nec new necw med medc medw medcw ned nedc nedw nedcw
	$(RM) *.oc *.ow *.ob *.on *.ov *.oe
	$(RM) *.odc *.odw *.odb *.odn *.odv *.ode
	$(RM) *.o

spotless: clean
	$(RM) tags *~

# Legacy convenience targets (sub-make; primary output is $(OUTDIR)/$(PRGFILE))
mec:
	${MAKE} -f freebsd.mak BTYP=c BCFG=$(BCFG) BCOR=$(BCOR) all

mew:
	${MAKE} -f freebsd.mak BTYP=w BCFG=$(BCFG) BCOR=$(BCOR) XFT=$(XFT) all

mecw:
	${MAKE} -f freebsd.mak BTYP=cw BCFG=$(BCFG) BCOR=$(BCOR) XFT=$(XFT) all

me:
	${MAKE} -f freebsd.mak BTYP=cw BCFG=$(BCFG) BCOR=$(BCOR) XFT=$(XFT) all
	$(CP) $(BOUTDIR)-$(BCOR)cw$(XFT_OUTDIR)/$(BCOR)cw \
	      $(BOUTDIR)-$(BCOR)cw$(XFT_OUTDIR)/me

# NanoEmacs legacy
nec:
	${MAKE} -f freebsd.mak BTYP=c BCFG=$(BCFG) BCOR=ne all

new:
	${MAKE} -f freebsd.mak BTYP=w BCFG=$(BCFG) BCOR=ne XFT=$(XFT) all

necw:
	${MAKE} -f freebsd.mak BTYP=cw BCFG=$(BCFG) BCOR=ne XFT=$(XFT) all

ne: nec

# Debug legacy
medc:
	${MAKE} -f freebsd.mak BTYP=c BCFG=debug BCOR=$(BCOR) all

medw:
	${MAKE} -f freebsd.mak BTYP=w BCFG=debug BCOR=$(BCOR) XFT=$(XFT) all

medcw:
	${MAKE} -f freebsd.mak BTYP=cw BCFG=debug BCOR=$(BCOR) XFT=$(XFT) all

med: medcw

nedc:
	${MAKE} -f freebsd.mak BTYP=c BCFG=debug BCOR=ne all

nedw:
	${MAKE} -f freebsd.mak BTYP=w BCFG=debug BCOR=ne XFT=$(XFT) all

nedcw:
	${MAKE} -f freebsd.mak BTYP=cw BCFG=debug BCOR=ne XFT=$(XFT) all

ned: nedc
