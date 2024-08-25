##version=091224b1
OS=$(shell uname -o | sed 's/FreeBSD/freebsd/')
VERSION=$(shell grep -E 'meYEAR|meMONTH|meDAY' src/evers.h | head -n 3 | awk '{ print $$3 }' | paste -sd '-' | sed 's/[-"]//g')
OSV=$(shell grep -E 'VERSION_ID=' /etc/os-release | sed -E 's/VERSION_ID="?([0-9]+).*"?/\1/')
KERNEL=$(shell uname -r | grep -oE '^[0-9]')
DIST=$(shell grep -E '^ID=' /etc/os-release | sed -E 's/ID=//')
OSVERSION="$(DIST)$(OSV)"
RELEASE=freebsd-$(KERNEL)-$(DIST)-$(OSV)-microemacs-$(VERSION)
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
	make -f release.gmk app=$(app) RELEASE=$(RELEASE) ext=bin
