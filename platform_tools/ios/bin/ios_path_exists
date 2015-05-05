#!/bin/bash

###############################################################################
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
###############################################################################
#
# Check if ios path exists.
#

set -x -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/ios_setup.sh

if [[ "$#" -ne "1" ]]; then
	echo "Usage: ios_path_exists <path_in_Documents_folder_of_ios_device>"
	exit 1
fi

RET=ios_path_exists $1
exit $RET
