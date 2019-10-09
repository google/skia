#!/bin/bash
# Copyright 2018 Google LLC
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
EMAR=`which emar`

RELEASE_CONF="-Oz --closure 1 --llvm-lto 3 -DSK_RELEASE --pre-js $BASE_DIR/release.js \
              -DGR_GL_CHECK_ALLOC_WITH_GET_ERROR=0"
EXTRA_CFLAGS="\"-DSK_RELEASE\", \"-DGR_GL_CHECK_ALLOC_WITH_GET_ERROR=0\","
if [[ $@ == *debug* ]]; then
  echo "Building a Debug build"
  EXTRA_CFLAGS="\"-DSK_DEBUG\""
  RELEASE_CONF="-O0 --js-opts 0 -s DEMANGLE_SUPPORT=1 -s ASSERTIONS=1 -s GL_ASSERTIONS=1 -g4 \
                --source-map-base /node_modules/canvaskit/bin/ -DSK_DEBUG --pre-js $BASE_DIR/debug.js"
  BUILD_DIR=${BUILD_DIR:="out/canvaskit_wasm_debug"}
elif [[ $@ == *profiling* ]]; then
  echo "Building a build for profiling"
  RELEASE_CONF="-O3 --source-map-base /node_modules/canvaskit/bin/ --profiling -g4 -DSK_RELEASE \
                --pre-js $BASE_DIR/release.js -DGR_GL_CHECK_ALLOC_WITH_GET_ERROR=0"
  BUILD_DIR=${BUILD_DIR:="out/canvaskit_wasm_profile"}
else
  BUILD_DIR=${BUILD_DIR:="out/canvaskit_wasm"}
fi

mkdir -p $BUILD_DIR

GN_GPU="skia_enable_gpu=true skia_gl_standard = \"webgl\""
GN_GPU_FLAGS="\"-DSK_DISABLE_LEGACY_SHADERCONTEXT\","
WASM_GPU="-lEGL -lGLESv2 -DSK_SUPPORT_GPU=1 \
          -DSK_DISABLE_LEGACY_SHADERCONTEXT --pre-js $BASE_DIR/cpu.js --pre-js $BASE_DIR/gpu.js"
if [[ $@ == *cpu* ]]; then
  echo "Using the CPU backend instead of the GPU backend"
  GN_GPU="skia_enable_gpu=false"
  GN_GPU_FLAGS=""
  WASM_GPU="-DSK_SUPPORT_GPU=0 --pre-js $BASE_DIR/cpu.js"
fi

SKOTTIE_JS="--pre-js $BASE_DIR/skottie.js"
SKOTTIE_BINDINGS="$BASE_DIR/skottie_bindings.cpp"

SKOTTIE_LIB="$BUILD_DIR/libskottie.a \
             $BUILD_DIR/libsksg.a"

if [[ $@ == *no_skottie* ]]; then
  echo "Omitting Skottie"
  SKOTTIE_JS=""
  SKOTTIE_LIB=""
  SKOTTIE_BINDINGS=""
fi

MANAGED_SKOTTIE_BINDINGS="\
  -DSK_INCLUDE_MANAGED_SKOTTIE=1 \
  modules/skottie/utils/SkottieUtils.cpp"
if [[ $@ == *no_managed_skottie* ]]; then
  echo "Omitting managed Skottie"
  MANAGED_SKOTTIE_BINDINGS="-DSK_INCLUDE_MANAGED_SKOTTIE=0"
fi

GN_PARTICLES="skia_enable_sksl_interpreter=true"
PARTICLES_BINDINGS="$BASE_DIR/particles_bindings.cpp"
PARTICLES_LIB="$BUILD_DIR/libparticles.a"

if [[ $@ == *no_particles* ]]; then
  echo "Omitting Particles"
  GN_PARTICLES="skia_enable_sksl_interpreter=false"
  PARTICLES_BINDINGS=""
  PARTICLES_LIB=""
fi

HTML_CANVAS_API="--pre-js $BASE_DIR/htmlcanvas/preamble.js \
--pre-js $BASE_DIR/htmlcanvas/util.js \
--pre-js $BASE_DIR/htmlcanvas/color.js \
--pre-js $BASE_DIR/htmlcanvas/font.js \
--pre-js $BASE_DIR/htmlcanvas/canvas2dcontext.js \
--pre-js $BASE_DIR/htmlcanvas/htmlcanvas.js \
--pre-js $BASE_DIR/htmlcanvas/imagedata.js \
--pre-js $BASE_DIR/htmlcanvas/lineargradient.js \
--pre-js $BASE_DIR/htmlcanvas/path2d.js \
--pre-js $BASE_DIR/htmlcanvas/pattern.js \
--pre-js $BASE_DIR/htmlcanvas/radialgradient.js \
--pre-js $BASE_DIR/htmlcanvas/postamble.js "
if [[ $@ == *no_canvas* ]]; then
  echo "Omitting bindings for HTML Canvas API"
  HTML_CANVAS_API=""
fi

GN_FONT="skia_enable_fontmgr_empty=false skia_enable_fontmgr_custom_empty=false"
FONT_CFLAGS=""
BUILTIN_FONT="$BASE_DIR/fonts/NotoMono-Regular.ttf.cpp"
if [[ $@ == *no_font* ]]; then
  echo "Omitting the built-in font(s), font manager and all code dealing with fonts"
  BUILTIN_FONT=""
  FONT_CFLAGS="-DSK_NO_FONTS"
  GN_FONT="skia_enable_fontmgr_empty=true"
