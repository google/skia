#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Assumes this is in a docker container with a skia repo mounted at /SRC
# and an output directory mounted at /OUT.

if [ -d /OUT ]; then
    rm -rf /OUT/*  # Clean out previous builds
    export SKQP_OUTPUT_DIR=/OUT
fi
/SRC/tools/skqp/make_universal_apk.py x86
