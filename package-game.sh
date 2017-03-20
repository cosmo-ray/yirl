#!/bin/bash
rm -rvf $2
cp -rvf $1 $2
cp -rvf ./include $2/
cp -rvf ./modules $2/
./package-maker.sh core/generic-loader/yirl-loader $2/yirl-loader
