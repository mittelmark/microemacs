##-*- makefile -*-############################################################
#
#  Author        : $Author$
#  Created By    : Detlef Groth
#  Created       : Fri May 30 07:12:21 2025
#  Last Modified : <250530.0720>
#
#  Description	 :
#
#  Notes         :
#
##############################################################################
#
#  Copyright (c) 2025 Detlef Groth.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Detlef Groth.
#
##############################################################################

CURRENT_MAKEFILE := $(lastword $(MAKEFILE_LIST))

## argument delegation
ARGS=

## default: list existing tasks 
FILE=hello.c3
.PHONY: tasks
tasks:  ## list all tasks
	@grep -Eo '^[a-z0-9]+:.+' $(CURRENT_MAKEFILE) | sed -E 's/:.+##/\t- /g'

compile: ## compile the file 
	c3c compile $(FILE) -o `basename $(FILE) .c3`

exec:   compile ## compile and run the file
	c3c compile $(FILE) -o `basename $(FILE) .c3`
	./`basename $(FILE) .c3`
format:  ## format the file
	astyle --indent=spaces=2 $(FILE)
