#!/bin/sh

# Copyright (C) 2010 David Lee <live4thee@gmail.com>
# Copyright (C) 2010 Qing He <qing.x.he@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

# This script runs a given script with `llscheme -p' and compare the
# output with those from GNU/Guile or Gambc gsi.

usage() {
    cat >&2 <<EOF
usage:
$(basename "$0") <gsi|guile> <.scm>
EOF

    exit -1
}

test $# -eq 2 || usage

case "$1" in
    gsi) CMP_SCRIPT="./run-with-gsi.scm";;
    guile) CMP_SCRIPT="./run-with-guile.scm";;
    *) usage;;
esac

test -f $CMP_SCRIPT || { echo "$CMP_SCRIPT: file not found."; exit -1; }

DIFF='/usr/bin/diff -u'
SCMFILE="$2"
SCMOUT="./a.out"

TMP1=`mktemp -t compare-with.sh.1.XXXXXX`
TMP2=`mktemp -t compare-with.sh.2.XXXXXX`
trap "rm -f $TMP1 $TMP2 $SCMOUT* 2>/dev/null" 0

llscheme -p -o $SCMOUT < "$SCMFILE"
test $? -eq 0 || exit -1

$SCMOUT > $TMP1
$CMP_SCRIPT "$SCMFILE" > $TMP2

$DIFF $TMP1 $TMP2
exit 0
