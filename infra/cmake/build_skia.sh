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
# Delete everything to do a clean build
rm -rf /SRC/out/CMAKE
mkdir --mode=0777 -p /SRC/out/CMAKE

cd /SRC
gn gen out/CMAKE --ide=json --json-ide-script=../../gn/gn_to_cmake.py

cd /SRC/out/CMAKE
cmake .

make -j 50

# Copy build products
cp /SRC/out/CMAKE/* /OUT
chmod +rw /OUT/*
