##version=091224b1
OS=`uname -o | sed 's/FreeBSD/freebsd/'`
VERSION=`grep -E 'meYEAR|meMONTH|meDAY' src/evers.h  | head -n 3 | grep -Eo '[0-9b]+' | perl -ane 'print $$F[0]'`
OSV=`grep -E 'VERSION_ID=' /etc/os-release | sed -E 's/VERSION_ID="?([0-9]+).*"?/\1/' | sed -e 's/25/14'`
KERNEL=`uname -r | grep -oE '^[0-9]'`
OSVERSION="$(DIST)$(OSV)"
MACHINE=`uname -m`
RELEASE=freebsd-$(OSV)-$(MACHINE)-microemacs-$(VERSION)
app=mecb
default:
	echo "Makefile for FreeBSD systems"
	echo "VERSION: '$(VERSION)' OS $(OS) OSVERSION $(OSVERSION) OSV $(OSV)"
bfs/bin:
	cd bfs && make
mec:
	cd src && make -f freebsd.mak mec

mew:
	cd src && make -f freebsd.mak mew

mecw:
	cd src && make -f freebsd.mak mecw

mecb: bfs/bin mec
	./bfs/bfs -a ./src/mec -o $(RELEASE)-mecb.bin ./jasspa

mewb: bfs/bin mew
	./bfs/bfs -a ./src/mew -o $(RELEASE)-mewb.bin ./jasspa

mecwb: bfs/bin mecw
	./bfs/bfs -a ./src/mecw -o $(RELEASE)-mecwb.bin ./jasspa

release: $(app)
	gmake -f release.gmk app=$(app) RELEASE=$(RELEASE) ext=bin
