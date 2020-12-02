#!/bin/bash

BUILD_DST=.

if [ $# -eq 3 ]; then
    # if it's build from docker, chance are, submodules haven't been pull yet 
    git submodule update --init --recursive
    BUILD_DST=$3
elif [ $# -ne 2 ]; then
    echo "Usage: package-game.sh src dst [docker-build]"
    return 1
fi

rm -rvf $2
cp -rvf $1 $2
cp -rvf ./include $2/
cp -rvf ./modules $2/
cp -rvf ./scripts-dependancies/ $2/
cp -v ./DejaVuSansMono.ttf $2/
install -D $BUILD_DST/tinycc/libtcc1.a $2/tinycc/libtcc1.a

if [ $# -eq 2 ]; then
    ./package-maker.sh ./yirl-loader $2/yirl-loader
else
    cp -rvf $BUILD_DST $2/yirl-loader
fi
DIR_NAME=$( basename $2 )
EXEC_NAME=$DIR_NAME\_start.sh
echo '#!/bin/bash' > $2/$EXEC_NAME
echo yirl-loader/yirl-loader.sh -d ./ -n $DIR_NAME >> $2/$EXEC_NAME
chmod +x $2/$EXEC_NAME
