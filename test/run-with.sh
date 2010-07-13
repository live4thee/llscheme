#!/bin/sh
#
# This script evaluate stdin line by line with GNU Guile.
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
#

usage() {
    echo "usage: $(basename "$0") <guile|gsi>"
    exit -1
}

[ $# -eq 1 ] || usage

case "$1" in
    guile) SCM_CMD='guile -c';;
    gsi)   SCM_CMD='gsi -e';;
    *)     usage;;
esac

while read line; do
  if echo $line | grep -Eq '^\s*;|^\s*$'; then
    continue
  fi

  $SCM_CMD "(display $line)(newline)"
done
