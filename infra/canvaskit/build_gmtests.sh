#!/bin/bash
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container of emsdk-base
# and a Skia checkout has been mounted at /SRC and the output directory
# is mounted at /OUT

set +e
set -x
# Clean out previous builds (ignoring any errors for things like folders)
# (e.g. we don't want to delete /OUT/depot_tools/)
rm -f /OUT/*
set -e

# BASE_DIR is the dir this script is in ($SKIA_ROOT/infra/canvaskit)
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
CANVASKIT_DIR=$BASE_DIR/../../modules/canvaskit

BUILD_DIR=/OUT $CANVASKIT_DIR/compile_gm.sh $@

# Make sure everybody can read and write the contents of /OUT
chmod -R 0777 /OUT/*

