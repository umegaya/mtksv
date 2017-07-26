#!/bin/bash

PWD=`dirname $0`
OUTDIR=$PWD/../../../build/dotnet

mv $OUTDIR/mtkdn $PWD
mv $OUTDIR/stub/libserver.so $PWD
CID=`docker run --rm -d mtktool/builder-dotnet sleep 10`
docker cp $CID:/etc/mono/config $PWD 
docker kill $CID
