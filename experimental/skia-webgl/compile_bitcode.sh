#! /bin/bash

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

echo "Compiling bitcode"

EMCC=`which emcc`
EMCXX=`which em++`

./bin/gn gen ./out/skia_wasm \
  --args="cc=\"${EMCC}\" \
  cxx=\"${EMCXX}\" \
  extra_cflags_cc=[\"-frtti\",\"-s\",\"USE_FREETYPE=1\"] \
  extra_cflags=[\"-Wno-unknown-warning-option\",\"-s\",\"USE_FREETYPE=1\",\"-s\",\"USE_LIBPNG=1\"] \
  is_debug=false \
  is_official_build=true \
  is_component_build=false \
  target_cpu=\"wasm\" \
  \
  skia_use_egl=true \
  skia_use_vulkan=false \
  skia_use_libwebp=false \
  skia_use_libpng=true \
  skia_use_lua=false \
  skia_use_dng_sdk=false \
  skia_use_fontconfig=false \
  skia_use_libjpeg_turbo=false \
  skia_use_libheif=false \
  skia_use_expat=false \
  skia_use_vulkan=false \
  skia_use_freetype=true \
  skia_use_icu=false \
  skia_use_expat=false \
  skia_use_piex=false \
  skia_use_zlib=true \
  \
  skia_enable_gpu=true \
  skia_enable_pdf=false"

ninja -C out/skia_wasm