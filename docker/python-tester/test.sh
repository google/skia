#! /bin/sh
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# For use with:
#   docker build -t python-tester docker/python-tester
#   docker run --rm --volume="$PWD":/SRC python-tester /SRC/docker/python-tester/test.sh

set -x -e
TMP="$(mktemp -d)"
git clone --quiet "$(cd "$(dirname "$0")/../.."; pwd)" "$TMP"/skia
cd "$TMP"/skia

python2 tools/git-sync-deps
git clean -ffxd
python3 tools/git-sync-deps

echo SUCCESS
