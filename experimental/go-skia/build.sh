#!/bin/bash

cd ../..
./bin/sync
gn gen out/Shared --args='is_official_build=false is_component_build=true'
ninja -C out/Shared
# SKIA_LIB_DIR="${PWD}/out/Shared"
