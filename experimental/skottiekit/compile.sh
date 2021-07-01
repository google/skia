#!/bin/bash
# Copyright 2020 Google LLC
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

source $EMSDK/emsdk_env.sh
EMCC=`which emcc`
EMCXX=`which em++`
EMAR=`which emar`

RELEASE_CONF="-Oz --closure 1 --llvm-lto 1 -DSK_RELEASE --pre-js $BASE_DIR/release.js \
              -DGR_GL_CHECK_ALLOC_WITH_GET_ERROR=0 -DSK_DISABLE_TRACING"
EXTRA_CFLAGS="\"-DSK_RELEASE\", \"-DSK_DISABLE_TRACING\""
IS_OFFICIAL_BUILD="true"

if [[ $@ == *full-build* ]]; then
  # Full Skottie with all bells and whistles.
  BUILD_TYPE="full"
  BUILD_CFG="\
    skia_enable_fontmgr_custom_embedded=true \
    skia_enable_fontmgr_custom_empty=false \
    skia_use_freetype=true \
    skia_use_libgifcodec=true \
    skia_use_harfbuzz=true \
    skia_use_icu=true \
    skia_use_libpng_decode=true \
    skia_use_wuffs=true \
    skia_use_zlib=true \
    \
    skia_use_system_freetype2=false \
    skia_use_system_harfbuzz=false \
    skia_use_system_icu=false \
    skia_use_system_libpng=false \
    skia_use_system_zlib=false\
  "
else
  # Smallest usable Skottie.
  BUILD_TYPE="minimal"
  BUILD_CFG="\
    skia_enable_fontmgr_custom_embedded=false \
    skia_enable_fontmgr_custom_empty=true \
    skia_use_freetype=false \
    skia_use_libgifcodec=false \
    skia_use_harfbuzz=false \
    skia_use_icu=false \
    skia_use_libpng_decode=false \
    skia_use_wuffs=false \
    skia_use_zlib=false \
  "
fi

if [[ $@ == *debug* ]]; then
  echo "Building a *${BUILD_TYPE}* Debug build"
  EXTRA_CFLAGS="\"-DSK_DEBUG\""
  RELEASE_CONF="-O0 --js-opts 0 -s DEMANGLE_SUPPORT=1 -s ASSERTIONS=1 -s GL_ASSERTIONS=1 -g4 \
                --source-map-base /bin/ -DSK_DEBUG --pre-js $BASE_DIR/debug.js"
  BUILD_DIR=${BUILD_DIR:="out/skottiekit_debug"}
elif [[ $@ == *profiling* ]]; then
  echo "Building a *${BUILD_TYPE}* build for profiling"
  RELEASE_CONF+=" --profiling-funcs --closure 0"
  BUILD_DIR=${BUILD_DIR:="out/skottiekit_profile"}
else
  BUILD_DIR=${BUILD_DIR:="out/skottiekit"}
fi

