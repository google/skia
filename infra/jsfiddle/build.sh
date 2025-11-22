#!/bin/bash
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Build the jsfiddle-final image.

set -x -e

IMAGE_NAME=jsfiddle-final

# Copy files into the right locations in ${ROOT}.
copy_release_files()
{
INSTALL="install -D --verbose --backup=none"
REL=$(dirname "$BASH_SOURCE")

bazelisk build //modules/canvaskit:canvaskit --config=ck_full_webgl2_release
ls -R ${REL}/../../bazel-bin
${INSTALL} --mode=644 -T ${REL}/../../bazel-bin/modules/canvaskit/canvaskit/canvaskit.js ${ROOT}/canvaskit.js
${INSTALL} --mode=644 -T ${REL}/../../bazel-bin/modules/canvaskit/canvaskit/canvaskit.wasm ${ROOT}/canvaskit.wasm

bazelisk build //modules/canvaskit:version.js --config=ck_full_webgl2_release
ls -R ${REL}/../../bazel-bin
cat ${REL}/../../bazel-bin/modules/canvaskit/version.js > ${ROOT}/version.js

${INSTALL} --mode=644 -T ${REL}/../../modules/canvaskit/npm_build/types/index.d.ts ${ROOT}/index.d.ts
${INSTALL} --mode=644 -T Dockerfile ${ROOT}/Dockerfile
}

source ../docker/docker_build.sh
