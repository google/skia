#!/usr/bin/env bash

EMSDK="/usr/local/google/home/kjlubick/wasm/"

EM_VERSION="1.38.6"

source ${EMSDK}/emsdk_env.sh

echo "Compiling"

${EMSDK}/emscripten/${EM_VERSION}/em++ -O3 -std=c++14 \
-Iinclude/config \
-Iinclude/core \
-Iinclude/private \
-Iinclude/pathops \
-Iinclude/utils \
-s WASM=1 \
-s NO_EXIT_RUNTIME=1 \
-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
-s ERROR_ON_MISSING_LIBRARIES=1 \
-o out/wasm/hello_wasm.html \
experimental/wasm/wasm_main.cpp \
src/core/SkPath.cpp \
src/core/SkPathRef.cpp \
src/core/SkPoint.cpp \
src/core/SkPathMeasure.cpp \
src/core/SkRect.cpp \
src/core/SkGeometry.cpp \
src/ports/SkMemory_malloc.cpp \
src/ports/SkDebug_stdio.cpp

# Add the following for debugging (bloats production code size otherwise)
# list of all (most?) settings: https://github.com/kripken/emscripten/blob/incoming/src/settings.js
#-s ASSERTIONS=1 \
#-s DEMANGLE_SUPPORT=1 \

# To build with ASM.js (instead of WASM)
# This doesn't give the same results as native c++ or wasm....
# -s WASM=0 \
#-s ALLOW_MEMORY_GROWTH=1 \

python -m SimpleHTTPServer 8000
