#!/bin/bash
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# If compilation fails, we want to exit right away
set -e
# We only want to run include-what-you-use if SKIA_ENFORCE_IWYU_FOR_THIS_FILE is in the arguments
# passed in (i.e. the "skia_opt_file_into_iwyu" feature is enabled) and we are not linking
# (as detected by the presence of -fuse-ld).
if [[ "$@" != *SKIA_ENFORCE_IWYU_FOR_THIS_FILE* || "$@" == *use-ld* ]]; then
  external/clang_linux_amd64/bin/clang $@
  exit 0
else
  # Now try to compile with Clang, and then verify with IWYU
  external/clang_linux_amd64/bin/clang $@
  # IWYU always returns a non-zero code because it doesn't produce the .o file (that's why
  # we ran Clang first). As such, we do not want bash to fail after running IWYU.
  set +e
  # Get absolute path to the mapping file because resolving the relative path is tricky, given
  # how Bazel locates the toolchain files.
  MAPPING_FILE=$(realpath $(dirname ${BASH_SOURCE[0]}))"/IWYU_mapping.imp"
  # IWYU always outputs something to stderr, which can be noisy if everything is fixed.
  # Otherwise, we send the exact same arguments to include-what-you-use that we would for
  # regular compilation with clang.
  external/clang_linux_amd64/usr/bin/include-what-you-use \
      -Xiwyu --mapping_file=$MAPPING_FILE $@ 2>/dev/null
  # IWYU returns 2 if everything looks good. It returns some other non-zero exit code otherwise.
  if [ $? -eq 2 ]; then
    exit 0 # keep the build going
  else
    # Run IWYU again, but this time display the output. Then return non-zero to fail the build.
    # These flags are a little different, but only in ways that affect what was displayed, not the
    # analysis. If we aren't sure why IWYU wants to include something, try changing verbose to 3.
    external/clang_linux_amd64/usr/bin/include-what-you-use \
        -Xiwyu --mapping_file=$MAPPING_FILE -Xiwyu --no_comments \
        -Xiwyu --quoted_includes_first -Xiwyu --verbose=3 $@
    exit 1 # fail the build
  fi
fi
