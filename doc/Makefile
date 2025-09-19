##-*- makefile -*-############################################################
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : groth
#  Created       : Sat Nov 18 16:50:59 2023
#  Last Modified : <250919.0818>
#
#  Description	
#
#  Notes
#
#  History
#	
#  $Log$
#
##############################################################################
#
#  Copyright (c) 2023 groth.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from groth.
#
##############################################################################

page=m1cmd000
html-single:
	bash ../bin/me-tools.sh smd2html me.smd $(page) html
html-all:
	bash ../bin/me-tools.sh smd2html me.smd home html
	bash ../bin/me-tools.sh me2html me.smd html amicr 0 90
	bash ../bin/me-tools.sh me2html me.smd html m1cmd 0 2	
	bash ../bin/me-tools.sh me2html me.smd html m2cmd 0 163	
	bash ../bin/me-tools.sh me2html me.smd html m2cmd 200 203
	bash ../bin/me-tools.sh me2html me.smd html m2nar 0 12
	bash ../bin/me-tools.sh me2html me.smd html m3fcl 1 12	
	bash ../bin/me-tools.sh me2html me.smd html m3mac 0 100
	bash ../bin/me-tools.sh me2html me.smd html m3osd 0 17		
	bash ../bin/me-tools.sh me2html me.smd html m4fil 0 37
	bash ../bin/me-tools.sh me2html me.smd html m5var 0 12
	bash ../bin/me-tools.sh me2html me.smd html m5var 19 93
	bash ../bin/me-tools.sh me2html me.smd html m9typ 0 64
ehf:
	bash ../bin/me-tools.sh smd2ehf header.txt me.smd > me.ehf
