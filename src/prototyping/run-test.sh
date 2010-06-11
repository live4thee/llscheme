#!/bin/sh
python llscheme.py > test.ll
cat test.ll
llvm-as test.ll && ( lli test.bc; echo "result: $?" )
