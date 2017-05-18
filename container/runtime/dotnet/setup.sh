#!/bin/bash

PWD=`dirname $0`
OUTDIR=$PWD/../../../build/dotnet

mv $OUTDIR/mtkdn $PWD
mv $OUTDIR/stub/libserver.so $PWD
