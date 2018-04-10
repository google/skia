#!/bin/bash

# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e
#set -x

tmp=$(mktemp -d)
git clone --quiet --depth 1 https://skia.googlesource.com/skcms.git $tmp

git rm --quiet    LICENSE
git rm --quiet    skcms.h
git rm --quiet    skcms.c
git rm --quiet -r src

cp    $tmp/LICENSE .
cp    $tmp/skcms.h .
cp    $tmp/skcms.c .
cp -r $tmp/src     .

git add .
git commit -m "skcms→$(git -C $tmp log -1 --pretty=format:'%h %s')"

rm -rf $tmp
