#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Assumes this is in a docker container with a skia repo mounted at /SRC

cd "$(dirname "$0")/../.."
[ -d /OUT ] && export SKQP_OUTPUT_DIR=/OUT
python tools/skqp/make_universal_apk.py x86
