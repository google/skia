#! /bin/sh

# Copyright 2018 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if ! [ -f "$1" ]; then
    printf 'Usage:\n  %s META_JSON_FILE_PATH\n\n' "$0" >&2
    exit 1
fi

set -e -x

SKIA="$(dirname "$0")/../.."

go get -u go.skia.org/infra/golden/go/search

go run "${SKIA}/tools/skqp/make_gmkb.go" "$1" \
    "${SKIA}/platform_tools/android/apps/skqp/src/main/assets/gmkb"

