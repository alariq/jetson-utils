#!/bin/sh

# ensure that LD_LIBRARY_PATH has path to TensorRT;
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/a/opt/TensorRT-8.6.1.6/lib

# to make cuda happy as it does not support some new c++ library shit
export CC=gcc-8
export CXXC=g++-12


BUILDDIR=build-nocuda

# add path to my TensorRT
cmake -B ${BUILDDIR} -S . -DWITH_CUDA=OFF -DWITH_OPENCL=ON -DWITH_IMGUI=ON -DUSE_OPENGL_ES2=ON -DCMAKE_BUILD_TYPE=Debug
[ $? -eq 0 ] && cmake --build ${BUILDDIR} --parallel
[ $? -eq 0 ] && cmake --install ${BUILDDIR} --prefix /home/${USER}/external-nocuda



