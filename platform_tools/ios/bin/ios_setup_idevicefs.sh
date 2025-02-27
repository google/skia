#!/bin/bash
###############################################################################
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
###############################################################################
#
# ios_setup_idevicefs.sh: defines functions used by other iOS scripts.

if [ -z "$IOS_BUNDLE_ID" ]; then
  echo "Please set the IOS_BUNDLE_ID variable."
  exit 1
fi

# Location on the ios device where all data are stored. This is
# relative to the mount point.
IOS_DOCS_DIR="@${IOS_BUNDLE_ID}/Documents"

ios_rm() {
  local TARGET="$IOS_DOCS_DIR/$1"
  idevicefs rm -Rfd "$TARGET"
}

ios_mkdir() {
  local TARGET="$IOS_DOCS_DIR/$1"
  cpy="$(mktemp -d)"
  idevicefs push "$cpy" "$TARGET"
  rm -rf "$cpy"
}

ios_cat() {
  ios_pull "$1" /dev/stdout
}

# ios_restart: restarts the iOS device.
ios_restart() {
  idevicediagnostics restart
}

# ios_pull(ios_src, host_dst): Copies the content of ios_src to host_dst.
# The path is relative to the 'Documents' folder on the device.
ios_pull() {
  # read input params
  local IOS_SRC="$IOS_DOCS_DIR/$1"
  local HOST_DST="$2"

  idevicefs pull "$IOS_SRC" "$HOST_DST"
}

# ios_push(host_src, ios_dst)
ios_push() {
  # read input params
  local HOST_SRC="$1"
  local IOS_DST="$IOS_DOCS_DIR/$2"

  dir="$(dirname $2)"
  ios_path_exists "$dir" || ios_mkdir "$dir"
  idevicefs push "$HOST_SRC" "$IOS_DST"
}

ios_path_exists() {
  local TARGET_PATH="$IOS_DOCS_DIR/$1"
  idevicefs ls "$TARGET_PATH" > /dev/null
  return $?
}
