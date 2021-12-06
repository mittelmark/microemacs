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
	cp -r jasspa/macros me-bfs/macros
	rm -f me-bfs/macros/*~
	-rm me-bfs/macros/null
	cp -r jasspa/contrib me-bfs/
	
	./bin/bfs -a bin/me -o me-linux.bin ./me-bfs
	rm -rf me-bfs/*
