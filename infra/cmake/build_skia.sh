#!/bin/bash
# Copyright 2019 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container of emsdk-base
# and a Skia checkout has been mounted at /SRC and the output directory
# has been mounted at /OUT

# For example:
# docker run -v $SKIA_ROOT:/SRC -v /tmp/cmake_out:/OUT gcr.io/skia-public/cmake-release:3.13.1_v1 /SRC/infra/cmake/build_skia.sh


set -xe

#BASE_DIR is the dir this script is in ($SKIA_ROOT/infra/cmake)
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
SKIA_DIR=`cd $BASE_DIR/../.. && pwd`

# Delete everything to do a clean build
rm -rf $SKIA_DIR/out/CMAKE
mkdir --mode=0777 -p $SKIA_DIR/out/CMAKE

cd $SKIA_DIR
./bin/fetch-gn
gn gen out/CMAKE --args='is_debug=false' --ide=json --json-ide-script=../../gn/gn_to_cmake.py

cd $SKIA_DIR/out/CMAKE
cmake -G"CodeBlocks - Unix Makefiles" .
cmake --build . --parallel 8

# Copy build products, ignoring the warning
# for not copying directories.
cp $SKIA_DIR/out/CMAKE/* /OUT || true
chmod +rw /OUT/*
