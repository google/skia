#!/bin/bash -e

# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Write the SKIA_VERSION to a JavaScript file.

if [ "$1" == "" ]
then
  echo "Must supply output version.js file path." >&2
  exit 1
fi

GIT="git"
if test -f "/cipd/bin/git"; then
  # The `cd` Docker image includes git at this location. If present use it.
  # This image us used when building this target via Louhi.
  GIT="/cipd/bin/git"
fi

SCRIPT_DIR=$(dirname $(realpath $0))
VERSION_JS_PATH=$1
GIT_REVISION=$($GIT -C ${SCRIPT_DIR} rev-parse HEAD)
OUTPUT_DIR=$(dirname ${VERSION_JS_PATH})

mkdir -p $(dirname ${VERSION_JS_PATH})
echo "const SKIA_VERSION = '${GIT_REVISION}';" > ${VERSION_JS_PATH}
