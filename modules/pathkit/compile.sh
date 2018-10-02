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
RELEASE_CONF="-Oz --closure 1 -s EVAL_CTORS=1 --llvm-lto 3 -s ELIMINATE_DUPLICATE_FUNCTIONS=1 -DSK_RELEASE"
# It is very important for the -DSK_RELEASE/-DSK_DEBUG to match on the libskia.a, otherwise
# things like SKDEBUGCODE are sometimes compiled in and sometimes not, which can cause headaches
# like sizeof() mismatching between .cpp files and .h files.
EXTRA_CFLAGS="\"-DSK_RELEASE\""
if [[ $@ == *test* ]]; then
  echo "Building a Testing/Profiling build"
  RELEASE_CONF="-O2 --profiling -DPATHKIT_TESTING -DSK_RELEASE"
elif [[ $@ == *debug* ]]; then
  echo "Building a Debug build"
  # -g4 creates source maps that can apparently let you see the C++ code
  # in the browser's debugger.
  EXTRA_CFLAGS="\"-DSK_DEBUG\""
  RELEASE_CONF="-O0 --js-opts 0 -s SAFE_HEAP=1 -s ASSERTIONS=1 -g4 -DPATHKIT_TESTING -DSK_DEBUG"
fi

WASM_CONF="-s WASM=1"
if [[ $@ == *asm.js* ]]; then
  echo "Building with asm.js instead of WASM"
  WASM_CONF="-s WASM=0 -s ALLOW_MEMORY_GROWTH=1"
fi

OUTPUT="-o $BUILD_DIR/pathkit.js"
source $EMSDK/emsdk_env.sh
NINJA=`which ninja`
EMCC=`which emcc`
EMCXX=`which em++`


mkdir -p $BUILD_DIR

if [[ -z $NINJA ]]; then
  git clone "https://chromium.googlesource.com/chromium/tools/depot_tools.git" --depth 1 $BUILD_DIR/depot_tools
  NINJA=$BUILD_DIR/depot_tools/ninja
fi

set -ex

echo "Compiling bitcode"

./bin/fetch-gn
./bin/gn gen ${BUILD_DIR} \
  --args="cc=\"${EMCC}\" \
  cxx=\"${EMCXX}\" \
  extra_cflags=[\"-DSK_DISABLE_READBUFFER=1\",${EXTRA_CFLAGS}] \
  is_debug=false \
  is_official_build=true \
  is_component_build=false \
  target_cpu=\"wasm\" \
  \
  skia_use_egl=false \
  skia_use_vulkan=false \
  skia_use_libwebp=false \
  skia_use_libpng=false \
  skia_use_lua=false \
  skia_use_dng_sdk=false \
  skia_use_fontconfig=false \
  skia_use_libjpeg_turbo=false \
  skia_use_libheif=false \
  skia_use_expat=false \
  skia_use_vulkan=false \
  skia_use_freetype=false \
  skia_use_icu=false \
  skia_use_expat=false \
  skia_use_piex=false \
  skia_use_zlib=false \
  \
  skia_enable_gpu=false \
  skia_enable_fontmgr_empty=true \
  skia_enable_pdf=false"

${NINJA} -C ${BUILD_DIR} libpathkit.a

echo "Generating WASM"

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
-DSK_DISABLE_READBUFFER=1 \
-fno-rtti -fno-exceptions -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 \
$WASM_CONF \
-s BINARYEN_IGNORE_IMPLICIT_TRAPS=1 \
-s ERROR_ON_MISSING_LIBRARIES=1 \
-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
-s EXPORT_NAME="PathKitInit" \
-s MODULARIZE=1 \
-s NO_EXIT_RUNTIME=1 \
-s NO_FILESYSTEM=1 \
-s STRICT=1 \
$OUTPUT \
$BASE_DIR/pathkit_wasm_bindings.cpp \
${BUILD_DIR}/libpathkit.a

if [[ $@ == *serve* ]]; then
  pushd $BUILD_DIR
  python serve.py
fi

