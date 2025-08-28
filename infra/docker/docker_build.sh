#!/bin/bash
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This bash file is intended to be used for building docker images with louhi
# implement copy_release_files and set IMAGE_NAME before invoking this
set -e

ROOT=`mktemp -d`
mkdir -p ${ROOT}

copy_release_files

docker build -t ${IMAGE_NAME} ${ROOT} --progress=plain
