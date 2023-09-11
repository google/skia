#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -ex

BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
HTML_SHELL=$BASE_DIR/shell.html
BUILD_DIR=${BUILD_DIR:="out/pathkit"}
mkdir -p $BUILD_DIR
# sometimes the .a files keep old symbols around - cleaning them out makes sure
# we get a fresh build.
rm -f $BUILD_DIR/*.a

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
RELEASE_CONF="-Oz --closure 1 -DSK_RELEASE"
# It is very important for the -DSK_RELEASE/-DSK_DEBUG to match on the libskia.a, otherwise
# things like SKDEBUGCODE are sometimes compiled in and sometimes not, which can cause headaches
# like sizeof() mismatching between .cpp files and .h files.
EXTRA_CFLAGS="\"-DSK_RELEASE\""
if [[ $@ == *test* ]]; then
  echo "Building a Testing/Profiling build"
  RELEASE_CONF="-O2 --profiling -DPATHKIT_TESTING -DSK_RELEASE"
elif [[ $@ == *debug* ]]; then
  echo "Building a Debug build"
  EXTRA_CFLAGS="\"-DSK_DEBUG\""
  RELEASE_CONF="-O0 --js-opts 0 -sSAFE_HEAP=1 -sASSERTIONS=1 -g3 -DPATHKIT_TESTING -DSK_DEBUG"
fi

WASM_CONF="-sWASM=1"
if [[ $@ == *asm.js* ]]; then
  echo "Building with asm.js instead of WASM"
  WASM_CONF="-sWASM=0 -sALLOW_MEMORY_GROWTH=1"
fi

OUTPUT="-o $BUILD_DIR/pathkit.js"

source $EMSDK/emsdk_env.sh
EMCXX=`which em++`

./bin/fetch-ninja
NINJA=third_party/ninja/ninja

echo "Compiling bitcode"

./bin/fetch-gn

./bin/gn gen ${BUILD_DIR} \
  --args="skia_emsdk_dir=\"${EMSDK}\" \
  extra_cflags=[
    \"-sMAIN_MODULE=1\",
    ${EXTRA_CFLAGS}
  ] \
  is_debug=false \
  is_official_build=true \
  is_trivial_abi=true \
  is_component_build=false \
  werror=true \
  target_cpu=\"wasm\" "

${NINJA} -C ${BUILD_DIR} libpathkit.a

echo "Generating WASM"

${EMCXX} $RELEASE_CONF -std=c++17 \
-I. \
--bind \
--no-entry \
--pre-js $BASE_DIR/helper.js \
--pre-js $BASE_DIR/chaining.js \
-fno-rtti -fno-exceptions -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 \
"-DSK_TRIVIAL_ABI=[[clang::trivial_abi]]" \
$WASM_CONF \
-sERROR_ON_UNDEFINED_SYMBOLS=1 \
-sEXPORT_NAME="PathKitInit" \
-sMODULARIZE=1 \
-sNO_EXIT_RUNTIME=1 \
-sNO_FILESYSTEM=1 \
-sDYNAMIC_EXECUTION=0 \
-sINITIAL_MEMORY=32MB \
-sALLOW_MEMORY_GROWTH=1 \
-sSTRICT=1 \
$OUTPUT \
$BASE_DIR/pathkit_wasm_bindings.cpp \
${BUILD_DIR}/libpathkit.a


