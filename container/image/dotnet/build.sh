#!/bin/bash

echo `dirname $0`
pushd `dirname $0` > /dev/null
SCPDIR=`pwd -P`
popd > /dev/null
ROOT=$SCPDIR/../../..
OUTDIR=$ROOT/build/dotnet/server

mkdir -p $OUTDIR
docker run --rm -ti -v $1:/codes/Server -v $ROOT/ext/mtk/bindings/csharp:/codes/Mtk -v $ROOT/tools/dotnet:/mtk/bin -v $OUTDIR:/tmp/out \
	mtktool/builder-dotnet make -C /mtk/bin lib
mv $OUTDIR/libserver.so $SCPDIR
