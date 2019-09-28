#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container of gold-karma-chrome-tests
# and a Skia checkout has been mounted at /SRC, the output directory
# is mounted at /OUT, and any lottie json files are in a folder and mounted
# at /LOTTIE_FILES.

# For example:
# docker run -v ~/lottie-samples:/LOTTIE_FILES -v $LOTTIE_ROOT/build/player:/LOTTIE_BUILD -v $SKIA_ROOT:/SRC -v /tmp/dockerout:/OUT gcr.io/skia-public/gold-lottie-web-puppeteer:v2 /SRC/infra/lottiecap/docker/lottiecap_gold.sh

set -ex

#BASE_DIR is the dir this script is in ($SKIA_ROOT/infra/lottiecap/docker)
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
LOTTIECAP_DIR=$BASE_DIR/../../../tools/lottiecap

# Start the aggregator in the background
/opt/gold-aggregator $@ &

cd $LOTTIECAP_DIR

# lottie files may have spaces in their names, so a naive bash for loop
# did not work here.
find /LOTTIE_FILES -not -path /LOTTIE_FILES -exec \
    node ./lottiecap.js --port 8082 \
            --lottie_player /LOTTIE_BUILD/lottie.min.js \
            --in_docker \
            --post_to http://localhost:8081/report_gold_data \
            --input {} \;


# Tell the aggregator to dump the json
# This curl command gets the HTTP code and stores it into $CODE
CODE=`curl -s -o /dev/null -I -w "%{http_code}" -X POST localhost:8081/dump_json`
if [ $CODE -ne 200 ]; then
    # If we don't get 200 back, something is wrong with writing to disk, so exit with error
    exit 1
fi
