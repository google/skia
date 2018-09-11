#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
HTML_SHELL=$BASE_DIR/shell.html
BUILD_DIR=${BUILD_DIR:="out/pathkit"}

# This expects the environment variable EMSDK to be set
if [[ ! -d $EMSDK ]]; then
  echo "Be sure to set the EMSDK environment variable."
  exit 1
fi

# Navigate to SKIA_HOME from where this file is located.
pushd $BASE_DIR/../..

echo "Putting output in $BUILD_DIR (pwd = `pwd`)"

# Run this from $SKIA_HOME, not from the directory this file is in.
if [[ ! -d ./src ]]; then
  echo "Cannot locate Skia source. Is the source checkout okay? Exiting."
  exit 1
fi

if [[ $@ == *help* ]]; then
  echo "By default, this script builds a production WASM build of PathKit."
  echo ""
  echo "It is put in ${BUILD_DIR}, configured by the BUILD_DIR environment"
  echo "variable. Additionally, the EMSDK environment variable must be set."
  echo "This script takes several optional parameters:"
  echo "  test = Make a build suitable for running tests or profiling"
  echo "  debug = Make a build suitable for debugging (defines SK_DEBUG)"
  echo "  asm.js = Build for asm.js instead of WASM (very experimental)"
  echo "  serve = starts a webserver allowing a user to navigate to"
  echo "          localhost:8000/pathkit.html to view the demo page."
  exit 0
fi


# Use -O0 for larger builds (but generally quicker)
# Use -Oz for (much slower, but smaller/faster) production builds
export EMCC_CLOSURE_ARGS="--externs $BASE_DIR/externs.js "
RELEASE_CONF="-Oz --closure 1 -s EVAL_CTORS=1 --llvm-lto 3 -s ELIMINATE_DUPLICATE_FUNCTIONS=1"
if [[ $@ == *test* ]]; then
  echo "Building a Testing/Profiling build"
  RELEASE_CONF="-O2 --profiling -DPATHKIT_TESTING -DSK_RELEASE"
elif [[ $@ == *debug* ]]; then
  echo "Building a Debug build"
  # -g4 creates source maps that can apparently let you see the C++ code
  # in the browser's debugger.
  RELEASE_CONF="-O0 --js-opts 0 -s SAFE_HEAP=1 -s ASSERTIONS=1 -g4 -DPATHKIT_TESTING -DSK_DEBUG"
fi

WASM_CONF="-s WASM=1"
if [[ $@ == *asm.js* ]]; then
  echo "Building with asm.js instead of WASM"
  WASM_CONF="-s WASM=0 -s ALLOW_MEMORY_GROWTH=1"
fi

OUTPUT="-o $BUILD_DIR/pathkit.js"
source $EMSDK/emsdk_env.sh

echo "Compiling"

set -e

mkdir -p $BUILD_DIR

