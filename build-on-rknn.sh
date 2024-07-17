#!/bin/sh

BUILDDIR=build-rknn

cmake -B ${BUILDDIR}  -S . -DWITH_CUDA=OFF -DWITH_OPENCL=ON -DWITH_IMGUI=ON -DUSE_OPENGL_ES2=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
[ $? -eq 0 ] && cmake --build ${BUILDDIR} -j3 #--parallel
[ $? -eq 0 ] && cmake --install ${BUILDDIR} --prefix ${HOME}/external-nocuda



