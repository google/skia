#!/bin/bash
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -x -e

IMAGE_NAME=emsdk-base

# Copy files into the right locations in ${ROOT}.
copy_release_files()
{
INSTALL="install -D --verbose --backup=none"

${INSTALL} --mode=644 -T Dockerfile ${ROOT}/Dockerfile
}

source ../docker_build.sh
