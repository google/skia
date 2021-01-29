#!/bin/bash
# Copyright 2018 Google LLC
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

RELEASE_CONF="-Oz --closure 1 -DSK_RELEASE --pre-js $BASE_DIR/release.js"
EXTRA_CFLAGS="\"-DSK_RELEASE\","
IS_OFFICIAL_BUILD="true"

# Tracing will be disabled in release/profiling unless this flag is seen. Tracing will
# be on debug builds always.
if [[ $@ != *force_tracing* ]] ; then
  RELEASE_CONF+=" -DSK_DISABLE_TRACING"
  EXTRA_CFLAGS+="\"-DSK_DISABLE_TRACING\","
fi

if [[ $@ == *debug* ]]; then
  echo "Building a Debug build"
  EXTRA_CFLAGS="\"-DSK_DEBUG\","
  RELEASE_CONF="-O0 --js-opts 0 -s DEMANGLE_SUPPORT=1 -s ASSERTIONS=1 -s GL_ASSERTIONS=1 -g3 \
                --source-map-base /node_modules/canvaskit/bin/ -DSK_DEBUG --pre-js $BASE_DIR/debug.js"
  BUILD_DIR=${BUILD_DIR:="out/canvaskit_wasm_debug"}
elif [[ $@ == *profiling* ]]; then
  echo "Building a build for profiling"
  RELEASE_CONF+=" --profiling-funcs --closure 0"
  BUILD_DIR=${BUILD_DIR:="out/canvaskit_wasm_profile"}
elif [[ $@ == *simd* ]]; then
  echo "Building with SIMD operations"
  BUILD_DIR=${BUILD_DIR:="out/canvaskit_wasm_experimental_simd"}
else
  BUILD_DIR=${BUILD_DIR:="out/canvaskit_wasm"}
fi

if [[ $@ == *simd* ]]; then
  RELEASE_CONF+=" -msimd128"
  EXTRA_CFLAGS+="\"-msimd128\","
fi

