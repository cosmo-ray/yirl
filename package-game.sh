#!/bin/bash
rm -rvf $2
cp -rvf $1 $2
cp -rvf ./include $2/
cp -rvf ./modules $2/
cp -rvf ./scripts-dependancies/ $2/
cp -v ./DejaVuSansMono.ttf $2/
cp -rvf tinycc/libtcc1.a $2/

./package-maker.sh ./yirl-loader $2/yirl-loader
DIR_NAME=$( basename $2 )
EXEC_NAME=$DIR_NAME\_start.sh
echo yirl-loader/yirl-loader.sh -d ./ -n $DIR_NAME > $2/$EXEC_NAME
chmod +x $2/$EXEC_NAME
