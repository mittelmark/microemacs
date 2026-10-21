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
SMD=me-full.smd

# The manual is assembled in page order from four files:
#   front.smd     - home, acknowledgments, copyright, contact (front matter)
#   history.smd   - MicroEmacs development history
#   faq.smd       - the Frequently Asked Questions
#   reference.smd - the command, macro, variable and file type reference
# They are kept separate to reduce merge conflicts. me-full.smd is generated,
# do not edit it directly; edit front/history/faq/reference instead.
$(SMD): front.smd history.smd faq.smd reference.smd
	cat front.smd history.smd faq.smd reference.smd > $(SMD)

html-single: $(SMD)
	bash ../bin/me-tools.sh smd2html $(SMD) $(page) html
html-all: $(SMD)
	bash ../bin/me-tools.sh smd2html $(SMD) home html
	bash ../bin/me-tools.sh me2html $(SMD) html amicr 0 92
	bash ../bin/me-tools.sh me2html $(SMD) html m1cmd 0 2	
	bash ../bin/me-tools.sh me2html $(SMD) html m2cmd 0 163	
	bash ../bin/me-tools.sh me2html $(SMD) html m2cmd 200 203
	bash ../bin/me-tools.sh me2html $(SMD) html m2mod 0 32
	bash ../bin/me-tools.sh me2html $(SMD) html m2nar 0 12
	bash ../bin/me-tools.sh me2html $(SMD) html m3fcl 1 19	
	bash ../bin/me-tools.sh me2html $(SMD) html m3mac 0 107
	bash ../bin/me-tools.sh me2html $(SMD) html m3osd 0 17		
	bash ../bin/me-tools.sh me2html $(SMD) html m4fil 0 37
	bash ../bin/me-tools.sh me2html $(SMD) html m5var 0 12
	bash ../bin/me-tools.sh me2html $(SMD) html m5var 19 93
	bash ../bin/me-tools.sh me2html $(SMD) html m8fil 0 8
	bash ../bin/me-tools.sh me2html $(SMD) html m9typ 0 75
ehf: $(SMD)
	bash ../bin/me-tools.sh smd2ehf header.txt $(SMD) > me.ehf
clean:
	rm -f $(SMD) me.ehf
