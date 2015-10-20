#!/bin/bash
# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Outputs the Last Known Good Revision of Skia.

prodcertstatus -q || (echo "Please run prodaccess." 1>&2; exit 1)
source gbash.sh || exit 2

set -e

# Retrieve last known good revision. (App-engine script is very flaky, so retry
# 10 times.)
LKGR=""
for i in `seq 1 10`; do
  LKGR=$(curl "http://skia-tree-status-staging.appspot.com/lkgr")
  if [[ ${#LKGR} -gt 4 ]]; then
    break
  fi
done
[[ ${#LKGR} -gt 4 ]] || gbash::die "Unable to get Skia LKGR (got '${LKGR}')"
echo "${LKGR}"
