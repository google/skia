# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#!/bin/bash
#
# setup_toolchain.sh: Sets toolchain environment variables used by other scripts.

# Fail-fast if anything in the script fails.
set -e

# check that the preconditions for this script are met
if [ $(type -t verbose) != 'function' ]; then
  echo "ERROR: The verbose function is expected to be defined"
  return 1
fi

if [ $(type -t exportVar) != 'function' ]; then
  echo "ERROR: The exportVar function is expected to be defined"
  return 1
fi

if [ $(type -t absPath) != 'function' ]; then
  echo "ERROR: The absPath function is expected to be defined"
  return 1
fi

if [ -z "$SCRIPT_DIR" ]; then
  echo "ERROR: The SCRIPT_DIR variable is expected to be defined"
  return 1
fi

function default_toolchain() {
  TOOLCHAINS=${SCRIPT_DIR}/../toolchains

  ANDROID_ARCH=${ANDROID_ARCH-arm}
  NDK=r11c

  if [[ $ANDROID_ARCH == *64* ]]; then
      API=21  # Android 5.0
  else
      API=14  # Android 4.0
  fi
  
  if [ "$SKIA_VULKAN" == "true" ]; then
      API=24  # Android N Preview
  fi

  TOOLCHAIN=$ANDROID_ARCH-$NDK-$API
  HOST=`uname | tr '[A-Z]' '[a-z]'`

  exportVar ANDROID_TOOLCHAIN "${TOOLCHAINS}/${TOOLCHAIN}"

  if [ ! -d "$ANDROID_TOOLCHAIN" ]; then
    mkdir -p $TOOLCHAINS
    pushd $TOOLCHAINS
    curl -o $NDK.zip https://dl.google.com/android/repository/android-ndk-$NDK-$HOST-x86_64.zip
    unzip $NDK.zip
    UNZIPPED=android-ndk-$NDK
    ./$UNZIPPED/build/tools/make-standalone-toolchain.sh \
        --use-llvm              \
        --arch=$ANDROID_ARCH    \
        --platform=android-$API \
        --install_dir=$TOOLCHAIN
    cp $UNZIPPED/prebuilt/android-$ANDROID_ARCH/gdbserver/gdbserver $TOOLCHAIN
    cp -r $UNZIPPED/prebuilt/${HOST}-x86_64 $TOOLCHAIN/host_prebuilt
    rm $NDK.zip
    rm -rf $UNZIPPED
    popd
  fi

  verbose "Targeting NDK API $API (NDK Revision $NDK)"
}

#check to see if the toolchain has been defined and if not setup the default toolchain
if [ -z "$ANDROID_TOOLCHAIN" ]; then
  default_toolchain
  if [ ! -d "$ANDROID_TOOLCHAIN" ]; then
    echo "ERROR: unable to download/setup the required toolchain (${ANDROID_TOOLCHAIN})"
    return 1;
  fi
fi

GCC=$(command ls $ANDROID_TOOLCHAIN/bin/*-gcc | head -n1)
if [ -z "$GCC" ]; then
  echo "ERROR: Could not find Android cross-compiler in: ${ANDROID_TOOLCHAIN}/bin"
  return 1
fi

# Remove the '-gcc' at the end to get the full toolchain prefix
ANDROID_TOOLCHAIN_PREFIX=${GCC%%-gcc}

CCACHE=${ANDROID_MAKE_CCACHE-$(which ccache || true)}

# Cross compiling Android on Mac is not currently supported by gyp.
# It doesn't appear to be supported on Windows either.
# As of now, we will only support cross compiling on Linux.
# libjpeg-turbo assembly code for x86 and x86-64 Android devices
# must be disabled for Android on non-Linux platforms because
# of this issue.  We still support compiling on Mac and other
# variants for local development, but shipping versions of Skia
# should be compiled on Linux for performance reasons.
# TODO (msarett): Collect more information about this.
if [ $HOST == "linux" ]; then
  if [ "$USE_CLANG" != "true" ]; then
    exportVar CC_target "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-gcc"
    exportVar CXX_target "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-g++"
    exportVar LINK_target "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-gcc"
    exportVar CC_host "$CCACHE cc"
    exportVar CXX_host "$CCACHE c++"
    exportVar LINK_host "$CCACHE cc"
  else
    exportVar CC_target "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-clang"
    exportVar CXX_target "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-clang++"
    exportVar LINK_target "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-clang"
    exportVar CC_host "$CCACHE clang"
    exportVar CXX_host "$CCACHE clang++"
    exportVar LINK_host "$CCACHE clang"
  fi

  exportVar AR_target "$ANDROID_TOOLCHAIN_PREFIX-ar"
  exportVar RANLIB_target "$ANDROID_TOOLCHAIN_PREFIX-ranlib"
  exportVar OBJCOPY_target "$ANDROID_TOOLCHAIN_PREFIX-objcopy"
  exportVar STRIP_target "$ANDROID_TOOLCHAIN_PREFIX-strip"
  exportVar AR_host "ar"
  exportVar RANLIB_host "ranlib"
  exportVar OBJCOPY_host "objcopy"
  exportVar STRIP_host "strip"
else
  if [ "$USE_CLANG" != "true" ]; then
    exportVar CC "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-gcc"
    exportVar CXX "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-g++"
    exportVar LINK "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-gcc"
  else
    exportVar CC "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-clang"
    exportVar CXX "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-clang++"
    exportVar LINK "$CCACHE $ANDROID_TOOLCHAIN_PREFIX-clang"
  fi

  exportVar AR "$ANDROID_TOOLCHAIN_PREFIX-ar"
  exportVar RANLIB "$ANDROID_TOOLCHAIN_PREFIX-ranlib"
  exportVar OBJCOPY "$ANDROID_TOOLCHAIN_PREFIX-objcopy"
  exportVar STRIP "$ANDROID_TOOLCHAIN_PREFIX-strip"
fi

# Create symlinks for nm & readelf and add them to the path so that the ninja
# build uses them instead of attempting to use the one on the system.
# This is required to build using ninja on a Mac.
if [ $HOST == "darwin" ]; then
  ln -sf $ANDROID_TOOLCHAIN_PREFIX-nm $ANDROID_TOOLCHAIN/bin/nm
  ln -sf $ANDROID_TOOLCHAIN_PREFIX-readelf $ANDROID_TOOLCHAIN/bin/readelf
  ln -sf $ANDROID_TOOLCHAIN_PREFIX-as $ANDROID_TOOLCHAIN/bin/as
fi

exportVar PATH ${ANDROID_TOOLCHAIN}/bin:${PATH}
