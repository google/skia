#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Assumes this is in a docker container with a skia repo mounted at /SRC

[ -d /OUT ] || echo '/OUT is missing!'

SKQP_OUTPUT_DIR=/OUT python /SRC/tools/skqp/make_universal_apk.py x86
