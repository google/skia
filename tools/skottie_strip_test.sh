#!/bin/bash

SKIA=~/sk
EMSDK=~/sdk/emsdk

source $EMSDK/emsdk_env.sh

if [[ -z "$1" ]]; then
    echo "Usage: $0 <samples_dir>"
    exit 1
fi

#for f in $1/*; do
#    node $SKIA/tools/skottie_strip.js "$f"
#done

node $SKIA/tools/skottie_strip.js $1/* &&
mkdir -p ${1}_min &&
for f in $1/*_min.json; do
    name=$(basename "$f" | sed 's/_min.json$/.json/')
    mv "$f" "${1}_min/$name"
done &&
$SKIA/out/Release/dm --src json --jsons ${1}_min --config 8888 -w /tmp/dm_min >/dev/null 2>&1 &&
$SKIA/out/Release/skdiff /tmp/dm_master /tmp/dm_min /tmp/skdiff

rm -f $1/*_min.json
