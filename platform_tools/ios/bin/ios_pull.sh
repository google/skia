#!/bin/bash
#
###############################################################################
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
###############################################################################
#
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/ios_setup.sh 

TARGET_DIR=$1

ios_pull "*" "$TARGET_DIR"

