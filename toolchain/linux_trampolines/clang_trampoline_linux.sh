#!/bin/bash
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

export LD_LIBRARY_PATH="external/clang_linux_amd64/usr/lib/x86_64-linux-gnu"

set -euo pipefail

if [[ "$@" == *DSKIA_SKIP_LINKING* ]]; then
  # The output executable binary file is listed as the second argument to this script, and we must
  # make sure it exists or Bazel will fail a validation step.
  touch $2
  exit 0
fi

# We only want to run include-what-you-use if DSKIA_ENFORCE_IWYU is in the arguments
# passed in (i.e. the "skia_enforce_iwyu" feature is enabled) and we are not linking
# (as detected by the presence of -fuse-ld).
if [[ "$@" != *DSKIA_ENFORCE_IWYU* || "$@" == *use-ld* ]]; then
  external/clang_linux_amd64/bin/clang $@
  exit 0
fi

supported_files_or_dirs=(
  "gm/"
  "include/private/base/"
  "modules/skunicode/"
  "src/base/"
  "src/codec/"
  "src/effects/"
  "src/encode/"
  "src/gpu/ganesh/effects/"
  "src/gpu/ganesh/image/"
  "src/gpu/ganesh/surface/"
  "src/image/"
  "src/pathops/"
  "src/shaders/"
  "src/sksl/"
  "src/svg/"
  "src/text/"
  "src/utils/"
  "tests/"
  "tools/debugger/"
  "tools/viewer/"
  "src/core/SkA"
  "src/core/SkB"
  "src/core/SkC"
  "src/core/SkD"
  "src/core/SkE"
  "src/core/SkF"
  "src/core/SkG"
  "src/core/SkH"
  "src/core/SkI"
  "src/core/SkJ"
  "src/core/SkK"
  "src/core/SkL"
  "src/core/SkM"
  "src/core/SkN"
  "src/core/SkO"
  "src/core/SkPaint.cpp"
  "src/core/SkPath.cpp"
  "src/core/SkPathBuilder.cpp"
  "src/core/SkPathRef.cpp"
  "src/core/SkPathUtils.cpp"
  "src/core/SkPicture"
  "src/core/SkPixelRef.cpp"
  "src/core/SkPixmap.cpp"
  "src/core/SkPixmapDraw.cpp"
  "src/core/SkPoint.cpp"
  "src/core/SkRRect.cpp"
  "src/core/SkRe"
  "src/core/SkRuntime"
  "src/core/SkScalar.cpp"
  "src/core/SkSpecialImage.cpp"
  "src/core/SkStream.cpp"
  "src/core/SkStrike"
  "src/core/SkString.cpp"
  "src/core/SkTextBlob.cpp"
  "src/core/SkTime.cpp"
  "src/core/SkTypeface.cpp"
  "src/core/SkWriteBuffer.cpp"
  "src/core/SkWritePixelsRec.cpp"
  "src/core/SkYUVAInfo.cpp"
  "src/core/SkYUVAPixmaps.cpp"
  "src/gpu/ganesh/Device.cpp"
  "src/gpu/ganesh/GrBackendSurface.cpp"
  "src/gpu/ganesh/GrBackendUtils.cpp"
  "src/gpu/ganesh/GrBlurUtils.cpp"
  "src/gpu/ganesh/GrCanvas.cpp"
  "src/gpu/ganesh/GrCaps.cpp"
  "src/gpu/ganesh/GrContext_Base.cpp"
  "src/gpu/ganesh/GrDef"
  "src/gpu/ganesh/GrDirectContext.cpp"
  "src/gpu/ganesh/GrFragmentProcessors.cpp"
  "src/gpu/ganesh/GrImageContext.cpp"
  "src/gpu/ganesh/GrImageUtils.cpp"
  "src/gpu/ganesh/GrMemoryPool.cpp"
  "src/gpu/ganesh/GrMeshBuffers.cpp"
  "src/gpu/ganesh/GrProcessor.cpp"
  "src/gpu/ganesh/GrPromiseImageTexture.cpp"
  "src/gpu/ganesh/GrRecordingContext.cpp"
  "src/gpu/ganesh/GrRenderTargetProxy.cpp"
  "src/gpu/ganesh/GrResourceProvider.cpp"
  "src/gpu/ganesh/GrSurfaceCharacterization.cpp"
  "src/gpu/ganesh/GrSurfaceProxy.cpp"
  "src/gpu/ganesh/GrSurfaceProxyView.cpp"
  "src/gpu/ganesh/GrTextureProxy.cpp"
  "src/gpu/ganesh/GrXferProcessor.cpp"
  "src/gpu/ganesh/SkGr.cpp"
  "src/gpu/ganesh/effects/GrPerlinNoise2Effect.cpp"
  "src/gpu/ganesh/gl/GrGLBackendSurface.cpp"
  "src/gpu/ganesh/gl/GrGLCaps.cpp"
  "src/gpu/ganesh/gl/GrGLDirectContext.cpp"
  "src/gpu/ganesh/gl/GrGLGpu.cpp"
  "src/gpu/ganesh/ops/AtlasTextOp.cpp"
  "src/pdf/SkJpeg"

  # See //bazel/generate_cpp_files_for_headers.bzl and //include/BUILD.bazel for more.
  "include/gen/"
  "src/gen/"
)

