#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container of perf-karma-chrome-tests
# and a Skia checkout has been mounted at /SRC and the output directory
# is mounted at /OUT

set -ex

#BASE_DIR is the dir this script is in ($SKIA_ROOT/infra/pathkit)
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
PATHKIT_DIR=$BASE_DIR/../../modules/pathkit

# We avoid a lot of file permission errors by copying the tests and binary into the Docker
# container and running npm ci there.
cp -R $PATHKIT_DIR /tmp/work
cd /tmp/work
npm ci --cache /tmp/npmcache

# Start the aggregator in the background
/opt/perf-aggregator $@ &
# Run the tests 10 times to get a wide set of data
for i in `seq 1 10`;
do
    npx karma start /tmp/work/karma.bench.conf.js --single-run
done
# Tell the aggregator to dump the json
# This curl command gets the HTTP code and stores it into $CODE
CODE=`curl -s -o /dev/null -I -w "%{http_code}" -X POST localhost:8081/dump_json`
if [ $CODE -ne 200 ]; then
    # If we don't get 200 back, something is wrong with writing to disk, so exit with error
    exit 1
fi
