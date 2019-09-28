#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container of emsdk-release
# and a Skia checkout has been mounted at /SRC and the output directory
# is mounted at /OUT

# For example:
# docker run -v $SKIA_ROOT:/SRC -v $SKIA_ROOT/out/dockerpathkit:/OUT gcr.io/skia-public/emsdk-release:1.38.16_v1 /SRC/infra/pathkit/build_pathkit.sh

set +e
set -x
# Clean out previous builds (ignoring any errors for things like folders)
# (e.g. we don't want to delete /OUT/depot_tools/)
rm -f /OUT/*
set -e

#BASE_DIR is the dir this script is in ($SKIA_ROOT/infra/pathkit)
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
PATHKIT_DIR=$BASE_DIR/../../modules/pathkit

BUILD_DIR=/OUT $PATHKIT_DIR/compile.sh $@

