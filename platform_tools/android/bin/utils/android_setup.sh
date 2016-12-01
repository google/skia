#!/bin/bash
###############################################################################
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
###############################################################################
#
# android_setup.sh: Sets environment variables used by other Android scripts.

# Fail-fast if anything in the script fails.
set -e

IS_DEBUG="false"

while (( "$#" )); do
  if [[ "$1" == "-d" ]]; then
    DEVICE_ID=$2
    shift
  elif [[ "$1" == "-i" || "$1" == "--resourcePath" ]]; then
    RESOURCE_PATH=$2
    APP_ARGS=("${APP_ARGS[@]}" "${1}" "${2}")
    shift
  elif [[ "$1" == "-s" ]]; then
    DEVICE_SERIAL="-s $2"
    shift
  elif [[ "$1" == "--debug" ]]; then
    IS_DEBUG="true"
  elif [[ "$1" == "--logcat" ]]; then
    LOGCAT=1
  elif [[ "$1" == "--verbose" ]]; then
    VERBOSE="true"
  elif [[ "$1" == "--vulkan" ]]; then
    SKIA_VULKAN="true"
  else
    APP_ARGS=("${APP_ARGS[@]}" "${1}")
  fi
  shift
done

function verbose {
  if [[ -n $VERBOSE ]]; then
    echo $@
  fi
}

function exportVar {
  NAME=$1
  VALUE=$2
  verbose export $NAME=\"$VALUE\"
  export $NAME="$VALUE"
}

function absPath {
  (cd $1; pwd)
}

UTIL_DIR=$(absPath "$(dirname "$BASH_SOURCE[0]}")")

if [ -z "$ANDROID_SDK_ROOT" ]; then
  if ANDROID_SDK_ROOT="$(dirname $(which android))/.."; then
    exportVar ANDROID_SDK_ROOT $ANDROID_SDK_ROOT
  else
     echo "No ANDROID_SDK_ROOT set and can't auto detect it from location of android binary."
     exit 1
  fi
fi

if [ -z "$ANDROID_HOME" ]; then
  echo "ANDROID_HOME not set so we are setting it to a default value of ANDROID_SDK_ROOT"
  exportVar ANDROID_HOME $ANDROID_SDK_ROOT
fi

if [ -z "$ANDROID_NDK_ROOT" ]; then
  if [ -d "${ANDROID_SDK_ROOT}/ndk-bundle" ]; then
    exportVar ANDROID_NDK_ROOT ${ANDROID_SDK_ROOT}/ndk-bundle
  else
     echo "No ANDROID_NDK_ROOT set and can't auto detect it from location of SDK."
     exit 1
  fi
fi

# Helper function to configure the GN defines to the appropriate values
# based on the target device.
setup_device() {
  DEFINES="ndk=\"${ANDROID_NDK_ROOT}\" is_debug=${IS_DEBUG}"

  if [ $SKIA_VULKAN == "true" ]; then
    DEFINES="${DEFINES} ndk_api=24"
  fi

  # Setup the build variation depending on the target device
  TARGET_DEVICE="$1"

  if [ -z "$TARGET_DEVICE" ]; then
    if [ -f .android_config ]; then
      TARGET_DEVICE=$(cat .android_config)
      verbose "no target device (-d), using ${TARGET_DEVICE} from most recent build"
    else
      TARGET_DEVICE="arm_v7"
      verbose "no target device (-d), using ${TARGET_DEVICE}"
    fi
  fi

  case $TARGET_DEVICE in
    arm_v7 | nexus_4 | nexus_5 | nexus_6 | nexus_7 | nexus_10)
      DEFINES="${DEFINES} target_cpu=\"arm\""
      GDBSERVER_DIR="${ANDROID_NDK_ROOT}/prebuilt/android-arm"
      IS_64_BIT=false
      ;;
    arm64 | nexus_9 | nexus_5x | nexus_6p | pixel)
      DEFINES="${DEFINES} target_cpu=\"arm64\""
      GDBSERVER_DIR="${ANDROID_NDK_ROOT}/prebuilt/android-arm64"
      IS_64_BIT=true
      ;;
    x86)
      DEFINES="${DEFINES} target_cpu=\"x86\""
      GDBSERVER_DIR="${ANDROID_NDK_ROOT}/prebuilt/android-x86"
      IS_64_BIT=false
      ;;
    x86_64 | x64)
      DEFINES="${DEFINES} target_cpu=\"x64\""
      GDBSERVER_DIR="${ANDROID_NDK_ROOT}/prebuilt/android-x86_64"
      IS_64_BIT=true
      ;;
    mips)
      DEFINES="${DEFINES} target_cpu=\"mipsel\""
      GDBSERVER_DIR="${ANDROID_NDK_ROOT}/prebuilt/android-mips"
      IS_64_BIT=false
      #DEFINES="${DEFINES} skia_resource_cache_mb_limit=32"
      ;;
    mips64)
      DEFINES="${DEFINES} target_cpu=\"mips64el\""
      GDBSERVER_DIR="${ANDROID_NDK_ROOT}/prebuilt/android-mips64"
      IS_64_BIT=true
      ;;
    *)
      echo "ERROR: unknown device $TARGET_DEVICE"
      exit 1
      ;;
  esac

  verbose "The build is targeting the device: $TARGET_DEVICE"
  exportVar DEVICE_ID $TARGET_DEVICE
  exportVar GN_ARGS "$DEFINES"
  exportVar GDBSERVER_DIR $GDBSERVER_DIR
  exportVar IS_64_BIT $IS_64_BIT

  SKIA_SRC_DIR=$(cd "${UTIL_DIR}/../../../.."; pwd)
  DEFAULT_SKIA_OUT="${SKIA_SRC_DIR}/out/android-${TARGET_DEVICE}"
  exportVar SKIA_OUT "${SKIA_OUT:-${DEFAULT_SKIA_OUT}}"
}