mkdir -p $BUILD_DIR
# sometimes the .a files keep old symbols around - cleaning them out makes sure
# we get a fresh build.
rm -f $BUILD_DIR/*.a

GN_GPU="skia_enable_gpu=true skia_gl_standard = \"webgl\""
GN_GPU_FLAGS="\"-DSK_DISABLE_LEGACY_SHADERCONTEXT\","
WASM_GPU="-lEGL -lGL -lGLESv2 -DSK_SUPPORT_GPU=1 -DSK_GL \
          -DSK_DISABLE_LEGACY_SHADERCONTEXT --pre-js $BASE_DIR/cpu.js --pre-js $BASE_DIR/gpu.js\
          -s USE_WEBGL2=1"
if [[ $@ == *cpu* ]]; then
  echo "Using the CPU backend instead of the GPU backend"
  GN_GPU="skia_enable_gpu=false"
  GN_GPU_FLAGS=""
  WASM_GPU="-DSK_SUPPORT_GPU=0 --pre-js $BASE_DIR/cpu.js -s USE_WEBGL2=0"
fi

SKOTTIE_LIB="$BUILD_DIR/libskottie.a \
             $BUILD_DIR/libsksg.a"


MANAGED_SKOTTIE_BINDINGS="\
  -DSK_INCLUDE_MANAGED_SKOTTIE=1 \
  modules/skottie/utils/SkottieUtils.cpp"
if [[ $@ == *no_managed_skottie* ]]; then
  echo "Omitting managed Skottie"
  MANAGED_SKOTTIE_BINDINGS="-DSK_INCLUDE_MANAGED_SKOTTIE=0"
fi

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

# Inspired by https://github.com/Zubnix/skia-wasm-port/blob/master/build_bindings.sh
./bin/gn gen ${BUILD_DIR} \
  --args="cc=\"${EMCC}\" \
  cxx=\"${EMCXX}\" \
  ar=\"${EMAR}\" \
  extra_cflags_cc=[\"-frtti\"] \
  extra_cflags=[\"-s\", \"WARN_UNALIGNED=1\", \"-s\", \"MAIN_MODULE=1\",
    \"-DSKNX_NO_SIMD\", \"-DSK_DISABLE_AAA\",
    \"-DSK_DISABLE_EFFECT_DESERIALIZATION\",
    \"-DSK_FORCE_8_BYTE_ALIGNMENT\",
    ${GN_GPU_FLAGS}
    ${EXTRA_CFLAGS}
  ] \
  is_debug=false \
  is_official_build=${IS_OFFICIAL_BUILD} \
  is_component_build=false \
  werror=true \
  target_cpu=\"wasm\" \
  \
  ${BUILD_CFG} \
  skia_use_angle=false \
  skia_use_dng_sdk=false \
  skia_use_egl=true \
  skia_use_expat=false \
  skia_use_fontconfig=false \
  skia_use_libheif=false \
  skia_use_libjpeg_turbo_decode=true \
  skia_use_libjpeg_turbo_encode=false \
  skia_use_libpng_encode=false \
  skia_use_libwebp_decode=false \
  skia_use_libwebp_encode=false \
  skia_use_lua=false \
  skia_use_piex=false \
  skia_use_system_libjpeg_turbo=false \
  skia_use_vulkan=false \
  skia_enable_fontmgr_custom_directory=false \
  \
  ${GN_GPU} \
  \
  skia_enable_skshaper=true \
  skia_enable_skgpu_v2=false \
  skia_enable_pdf=false"

# Build all the libs we will need below

${NINJA} -C ${BUILD_DIR} libskia.a libskottie.a libsksg.a libskshaper.a

export EMCC_CLOSURE_ARGS="--externs $BASE_DIR/externs.js "

echo "Generating final wasm"

# Emscripten prefers that the .a files go last in order, otherwise, it
# may drop symbols that it incorrectly thinks aren't used. One day,
# Emscripten will use LLD, which may relax this requirement.
${EMCXX} \
    -I. \
    $RELEASE_CONF \
    -DSK_DISABLE_AAA \
    -DSK_FORCE_8_BYTE_ALIGNMENT \
    $WASM_GPU \
    -std=c++17 \
    --bind \
    --no-entry \
    --pre-js $BASE_DIR/preamble.js \
    --pre-js $BASE_DIR/helper.js \
    --pre-js $BASE_DIR/interface.js \
    --pre-js $BASE_DIR/postamble.js \
    $BASE_DIR/skottiekit_bindings.cpp \
    modules/skresources/src/SkResources.cpp \
    $MANAGED_SKOTTIE_BINDINGS \
    $BUILD_DIR/libskottie.a \
    $BUILD_DIR/libsksg.a \
    $BUILD_DIR/libskshaper.a \
    $BUILD_DIR/libskia.a \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORT_NAME="SkottieKitInit" \
    -s FORCE_FILESYSTEM=0 \
    -s FILESYSTEM=0 \
    -s MODULARIZE=1 \
    -s NO_EXIT_RUNTIME=1 \
    -s STRICT=1 \
    -s INITIAL_MEMORY=128MB \
    -s WARN_UNALIGNED=1 \
    -s WASM=1 \
    -o $BUILD_DIR/skottiekit.js