elif [[ $@ == *no_embedded_font* ]]; then
  echo "Omitting the built-in font(s)"
  BUILTIN_FONT=""
  GN_FONT="skia_enable_fontmgr_empty=false skia_enable_fontmgr_custom_empty=true"
else
  # Generate the font's binary file (which is covered by .gitignore)
  python tools/embed_resources.py \
      --name SK_EMBEDDED_FONTS \
      --input $BASE_DIR/fonts/NotoMono-Regular.ttf \
      --output $BASE_DIR/fonts/NotoMono-Regular.ttf.cpp \
      --align 4
fi

GN_SHAPER="skia_use_icu=true skia_use_system_icu=false skia_use_system_harfbuzz=false"
SHAPER_LIB="$BUILD_DIR/libharfbuzz.a \
            $BUILD_DIR/libicu.a"
SHAPER_TARGETS="libharfbuzz.a libicu.a"
if [[ $@ == *primitive_shaper* ]]; then
  echo "Using the primitive shaper instead of the harfbuzz/icu one"
  GN_SHAPER="skia_use_icu=false skia_use_harfbuzz=false"
  SHAPER_LIB=""
  SHAPER_TARGETS=""
fi

PARAGRAPH_JS="--pre-js $BASE_DIR/paragraph.js"
PARAGRAPH_LIB="$BUILD_DIR/libskparagraph.a"
PARAGRAPH_BINDINGS="-DSK_INCLUDE_PARAGRAPH=1 \
  $BASE_DIR/paragraph_bindings.cpp"

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
  extra_cflags=[\"-s\",\"USE_FREETYPE=1\",\"-s\",\"USE_LIBPNG=1\", \"-s\", \"WARN_UNALIGNED=1\",
    \"-DSKNX_NO_SIMD\", \"-DSK_DISABLE_AAA\", \"-DSK_DISABLE_READBUFFER\",
    \"-DSK_DISABLE_EFFECT_DESERIALIZATION\",
    ${GN_GPU_FLAGS}
    ${EXTRA_CFLAGS}
  ] \
  is_debug=false \
  is_official_build=true \
  is_component_build=false \
  werror=true \
  target_cpu=\"wasm\" \
  use_PIC=false \
  \
  skia_use_angle=false \
  skia_use_dng_sdk=false \
  skia_use_egl=true \
  skia_use_expat=false \
  skia_use_fontconfig=false \
  skia_use_freetype=true \
  skia_use_libheif=false \
  skia_use_libjpeg_turbo=true \
  skia_use_libpng=true \
  skia_use_libwebp=false \
  skia_use_lua=false \
  skia_use_piex=false \
  skia_use_system_libpng=true \
  skia_use_system_freetype2=true \
  skia_use_system_libjpeg_turbo=false \
  skia_use_vulkan=false \
  skia_use_wuffs = true \
  skia_use_zlib=true \
  \
  ${GN_SHAPER} \
  ${GN_GPU} \
  ${GN_FONT} \
  ${GN_PARTICLES} \
  \
  skia_enable_skshaper=true \
  skia_enable_ccpr=false \
  skia_enable_nvpr=false \
  skia_enable_skparagraph=true \
  skia_enable_pdf=false"

# Build all the libs, we'll link the appropriate ones down below
${NINJA} -C ${BUILD_DIR} libskia.a libskottie.a libsksg.a \
    libskparagraph.a libskshaper.a libparticles.a $SHAPER_TARGETS

export EMCC_CLOSURE_ARGS="--externs $BASE_DIR/externs.js "

echo "Generating final wasm"

# Emscripten prefers that the .a files go last in order, otherwise, it
# may drop symbols that it incorrectly thinks aren't used. One day,
# Emscripten will use LLD, which may relax this requirement.
${EMCXX} \
    $RELEASE_CONF \
    -I. \
    -Ithird_party/icu \
    -Ithird_party/skcms \
    -Ithird_party/externals/icu/source/common/ \
    -DSK_DISABLE_READBUFFER \
    -DSK_DISABLE_AAA \
    $WASM_GPU \
    $FONT_CFLAGS \
    -std=c++14 \
    --bind \
    --pre-js $BASE_DIR/preamble.js \
    --pre-js $BASE_DIR/helper.js \
    --pre-js $BASE_DIR/interface.js \
    $PARAGRAPH_JS \
    $SKOTTIE_JS \
    $HTML_CANVAS_API \
    --pre-js $BASE_DIR/postamble.js \
    --post-js $BASE_DIR/ready.js \
    $BASE_DIR/canvaskit_bindings.cpp \
    $PARTICLES_BINDINGS \
    $SKOTTIE_BINDINGS \
    $MANAGED_SKOTTIE_BINDINGS \
    $PARAGRAPH_BINDINGS \
    $SKOTTIE_LIB \
    $PARTICLES_LIB \
    $PARAGRAPH_LIB \
    $BUILD_DIR/libskshaper.a \
    $SHAPER_LIB \
    $BUILD_DIR/libskia.a \
    $BUILTIN_FONT \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORT_NAME="CanvasKitInit" \
    -s FORCE_FILESYSTEM=0 \
    -s MODULARIZE=1 \
    -s NO_EXIT_RUNTIME=1 \
    -s STRICT=1 \
    -s TOTAL_MEMORY=128MB \
    -s USE_FREETYPE=1 \
    -s USE_LIBPNG=1 \
    -s WARN_UNALIGNED=1 \
    -s USE_WEBGL2=0 \
    -s WASM=1 \
    -o $BUILD_DIR/canvaskit.js