# adb_pull_if_needed(android_src, host_dst)
adb_pull_if_needed() {

  # get adb location
  source $SCRIPT_DIR/utils/setup_adb.sh

  # read input params
  ANDROID_SRC="$1"
  HOST_DST="$2"

  if [ -f $HOST_DST ];
  then
    #get the MD5 for dst and src depending on OS and/or OS revision
    ANDROID_MD5_SUPPORT=`$ADB $DEVICE_SERIAL shell ls -ld /system/bin/md5`
    if [ "${ANDROID_MD5_SUPPORT:0:15}" != "/system/bin/md5" ]; then
      ANDROID_MD5=`$ADB $DEVICE_SERIAL shell md5 $ANDROID_DST`
    else
      ANDROID_MD5=`$ADB $DEVICE_SERIAL shell md5sum $ANDROID_DST`
    fi
    if [ $(uname) == "Darwin" ]; then
      HOST_MD5=`md5 -q $HOST_DST`
    else
      HOST_MD5=`md5sum $HOST_DST`
    fi

    if [ "${ANDROID_MD5:0:32}" != "${HOST_MD5:0:32}" ]; then
      echo -n "$HOST_DST "
      $ADB $DEVICE_SERIAL pull $ANDROID_SRC $HOST_DST
    fi
  else
    echo -n "$HOST_DST "
    $ADB $DEVICE_SERIAL pull $ANDROID_SRC $HOST_DST
  fi
}

# adb_push_if_needed(host_src, android_dst)
adb_push_if_needed() {

  # get adb location
  source $SCRIPT_DIR/utils/setup_adb.sh

  # read input params
  local HOST_SRC="$1"
  local ANDROID_DST="$2"

  # disable crashing on failed commands since newer (N+) versions of Android
  # return an error when attempting to run ls on a directory or file that does
  # not exist.
  set +e

  ANDROID_LS=`$ADB $DEVICE_SERIAL shell ls -ld $ANDROID_DST`
  HOST_LS=`ls -ld $HOST_SRC`
  if [ "${ANDROID_LS:0:1}" == "d" -a "${HOST_LS:0:1}" == "-" ];
  then
    ANDROID_DST="${ANDROID_DST}/$(basename ${HOST_SRC})"
  fi

  ANDROID_LS=`$ADB $DEVICE_SERIAL shell ls -ld $ANDROID_DST 2> /dev/null`
  if [ "${ANDROID_LS:0:1}" == "-" ]; then
    #get the MD5 for dst and src depending on OS and/or OS revision
    ANDROID_MD5_SUPPORT=`$ADB $DEVICE_SERIAL shell ls -ld /system/bin/md5 2> /dev/null`
    if [ "${ANDROID_MD5_SUPPORT:0:1}" == "-" ]; then
      ANDROID_MD5=`$ADB $DEVICE_SERIAL shell md5 $ANDROID_DST`
    else
      ANDROID_MD5=`$ADB $DEVICE_SERIAL shell md5sum $ANDROID_DST`
    fi

    if [ $(uname) == "Darwin" ]; then
      HOST_MD5=`md5 -q $HOST_SRC`
    else
      HOST_MD5=`md5sum $HOST_SRC`
    fi

    if [ "${ANDROID_MD5:0:32}" != "${HOST_MD5:0:32}" ]; then
      echo -n "$ANDROID_DST "
      $ADB $DEVICE_SERIAL push $HOST_SRC $ANDROID_DST
    fi
  elif [ "${ANDROID_LS:0:1}" == "d" ]; then
    for FILE_ITEM in `ls $HOST_SRC`; do
      adb_push_if_needed "${HOST_SRC}/${FILE_ITEM}" "${ANDROID_DST}/${FILE_ITEM}"
    done
  else
    HOST_LS=`ls -ld $HOST_SRC`
    if [ "${HOST_LS:0:1}" == "d" ]; then
      $ADB $DEVICE_SERIAL shell mkdir -p $ANDROID_DST
      adb_push_if_needed $HOST_SRC $ANDROID_DST
    else
      echo -n "$ANDROID_DST "
      $ADB $DEVICE_SERIAL shell mkdir -p "$(dirname "$ANDROID_DST")"
      $ADB $DEVICE_SERIAL push $HOST_SRC $ANDROID_DST
    fi
  fi

  # turn error checking back on
  set -e
}

setup_device "${DEVICE_ID}"
