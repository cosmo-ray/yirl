#!/bin/bash

CFLAGS=""
CXXFLAGS=""

cmake -B ./sdl-gpu-build ./sdl-gpu/  -DCMAKE_C_FLAGS="-fPIC" -DCMAKE_CXX_FLAGS="-fPIC"
