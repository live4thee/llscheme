#! /bin/sh

set -x
srcdir=`dirname $0`
[ -z "$srcdir" ] && srcdir=.

ORIGDIR=`pwd`
cd $srcdir; mkdir -p m4

autoreconf -v --install || exit 1
cd $ORIGDIR || exit $?

$srcdir/configure --enable-maintainer-mode "$@"
