#!/bin/bash
#to remove:
#end to remove

cd build/
git submodule update --init --recursive
./configure --clone-sdl-mixer --ndebug
make clean_all #in case copied directry wasn't empty
cd tinycc
./configure --extra-cflags=-fPIC
make
cd ..
cd SDL_mixer
./autogen.sh
./configure CFLAGS="-fPIC"
make
cd ..
make quickjs-2020-03-16
ls quickjs-2020-03-16
make
rm -rvf /yirl/docker-package/
./package-maker.sh ./yirl-loader /yirl/docker-package/
