#!/bin/sh

rm -rf ./build
mkdir build
cd ./build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=${HOME}/external-test ..
make -j4
cd ..




