#!/bin/bash
cppcheck -I include/core/ --enable=warning --enable=style --enable=performance --enable=portability  core/*.c  core/*/*.c

rats core/*.c  core/*/*.c
