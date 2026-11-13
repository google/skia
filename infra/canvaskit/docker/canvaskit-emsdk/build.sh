#!/bin/bash
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -x -e

IMAGE_NAME=canvaskit-emsdk

# copy_release_files is called by docker_build.sh to prepare the
# temporary docker build context.
copy_release_files()
{
  INSTALL="install -D --verbose --backup=none"
  # Copy the Dockerfile into the root of the temporary build context
  ${INSTALL} --mode=644 -T Dockerfile ${ROOT}/Dockerfile
}

# Source the common build script which creates a temp dir,
# calls copy_release_files, and runs 'docker build'.
source ../../../docker/docker_build.sh
