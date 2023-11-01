dict=enus
default:
	echo "Makefile for Linux systems"
	echo "Usage make bfs-bin|me-bin|me-standalone"
bfs-bin:
	cd bfs && make
	cp bfs/bfs bin/
me-bin:
	cd src && make -f linux32gcc.gmk
	cp src/.linux32gcc-release-mecw/mecw bin/

me-bfs-linux:
	-rm -rf me-bfs/*
	-mkdir me-bfs
	-mkdir me-bfs/jasspa
	-mkdir me-bfs/jasspa/spelling
	cp -r jasspa/macros me-bfs/jasspa/
	rm -f me-bfs/jasspa/macros/*~
	-rm me-bfs/jasspa/macros/null
	#cp -r jasspa/contrib me-bfs/jasspa/
	cp jasspa/spelling/*$(dict)*f me-bfs/jasspa/spelling/
	cd me-bfs && ../bin/bfs -a ../src/.linux32gcc-release-mecw/mecw -o ../me-linux.bin ./jasspa
	cd me-bfs && ../bin/bfs -c macros-`date +%Y-%m-%d`.bfs ./jasspa
	cp me-bfs/macros-`date +%Y-%m-%d`.bfs .

me-bfs-bin: me-bfs-linux
	cd me-bfs && ../bin/bfs -a ../src/.win32mingw-release-mew/mew32.exe -o ../me-windows.exe ./jasspa
	cd me-bfs && ../bin/bfs -a ../src/.win32mingw-release-mec/mec32.exe -o ../mec-windows.exe ./jasspa
	#rm -rf me-bfs/*
mingw-w32-compile:
	cd src && make -f win32mingw.mak CC=i686-w64-mingw32-gcc RC=i686-w64-mingw32-windres
	cd src && make -f win32mingw.mak CC=i686-w64-mingw32-gcc RC=i686-w64-mingw32-windres BTYP=c
mingw-w32-run:	
	cd src && MEPATH=Z:/home/groth/workspace/microemacs/jasspa/macros wine ./.win32mingw-release-mew/mew32.exe
app-image:
	chmod 755 jme.AppDir/AppRun
	cp -r jasspa jasspa-bfs
	cp jasspa/spelling/* jasspa-bfs/macros/
	rm -rf jasspa-bfs/contrib
	rm -rf jasspa-bfs/company
	rm -rf jasspa-bfs/pixmaps
	rm -rf jasspa-bfs/spelling
	rm -f jasspa-bfs/macros/*~
	./bin/bfs -a bin/mecw-ubuntu-18 -o jme.AppDir/usr/bin/jme ./jasspa-bfs
	appimagetool-x86_64.AppImage jme.AppDir
	./Jasspa_MicroEmacs-x86_64.AppImage -V -n
	rm -rf jasspa-bfs

