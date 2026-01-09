##-*- makefile -*-############################################################
#
# Copyright (C) 2026 MicroEmacs User.
#
# All rights reserved.
#
# Synopsis:    
# Authors:     MicroEmacs User
#
##############################################################################

CURRENT_MAKEFILE := $(lastword $(MAKEFILE_LIST))

## argument delegation
ARGS=

## default: list existing tasks 
.PHONY: tasks
tasks:  ## list all tasks
	@grep -Eo '^[a-z0-9]+:.+' $(CURRENT_MAKEFILE) | sed -E 's/:\s+##/\t- /g'

lua:
	# sudo apt install lua5.4
	# sudo apt install luarocks
	# sudo luarocks install luautf8
	# sudo luarocks install luv
	haxe --lua bin/cli.lua -m Main Main.hx
	lua bin/cli.lua Makefile | head -n 5
neko:
	# sudo apt install neko
	haxe --neko bin/cli.n -m Main Main.hx
	neko bin/cli.n Makefile | head -n 5
haxe:
	haxe --run Main Makefile | head -n 5
cpp:
	# sudo apt install haxelib
	# sudo haxelib setup
	# sudo haxelib install hxcpp
	haxe --main Main --cpp cpp
	./cpp/Main Makefile | head -n 5

java:
	haxe --java bin --main Main
	java -jar bin/Main.jar Makefile | head -n 5
jvm:
	# sudo haxelib install hxjava
	# wget https://download.oracle.com/java/25/latest/jdk-25_linux-x64_bin.deb
	# wget sudo dpkg -i sudo dpkg 
	haxe --jvm bin/Main.jar --main Main
	java -jar bin/Main.jar Makefile | head -n 5
	
python:
	haxe --main Main --python bin/cli.py
	python3 bin/cli.py Makefile | head -n 5

mono:	
	## sudo haxelib install hxcs
	## install Mono
	haxe --cs cs --main Main
	mono cs/bin/Main.exe Makefile | head -n 5
