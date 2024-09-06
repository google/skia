#!/bin/bash
# Copyright 2024 Google, LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -ex

# To be run on Louhi to create the fiddler final image which has both the
# compiled go code (fiddler) and the pre-compiled C++ fiddle and sec_wrap binary.

# BASE_DIR is where this script is
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`

# Run docker build from the Skia root so the entire Skia repo is part of the
# Docker context
cd $BASE_DIR/../../ && pwd && ls -ahl && docker build --tag fiddler-final . --file infra/fiddler-backend/Dockerfile
