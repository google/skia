#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -ex

# Navigate to SKIA_HOME from where this file is located.
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
pushd $BASE_DIR/../..
./bin/fetch-gn

IS_OFFICIAL_BUILD="true"
IS_DEBUG="false"
FORCE_TRACING="false"
PROFILE_BUILD="false"
# Tracing will be disabled in release/profiling unless this flag is seen. Tracing will
# be on debug builds always.
if [[ $@ != *force_tracing* ]] ; then
  FORCE_TRACING="true"
fi

if [[ $@ == *debug_build* ]]; then
  echo "Building a Debug build"
  IS_DEBUG="true"
  IS_OFFICIAL_BUILD="false"
  BUILD_DIR=${BUILD_DIR:="out/canvaskit_wasm_debug"}
elif [[ $@ == *profiling* ]]; then
  echo "Building a build for profiling"
  PROFILE_BUILD="true"
  BUILD_DIR=${BUILD_DIR:="out/canvaskit_wasm_profile"}
else
  BUILD_DIR=${BUILD_DIR:="out/canvaskit_wasm"}
fi

mkdir -p $BUILD_DIR
# sometimes the .a files keep old symbols around - cleaning them out makes sure
# we get a fresh build.
rm -f $BUILD_DIR/*.a

ENABLE_GANESH="true"
ENABLE_GRAPHITE="false"
ENABLE_WEBGL="false"
ENABLE_WEBGPU="false"
if [[ $@ == *cpu* ]]; then
  echo "Using the CPU backend instead of the GPU backend"
  ENABLE_GANESH="false"
elif [[ $@ == *webgpu* ]]; then
  echo "Using WebGPU instead of WebGL"
  ENABLE_WEBGPU="true"
  ENABLE_GRAPHITE="true"
  ENABLE_GANESH="false"
else
  ENABLE_WEBGL="true"
fi

SERIALIZE_SKP="true"
if [[ $@ == *no_skp_serialization* ]]; then
  # This saves about 20kb compressed.
  echo "disabling SKP serialization"
  SERIALIZE_SKP="false"
fi
DESERIALIZE_EFFECTS="true"
if [[ $@ == *no_effects_deserialization* ]]; then
  # This saves about 60kb compressed.
  echo "disabling effects deserialization"
  DESERIALIZE_EFFECTS="false"
fi

ENABLE_SKOTTIE="true"
if [[ $@ == *no_skottie* ]]; then
  echo "Omitting Skottie"
  ENABLE_SKOTTIE="false"
fi

INCLUDE_VIEWER="false"
USE_EXPAT="false"
if [[ $@ == *viewer* ]]; then
  echo "Including viewer"
  INCLUDE_VIEWER="true"
  USE_EXPAT="true"
  IS_OFFICIAL_BUILD="false"
fi

ENABLE_PATHOPS="true"
if [[ $@ == *no_pathops* ]] ; then
  # This saves about 2kb compressed.
  echo "Omitting PathOps"
  ENABLE_PATHOPS="false"
fi

ENABLE_MATRIX="true"
if [[ $@ == *no_matrix* ]]; then
  echo "Omitting matrix helper code"
  ENABLE_MATRIX="false"
fi

ENABLE_CANVAS="true"
if [[ $@ == *no_canvas* || $@ == *no_matrix* ]]; then
  # Note: HTML Canvas bindings depend on the matrix helpers.
  echo "Omitting bindings for HTML Canvas API"
  ENABLE_CANVAS="false"
fi

GN_FONT="skia_enable_fontmgr_custom_directory=false "
WOFF2_FONT="skia_use_freetype_woff2=true"
ENABLE_FONT="true"
ENABLE_EMBEDDED_FONT="true"
if [[ $@ == *no_font* ]]; then
  echo "Omitting the built-in font(s), font manager and all code dealing with fonts"
  ENABLE_FONT="false"
  ENABLE_EMBEDDED_FONT="false"
  GN_FONT+="skia_enable_fontmgr_custom_embedded=false skia_enable_fontmgr_custom_empty=false "
else
  if [[ $@ == *no_embedded_font* ]]; then
    echo "Omitting the built-in font(s)"
    ENABLE_EMBEDDED_FONT="false"
  fi
  # Generate the font's binary file (which is covered by .gitignore)
  GN_FONT+="skia_enable_fontmgr_custom_embedded=true skia_enable_fontmgr_custom_empty=true "
fi

if [[ $@ == *no_woff2* ]]; then
  WOFF2_FONT="skia_use_freetype_woff2=false"
fi

ENABLE_ALIAS_FONT="true"
if [[ $@ == *no_alias_font* ]]; then
  ENABLE_ALIAS_FONT="false"
fi

LEGACY_DRAW_VERTICES="false"
if [[ $@ == *legacy_draw_vertices* ]]; then
  LEGACY_DRAW_VERTICES="true"
fi

DEBUGGER_ENABLED="false"
if [[ $@ == *enable_debugger* ]]; then
  DEBUGGER_ENABLED="true"
fi

GN_SHAPER="skia_use_icu=true skia_use_client_icu=false skia_use_libgrapheme=false skia_use_icu4x=false skia_use_system_icu=false skia_use_harfbuzz=true skia_use_system_harfbuzz=false"
if [[ $@ == *primitive_shaper* ]] || [[ $@ == *no_font* ]]; then
  echo "Using the primitive shaper instead of the harfbuzz/icu one"
  GN_SHAPER="skia_use_icu=false skia_use_harfbuzz=false"
fi

if [[ $@ == *client_unicode* ]] ; then
  echo "Using the client-provided skunicode data and harfbuz instead of the icu-provided data"
  GN_SHAPER="skia_use_icu=false skia_use_client_icu=true skia_use_libgrapheme=false skia_use_icu4x=false skia_use_harfbuzz=true skia_use_system_harfbuzz=false"
fi

ENABLE_PARAGRAPH="true"
if [[ $@ == *no_paragraph* ]] || [[ $@ == *primitive_shaper* ]] || [[ $@ == *no_font* ]]; then
  echo "Omitting paragraph (must have fonts and non-primitive shaper)"
  ENABLE_PARAGRAPH="false"
fi

DO_DECODE="true"
if [[ $@ == *no_codecs* ]]; then
  echo "Omitting codecs"
  DO_DECODE="false"
  ENCODE_JPEG="false"
  ENCODE_PNG="false"
  ENCODE_WEBP="false"
  NO_ENCODE_JPEG="true"
  NO_ENCODE_PNG="true"
  NO_ENCODE_WEBP="true"
else

  ENCODE_PNG="true"
  NO_ENCODE_PNG="false"
  if [[ $@ == *no_encode_png* ]]; then
    ENCODE_PNG="false"
    NO_ENCODE_PNG="true"
  fi

  ENCODE_JPEG="true"
  NO_ENCODE_JPEG="false"
  if [[ $@ == *no_encode_jpeg* ]]; then
    ENCODE_JPEG="false"
    NO_ENCODE_JPEG="true"
  fi

  ENCODE_WEBP="true"
  NO_ENCODE_WEBP="false"
  if [[ $@ == *no_encode_webp* ]]; then
    ENCODE_WEBP="false"
    NO_ENCODE_WEBP="true"
  fi

fi # no_codecs

./bin/fetch-ninja
NINJA=third_party/ninja/ninja

echo "Compiling"

./bin/gn gen ${BUILD_DIR} \
  --args="is_debug=${IS_DEBUG} \
  is_official_build=${IS_OFFICIAL_BUILD} \
  is_component_build=false \
  is_trivial_abi=true \
  werror=true \
  target_cpu=\"wasm\" \
  \
  skia_use_angle=false \
  skia_use_dng_sdk=false \
  skia_use_dawn=${ENABLE_WEBGPU} \
  skia_use_webgl=${ENABLE_WEBGL} \
  skia_use_webgpu=${ENABLE_WEBGPU} \
  skia_use_expat=${USE_EXPAT} \
  skia_use_fontconfig=false \
  skia_use_freetype=true \
  skia_use_libheif=false \
  skia_use_libjpeg_turbo_decode=${DO_DECODE} \
  skia_use_libjpeg_turbo_encode=${ENCODE_JPEG} \
  skia_use_no_jpeg_encode=${NO_ENCODE_JPEG} \
  skia_use_libpng_decode=${DO_DECODE} \
  skia_use_libpng_encode=${ENCODE_PNG} \
  skia_use_no_png_encode=${NO_ENCODE_PNG} \
  skia_use_libwebp_decode=${DO_DECODE} \
  skia_use_libwebp_encode=${ENCODE_WEBP} \
  skia_use_no_webp_encode=${NO_ENCODE_WEBP} \
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
  skia_enable_ganesh=${ENABLE_GANESH} \
  skia_enable_graphite=${ENABLE_GRAPHITE} \
  skia_build_for_debugger=${DEBUGGER_ENABLED} \
  skia_enable_skottie=${ENABLE_SKOTTIE} \
  \
  ${GN_SHAPER} \
  ${GN_FONT} \
  ${WOFF2_FONT} \
  \
  skia_enable_skshaper=true \
  skia_enable_skparagraph=true \
  skia_enable_pdf=false \
  skia_canvaskit_enable_rt_shader=true \
  skia_canvaskit_force_tracing=${FORCE_TRACING} \
  skia_canvaskit_profile_build=${PROFILE_BUILD} \
  skia_canvaskit_enable_skp_serialization=${SERIALIZE_SKP} \
  skia_canvaskit_enable_effects_deserialization=${DESERIALIZE_EFFECTS} \
  skia_canvaskit_include_viewer=${INCLUDE_VIEWER} \
  skia_canvaskit_enable_pathops=${ENABLE_PATHOPS} \
  skia_canvaskit_enable_matrix_helper=${ENABLE_MATRIX} \
  skia_canvaskit_enable_canvas_bindings=${ENABLE_CANVAS} \
  skia_canvaskit_enable_font=${ENABLE_FONT} \
  skia_canvaskit_enable_embedded_font=${ENABLE_EMBEDDED_FONT} \
  skia_canvaskit_enable_alias_font=${ENABLE_ALIAS_FONT} \
  skia_canvaskit_legacy_draw_vertices_blend_mode=${LEGACY_DRAW_VERTICES} \
  skia_canvaskit_enable_debugger=${DEBUGGER_ENABLED} \
  skia_canvaskit_enable_paragraph=${ENABLE_PARAGRAPH} \
  skia_canvaskit_enable_webgl=${ENABLE_WEBGL} \
  skia_canvaskit_enable_webgpu=${ENABLE_WEBGPU}"

${NINJA} -C ${BUILD_DIR} canvaskit.js
