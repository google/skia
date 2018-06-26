#!/usr/bin/env bash

# Run this from $SKIA_HOME, not from the directory this file is in.

EMSDK="/usr/local/google/home/kjlubick/wasm/"
EM_VERSION="1.38.6"

HTML_SHELL="./experimental/wasm/shell.html"

source ${EMSDK}/emsdk_env.sh

echo "Compiling"

# Use -Oz for (much slower, but smaller/faster) production builds
${EMSDK}/emscripten/${EM_VERSION}/em++ -O0 -std=c++14 \
-Iinclude/config \
-Iinclude/core \
-Iinclude/private \
-Iinclude/pathops \
-Iinclude/utils \
-Isrc/core \
-DDEBUG_STDOUT \
-s WASM=1 \
-s NO_EXIT_RUNTIME=1 \
-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
-s ERROR_ON_MISSING_LIBRARIES=1 \
-s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' \
--shell-file $HTML_SHELL \
-o out/wasm/hello_wasm.html \
experimental/wasm/wasm_main.cpp \
src/core/SkArenaAlloc.cpp \
src/core/SkGeometry.cpp \
src/core/SkPath.cpp \
src/core/SkPathRef.cpp \
src/core/SkUtils.cpp \
src/core/SkPoint.cpp \
src/core/SkMallocPixelRef.cpp \
src/core/SkMatrix.cpp \
src/core/SkRect.cpp \
src/core/SkMath.cpp \
src/core/SkString.cpp \
src/core/SkStringUtils.cpp \
src/pathops/*.cpp \
src/ports/SkDebug_stdio.cpp \
src/ports/SkMemory_malloc.cpp

# Add the following for debugging (bloats production code size otherwise)
# list of all (most?) settings: https://github.com/kripken/emscripten/blob/incoming/src/settings.js
#-s ASSERTIONS=1 \
#-s DEMANGLE_SUPPORT=1 \
#-g2

# To build with ASM.js (instead of WASM)
# This doesn't give the same results as native c++ or wasm....
#-s WASM=0 \
#-s ALLOW_MEMORY_GROWTH=1 \

python -m SimpleHTTPServer 8000
