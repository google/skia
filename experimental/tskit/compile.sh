#!/bin/bash
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -ex

BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
# This expects the environment variable EMSDK to be set
if [[ ! -d $EMSDK ]]; then
  cat >&2 << "EOF"
Be sure to set the EMSDK environment variable to the location of Emscripten SDK:

    https://emscripten.org/docs/getting_started/downloads.html
EOF
  exit 1
fi

# Navigate to SKIA_HOME from where this file is located.
pushd $BASE_DIR/../..

if [[ $@ == *debug* ]]; then
  echo "Building a Debug build"
  DEBUG=true
  EXTRA_CFLAGS="\"-DSK_DEBUG\", \"-DGR_TEST_UTILS\", "
  RELEASE_CONF="-O1 --js-opts 0 -s DEMANGLE_SUPPORT=1 -frtti -s ASSERTIONS=1 -s GL_ASSERTIONS=1 -g \
                -DSK_DEBUG"
  BUILD_DIR=${BUILD_DIR:="out/tskit_debug"}
else
  echo "Building a release build"
  DEBUG=false
  BUILD_DIR=${BUILD_DIR:="out/tskit"}
  RELEASE_CONF="-O3 --closure 1 -DSK_RELEASE"
  EXTRA_CFLAGS="\"-DSK_RELEASE\", \"-DGR_TEST_UTILS\", "
fi

EXTENSION_OPT="--pre-js $BASE_DIR/build/extension.js \
$BASE_DIR/bindings/extension.cpp
"
if [[ $@ = *no_ext* ]] ; then
  echo "disabling extension"
  EXTENSION_OPT=""
fi

mkdir -p $BUILD_DIR
# sometimes the .a files keep old symbols around - cleaning them out makes sure
# we get a fresh build.
rm -f $BUILD_DIR/*.a

source $EMSDK/emsdk_env.sh
EMCC=`which emcc`
EMCXX=`which em++`
EMAR=`which emar`

export EMCC_CLOSURE_ARGS="--externs $BASE_DIR/build/externs.js "

EMCC_DEBUG=1 ${EMCXX} \
    $RELEASE_CONF \
    -I. \
    -std=c++17 \
    --bind \
    --no-entry \
    --pre-js $BASE_DIR/build/load.js \
    --pre-js $BASE_DIR/build/memory.js \
    --pre-js $BASE_DIR/build/core.js \
    $BASE_DIR/bindings/core.cpp \
    $EXTENSION_OPT \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORT_NAME="InitTSKit" \
    -s EXPORTED_FUNCTIONS=['_malloc','_free'] \
    -s FORCE_FILESYSTEM=0 \
    -s FILESYSTEM=0 \
    -s MODULARIZE=1 \
    -s NO_EXIT_RUNTIME=1 \
    -s INITIAL_MEMORY=256MB \
    -s WASM=1 \
    -o $BUILD_DIR/tskit.js
