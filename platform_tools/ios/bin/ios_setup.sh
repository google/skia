#!/bin/bash
###############################################################################
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
###############################################################################
#
# ios_setup.sh: Sets environment variables used by other iOS scripts.

set -x -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if ! command -v idevicefs 2>&1 > /dev/null; then
  echo "no idevicefs; falling back to ifuse"
  source $SCRIPT_DIR/ios_setup_ifuse.sh
else
  source $SCRIPT_DIR/ios_setup_idevicefs.sh
fi
