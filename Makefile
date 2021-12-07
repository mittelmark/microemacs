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
	cd me-bfs && ../bin/bfs -a ../bin/me -o ../me-linux.bin ./jasspa
	rm -rf me-bfs/*
