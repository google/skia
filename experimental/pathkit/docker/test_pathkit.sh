#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container of emsdk-base
# and a Skia checkout has been mounted at /SRC and the output directory
# is mounted at /OUT

# For example:
# docker run -v $SKIA_ROOT:/SRC -v /tmp/dockerout:/OUT gcr.io/skia-public/karma-chrome-tests:68.0.3440.106_v1 /SRC/experimental/pathkit/docker/test_pathkit.sh

set -ex

#BASE_DIR is the dir this script is in
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`

# Start the aggregator
go run $BASE_DIR/../gold/pathkit_aggregator.go $@ &
# Run the tests
npx karma start $BASE_DIR/../karma-docker.conf.js --single-run
# Tell the aggregator to dump the json
curl localhost:8081/dump_json
