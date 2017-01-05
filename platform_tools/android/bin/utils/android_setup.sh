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
  if [[ "$1" == "-C" ]]; then
    SKIA_OUT=$2
    shift
  elif [[ "$1" == "-i" || "$1" == "--resourcePath" ]]; then
    RESOURCE_PATH=$2
    APP_ARGS=("${APP_ARGS[@]}" "${1}" "${2}")
    shift
  elif [[ "$1" == "-s" ]]; then
    DEVICE_SERIAL="-s $2"
    shift
  elif [[ "$1" == "--logcat" ]]; then
    LOGCAT=1
  elif [[ "$1" == "--verbose" ]]; then
    VERBOSE="true"
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

if [ -z "$ANDROID_NDK_ROOT" ]; then
  if [ -d "${ANDROID_SDK_ROOT}/ndk-bundle" ]; then
    exportVar ANDROID_NDK_ROOT ${ANDROID_SDK_ROOT}/ndk-bundle
  else
     echo "No ANDROID_NDK_ROOT set and can't auto detect it from location of SDK."
     exit 1
  fi
fi

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