excluded_files=(
# Causes IWYU 8.17 to assert because it includes SkVX.h
# "iwyu.cc:1977: Assertion failed: TODO(csilvers): for objc and clang lang extensions"
  "tests/SkVxTest.cpp"
  "src/base/SkHalf.cpp"
  "src/core/SkMipmap.cpp"
  "src/core/SkMaskBlurFilter.cpp"
  "src/core/SkM44.cpp"
)

function opted_in_to_IWYU_checks() {
  # Need [@] for entire list: https://stackoverflow.com/a/46137325
  for path in ${supported_files_or_dirs[@]}; do
    # If this was a generated file, it will be in a different subdirectory, starting with
    # bazel-out, (e.g. bazel-out/k8-iwyu-dbg/bin/src/gen/SkRefCnt.cpp) so check that location also.
    if [[ $1 == *"-c $path"* ]] || [[ $1 == *"-c bazel-out"*"$path"* ]]; then
        for e_path in ${excluded_files[@]}; do
          if [[ $1 == *"-c $e_path"* ]]; then
            echo ""
            return 0
          fi
        done
      echo $path
      return 0
    fi
  done
  echo ""
  return 0
}

# We want to concatenate all args into a string so we can do some
# string matching in the opted_in_to_IWYU_checks function.
# https://unix.stackexchange.com/a/197794
opt_in=$(opted_in_to_IWYU_checks "'$*'")
if [[ -z $opt_in ]]; then
  external/clang_linux_amd64/bin/clang $@
  exit 0
else
  # IWYU always [1] returns a non-zero code because it doesn't produce the .o file (that's why
  # we ran Clang first). As such, we do not want bash to fail after running IWYU.
  # [1] Until v0.18 at least
  set +e
  # Get absolute path to the mapping file because resolving the relative path is tricky, given
  # how Bazel locates the toolchain files.
  MAPPING_FILE=$(realpath $(dirname ${BASH_SOURCE[0]}))"/IWYU_mapping.imp"
  # IWYU always outputs something to stderr, which can be noisy if everything is fixed.
  # Otherwise, we send the exact same arguments to include-what-you-use that we would for
  # regular compilation with clang.
  # We always allow SkTypes.h because it sets some defines that later #ifdefs use and IWYU is
  # not consistent with detecting that.
  external/clang_linux_amd64/bin/include-what-you-use $@ \
      -Xiwyu --keep="include/core/SkTypes.h" \
      -Xiwyu --keep="include/private/base/SkDebug.h" \
      -Xiwyu --no_default_mappings \
      -Xiwyu --error=3 \
      -Xiwyu --mapping_file=$MAPPING_FILE 2>/dev/null
  # IWYU returns 0 if everything looks good. It returns some other non-zero exit code otherwise.
  if [ $? -eq 0 ]; then
    # The expected .d file is the third argument. Bazel expects this file to be created, even
    # if it is empty. We don't really need to create this file or compile the target since
    # we will be skipping linking anyway and not using the output for real.
    touch $3
    # The expected .o file is the last argument passed into clang. Make sure this file exists
    # or Bazel validation will fail
    touch ${!#}
    exit 0 # keep the build going
  else
    # Run IWYU again, but this time display the output. Then return non-zero to fail the build.
    # These flags are a little different, but only in ways that affect what was displayed, not the
    # analysis. If we aren't sure why IWYU wants to include something, try changing verbose to 3.
    external/clang_linux_amd64/bin/include-what-you-use $@ \
        -Xiwyu --keep="include/core/SkTypes.h" \
        -Xiwyu --keep="include/private/base/SkDebug.h" \
        -Xiwyu --no_default_mappings \
        -Xiwyu --mapping_file=$MAPPING_FILE -Xiwyu --no_comments \
        -Xiwyu --quoted_includes_first -Xiwyu --verbose=3
    exit 1 # fail the build
  fi
fi
