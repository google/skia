#!/bin/bash
# Copyright 2019 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container of cmake-release
# and a Skia checkout has been mounted at /SRC and the output directory
# has been mounted at /OUT

# For example:
# docker run -v $SKIA_ROOT:/SRC -v /tmp/cmake_out:/OUT gcr.io/skia-public/cmake-release:3.13.1_v1 /SRC/infra/cmake/build_skia.sh


set -xe

#BASE_DIR is the dir this script is in ($SKIA_ROOT/infra/cmake)
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
SKIA_DIR=`cd ${BASE_DIR}/../.. && pwd`

OUT="$(mktemp -d)/CMAKE"

cd ${SKIA_DIR}
./bin/fetch-gn
gn gen ${OUT} --args='is_debug=false' --ide=json --json-ide-script=$SKIA_DIR/gn/gn_to_cmake.py

cd ${OUT}
export CC=/usr/local/bin/clang
export CXX=/usr/local/bin/clang++
cmake -G"CodeBlocks - Unix Makefiles" .
cmake --build . --parallel 8

# Copy build products, ignoring the warning
# for not copying directories.
cp ${OUT}/* /OUT || true
chmod +rw /OUT/*
