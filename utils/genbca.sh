#!/bin/sh
#
# Generate a LLVM byte-code archieve of llscheme runtime.
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

if [ $# -ne 1 ]; then
    echo "usage: $(basename $0) <path-to-llscheme-runtime>"
    exit -1
fi

LLVMAR=$(which llvm-ar)
CLANG=$(which clang 2>/dev/null)
BCAFILE=liblsrt.bca

# use llvm-gcc if it's available
if which llvm-gcc 2>/dev/null; then
    CLANG=$(which llvm-gcc)
fi

set -e

cd "$1"
$CLANG -emit-llvm -c *.c -I./include -I../include
$LLVMAR rcs $BCAFILE *.o
cd - 1 >/dev/null

/bin/mv "$1"/$BCAFILE . && echo "generated: $BCAFILE"

exit 0
