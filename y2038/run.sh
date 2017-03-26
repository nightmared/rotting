#!/bin/sh
gcc -fPIC -ldl -shared test.c -o test.o
LD_PRELOAD=$(pwd)/test.o $@
