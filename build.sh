#!/bin/bash

set -x -e

ABI=arm64-v8a
LIBS_DIR=experimental/CTS18/app/libs/${ABI}/
OUT_DIR=out/${ABI}

python ./tools/git-sync-deps
./bin/gn gen ${OUT_DIR}  --args='ndk="/usr/local/google/home/stephana/Android/Sdk/ndk-bundle" target_cpu="arm64" is_component_build=true'
ninja -C out/arm64-v8a

mkdir -p $LIBS_DIR
cp ${OUT_DIR}/libgmrunner.so ${LIBS_DIR}
cp ${OUT_DIR}/libskia.so ${LIBS_DIR}
