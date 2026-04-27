#!/bin/sh
# $XTermId: minstall.sh,v 1.7 2021/02/18 21:11:18 tom Exp $
# -----------------------------------------------------------------------------
# this file is part of xterm
#
# Copyright 2006-2010,2021 by Thomas E. Dickey
#
#                         All Rights Reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name(s) of the above copyright
# holders shall not be used in advertising or otherwise to promote the
# sale, use or other dealings in this Software without prior written
# authorization.
# -----------------------------------------------------------------------------
# Install manpages, substituting a reasonable section value since XFree86 4.x
# and derived imakes do not use constants...
#
# Parameters:
#	$1 = program to invoke as "install"
#	$2 = manpage to install
#	$3 = final installed-path
#	$4 = top-level application directory
#	$5 = path of locale.alias
#

# override locale...
LANG=C;		export LANG
LANGUAGE=C;	export LANGUAGE
LC_ALL=C;	export LC_ALL
LC_CTYPE=C;	export LC_CTYPE
XTERM_LOCALE=C	export XTERM_LOCALE

# these could interfere with the "man" command.
unset MANPAGER
unset PAGER

# get parameters
MINSTALL="$1"
OLD_FILE="$2"
END_FILE="$3"
ROOT_DIR="$4"
ALIAS_IS="$5"

suffix=`echo "$END_FILE" | sed -e 's%^.*\.%%'`
NEW_FILE=temp$$

MY_MANSECT=$suffix

# provide for renaming via --program-prefix, etc.
lower=abcdefghikjlmnopqrstuvwxyz
upper=ABCDEFGHIKJLMNOPQRSTUVWXYZ

OLD_LOWER=`basename "$OLD_FILE" | sed -e 's/\..*$//'`
NEW_LOWER=`basename "$END_FILE" | sed -e 's/\..*$//'`

OLD_UPPER=`echo "$OLD_LOWER" | tr $lower $upper`
NEW_UPPER=`echo "$NEW_LOWER" | tr $lower $upper`

OLD_CHAR0=`echo "$OLD_LOWER" | sed -e 's/^\(.\).*/\1/' | tr $lower $upper`
NEW_CHAR0=`echo "$NEW_LOWER" | sed -e 's/^\(.\).*/\1/' | tr $lower $upper`

OLD_CHARS=`echo "$OLD_LOWER" | sed -e 's/^.//'`
NEW_CHARS=`echo "$NEW_LOWER" | sed -e 's/^.//'`

OLD_FIRST=${OLD_CHAR0}${OLD_CHARS}
NEW_FIRST=${NEW_CHAR0}${NEW_CHARS}

# "X" is usually in the miscellaneous section, along with "undocumented".
# Use that to guess an appropriate section.
X_MANPAGE=X
X_MANSECT=`man $X_MANPAGE 2>&1 | grep $X_MANPAGE'([^)]*)' | head -n 1 | tr '\012' '\040' | sed -e 's/^[^0123456789]*\([^) ][^) ]*\).*/\1/'`
test -z "$X_MANSECT" && X_MANSECT=$suffix

sed	-e 's%__vendorversion__%"X Window System"%' \
	-e "s%__projectroot__%$ROOT_DIR%" \
	-e "s%__mansuffix__%$MY_MANSECT%g" \
	-e "s%__miscmansuffix__%$X_MANSECT%g" \
	-e "s%__locale_alias__%$ALIAS_IS%g" \
	-e "s%$OLD_LOWER%$NEW_LOWER%g" \
	-e "s%$OLD_UPPER%$NEW_UPPER%g" \
	-e "s%$OLD_FIRST%$NEW_FIRST%g" \
	"$OLD_FILE" >$NEW_FILE

echo "$MINSTALL $OLD_FILE $END_FILE"
eval "$MINSTALL $NEW_FILE $END_FILE"

rm -f $NEW_FILE
