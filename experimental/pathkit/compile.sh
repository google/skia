#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
HTML_SHELL=$BASE_DIR/shell.html
BUILD_DIR=${BUILD_DIR:="out/pathkit"}

# This expects the environment variable EMSDK to be set
if [[ ! -d $EMSDK ]]; then
  echo "Be sure to set the EMSDK environment variable."
  exit 1
fi

# Navigate to SKIA_HOME from where this file is located.
pushd $BASE_DIR/../..

echo "Putting output in $BUILD_DIR (pwd = `pwd`)"

# Run this from $SKIA_HOME, not from the directory this file is in.
if [[ ! -d ./src ]]; then
  echo "Cannot locate Skia source. Is the source checkout okay? Exiting."
  exit 1
fi

if [[ $@ == *help* ]]; then
  echo "By default, this script builds a production WASM build of PathKit."
  echo ""
  echo "It is put in ${BUILD_DIR}, configured by the BUILD_DIR environment"
  echo "variable. Additionally, the EMSDK environment variable must be set."
  echo "This script takes several optional parameters:"
  echo "  test = Make a build suitable for running tests or profiling"
  echo "  debug = Make a build suitable for debugging (defines SK_DEBUG)"
  echo "  asm.js = Build for asm.js instead of WASM (very experimental)"
  echo "  serve = starts a webserver allowing a user to navigate to"
  echo "          localhost:8000/pathkit.html to view the demo page."
  exit 0
fi


# Use -O0 for larger builds (but generally quicker)
# Use -Oz for (much slower, but smaller/faster) production builds
export EMCC_CLOSURE_ARGS="--externs $BASE_DIR/externs.js "
RELEASE_CONF="-Oz --closure 1 -s EVAL_CTORS=1 --llvm-lto 3 -s ELIMINATE_DUPLICATE_FUNCTIONS=1"
if [[ $@ == *test* ]]; then
  echo "Building a Testing/Profiling build"
  RELEASE_CONF="-O2 --profiling -DPATHKIT_TESTING -DSK_RELEASE"
elif [[ $@ == *debug* ]]; then
  echo "Building a Debug build"
  # -g4 creates source maps that can apparently let you see the C++ code
  # in the browser's debugger.
  RELEASE_CONF="-O0 --js-opts 0 -s SAFE_HEAP=1 -s ASSERTIONS=1 -g4 -DPATHKIT_TESTING -DSK_DEBUG"
fi

WASM_CONF="-s WASM=1"
if [[ $@ == *asm.js* ]]; then
  echo "Building with asm.js instead of WASM"
  WASM_CONF="-s WASM=0 -s ALLOW_MEMORY_GROWTH=1"
fi

OUTPUT="-o $BUILD_DIR/pathkit.js"
source $EMSDK/emsdk_env.sh

echo "Compiling"

set -e

mkdir -p $BUILD_DIR

em++ $RELEASE_CONF -std=c++14 \
-Iinclude/config \
-Iinclude/core \
-Iinclude/effects \
-Iinclude/gpu \
-Iinclude/pathops \
-Iinclude/private \
-Iinclude/utils \
-Isrc/core \
-Isrc/gpu \
-Isrc/shaders \
-Isrc/opts \
-Isrc/utils \
--bind \
--pre-js $BASE_DIR/helper.js \
--pre-js $BASE_DIR/chaining.js \
-DWEB_ASSEMBLY=1 \
-fno-rtti -fno-exceptions -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 \
$WASM_CONF \
-s MODULARIZE=1 \
-s EXPORT_NAME="PathKitInit" \
-s NO_EXIT_RUNTIME=1 \
-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
-s ERROR_ON_MISSING_LIBRARIES=1 \
-s NO_FILESYSTEM=1 \
-s BINARYEN_IGNORE_IMPLICIT_TRAPS=1 \
-s STRICT=1 \
$OUTPUT \
$BASE_DIR/pathkit_wasm_bindings.cpp \
src/core/SkAnalyticEdge.cpp \
src/core/SkArenaAlloc.cpp \
src/core/SkEdge.cpp \
src/core/SkEdgeBuilder.cpp \
src/core/SkEdgeClipper.cpp \
src/core/SkFDot6Constants.cpp \
src/core/SkFlattenable.cpp \
src/core/SkGeometry.cpp \
src/core/SkLineClipper.cpp \
src/core/SkMallocPixelRef.cpp \
src/core/SkMath.cpp \
src/core/SkMatrix.cpp \
src/core/SkOpts.cpp \
src/core/SkPaint.cpp \
src/core/SkPath.cpp \
src/core/SkPathEffect.cpp \
src/core/SkPathMeasure.cpp \
src/core/SkPathRef.cpp \
src/core/SkPoint.cpp \
src/core/SkRRect.cpp \
src/core/SkRect.cpp \
src/core/SkStream.cpp \
src/core/SkString.cpp \
src/core/SkStringUtils.cpp \
src/core/SkStroke.cpp \
src/core/SkStrokeRec.cpp \
src/core/SkStrokerPriv.cpp \
src/core/SkUtils.cpp \
src/effects/SkDashPathEffect.cpp \
src/effects/SkTrimPathEffect.cpp \
src/pathops/*.cpp \
src/ports/SkDebug_stdio.cpp \
src/ports/SkMemory_malloc.cpp \
src/utils/SkDashPath.cpp \
src/utils/SkParse.cpp \
src/utils/SkParsePath.cpp \
src/utils/SkUTF.cpp

if [[ $@ == *serve* ]]; then
  pushd $BUILD_DIR
  python serve.py
fi

