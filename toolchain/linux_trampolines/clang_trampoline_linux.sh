#!/bin/bash
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

export LD_LIBRARY_PATH="external/clang_linux_amd64/usr/lib/x86_64-linux-gnu"

set -euo pipefail

# We only want to run include-what-you-use if DSKIA_ENFORCE_IWYU is in the arguments
# passed in (i.e. the "skia_enforce_iwyu" feature is enabled) and we are not linking
# (as detected by the presence of -fuse-ld).
if [[ "$@" != *DSKIA_ENFORCE_IWYU* || "$@" == *use-ld* ]]; then
  external/clang_linux_amd64/bin/clang $@
  exit 0
fi

supported_files_or_dirs=(
  "include/private/base/"
  "modules/skunicode/"
  "src/base/"
  "src/codec/"
  "src/effects/"
  "src/image/"
  "src/encode/"
  "src/pathops/"
  "src/sksl/"
  "src/svg/"
  "src/utils/"
  "tools/debugger/"
  "tests/"
  "src/core/SkBitmap.cpp"
  "src/core/SkCanvas.cpp"
  "src/core/SkColor.cpp"
  "src/core/SkColorSpace.cpp"
  "src/core/SkCubicClipper.cpp"
  "src/core/SkCubicMap.cpp"
  "src/core/SkData.cpp"
  "src/core/SkDataTable.cpp"
  "src/core/SkEdgeBuilder.cpp"
  "src/core/SkEdgeClipper.cpp"
  "src/core/SkFlattenable.cpp"
  "src/core/SkGeometry.cpp"
  "src/core/SkGlyph.cpp"
  "src/core/SkICC.cpp"
  "src/core/SkImageInfo.cpp"
  "src/core/SkLineClipper.cpp"
  "src/core/SkMD5.cpp"
  "src/core/SkMaskFilter.cpp"
  "src/core/SkMatrix.cpp"
  "src/core/SkPaint.cpp"
  "src/core/SkPath.cpp"
  "src/core/SkPathBuilder.cpp"
  "src/core/SkPathRef.cpp"
  "src/core/SkPathUtils.cpp"
  "src/core/SkPixmap.cpp"
  "src/core/SkPixmapDraw.cpp"
  "src/core/SkPoint.cpp"
  "src/core/SkRRect.cpp"
  "src/core/SkReadBuffer.cpp"
  "src/core/SkReadPixelsRec.cpp"
  "src/core/SkRect.cpp"
  "src/core/SkScalar.cpp"
  "src/core/SkStream.cpp"
  "src/core/SkString.cpp"
  "src/core/SkWriteBuffer.cpp"
  "src/core/SkWritePixelsRec.cpp"
  "src/gpu/ganesh/Device.cpp"
  "src/gpu/ganesh/GrCaps.cpp"
  "src/gpu/ganesh/GrMemoryPool.cpp"
  "src/gpu/ganesh/GrProcessor.cpp"
  "src/gpu/ganesh/GrRenderTargetProxy.cpp"
  "src/gpu/ganesh/GrResourceProvider.cpp"
  "src/gpu/ganesh/GrSurfaceProxy.cpp"
  "src/gpu/ganesh/GrSurfaceProxyView.cpp"
  "src/gpu/ganesh/GrTextureProxy.cpp"
  "src/gpu/ganesh/SkGr.cpp"

  # See //bazel/generate_cpp_files_for_headers.bzl and //include/BUILD.bazel for more.
  "include/gen/"
  "src/gen/"
)

excluded_files=(
# Causes IWYU 8.17 to assert because it includes SkVX.h
# "iwyu.cc:1977: Assertion failed: TODO(csilvers): for objc and clang lang extensions"
  "tests/SkVxTest.cpp"
  "src/base/SkHalf.cpp"
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
  # Now try to compile with Clang, and then verify with IWYU
  external/clang_linux_amd64/bin/clang $@
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
