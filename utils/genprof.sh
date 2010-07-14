#!/bin/bash
#
# Compile a given Scheme file and profile it.
#
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

if [ $# -ne 2 ]; then
    echo "usage: $(basename $0) <bca-file> <scm-file>"
    exit -1
fi

BCAFILE="$1"
SCM_SRC="$2"

BASENAME=$(basename "$SCM_SRC")
FILENAME=${BASENAME%.*}
LL_FILE=${FILENAME}.ll
BC_FILE=${FILENAME}.bc
trap "rm -f $LL_FILE 2>/dev/null" 0

# exit on error
set -e

llscheme -S -o $LL_FILE < "$SCM_SRC"
llvm-as $LL_FILE -o $BC_FILE
llvm-ld $BC_FILE $BCAFILE -lgmp -lgc -o $FILENAME

cat <<EOF
LLVM byte-code file '$BC_FILE' is ready for profiling, for example:
./profile.pl -load /usr/lib/libgc.so -load /usr/lib/libgmp.so $BC_FILE
EOF

exit 0
