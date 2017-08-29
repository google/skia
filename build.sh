#!/bin/bash

set -x -e

# ABI=arm64-v8a
# LIBS_DIR=experimental/CTS18/app/libs/${ABI}/
# OUT_DIR=out/${ABI}

# python ./tools/git-sync-deps
# ./bin/gn gen ${OUT_DIR}  --args='ndk="/usr/local/google/home/stephana/Android/Sdk/ndk-bundle" target_cpu="arm64" is_component_build=true'
# ninja -C out/arm64-v8a

# mkdir -p $LIBS_DIR
# cp ${OUT_DIR}/libgmrunner.so ${LIBS_DIR}
# cp ${OUT_DIR}/libskia.so ${LIBS_DIR}

declare -A BUILD_IDS
BUILD_IDS[arm64]=arm64-v8a
# BUILD_IDS[arm]=armeabi-v7a
BUILD_IDS[arm]=armeabi
BUILD_IDS[x64]=x86_64
BUILD_IDS[x86]=x86
BUILD_IDS[mipsel]=mips
BUILD_IDS[mips64el]=mips64

LIBS_DIR=experimental/CTS18/app/libs

python ./tools/git-sync-deps
for BUILD_ID in ${!BUILD_IDS[@]}; do
  OUT_DIR=out/${BUILD_ID}
  # ./bin/gn gen ${OUT_DIR} --args='ndk="/usr/local/google/home/stephana/Android/Sdk/ndk-bundle" target_cpu="'"$BUILD_ID"'" is_component_build=true'
  ./bin/gn gen ${OUT_DIR} --args='is_debug=true ndk="/usr/local/google/home/stephana/Android/Sdk/ndk-bundle" target_cpu="'"$BUILD_ID"'"'
  ninja -C ${OUT_DIR}

  echo ${BUILD_IDS[$BUILD_ID]}
  TARGET_DIR=${LIBS_DIR}/${BUILD_IDS[$BUILD_ID]}
  mkdir -p ${TARGET_DIR}
  echo $TARGET_DIR

  cp ${OUT_DIR}/libskia.a ${TARGET_DIR}
  cp ${OUT_DIR}/libgmrunner.so ${TARGET_DIR}
done