mkdir -p $BUILD_DIR
# sometimes the .a files keep old symbols around - cleaning them out makes sure
# we get a fresh build.
rm -f $BUILD_DIR/*.a

GN_GPU="skia_enable_gpu=true skia_gl_standard = \"webgl\""
GN_GPU_FLAGS="\"-DSK_DISABLE_LEGACY_SHADERCONTEXT\","
WASM_GPU="-lGL -DSK_SUPPORT_GPU=1 -DSK_GL \
          -DSK_DISABLE_LEGACY_SHADERCONTEXT --pre-js $BASE_DIR/cpu.js --pre-js $BASE_DIR/gpu.js\
          -s USE_WEBGL2=1"
if [[ $@ == *cpu* ]]; then
  echo "Using the CPU backend instead of the GPU backend"
  GN_GPU="skia_enable_gpu=false"
  GN_GPU_FLAGS=""
  WASM_GPU="-DSK_SUPPORT_GPU=0 --pre-js $BASE_DIR/cpu.js -s USE_WEBGL2=0"
fi

SKP_JS="--pre-js $BASE_DIR/skp.js"
GN_SKP_FLAGS=""
WASM_SKP="-DSK_SERIALIZE_SKP"
if [[ $@ == *no_skp_serialization* ]]; then
  # This saves about 20kb compressed.
  SKP_JS=""
  WASM_SKP=""
  GN_SKP_FLAGS="\"-DSK_DISABLE_EFFECT_DESERIALIZATION\","
fi
if [[ $@ == *no_effects_deserialization* ]]; then
  # This saves about 60kb compressed.
  echo "disabling effects deserialization"
  GN_SKP_FLAGS="\"-DSK_DISABLE_EFFECT_DESERIALIZATION\","
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

GN_VIEWER="skia_use_expat=false skia_enable_ccpr=false"
VIEWER_BINDINGS=""
VIEWER_LIB=""

if [[ $@ == *viewer* ]]; then
  echo "Including viewer"
  GN_VIEWER="skia_use_expat=true skia_enable_ccpr=true"
  VIEWER_BINDINGS="$BASE_DIR/viewer_bindings.cpp"
  VIEWER_LIB="$BUILD_DIR/libviewer_wasm.a"
  IS_OFFICIAL_BUILD="false"
fi

MANAGED_SKOTTIE_BINDINGS="\
  -DSK_INCLUDE_MANAGED_SKOTTIE=1 \
  modules/skottie/utils/SkottieUtils.cpp"
if [[ $@ == *no_managed_skottie* || $@ == *no_skottie* ]]; then
  echo "Omitting managed Skottie"
  MANAGED_SKOTTIE_BINDINGS="-DSK_INCLUDE_MANAGED_SKOTTIE=0"
fi

PARTICLES_JS="--pre-js $BASE_DIR/particles.js"
PARTICLES_BINDINGS="$BASE_DIR/particles_bindings.cpp"
PARTICLES_LIB="$BUILD_DIR/libparticles.a"

if [[ $@ == *no_particles* ]]; then
  echo "Omitting Particles"
  PARTICLES_JS=""
  PARTICLES_BINDINGS=""
  PARTICLES_LIB=""
fi

if [[ $@ != *no_particles* || $@ != *no_skottie* ]] ; then
  PARTICLES_BINDINGS+=" modules/skresources/src/SkResources.cpp"
fi

WASM_PATHOPS="-DSK_INCLUDE_PATHOPS"
PATHOPS_JS="--pre-js $BASE_DIR/pathops.js"
if [[ $@ == *no_pathops* ]] ; then
  # This saves about 2kb compressed.
  WASM_PATHOPS=""
  PATHOPS_JS=""
fi

WASM_RT_SHADER="-DSK_INCLUDE_RUNTIME_EFFECT"
RT_SHADER_JS="--pre-js $BASE_DIR/rt_shader.js"
if [[ $@ == *no_rt_shader* ]] ; then
  WASM_RT_SHADER=""
  RT_SHADER_JS=""
fi

MATRIX_HELPER_JS="--pre-js $BASE_DIR/matrix.js"
if [[ $@ == *no_matrix* ]]; then
  echo "Omitting matrix helper code"
  MATRIX_HELPER_JS=""
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
if [[ $@ == *no_canvas* || $@ == *no_matrix* ]]; then
  # Note: HTML Canvas bindings depend on the matrix helpers.
  echo "Omitting bindings for HTML Canvas API"
  HTML_CANVAS_API=""
fi

GN_FONT="skia_enable_fontmgr_custom_directory=false "
WOFF2_FONT="skia_use_freetype_woff2=true"
FONT_CFLAGS=""
BUILTIN_FONT=""
FONT_JS="--pre-js $BASE_DIR/font.js"
if [[ $@ == *no_font* ]]; then
  echo "Omitting the built-in font(s), font manager and all code dealing with fonts"
  FONT_CFLAGS="-DSK_NO_FONTS"
  WOFF2_FONT=""
  FONT_JS=""
  GN_FONT+="skia_enable_fontmgr_custom_embedded=false skia_enable_fontmgr_custom_empty=false"
elif [[ $@ == *no_embedded_font* ]]; then
  echo "Omitting the built-in font(s)"
  GN_FONT+="skia_enable_fontmgr_custom_embedded=false skia_enable_fontmgr_custom_empty=true"
else
  # Generate the font's binary file (which is covered by .gitignore)
  python tools/embed_resources.py \
      --name SK_EMBEDDED_FONTS \
      --input $BASE_DIR/fonts/NotoMono-Regular.ttf \
      --output $BASE_DIR/fonts/NotoMono-Regular.ttf.cpp \
      --align 4
  BUILTIN_FONT="$BASE_DIR/fonts/NotoMono-Regular.ttf.cpp"
  GN_FONT+="skia_enable_fontmgr_custom_embedded=true skia_enable_fontmgr_custom_empty=false"
fi

if [[ $@ == *no_woff2* ]]; then
  WOFF2_FONT="skia_use_freetype_woff2=false"
fi

if [[ $@ == *no_alias_font* ]]; then
EXTRA_CFLAGS+="\"-DCANVASKIT_NO_ALIAS_FONT\","
FONT_CFLAGS+=" -DCANVASKIT_NO_ALIAS_FONT"
fi

GN_SHAPER="skia_use_icu=true skia_use_system_icu=false skia_use_harfbuzz=true skia_use_system_harfbuzz=false"
SHAPER_LIB="$BUILD_DIR/libharfbuzz.a \
            $BUILD_DIR/libicu.a"
if [[ $@ == *primitive_shaper* ]] || [[ $@ == *no_font* ]]; then
  echo "Using the primitive shaper instead of the harfbuzz/icu one"
  GN_SHAPER="skia_use_icu=false skia_use_harfbuzz=false"
  SHAPER_LIB=""
fi

PARAGRAPH_JS="--pre-js $BASE_DIR/paragraph.js"
PARAGRAPH_LIB="$BUILD_DIR/libskparagraph.a"
PARAGRAPH_BINDINGS="-DSK_INCLUDE_PARAGRAPH=1 \
  $BASE_DIR/paragraph_bindings.cpp"

if [[ $@ == *no_paragraph* ]] || [[ $@ == *primitive_shaper* ]] || [[ $@ == *no_font* ]]; then
  echo "Omitting paragraph (must have fonts and non-primitive shaper)"
  PARAGRAPH_JS=""
  PARAGRAPH_LIB=""
  PARAGRAPH_BINDINGS=""
fi

DO_DECODE="true"
if [[ $@ == *no_codecs* ]]; then
  echo "Omitting codecs"
  DO_DECODE="false"
  ENCODE_PNG="false"
  ENCODE_JPEG="false"
  ENCODE_WEBP="false"
else

  ENCODE_PNG="true"
  if [[ $@ == *no_encode_png* ]]; then
    ENCODE_PNG="false"
  fi

  ENCODE_JPEG="false"
  if [[ $@ == *force_encode_jpeg* ]]; then
    ENCODE_JPEG="true"
  fi

  ENCODE_WEBP="false"
  if [[ $@ == *force_encode_webp* ]]; then
    ENCODE_WEBP="true"
  fi

fi # no_codecs

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
  extra_cflags=[\"-s\", \"WARN_UNALIGNED=1\", \"-s\", \"MAIN_MODULE=1\",
    \"-DSKNX_NO_SIMD\", \"-DSK_DISABLE_AAA\",
    \"-DSK_FORCE_8_BYTE_ALIGNMENT\",
    ${GN_GPU_FLAGS}
    ${GN_SKP_FLAGS}
    ${EXTRA_CFLAGS}
  ] \
  is_debug=false \
  is_official_build=${IS_OFFICIAL_BUILD} \
  is_component_build=false \
  werror=true \
  target_cpu=\"wasm\" \
  \
  skia_use_angle=false \
  skia_use_dng_sdk=false \
  skia_use_webgl=true \
  skia_use_fontconfig=false \
  skia_use_freetype=true \
  skia_use_libheif=false \
  skia_use_libjpeg_turbo_decode=${DO_DECODE} \
  skia_use_libjpeg_turbo_encode=${ENCODE_JPEG} \
  skia_use_libpng_decode=${DO_DECODE} \
  skia_use_libpng_encode=${ENCODE_PNG} \
  skia_use_libwebp_decode=${DO_DECODE} \
  skia_use_libwebp_encode=${ENCODE_WEBP} \
  skia_use_lua=false \
  skia_use_piex=false \
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
  ${WOFF2_FONT} \
  ${GN_VIEWER} \
  \
  skia_enable_skshaper=true \
  skia_enable_nvpr=false \
  skia_enable_skparagraph=true \
  skia_enable_pdf=false"

# Build all the libs we will need below
parse_targets() {
  for LIBPATH in $@; do
    basename $LIBPATH
  done
}
${NINJA} -C ${BUILD_DIR} libskia.a libskshaper.a \
  $(parse_targets $SKOTTIE_LIB $VIEWER_LIB $PARTICLES_LIB $SHAPER_LIB $PARAGRAPH_LIB)

export EMCC_CLOSURE_ARGS="--externs $BASE_DIR/externs.js "

echo "Generating final wasm"

# Disable '-s STRICT=1' outside of Linux until
# https://github.com/emscripten-core/emscripten/issues/12118 is resovled.
STRICTNESS="-s STRICT=1"
if [[ `uname` != "Linux" ]]; then
  echo "Disabling '-s STRICT=1'. See: https://github.com/emscripten-core/emscripten/issues/12118"
  STRICTNESS=""
fi

# Emscripten prefers that the .a files go last in order, otherwise, it
# may drop symbols that it incorrectly thinks aren't used. One day,
# Emscripten will use LLD, which may relax this requirement.
EMCC_DEBUG=1 ${EMCXX} \
    $RELEASE_CONF \
    -I. \
    -Ithird_party/icu \
    -Ithird_party/skcms \
    -Ithird_party/externals/icu/source/common/ \
    -DSK_DISABLE_AAA \
    -DSK_FORCE_8_BYTE_ALIGNMENT \
    -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 \
    -fno-rtti \
    $WASM_GPU \
    $WASM_PATHOPS \
    $WASM_RT_SHADER \
    $WASM_SKP \
    $FONT_CFLAGS \
    -std=c++17 \
    --bind \
    --no-entry \
    --pre-js $BASE_DIR/preamble.js \
    --pre-js $BASE_DIR/color.js \
    --pre-js $BASE_DIR/memory.js \
    --pre-js $BASE_DIR/helper.js \
    --pre-js $BASE_DIR/util.js \
    --pre-js $BASE_DIR/interface.js \
    $MATRIX_HELPER_JS \
    $PARAGRAPH_JS \
    $SKOTTIE_JS \
    $PARTICLES_JS \
    $PATHOPS_JS \
    $FONT_JS \
    $SKP_JS \
    $RT_SHADER_JS \
    $HTML_CANVAS_API \
    --pre-js $BASE_DIR/postamble.js \
    $BASE_DIR/canvaskit_bindings.cpp \
    $PARTICLES_BINDINGS \
    $SKOTTIE_BINDINGS \
    $VIEWER_BINDINGS \
    $MANAGED_SKOTTIE_BINDINGS \
    $PARAGRAPH_BINDINGS \
    $SKOTTIE_LIB \
    $VIEWER_LIB \
    $PARTICLES_LIB \
    $PARAGRAPH_LIB \
    $BUILD_DIR/libskshaper.a \
    $SHAPER_LIB \
    $BUILD_DIR/libskia.a \
    $BUILTIN_FONT \
    -s LLD_REPORT_UNDEFINED \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORT_NAME="CanvasKitInit" \
    -s EXPORTED_FUNCTIONS=['_malloc','_free'] \
    -s FORCE_FILESYSTEM=0 \
    -s FILESYSTEM=0 \
    -s MODULARIZE=1 \
    -s NO_EXIT_RUNTIME=1 \
    -s INITIAL_MEMORY=128MB \
    -s WASM=1 \
    $STRICTNESS \
    -o $BUILD_DIR/canvaskit.js
