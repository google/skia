#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Assumes this is in a docker container with a skia repo mounted at /SRC

pushd ../../

./tools/skqp/make_universal_apk x86

# Clean out previous builds
rm /OUT/*
cp out/skqp/skqp-universal-debug.apk /OUT/skqp-universal-x86-debug.apk

popd
