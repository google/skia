#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
HTML_SHELL=$BASE_DIR/shell.html
BUILD_DIR="out/pathkit"

# This expects the environment variable EMSDK to be set
if [[ ! -d $EMSDK ]]; then
  echo "Be sure to set the EMSDK environment variable."
  exit 1
fi

# Navigate to SKIA_HOME from where this file is located.
pushd $BASE_DIR/../..

# Run this from $SKIA_HOME, not from the directory this file is in.
if [[ ! -d ./src ]]; then
  echo "Cannot locate Skia source. Is the source checkout okay? Exiting."
  exit 1
fi

if [[ $@ == *help* ]]; then
  echo "By default, this script builds a production WASM build of PathKit."
  echo ""
  echo "This script takes several optional parameters:"
  echo "  dev = Make a build suitable for running tests or debugging"
  echo "  asm.js = Build for asm.js instead of WASM"
  echo "  serve = starts a webserver allowing a user to navigate to"
  echo "          localhost:8000/pathkit.html to view the demo page."
  exit 0
fi


# Use -O0 for larger builds (but generally quicker)
# Use -Oz for (much slower, but smaller/faster) production builds
RELEASE_CONF="-Oz"
if [[ $@ == *dev* ]]; then
  echo "Building a Debug/Testing build"
  RELEASE_CONF="-O0 -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1 -g2 -DPATHKIT_TESTING"
fi

WASM_CONF="-s WASM=1"
if [[ $@ == *asm.js* ]]; then
  echo "Building with asm.js instead of WASM"
  WASM_CONF="-s WASM=0 -s ALLOW_MEMORY_GROWTH=1 --separate-asm"
fi

OUTPUT="-o $BUILD_DIR/pathkit.js"

source $EMSDK/emsdk_env.sh

echo "Compiling"

set -e

mkdir -p $BUILD_DIR

em++ $RELEASE_CONF -std=c++14 \
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
$WASM_CONF \
-s MODULARIZE=1 \
-s EXPORT_NAME="PathKitInit" \
-s NO_EXIT_RUNTIME=1 \
-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
-s ERROR_ON_MISSING_LIBRARIES=1 \
$OUTPUT \
$BASE_DIR/pathkit_wasm_bindings.cpp \
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

if [[ $@ == *serve* ]]; then
  pushd $BUILD_DIR
  python serve.py
fi

