dict=enus
default:
	echo "Makefile for Linux systems"
	echo "Usage make bfs-bin|me-bin|me-standalone"
bfs-bin:
	cd bfs && make
	cp bfs/bfs bin/
me-bin:
	cd src && make -f linux26.gmk
	cp src/me bin/

me-bfs-bin:
	-mkdir me-bfs
	-mkdir me-bfs/jasspa
	-mkdir me-bfs/jasspa/spelling
	cp -r jasspa/macros me-bfs/jasspa/macros
	rm -f me-bfs/jasspa/macros/*~
	-rm me-bfs/jasspa/macros/null
	cp -r jasspa/contrib me-bfs/jasspa/
	cp jasspa/spelling/*$(dict)*f me-bfs/jasspa/spelling/
	cd me-bfs && ../bin/bfs -a ../src/mecw -o ../me-linux.bin ./jasspa
	cd me-bfs && ../bin/bfs -a ../src/.win32mingw-release-mew/mew32.exe -o ../me-windows.exe ./jasspa
	cd me-bfs && ../bin/bfs -a ../src/.win32mingw-release-mec/mec32.exe -o ../mec-windows.exe ./jasspa
	cd me-bfs && ../bin/bfs -c macros-`date +%Y-%m-%d`.bfs ./jasspa
	cp me-bfs/macros-`date +%Y-%m-%d`.bfs .
	rm -rf me-bfs/*
mingw-w32-compile:
	cd src && mingw32-make -f win32mingw.mak CC=i686-w64-mingw32-gcc RC=i686-w64-mingw32-windres
	cd src && mingw32-make -f win32mingw.mak CC=i686-w64-mingw32-gcc RC=i686-w64-mingw32-windres BTYP=c
minw-w32-run:	
	cd src && MEPATH=Z:/home/groth/workspace/microemacs/jasspa/macros wine ./.win32mingw-release-mew/mew32.exe


