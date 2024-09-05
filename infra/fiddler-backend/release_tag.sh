#!/bin/bash
# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Build a tag for the Docker image based on the current datetime, user, and
# repo state.

set -e

DATETIME=`date -u "+%Y-%m-%dT%H_%M_%SZ"`
HASH=`git rev-parse HEAD`

echo "${DATETIME}-${HASH:0:7}"
