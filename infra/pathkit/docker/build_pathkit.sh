#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container of emsdk-base
# and a Skia checkout has been mounted at /SRC and the output directory
# is mounted at /OUT

# For example:
# docker run -v $SKIA_ROOT:/SRC -v $SKIA_ROOT/out/dockerpathkit:/OUT gcr.io/skia-public/emsdk-release:1.38.6_jre /SRC/infra/pathkit/docker/build_pathkit.sh

#BASE_DIR is the dir this script is in ($SKIA_ROOT/infra/pathkit/docker)
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
PATHKIT_DIR=$BASE_DIR/../../../experimental/pathkit

BUILD_DIR=/OUT $PATHKIT_DIR/compile.sh $@
