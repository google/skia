#!/bin/bash
#
# Copyright 2018 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

args=""
src=""

while [ "$1" ]; do
    arg=$1

    args="$args $1"
    shift

    if [ "$arg" == "-c" ]; then
        src=$1

        args="$args $1"
        shift
    fi
done

if [ "$src" ]; then
    clang-tidy -quiet -header-filter='.*' -warnings-as-errors='*' $src -- $args
fi
exec clang++ $args

