#!/bin/bash
# Copyright 2019 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container of emsdk-base
# and a Skia checkout has been mounted at /SRC

# For example:
# docker run -v $SKIA_ROOT:/SRC gcr.io/skia-public/cmake-release:3.13_v1 /SRC/infra/cmake/build_cmake.sh


set -xe
rm -rf /SRC/out/CMAKE
mkdir --mode=0777 /SRC/out/CMAKE

#BASE_DIR is the dir this script is in ($SKIA_ROOT/infra/canvaskit)
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`

cd /SRC
gn gen out/CMAKE --ide=json --json-ide-script=../../gn/gn_to_cmake.py

cd /SRC/out/CMAKE
cmake .

make -j 50

