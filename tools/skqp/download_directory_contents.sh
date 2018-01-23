#! /bin/sh

# Copyright 2018 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

cd "$(dirname "$0")/../../platform_tools/android/apps/skqp/src/main/assets"

checksum() {
    [ -f "$1" ] && { md5sum < "$1" | head -c 32; }
}

download() {
    if ! [ $1 = "$(checksum "$2")" ]; then
        mkdir -p "$(dirname "$2")"
        curl -s -o "$2" "https://storage.googleapis.com/skia-skqp-assets/$1"
    fi
}

download $(cat files.checksum) files.txt

COUNT=$(wc -l < files.txt)
INDEX=1
SHARD_COUNT=32

cat files.txt | while IFS= read -r LINE; do
    MD5=$(echo $LINE | awk -F\; '{print $1}')
    FILENAME=$(echo $LINE | awk -F\; '{print $2}')
    download $MD5 "$FILENAME" &
    if [ $(($INDEX % $SHARD_COUNT)) = 0 ]; then
        wait
        printf '\r %d / %d ' "$INDEX" "$COUNT"
    fi
    INDEX=$(($INDEX + 1))
done
printf '\rdone                \n'

