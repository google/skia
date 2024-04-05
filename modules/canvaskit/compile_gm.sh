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
EMCXX=`which em++`

if [[ $@ == *debug* ]]; then
  echo "Building a Debug build"
  DEBUG=true
  EXTRA_CFLAGS="\"-DSK_DEBUG\", \"-DGR_TEST_UTILS\", "
  RELEASE_CONF="-O1 --js-opts 0 -sDEMANGLE_SUPPORT=1 -frtti -sASSERTIONS=1 -sGL_ASSERTIONS=1 -g \
                -DSK_DEBUG --pre-js $BASE_DIR/debug.js"
  BUILD_DIR=${BUILD_DIR:="out/wasm_gm_tests_debug"}
else
  echo "Building a release build"
  DEBUG=false
  BUILD_DIR=${BUILD_DIR:="out/wasm_gm_tests"}
  RELEASE_CONF="-O3 -DSK_RELEASE --pre-js $BASE_DIR/release.js \
              -DGR_TEST_UTILS"
  EXTRA_CFLAGS="\"-DSK_RELEASE\", \"-DGR_TEST_UTILS\", "
fi

IS_OFFICIAL_BUILD="false"

mkdir -p $BUILD_DIR
# sometimes the .a files keep old symbols around - cleaning them out makes sure
# we get a fresh build.
rm -f $BUILD_DIR/*.a

GN_GPU="skia_enable_ganesh=true skia_gl_standard = \"webgl\""
GN_GPU_FLAGS="\"-DSK_DISABLE_LEGACY_SHADERCONTEXT\","
WASM_GPU="-lGL -DSK_GANESH -DSK_GL -DCK_ENABLE_WEBGL \
          -DSK_DISABLE_LEGACY_SHADERCONTEXT --pre-js $BASE_DIR/cpu.js --pre-js $BASE_DIR/webgl.js\
          -sUSE_WEBGL2=1"

GM_LIB="$BUILD_DIR/libgm_wasm.a"

GN_FONT="skia_enable_fontmgr_custom_directory=false "
BUILTIN_FONT="$BASE_DIR/fonts/NotoMono-Regular.ttf.cpp"
# Generate the font's binary file (which is covered by .gitignore)
python3 tools/embed_resources.py \
      --name SK_EMBEDDED_FONTS \
      --input $BASE_DIR/fonts/NotoMono-Regular.ttf \
      --output $BASE_DIR/fonts/NotoMono-Regular.ttf.cpp \
      --align 4
GN_FONT+="skia_enable_fontmgr_custom_embedded=true skia_enable_fontmgr_custom_empty=false"


GN_SHAPER="skia_use_icu=true skia_use_system_icu=false skia_use_harfbuzz=true skia_use_system_harfbuzz=false"

./bin/fetch-ninja
NINJA=third_party/ninja/ninja

./bin/fetch-gn

echo "Compiling bitcode"

# Inspired by https://github.com/Zubnix/skia-wasm-port/blob/master/build_bindings.sh
./bin/gn gen ${BUILD_DIR} \
  --args="skia_emsdk_dir=\"${EMSDK}\" \
  extra_cflags_cc=[\"-frtti\"] \
  extra_cflags=[\"-sMAIN_MODULE=1\",
    \"-DSKNX_NO_SIMD\", \"-DSK_FORCE_AAA\",
    \"-DSK_FORCE_8_BYTE_ALIGNMENT\",
    ${GN_GPU_FLAGS}
    ${EXTRA_CFLAGS}
  ] \
  is_debug=${DEBUG} \
  is_official_build=${IS_OFFICIAL_BUILD} \
  is_trivial_abi=true \
  is_component_build=false \
  werror=true \
  target_cpu=\"wasm\" \
  \
  skia_use_angle=false \
  skia_use_dng_sdk=false \
  skia_use_webgl=true \
  skia_use_fontconfig=false \
  skia_use_freetype=true \
  skia_use_libheif=true \
  skia_use_libjpeg_turbo_decode=true \
  skia_use_libjpeg_turbo_encode=true \
  skia_use_libpng_decode=true \
  skia_use_libpng_encode=true \
  skia_use_libwebp_decode=true \
  skia_use_libwebp_encode=true \
  skia_use_lua=false \
  skia_use_piex=true \
  skia_use_system_freetype2=false \
  skia_use_system_libjpeg_turbo=false \
  skia_use_system_libpng=false \
  skia_use_system_libwebp=false \
  skia_use_system_zlib=false\
  skia_use_vulkan=false \
  skia_use_wuffs=true \
  skia_use_zlib=true \
  \
  ${GN_SHAPER} \
  ${GN_GPU} \
  ${GN_FONT} \
  skia_use_expat=true \
  skia_enable_svg=true \
  skia_enable_skshaper=true \
  skia_enable_skparagraph=true \
  skia_enable_pdf=false"

# Build all the libs we will need below
parse_targets() {
  for LIBPATH in $@; do
    basename $LIBPATH
  done
}
${NINJA} -C ${BUILD_DIR} libskia.a libskshaper.a libskunicode_core.a libskunicode_icu.a \
  $(parse_targets $GM_LIB)

echo "Generating final wasm"

# Defines for the emscripten compilation step, which builds the tests
# Aim to match the defines that would be set by gn for the skia compilation step.
SKIA_DEFINES="
-DSK_FORCE_AAA \
-DSK_FORCE_8_BYTE_ALIGNMENT \
-DSK_HAS_WUFFS_LIBRARY \
-DSK_HAS_HEIF_LIBRARY \
-DSK_CODEC_DECODES_WEBP \
-DSK_CODEC_DECODES_PNG \
-DSK_CODEC_DECODES_JPEG \
-DSK_SHAPER_HARFBUZZ_AVAILABLE \
-DSK_UNICODE_AVAILABLE \
-DSK_UNICODE_ICU_IMPLEMENTATION \
-DSK_ENABLE_SVG \
-DSK_TRIVIAL_ABI=[[clang::trivial_abi]]"

GMS_TO_BUILD="gm/*.cpp"
TESTS_TO_BUILD="tests/*.cpp"

# When developing locally, it can be faster to focus only on the gms or tests you care about
# (since they all have to be recompiled/relinked) every time. To do so, mark the following as true
if false; then
   GMS_TO_BUILD="gm/gm.cpp"
   TESTS_TO_BUILD="tests/BulkRectTest.cpp tests/Test.cpp"
fi

# These gms do not compile or link with the WASM code. Thus, we omit them.
GLOBIGNORE="gm/compressed_textures.cpp:"\
"gm/animated_gif.cpp:"\
"gm/fiddle.cpp:"\
"gm/fontations.cpp:"\
"gm/fontations_ft_compare.cpp:"\
"gm/video_decoder.cpp:"

# These tests do not compile with the WASM code (require other deps).
GLOBIGNORE+="tests/CodecTest.cpp:"\
"tests/CodecAnimTest.cpp:"\
"tests/ColorSpaceTest.cpp:"\
"tests/DrawOpAtlasTest.cpp:"\
"tests/EncodeTest.cpp:"\
"tests/FontMgrAndroidParserTest.cpp:"\
"tests/FontMgrFontConfigTest.cpp:"\
"tests/FontationsTest.cpp:"\
"tests/FontationsFtCompTest.cpp:"\
"tests/FCITest.cpp:"\
"tests/JpegGainmapTest.cpp:"\
"tests/TypefaceMacTest.cpp:"

# These tests do complex things with TestContexts, which is not easily supported for the WASM
# test harness. Thus we omit them.
GLOBIGNORE+="tests/BackendAllocationTest.cpp:"\
"tests/EGLImageTest.cpp:"\
"tests/ImageTest.cpp:"\
"tests/SurfaceSemaphoreTest.cpp:"\
"tests/TextureBindingsResetTest.cpp:"\
"tests/VkHardwareBufferTest.cpp:"

# All the tests in these files crash.
GLOBIGNORE+="tests/GrThreadSafeCacheTest.cpp:"

# Bazel-related ignores (test runners, incompatible GMs, etc.).
GLOBIGNORE+="gm/png_codec.cpp"

# Emscripten prefers that the .a files go last in order, otherwise, it
# may drop symbols that it incorrectly thinks aren't used. One day,
# Emscripten will use LLD, which may relax this requirement.
EMCC_DEBUG=1 ${EMCXX} \
    $RELEASE_CONF \
    -I. \
    -DGR_TEST_UTILS \
    $SKIA_DEFINES \
    $WASM_GPU \
    -std=c++17 \
    --profiling-funcs \
    --profiling \
    --bind \
    --no-entry \
    --pre-js $BASE_DIR/gm.js \
    tools/Resources.cpp \
    $BASE_DIR/gm_bindings.cpp \
    $GMS_TO_BUILD \
    $TESTS_TO_BUILD \
    $GM_LIB \
    $BUILD_DIR/libskshaper.a \
    $BUILD_DIR/libskunicode_core.a \
    $BUILD_DIR/libskunicode_icu.a \
    $BUILD_DIR/libsvg.a \
    $BUILD_DIR/libskia.a \
    $BUILTIN_FONT \
    -sALLOW_MEMORY_GROWTH=1 \
    -sEXPORT_NAME="InitWasmGMTests" \
    -sEXPORTED_FUNCTIONS=['_malloc','_free'] \
    -sFORCE_FILESYSTEM=1 \
    -sFILESYSTEM=1 \
    -sMODULARIZE=1 \
    -sNO_EXIT_RUNTIME=1 \
    -sINITIAL_MEMORY=256MB \
    -sWASM=1 \
    -sSTRICT=1 \
    -o $BUILD_DIR/wasm_gm_tests.js
