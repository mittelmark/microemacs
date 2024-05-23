dict=enus
nodename=$(shell uname -n)
os=ubuntu-20-x86_64
ifeq ($(nodename),bariuke)
	os := fedora-38-x86_64
endif
ifeq ($(nodename),ubuntu18)
	os=ubuntu-18-x86_64
endif

ifeq ($(nodename),guitar)
	os=fedora-30-x86_64
endif
ifeq ($(nodename),micky)
	os=fedora-39-x86_64
endif
#kernel=$(shell uname -a | perl -pe 's/.+[uU]buntu-?([0-9]+).+/ubuntu$$1/; s/.+fc([0-9]+).+/fedora$$1/; s/.+(2[0-9])\..+Ubuntu.+/ubuntu$$1/; s/.+(el[8-9])_([0-9]+).+/el${1}_$2/')
kernel=$(shell uname -a | sed -E 's/.+fc([0-9]+).+/fedora\1/' | sed -E 's/.+[uU]buntu-?([0-9]+).+/ubuntu\1/;' | sed -E 's/.+(el[8-9])_([0-9]+).+/el\1_\2/')
version=091223
kernelname=$(shell uname -s | sed 's/Linux/linux/')
machine=$(uname -m)
default:
	echo "Makefile for Linux systems"
	echo "Usage make bfs-bin|me-bin|me-standalone"
	echo $(kernel)
bfs-bin:
	-mkdir bin
	cd bfs && make
	cp bfs/bfs bin/
	
bfs-win-bin:
	-mkdir bin
	cd bfs && make clean
	cd bfs && make -f Makefile.mingw CC=i686-w64-mingw32-gcc
	cp bfs/bfs.exe bin/
me-bin:
	cd src && make -f linux32gcc.gmk 
	cd src && make -f linux32gcc.gmk BTYP=c
	#cd src && make -f linux32gcc.gmk BTYP=w
	cp src/.linux32gcc-release-mecw/mecw bin/mew
	cp src/.linux32gcc-release-mec/mec bin/
	#cp src/.linux32gcc-release-mew/mew bin/	

me-bfs-linux:
	-rm -rf me-bfs/*
	-mkdir me-bfs
	-mkdir me-bfs/jasspa
	-mkdir me-bfs/jasspa/spelling
	cp -r jasspa/macros me-bfs/jasspa/
	rm -f me-bfs/jasspa/macros/*~
	rm -f me-bfs/jasspa/macros/*.bak
	-rm me-bfs/jasspa/macros/null
	#cp -r jasspa/contrib me-bfs/jasspa/
	cp jasspa/spelling/*$(dict)*f me-bfs/jasspa/spelling/
	cd me-bfs && ../bin/bfs -a ../src/.linux32gcc-release-mec/mec -o ../mec-linux.bin ./jasspa
	cd me-bfs && ../bin/bfs -a ../src/.linux32gcc-release-mecw/mecw -o ../mew-linux.bin ./jasspa	
	#cd me-bfs && ../bin/bfs -a ../src/.linux32gcc-release-mew/mew-o ../mew-linux.bin ./jasspa	
	cp mec-linux.bin mec-$(os).bin
	cp mew-linux.bin mew-$(os).bin	
	#cp mecw-linux.bin mecw-$(os).bin		
	cd me-bfs && ../bin/bfs -c macros-`date +%Y-%m-%d`.bfs ./jasspa
	cp me-bfs/macros-`date +%Y-%m-%d`.bfs .

me-bfs-bin: me-bfs-linux
	cd me-bfs && ../bin/bfs -a ../src/.win32mingw-release-mew/mew32.exe -o ../mew-bfs-windows.exe ./jasspa
	cd me-bfs && ../bin/bfs -a ../src/.win32mingw-release-mec/mec32.exe -o ../mec-bfs-windows.exe ./jasspa
	#rm -rf me-bfs/*
mingw-w32-compile:
	cd src && make -f win32mingw.mak CC=i686-w64-mingw32-gcc RC=i686-w64-mingw32-windres
	cd src && make -f win32mingw.mak CC=i686-w64-mingw32-gcc RC=i686-w64-mingw32-windres BTYP=c
mingw-w32-run:	
	cd src && MEPATH=Z:/home/groth/workspace/microemacs/jasspa/macros wine ./.win32mingw-release-mew/mew32.exe
app-image:
	chmod 755 jme.AppDir/AppRun
	rm -rf jasspa-bfs
	cp -r jasspa jasspa-bfs
	cp -r jasspa/spelling jme.AppDir/usr/share/
	rm -rf jasspa-bfs/contrib
	rm -rf jasspa-bfs/company
	rm -rf jasspa-bfs/pixmaps
	rm -rf jasspa-bfs/spelling
	rm -f jasspa-bfs/macros/*~
	rm -f jasspa-bfs/macros/*.bak
	./bin/bfs -a bin/mecw-ubuntu-18 -o jme.AppDir/usr/bin/jme ./jasspa-bfs
	appimagetool-x86_64.AppImage jme.AppDir
	./Jasspa_MicroEmacs-x86_64.AppImage -V -n
	#rm -rf jasspa-bfs
docu-html:
	tclsh bin/ehf2md.tcl jasspa/macros/me.ehf files.txt htm
	for file in `ls htm/*.md` ; do pandoc $$file -f gfm -o htm/`basename $$file .md`.htm -s --css null.css; done
run-tuser:
	MENAME=tuser MEPATH=`pwd`/tuser:`pwd`/jasspa/macros src/.linux32gcc-release-mecw/mecw
folder-unix:
	-mkdir MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mecb
	-mkdir MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mewb	
	cp mec-linux.bin MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mecb/mecb
	cp mew-linux.bin MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mewb/mewb
	cp bfs/bfs-readme.md bin/bfs license.txt README.txt MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mecb/ 	
	cp bfs/bfs-readme.md bin/bfs license.txt README.txt MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mewb/ 		
	cp doc/me.1 MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mewb/
	cp doc/me.1 MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mecb/
zip-unix: folder-unix
	zip MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mewb.zip MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mewb/*
	zip MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mecb.zip MicroEmacs09_$(version)_$(kernelname)-$(kernel)-$(machine)mecb/*	
folder-windows:
	-mkdir MicroEmacs09_$(version)_windows-32-mecb
	-mkdir MicroEmacs09_$(version)_windows-32-mewb	
	cp mew-bfs-windows.exe MicroEmacs09_$(version)_windows-32-mewb/mewb.exe
	cp mec-bfs-windows.exe MicroEmacs09_$(version)_windows-32-mecb/mewb.exe	
	cp bfs/bfs-readme.md bin/bfs.exe README.txt license.txt MicroEmacs09_$(version)_windows-32-mewb/
	cp bfs/bfs-readme.md bin/bfs.exe README.txt license.txt MicroEmacs09_$(version)_windows-32-mecb/
	cp doc/me.1 MicroEmacs09_$(version)_windows-32-mewb/
	cp doc/me.1 MicroEmacs09_$(version)_windows-32-mecb/
zip-windows: folder-windows
	zip MicroEmacs09_$(version)_windows-32-mewb.zip MicroEmacs09_$(version)_windows-32-mewb/*
	zip MicroEmacs09_$(version)_windows-32-mecb.zip MicroEmacs09_$(version)_windows-32-mecb/*
me-ehf:
	cd doc && make ehf
	cp doc/me.ehf jasspa/macros/
