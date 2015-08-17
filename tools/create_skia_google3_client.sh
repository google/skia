#!/bin/bash
# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Create a google3 CitC client in which code can be submitted to either google3
# or Git.
#
# Usage:
#      ./tools/create_skia_google3_client.sh <client_name>

source gbash.sh || exit

set -x -e

CLIENT="$1"
g4 citc "${CLIENT}"
cd "/google/src/cloud/${USER}/${CLIENT}/google3/third_party/skia/HEAD"

REV="$(cat README.google | grep -e "^Version" | sed "s/^Version: \(.*\)/\1/")"

MY_DIR="$(gbash::get_absolute_caller_dir)"
${MY_DIR}/git_clone_to_google3.sh --skia_rev "${REV}"
