#!/usr/bin/env bash

EMSDK="/usr/local/google/home/kjlubick/wasm/"

EM_VERSION="1.38.6"

source ${EMSDK}/emsdk_env.sh

echo "Compiling"

${EMSDK}/emscripten/${EM_VERSION}/em++ -O3 -std=c++14 \
-Iinclude/c \
-Iinclude/codec \
-Iinclude/config \
-Iinclude/core \
-Iinclude/effects \
-Iinclude/gpu \
-Iinclude/private \
-Iinclude/pathops \
-Iinclude/utils \
-Isrc/core \
-Isrc/gpu \
-Isrc/image \
-Isrc/shaders \
-Isrc/utils \
-s WASM=1 \
-s NO_EXIT_RUNTIME=1 \
-s ASSERTIONS=1 \
-s DEMANGLE_SUPPORT=1 \
-s WARN_ON_UNDEFINED_SYMBOLS=1 \
-o out/wasm/hello_wasm.html \
experimental/wasm/wasm_main.cpp \
src/core/SkPath.cpp \
src/core/SkPathRef.cpp \
src/core/SkPoint.cpp \
src/core/SkPathMeasure.cpp \
src/core/SkRect.cpp \
src/core/SkGeometry.cpp \
src/ports/SkMemory_malloc.cpp

python -m SimpleHTTPServer 8000