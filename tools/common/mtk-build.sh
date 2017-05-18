PWD=`dirname $0`
CACHE=$PWD/cache
MTKVER=`cat $CACHE`
CURMTKVER=`git submodule status $PWD/../../ext/mtk | awk '{print $1}'`

echo "git submodule status $PWD/../../ext/mtk | awk '{print $1}'"
echo "$MTKVER vs $CURMTKVER"

if [ "$MTKVER" !=  "$CURMTKVER" ]; then
	echo "do mtk build"
	make -C $PWD/../../ext/mtk linux
	echo "end mtk build"
	echo "$CURMTKVER" > $CACHE
fi
