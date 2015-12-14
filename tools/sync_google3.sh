#!/bin/bash
# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Syncs //depot/google3/third_party/skia/HEAD to the latest revision of Skia.
# If this script is not run from a Piper client, creates a new CitC client. Also
# updates README.google.
#
# Usage:
#      ./tools/sync_google3.sh

prodcertstatus -q || (echo "Please run prodaccess." 1>&2; exit 1)
source gbash.sh || exit 2

set -e

MY_DIR="$(gbash::get_absolute_caller_dir)"
SKIA_REV="$(git ls-remote https://skia.googlesource.com/skia refs/heads/master | cut -f 1)"

gbash::get_google3_dir && GOOGLE3="$(gbash::get_google3_dir)"
if [ -z "${GOOGLE3}" ]; then
  CLIENT_NAME="sync_skia_$(date '+%F-%H-%M-%S')"
  ${MY_DIR}/create_skia_google3_client.sh "${CLIENT_NAME}"
  GOOGLE3="/google/src/cloud/${USER}/${CLIENT_NAME}/google3"
fi
cd "${GOOGLE3}/third_party/skia/HEAD"
${MY_DIR}/git_clone_to_google3.sh --skia_rev "${SKIA_REV}"

echo "Synced client ${CLIENT_NAME} to ${SKIA_REV}"

# Grab previous Git revision.
GOOGLE3_REV="$(grep "Version: " README.google | cut -d ' ' -f 2)"

# Update README.google.
sed --in-place "s/^Version: .*/Version: ${SKIA_REV}/" README.google
sed --in-place "s/URL: https:\/\/skia.googlesource.com\/skia\/+archive\/.*\.tar\.gz/URL: https:\/\/skia.googlesource.com\/skia\/+archive\/${SKIA_REV}.tar.gz/" README.google

# Add README.google to the default change.
g4 reopen

# Generate commit description.
CURRENT_DATE=`date '+%d %B %Y'`
COMMIT_RANGE="${GOOGLE3_REV:0:9}..${SKIA_REV:0:9}"
CHANGES="$(git log ${COMMIT_RANGE} --date=short --no-merges --format='%ad %ae %s')"
COMMITS="$(wc -l <<EOF
${CHANGES}
EOF
)"
DESC="$(cat <<EOF
Roll Skia ${COMMIT_RANGE} (${COMMITS} commits)

https://chromium.googlesource.com/skia.git/+log/${COMMIT_RANGE}

$ git log ${COMMIT_RANGE} --date=short --no-merges --format='%ad %ae %s'
${CHANGES}

Change created by sync_google3.sh on ${CURRENT_DATE} by ${USER}
EOF
)"

# Create a new CL.
CHANGE="$(g4 change --desc "${DESC}.")"
CL="$(echo "${CHANGE}" | sed "s/Change \([0-9]\+\) created.*/\1/")"

echo "Created CL ${CL} (http://cl/${CL})"

# Run presubmit (will run TAP tests).
if g4 presubmit -c "${CL}"; then
  echo "CL is ready for review and submit at http://cl/${CL}"
else
  echo "Presubmit failed for CL ${CL} in client ${CLIENT_NAME}" 1>&2
  exit 3
fi
