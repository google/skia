#! /bin/sh

# Copyright 2018 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

# Change to the root and remove previously added build assets from the source
# tree of the SKQP app.
cd "$(dirname "$0")/../.."
cd platform_tools/android/apps
git clean -fxd skqp/build \
               skqp/src/main/assets/gmkb \
               skqp/src/main/assets/resources \
               skqp/src/main/libs \
               .gradle build viewer/build
