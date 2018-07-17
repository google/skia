#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Run this from $SKIA_HOME, not from the directory this file is in.
# This expects the environment variable EMSDK to be set
HTML_SHELL="./experimental/wasm/shell.html"

if [[ ! -d $EMSDK ]]; then
    echo "Be sure to set the EMSDK environment variable."
    exit 1
fi

source $EMSDK/emsdk_env.sh

echo "Compiling"

set -e

mkdir -p out/wasm

# Use -O0 for larger builds (but generally quicker)
# Use -Oz for (much slower, but smaller/faster) production builds
em++ -Oz -std=c++14 \
-Iinclude/config \
-Iinclude/core \
-Iinclude/gpu \
-Iinclude/pathops \
-Iinclude/private \
-Iinclude/utils \
-Isrc/core \
-Isrc/gpu \
-Isrc/shaders \
-Isrc/opts \
--bind \
-s WASM=1 \
-s NO_EXIT_RUNTIME=1 \
-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
-s ERROR_ON_MISSING_LIBRARIES=1 \
--shell-file $HTML_SHELL \
-o out/wasm/pathkit.html \
experimental/wasm/wasm_main.cpp \
src/core/SkAnalyticEdge.cpp \
src/core/SkArenaAlloc.cpp \
src/core/SkBlitter.cpp \
src/core/SkCoverageDelta.cpp \
src/core/SkEdge.cpp \
src/core/SkEdgeBuilder.cpp \
src/core/SkEdgeClipper.cpp \
src/core/SkFDot6Constants.cpp \
src/core/SkGeometry.cpp \
src/core/SkLineClipper.cpp \
src/core/SkMallocPixelRef.cpp \
src/core/SkMath.cpp \
src/core/SkMatrix.cpp \
src/core/SkOpts.cpp \
src/core/SkPath.cpp \
src/core/SkPathRef.cpp \
src/core/SkPoint.cpp \
src/core/SkRect.cpp \
src/core/SkRegion.cpp \
src/core/SkRegion_path.cpp \
src/core/SkScan_Path.cpp \
src/core/SkStream.cpp \
src/core/SkString.cpp \
src/core/SkStringUtils.cpp \
src/core/SkUtils.cpp \
src/pathops/*.cpp \
src/ports/SkDebug_stdio.cpp \
src/ports/SkMemory_malloc.cpp \
src/utils/SkParse.cpp \
src/utils/SkParsePath.cpp

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
