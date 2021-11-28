#!/bin/bash
#to remove:
#end to remove

if [ -n "$1" ]; then
  git clone $1 ./build
else
  git clone yirl ./build
fi

cd build/
git checkout origin/master

echo -n "current git revision: "
git log --pretty=oneline -1
git clean -df
git submodule update --init --recursive
./configure --ndebug
make clean_all #in case copied directry wasn't empty
cd tinycc
./configure --extra-cflags=-fPIC
make -j$(nproc)
cd ..
cd SDL_mixer
./autogen.sh
./configure CFLAGS="-fPIC"
make -j$(nproc)
cd ..
make sdl-gpu-build
make quickjs-2020-03-16
make -j$(nproc)
make # just in case last one doesn't work
rm -rvf /yirl/docker-package/
./package-maker.sh ./yirl-loader /yirl/docker-package/
cp -rvf tinycc/ /yirl/docker-package/
