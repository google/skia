#! /bin/bash

set -ex

BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
# This expects the environment variable EMSDK to be set
if [[ ! -d $EMSDK ]]; then
  echo "Be sure to set the EMSDK environment variable."
  exit 1
fi

BUILD_DIR=${BUILD_DIR:="out/skia_wasm"}

# Navigate to SKIA_HOME from where this file is located.
pushd $BASE_DIR/../..

source $EMSDK/emsdk_env.sh

EMCC=`which emcc`
EMCXX=`which em++`

echo "Compiling bindings"
${EMCC} -O1 -std=c++11 \
    -Iinclude/android \
    -Iinclude/atlastext \
    -Iinclude/c \
    -Iinclude/codec \
    -Iinclude/config \
    -Iinclude/core \
    -Iinclude/effects \
    -Iinclude/encode \
    -Iinclude/gpu \
    -Iinclude/gpu/gl \
    -Iinclude/pathops \
    -Iinclude/ports \
    -Iinclude/svg \
    -Iinclude/utils \
    -Iinclude/views \
    -Iinclude/utils/mac \
    ./experimental/skia-webgl/skia_bindings.cpp \
    -o $BUILD_DIR/skia_bindings.bc

echo "Generating final wasm"

${EMCC} \
    -O1 \
    -std=c++11 \
    -lEGL \
    -lGLESv2 \
    --bind \
    $BUILD_DIR/skia_bindings.bc \
    $BUILD_DIR/libskia.a \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s USE_WEBGL2=1 \
    -s NO_EXIT_RUNTIME=1 \
    -s USE_FREETYPE=1 \
    -s USE_LIBPNG=1 \
    -s WASM=1 \
    -s EXPORT_NAME="SkiaInit" \
    -s MODULARIZE=1 \
    -s FORCE_FILESYSTEM=0 \
    -s STRICT=1 \
    -o $BUILD_DIR/skia.js