em++ $RELEASE_CONF -std=c++14 \
-Iinclude/codec \
-Iinclude/config \
-Iinclude/core \
-Iinclude/effects \
-Iinclude/encode \
-Iinclude/gpu \
-Iinclude/pathops \
-Iinclude/ports \
-Iinclude/private \
-Iinclude/svg \
-Iinclude/utils \
-Isrc/core \
-Isrc/codec \
-Isrc/sksl \
-Isrc/gpu \
-Isrc/pdf \
-Isrc/image \
-Isrc/images \
-Isrc/sfnt \
-Isrc/opts \
-Isrc/shaders \
-Isrc/sksl \
-Isrc/svg \
-Isrc/utils \
-Ithird_party/libjpeg-turbo \
-Ithird_party/externals/libjpeg-turbo \
-lEGL \
-lGLESv2 \
--bind \
--pre-js $BASE_DIR/helper.js \
--pre-js $BASE_DIR/chaining.js \
-DWEB_ASSEMBLY=1 \
-fno-rtti -fno-exceptions -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 \
$WASM_CONF \
-s MODULARIZE=1 \
-s EXPORT_NAME="PathKitInit" \
-s NO_EXIT_RUNTIME=1 \
-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
-s ERROR_ON_MISSING_LIBRARIES=1 \
-s NO_FILESYSTEM=1 \
-s BINARYEN_IGNORE_IMPLICIT_TRAPS=1 \
-s STRICT=1 \
$OUTPUT \
$BASE_DIR/pathkit_wasm_bindings.cpp \
src/codec/SkJpegCodec.cpp \
src/codec/SkJpegDecoderMgr.cpp \
src/codec/SkJpegUtility.cpp \
src/codec/SkMaskSwizzler.cpp \
src/codec/SkMasks.cpp \
src/core/*.cpp \
src/effects/*.cpp \
src/gpu/*.cpp \
src/gpu/ccpr/*.cpp \
src/gpu/effects/*.cpp \
src/gpu/gl/GrGLAssembleInterface.cpp \
src/gpu/gl/GrGLBuffer.cpp \
src/gpu/gl/GrGLCaps.cpp \
src/gpu/gl/GrGLContext.cpp \
src/gpu/gl/GrGLCreateNullInterface.cpp \
src/gpu/gl/GrGLExtensions.cpp \
src/gpu/gl/GrGLGLSL.cpp \
src/gpu/gl/GrGLGpu.cpp \
src/gpu/gl/GrGLGpuCommandBuffer.cpp \
src/gpu/gl/GrGLGpuProgramCache.cpp \
src/gpu/gl/GrGLInterface.cpp \
src/gpu/gl/GrGLPath.cpp \
src/gpu/gl/GrGLPathRendering.cpp \
src/gpu/gl/GrGLProgram.cpp \
src/gpu/gl/GrGLProgramDataManager.cpp \
src/gpu/gl/GrGLRenderTarget.cpp \
src/gpu/gl/GrGLSemaphore.cpp \
src/gpu/gl/GrGLStencilAttachment.cpp \
src/gpu/gl/GrGLTestInterface.cpp \
src/gpu/gl/GrGLTexture.cpp \
src/gpu/gl/GrGLTextureRenderTarget.cpp \
src/gpu/gl/GrGLUniformHandler.cpp \
src/gpu/gl/GrGLUtil.cpp \
src/gpu/gl/GrGLVaryingHandler.cpp \
src/gpu/gl/GrGLVertexArray.cpp \
src/gpu/gl/builders/*.cpp \
src/gpu/gl/egl/GrGLMakeNativeInterface_egl.cpp \
src/gpu/glsl/*.cpp \
src/gpu/ops/*.cpp \
src/gpu/text/*.cpp \
src/image/*.cpp \
src/images/*.cpp \
src/jumper/SkJumper.cpp \
src/lazy/SkDiscardableMemoryPool.cpp \
src/opts/SkBitmapProcState_opts_none.cpp \
src/opts/SkBlitMask_opts_none.cpp \
src/opts/SkBlitRow_opts_none.cpp \
src/pathops/*.cpp \
src/pipe/SkPipeCanvas.cpp \
src/pipe/SkPipeReader.cpp \
src/ports/SkDebug_stdio.cpp \
src/ports/SkFontMgr_empty_factory.cpp \
src/ports/SkImageGenerator_none.cpp \
src/ports/SkMemory_malloc.cpp \
src/shaders/*.cpp \
src/sksl/*.cpp \
src/sksl/ir/*.cpp \
src/utils/SkDashPath.cpp \
src/utils/SkEventTracer.cpp \
src/utils/SkJSONWriter.cpp \
src/utils/SkParse.cpp \
src/utils/SkParsePath.cpp \
src/utils/SkPatchUtils.cpp \
src/utils/SkPolyUtils.cpp \
src/utils/SkShadowTessellator.cpp \
src/utils/SkShadowUtils.cpp \
src/utils/SkUTF.cpp \
third_party/skcms/skcms.cc

if [[ $@ == *serve* ]]; then
  pushd $BUILD_DIR
  python serve.py
fi

