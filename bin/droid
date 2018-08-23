#!/bin/bash

# Run a GN-built Android binary on the connected device.
#
# Example usage:
#  $ ninja -C out dm
#  $ droid out/dm --src gm --config gpu
#
# See https://skia.org/user/quick/gn for build instructions.

dst_dir=/data/local/tmp
path="$1"
name="$(basename "$path")"
shift

if ! [ -d resources ]; then
    echo run this from the skia tree
    exit 1
fi

dirs=''
for dir in $(find resources -type d); do dirs="$dirs \"${dir}\""; done

set -e
set -x

adb shell "cd \"$dst_dir\"; mkdir -p $dirs"
adb push --sync resources "${dst_dir}/"
adb push --sync "$path" "${dst_dir}/${name}"
adb shell "cd \"$dst_dir\"; chmod +x \"$name\"; \"./$name\" $*"
