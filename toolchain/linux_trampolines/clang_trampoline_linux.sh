#!/bin/bash
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

export LD_LIBRARY_PATH="external/clang_linux_amd64/usr/lib/x86_64-linux-gnu"

set -euo pipefail

if [[ "$@" == *SKIA_SKIP_LINKING* ]]; then
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
  "include/core/"
  "include/effects/"
  "include/encode/"
  "include/gpu/ganesh/gen/"
  "include/gpu/gen/"
  "include/gpu/vk/gen/"
  "include/private/"
  "modules/bentleyottmann/"
  "modules/skottie/"
  "modules/sksg/"
  "modules/skshaper/"
  "modules/skunicode/"
  "modules/svg/"
  "src/base/"
  "src/codec/"
  "src/core/"
  "src/effects/"
  "src/encode/"
  "src/gpu/ganesh/"
  "src/gpu/graphite/compute/"
  "src/gpu/graphite/geom/"
  "src/gpu/graphite/render/"
  "src/gpu/tessellate/"
  "src/gpu/gen/"
  "src/gpu/vk/"
  "src/image/"
  "src/pathops/"
  "src/pdf/"
  "src/ports/SkFontMgr_fontconfig"
  "src/ports/SkFontMgr_fontations"
  "src/shaders/"
  "src/sksl/"
  "src/svg/"
  "src/text/"
  "src/utils/"
  "tests/"
  "tools/debugger/"
  "tools/viewer/"
  "src/gpu/A"
  "src/gpu/B"
  "src/gpu/C"
  "src/gpu/D"
  "src/gpu/G"
  "src/gpu/J"
  "src/gpu/K"
  "src/gpu/M"
  "src/gpu/P"
  "src/gpu/R"
  "src/gpu/S"
  "src/gpu/MutableTextureState.cpp"
  "tools/DecodeUtils.cpp"
  "tools/EncodeUtils.cpp"
  "tools/GpuToolUtils.cpp"
  "tools/Resources.cpp"
  "tools/SvgPathExtractor.cpp"
  "tools/ToolUtils.cpp"
  "tools/fonts/FontToolUtils.cpp"
)

excluded_files=(
# Causes IWYU 8.17 to assert because it includes SkVX.h
# "iwyu.cc:1977: Assertion failed: TODO(csilvers): for objc and clang lang extensions"
  "tests/SkVxTest.cpp"
  "src/base/SkHalf.cpp"
  "src/core/SkMipmap.cpp"
  "src/core/SkMipmapHQDownSampler.cpp"
  "src/core/SkMaskBlurFilter.cpp"
  "src/core/SkM44.cpp"
  "src/core/SkPixmap.cpp"
  "modules/skottie/src/effects/MotionBlurEffect.cpp"
# This file sets and checks for defines in a way that confuses IWYU
  "src/gpu/vk/vulkanmemoryallocator/VulkanMemoryAllocatorWrapper.cpp"
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
