#!/bin/bash
# Copyright 2019 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


set -ex

BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
# This expects the environment variable EMSDK to be set
if [[ ! -d $EMSDK ]]; then
  echo "Be sure to set the EMSDK environment variable."
  exit 1
fi

# Navigate to SKIA_HOME from where this file is located.
pushd $BASE_DIR/../..

source $EMSDK/emsdk_env.sh
EMCC=`which emcc`
EMCXX=`which em++`

if [[ $@ == *debug* ]]; then
  echo "Building a Debug build"
  EXTRA_CFLAGS="\"-DSK_DEBUG\","
  RELEASE_CONF="-O0 --js-opts 0 -s DEMANGLE_SUPPORT=1 -s ASSERTIONS=1 -s GL_ASSERTIONS=1 -g4 \
                --source-map-base /node_modules/debugger/bin/ -DSK_DEBUG"
  BUILD_DIR=${BUILD_DIR:="out/debugger_wasm_debug"}
else
  echo "Building a Release build"
  EXTRA_CFLAGS="\"-DSK_RELEASE\", \"-DGR_GL_CHECK_ALLOC_WITH_GET_ERROR=0\","
  RELEASE_CONF="-Oz --closure 1 --llvm-lto 3 -DSK_RELEASE -DGR_GL_CHECK_ALLOC_WITH_GET_ERROR=0"
  BUILD_DIR=${BUILD_DIR:="out/debugger_wasm"}
fi

mkdir -p $BUILD_DIR

BUILTIN_FONT="$BASE_DIR/fonts/NotoMono-Regular.ttf.cpp"
# Generate the font's binary file (which is covered by .gitignore)
python tools/embed_resources.py \
    --name SK_EMBEDDED_FONTS \
    --input $BASE_DIR/fonts/NotoMono-Regular.ttf \
    --output $BASE_DIR/fonts/NotoMono-Regular.ttf.cpp \
    --align 4

GN_GPU_FLAGS="\"-DSK_DISABLE_LEGACY_SHADERCONTEXT\","
WASM_GPU="-lEGL -lGLESv2 -DSK_SUPPORT_GPU=1 \
          -DSK_DISABLE_LEGACY_SHADERCONTEXT --pre-js $BASE_DIR/cpu.js --pre-js $BASE_DIR/gpu.js"

# Turn off exiting while we check for ninja (which may not be on PATH)
set +e
NINJA=`which ninja`
if [[ -z $NINJA ]]; then
  git clone "https://chromium.googlesource.com/chromium/tools/depot_tools.git" --depth 1 $BUILD_DIR/depot_tools
  NINJA=$BUILD_DIR/depot_tools/ninja
fi
# Re-enable error checking
set -e

./bin/fetch-gn

echo "Compiling bitcode"

./bin/gn gen ${BUILD_DIR} \
  --args="cc=\"${EMCC}\" \
  cxx=\"${EMCXX}\" \
  extra_cflags_cc=[\"-frtti\"] \
  extra_cflags=[\"-s\",\"USE_FREETYPE=1\",\"-s\",\"USE_LIBPNG=1\", \"-s\", \"WARN_UNALIGNED=1\",
    \"-DSKNX_NO_SIMD\", \"-DSK_DISABLE_AAA\",
    ${GN_GPU_FLAGS}
    ${EXTRA_CFLAGS}
  ] \
  is_debug=false \
  is_official_build=true \
  is_component_build=false \
  werror=true \
  target_cpu=\"wasm\" \
  \
  skia_use_angle = false \
  skia_use_dng_sdk=false \
  skia_use_egl=true \
  skia_use_expat=false \
  skia_use_fontconfig=false \
  skia_use_freetype=true \
  skia_use_libheif=false \
  skia_use_libjpeg_turbo=true \
  skia_use_libpng=true \
  skia_use_libwebp=true \
  skia_use_wuffs=true \
  skia_use_lua=false \
  skia_use_piex=false \
  skia_use_system_libpng=true \
  skia_use_system_freetype2=true \
  skia_use_system_libjpeg_turbo = false \
  skia_use_system_libwebp=false \
  skia_use_vulkan=false \
  skia_use_zlib=true \
  skia_enable_gpu=true \
  skia_enable_tools=false \
  skia_enable_skshaper=false \
  skia_enable_ccpr=false \
  skia_enable_nvpr=false \
  skia_enable_skpicture=true \
  skia_enable_fontmgr_empty=false \
  skia_enable_pdf=false"

# Build all the libs, we'll link the appropriate ones down below
${NINJA} -C ${BUILD_DIR} libskia.a libdebugcanvas.a

export EMCC_CLOSURE_ARGS="--externs $BASE_DIR/externs.js "

echo "Generating final debugger wasm and javascript"

# Emscripten prefers that the .a files go last in order, otherwise, it
# may drop symbols that it incorrectly thinks aren't used. One day,
# Emscripten will use LLD, which may relax this requirement.
${EMCXX} \
    $RELEASE_CONF \
    -Iexperimental \
    -Iinclude/c \
    -Iinclude/codec \
    -Iinclude/config \
    -Iinclude/core \
    -Iinclude/effects \
    -Iinclude/gpu \
    -Iinclude/gpu/gl \
    -Iinclude/pathops \
    -Iinclude/private \
    -Iinclude/utils/ \
    -Isrc/core/ \
    -Isrc/gpu/ \
    -Isrc/sfnt/ \
    -Isrc/shaders/ \
    -Isrc/utils/ \
    -Ithird_party/icu \
    -Ithird_party/skcms \
    -Itools \
    -Itools/debugger \
    -DSK_DISABLE_AAA \
    -std=c++17 \
    $WASM_GPU \
    --pre-js $BASE_DIR/helper.js \
    --post-js $BASE_DIR/ready.js \
    --bind \
    $BASE_DIR/fonts/NotoMono-Regular.ttf.cpp \
    $BASE_DIR/debugger_bindings.cpp \
    $BUILD_DIR/libdebugcanvas.a \
    $BUILD_DIR/libskia.a \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORT_NAME="DebuggerInit" \
    -s FORCE_FILESYSTEM=0 \
    -s MODULARIZE=1 \
    -s NO_EXIT_RUNTIME=1 \
    -s STRICT=1 \
    -s TOTAL_MEMORY=128MB \
    -s USE_FREETYPE=1 \
    -s USE_LIBPNG=1 \
    -s WARN_UNALIGNED=1 \
    -s WASM=1 \
    -s USE_WEBGL2=1 \
    -o $BUILD_DIR/debugger.js

# TODO(nifong): write unit tests
