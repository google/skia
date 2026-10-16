#!/bin/bash
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -ex

BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
pushd $BASE_DIR/../..

./bin/fetch-gn
./bin/fetch-ninja

BUILD_DIR="out/skbidi_wasm"
mkdir -p $BUILD_DIR

./bin/gn gen ${BUILD_DIR} \
  --args="is_debug=false \
  is_official_build=true \
  is_component_build=false \
  is_trivial_abi=true \
  is_canvaskit=true \
  werror=true \
  target_cpu=\"wasm\" \
  cc=\"emcc\" \
  cxx=\"em++\" \
  \
  skia_use_angle=false \
  skia_use_dawn=false \
  skia_use_webgl=false \
  skia_use_webgpu=false \
  skia_use_expat=false \
  skia_use_fontconfig=false \
  skia_use_freetype=false \
  skia_use_libjpeg_turbo_decode=false \
  skia_use_libjpeg_turbo_encode=false \
  skia_use_libpng_decode=false \
  skia_use_libpng_encode=false \
  skia_use_libwebp_decode=false \
  skia_use_libwebp_encode=false \
  skia_use_lua=false \
  skia_use_piex=false \
  skia_use_system_libjpeg_turbo=false \
  skia_use_system_libpng=false \
  skia_use_system_libwebp=false \
  skia_use_system_zlib=false \
  skia_use_vulkan=false \
  skia_use_wuffs=false \
  skia_use_zlib=true \
  skia_enable_ganesh=false \
  skia_enable_graphite=false \
  skia_enable_pdf=false \
  skia_enable_skottie=false \
  skia_enable_skshaper=false \
  skia_enable_skparagraph=false \
  skia_enable_spirv_validation=false \
  skia_enable_svg=false \
  \
  skia_use_icu=false \
  skia_use_client_icu=false \
  skia_use_bidi=true \
  skia_use_libgrapheme=false \
  skia_use_icu4x=false \
  skia_use_harfbuzz=false \
  skia_use_system_harfbuzz=false \
  skia_canvaskit_enable_bidi=true \
  \
  skia_canvaskit_force_tracing=false"

./third_party/ninja/ninja -C ${BUILD_DIR} modules/canvaskit:skbidi

NODE_BIN=""
if [ -x "./third_party/externals/emsdk/node/20.18.0_64bit/bin/node" ]; then
  NODE_BIN="./third_party/externals/emsdk/node/20.18.0_64bit/bin/node"
elif command -v node > /dev/null 2>&1; then
  NODE_BIN="node"
fi

if [ -n "$NODE_BIN" ]; then
  $NODE_BIN modules/canvaskit/tests/skbidi_test.js "${BUILD_DIR}"
fi
