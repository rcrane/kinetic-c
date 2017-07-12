#!/bin/bash

KINETICCFLAGS="-std=c99 -O3 -DKINETIC_USE_TLS_1_2 -fPIC -D_XOPEN_SOURCE=600"
DIR=$(pwd)

cd vendor/json-c
sh autogen.sh --prefix="$DIR" CFLAGS="$KINETICCFLAGS"
make CFLAGS="$KINETICCFLAGS" -j6
cd ../..

#export OPENSSL_PATH=""
make CFLAGS="$KINETICCFLAGS" -j6
