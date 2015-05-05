#!/bin/bash

###############################################################################
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
###############################################################################
#
# Writes the content of the file identified by the first argument to standard
# output. The path is relative to the Documents directory of the installed app.
#

# Do not set -x here, since we only want the content of the file sent to
# standard out.
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $SCRIPT_DIR/ios_setup.sh

ios_cat $1
