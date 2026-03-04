##-*- makefile -*-############################################################
#
# Authors:     Detlef Groth, University of Potsdam
#
##############################################################################

CURRENT_MAKEFILE := $(lastword $(MAKEFILE_LIST))

## argument delegation
ARGS=Makefile
MAIN=Main
main := `echo $(MAIN) | tr A-Z a-z`

## default: list existing tasks 
.PHONY: tasks
tasks:  ## list all tasks
	@grep -Eo '^[a-z0-9]+:.+' $(CURRENT_MAKEFILE) | sed -E 's/:\s+##/\t- /g'
	echo "Examples"
	echo "make lua MAIN=Fibo ARGS=11"
lua:    ## execute Lua samples like make lua Fibo 12
	# sudo apt install lua5.4
	# sudo apt install luarocks
	# sudo luarocks install luautf8
	# sudo luarocks install luv
	@echo "task: @lua"
	if [[ ! -d bin ]]; then mkdir bin ; fi
	haxe --lua bin/$(main).lua -m $(MAIN) $(MAIN).hx
	lua bin/$(main).lua $(ARGS) | head -n 5
neko:
	# sudo apt install neko
	if [[ ! -d bin ]]; then mkdir bin ; fi
	haxe --neko bin/$(main).n -m $(MAIN) $(MAIN).hx
	neko bin/$(main).n $(ARGS) | head -n 5
haxe:
	haxe --run $(MAIN) $(ARGS) | head -n 5
cpp:
	# sudo apt install haxelib
	# sudo haxelib setup
	# sudo haxelib install hxcpp
	if [[ ! -d cpp-code ]]; then mkdir cpp-code ; fi
	haxe --main $(MAIN) --cpp cpp-code
	./cpp-code/$(MAIN) $(ARGS) | head -n 5

java:
	if [[ ! -d bin ]]; then mkdir bin ; fi
	haxe --java bin --main $(MAIN)
	java -jar bin/$(MAIN).jar $(ARGS) | head -n 5
jvm:
	# sudo haxelib install hxjava
	# wget https://download.oracle.com/java/25/latest/jdk-25_linux-x64_bin.deb
	# wget sudo dpkg -i sudo dpkg 
	if [[ ! -d bin ]]; then mkdir bin ; fi
	haxe --jvm bin/$(MAIN).jar --main $(MAIN)
	java -jar bin/$(MAIN).jar $(ARGS) | head -n 5
	
python:
	@echo "task: $@"
	if [[ ! -d bin ]]; then mkdir bin ; fi
	haxe --main $(MAIN) --python bin/$(main).py
	python3 bin/$(main).py $(ARGS) | head -n 5

mono:	
	## sudo haxelib install hxcs
	## install Mono
	haxe --cs cs --main $(MAIN)
	mono cs/bin/$(MAIN).exe $(ARGS) | head -n 5
all:
	make lua python neko haxe MAIN=$(MAIN) ARGS=$(ARGS)

person:
	haxe --run Person
	haxe --python person.py --main Person
	python3 person.py
