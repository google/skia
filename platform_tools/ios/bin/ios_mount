#!/bin/bash
#
###############################################################################
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
###############################################################################
#
# Mounts the iOS device locally. See the value of IOS_MOUNT_POINT in
# ios_setup.sh for the exact location.
#
set -x -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/ios_setup.sh

ios_mount
