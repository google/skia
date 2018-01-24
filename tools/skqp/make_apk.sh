#! /bin/sh

# Copyright 2018 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

ANDROID_NDK="$1"

if ! [ -d "$ANDROID_NDK" ] || ! [ -x "${ANDROID_NDK}/ndk-build" ]; then
    printf "\nUsage:\n  %s ANDROID_NDK_PATH\n" "$0" >&2
    exit 1
fi

case ":${PATH}:" in
    */depot_tools:*) ;;
    *)
        printf '\ndepot_tools should be in your $PATH.\n' >&2
        exit 1;;
esac

if ! [ -d "$ANDROID_HOME" ] || ! [ -x "${ANDROID_HOME}/platform-tools/adb" ]; then
    printf '\n$ANDROID_HOME not set or is broken.\n' >&2
    exit 1
fi

set -x

ARCH=${SKQP_ARCH:-arm}

cd "$(dirname "$0")/../.."

BUILD=out/skqp-${ARCH}

python tools/skqp/generate_gn_args $BUILD "$ANDROID_NDK" $ARCH

GIT_SYNC_DEPS_QUIET=Y tools/git-sync-deps

bin/gn gen $BUILD

rm -rf $BUILD/gen

platform_tools/android/bin/android_build_app -C $BUILD skqp

set +x

printf '\n\nAPK built: "%s/skqp.apk"\n\n' "$(pwd)/$BUILD"

