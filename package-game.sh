#!/bin/bash
rm -rvf $2
cp -rvf $1 $2
cp -rvf ./include $2/
cp -rvf ./modules $2/
cp -rvf tinycc/libtcc1.a $2/
export LD_LIBRARY_PATH=./
./package-maker.sh ./yirl-loader $2/yirl-loader
echo yirl-loader/yirl-loader.sh -d ./ > $2/start.sh
chmod +x $2/start.sh
