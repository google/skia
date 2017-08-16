#!/bin/bash

set -x -e

python ./tools/git-sync-deps
./bin/gn gen out/arm64 --args='ndk="/usr/local/google/home/stephana/Android/Sdk/ndk-bundle" target_cpu="arm64"'
ninja -C out/arm64