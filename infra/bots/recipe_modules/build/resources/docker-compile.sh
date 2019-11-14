#!/bin/bash
# Copyright 2019 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container with the following
# mounts:
#  /SRC: Skia checkout
#  /OUT: output directory for gn and ninja
#  /depot_tools: depot_tools

set -e
set -x

cd /SRC
./bin/fetch-gn
./bin/gn gen /OUT "--args=$1"
/start_dir/depot_tools/ninja -C /OUT
