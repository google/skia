#!/bin/bash
# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Syncs //depot/google3/third_party/skia/HEAD to the last known good revision of
# Skia. If this script is not run from a Piper client, creates a new CitC
# client. Also updates README.google.
#
# Usage:
#      ./tools/sync_google3.sh

source gbash.sh || exit

set -x -e

MY_DIR="$(gbash::get_absolute_caller_dir)"
LKGR="$(${MY_DIR}/get_skia_lkgr.sh)"

gbash::get_google3_dir && GOOGLE3="$(gbash::get_google3_dir)"
if [ -z "${GOOGLE3}" ]; then
  CLIENT_NAME="sync_skia_$(date '+%F-%H-%M-%S')"
  ${MY_DIR}/create_skia_google3_client.sh "${CLIENT_NAME}"
  GOOGLE3="/google/src/cloud/${USER}/${CLIENT_NAME}/google3"
fi
cd "${GOOGLE3}/third_party/skia/HEAD"
${MY_DIR}/git_clone_to_google3.sh --skia-rev "${LKGR}"

# Update README.google.
sed --in-place "s/^Version: .*/Version: ${LKGR}/" README.google
sed --in-place "s/URL: http:\/\/skia.googlesource.com\/skia\/+archive\/.*\.tar\.gz/URL: http:\/\/skia.googlesource.com\/skia\/+archive\/${LKGR}.tar.gz/" README.google
CURRENT_DATE=`date '+%d %B %Y'`
echo "Updated using sync_google3.sh on $CURRENT_DATE by $USER@google.com" >> README.google

# Add README.google to the default change.
g4 reopen
# Create a new CL.
CHANGE="$(g4 change --desc "Update skia HEAD to ${LKGR}.")"
CL="$(echo "${CHANGE}" | sed "s/Change \([0-9]\+\) created.*/\1/")"

# Run TAP.
tap_presubmit -c "${CL}" -p skia
