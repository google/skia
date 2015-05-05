#!/bin/bash

###############################################################################
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
###############################################################################
#
# Pull the given file/directory off the device.
#
set -x -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/ios_setup.sh

DEVICE_PATH=$1
HOST_PATH=$2

ios_pull $DEVICE_PATH $HOST_PATH
