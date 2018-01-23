#! /bin/sh

# Copyright 2018 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

EXTANT="$(mktemp "${TMPDIR:-/tmp}/extant.XXXXXXXXXX")"
gsutil ls gs://skia-skqp-assets/ | sed 's|^gs://skia-skqp-assets/||'  > "$EXTANT"

upload() {
    MD5=$(md5sum < "$1" | head -c 32)
    if ! grep -q "$MD5" "$EXTANT"; then
        URL="gs://skia-skqp-assets/$MD5"
        gsutil cp "$1" "$URL" > /dev/null 2>&1 &
    fi
    echo $MD5
}

cd "$(dirname "$0")/../../platform_tools/android/apps/skqp/src/main/assets"

rm -f files.checksum

FILES="$(mktemp "${TMPDIR:-/tmp}/files.XXXXXXXXXX")"

: > "$FILES"

COUNT=$(find * -type f | wc -l)
INDEX=1
SHARD_COUNT=32

find * -type f | sort | while IFS= read -r FILENAME; do
    printf '\r %d / %d   ' "$INDEX" "$COUNT"
    if ! [ -f "$FILENAME" ]; then
        echo error [${FILENAME}] >&2;
        exit 1;
    fi
    case "$FILENAME" in *\;*) echo bad filename: $FILENAME >&2; exit 1;; esac
    MD5=$(upload "$FILENAME")
    printf '%s;%s\n' "$MD5" "$FILENAME" >> "$FILES"

    if [ $(($INDEX % $SHARD_COUNT)) = 0 ]; then
        wait
    fi
    INDEX=$(( $INDEX + 1))
done
printf '\rdone          \n'
upload "$FILES" > files.checksum
wait

rm "$FILES" "$EXTANT"
