#!/bin/bash

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
    clang-tidy -quiet $src -- $args
fi
exec clang++ $args

