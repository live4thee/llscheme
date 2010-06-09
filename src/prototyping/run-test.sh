#!/bin/sh
python llscheme.py > test.ll
[ $? -eq 0 ] || exit -1
cat test.ll
llvm-as test.ll
lli test.bc
echo "result: $?"
