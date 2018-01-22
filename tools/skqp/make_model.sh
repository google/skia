#! /bin/sh
# Copyright 2018 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e -x

test -f "$1"

go get -u go.skia.org/infra/golden/go/search

go run tools/skqp/make_gmkb.go "$1" platform_tools/android/apps/skqp/src/main/assets/gmkb

git add platform_tools/android/apps/skqp/src/main/assets/gmkb

