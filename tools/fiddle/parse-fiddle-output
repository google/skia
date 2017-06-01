#!/bin/sh
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Parse the output of fiddle_main, for use in testing
while IFS= read -r line; do
    type=$(echo $line | sed -n 's/[^"]*"\([^"]*\)":.*/\1/p')
    if [ "$type" ]; then
        case "$type" in
            Raster|Gpu)  ext='.png';;
            Pdf)         ext='.pdf';;
            Skp)         ext='.skp';;
            Text|GLInfo) ext='.txt';;
        esac
        dst="${TMPDIR:-/tmp}/fiddle_${type}${ext}"
        echo $line | sed 's/[^"]*"[^"]*": "//; s/"\(,\|\)$//' \
            | base64 -d > "$dst"
        echo $dst
    fi
done
