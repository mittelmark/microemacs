##version=091224b1
OS=`uname -o | sed 's/FreeBSD/freebsd/'`
VERSION := `grep -E '^.define me(YEAR|MONTH|DAY)' src/evers.h | awk '{ print $$$3 }' | paste -sd ':' - | sed 's/"//g; s/://g'` 
OSV=`grep -E 'VERSION_ID=' /etc/os-release | sed -E 's/VERSION_ID="?([0-9]+).*"?/\1/' | sed -e 's/25/14/'`
KERNEL=`uname -r | grep -oE '^[0-9]'`
OSVERSION="$(DIST)$(OSV)"
MACHINE=`uname -m`
RELEASE=freebsd-$(OSV)-$(MACHINE)-microemacs-$(VERSION)
app=mecb
#
# Auto-detect libXft for mew/mecw (pkgconf must be installed).
# Override with XFT=0 or XFT=1 on the make command line.
#
PKG_CONFIG   ?= pkg-config
.if !defined(XFT)
XFT_OK != if ${PKG_CONFIG} --exists xft >/dev/null 2>&1; then echo 1; else echo 0; fi
.if ${XFT_OK} == 1
XFT = 1
.else
XFT = 0
.endif
.endif
default:
	echo "Makefile for FreeBSD systems"
	echo "VERSION: '$(VERSION)' OS $(OS) OSVERSION $(OSVERSION) OSV $(OSV)"
.if defined(XFT) && ${XFT} == 1
	echo "libXft TrueType: enabled (XFT=1)"
.else
	echo "libXft TrueType: not found or disabled (XFT=0)"
.endif
bfs/bin:
	cd bfs && make
mec:
	cd src && make -f freebsd.mak mec

mew:
	cd src && make -f freebsd.mak mew XFT=${XFT}

mecw:
	cd src && make -f freebsd.mak mecw XFT=${XFT}

mecb: bfs/bin mec
	./bfs/bfs -a ./src/mec -o $(RELEASE)-mecb.bin ./jasspa

mewb: bfs/bin mew
	./bfs/bfs -a ./src/mew -o $(RELEASE)-mewb.bin ./jasspa

mecwb: bfs/bin mecw
	./bfs/bfs -a ./src/mecw -o $(RELEASE)-mecwb.bin ./jasspa

release: $(app)
	gmake -f release.gmk app=$(app) RELEASE=$(RELEASE) ext=bin
