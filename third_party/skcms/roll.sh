#!/bin/bash

# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e
#set -x

here=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
skcms=$(mktemp -d)

if [ "$#" == "0" ]; then
    git clone --quiet --depth 1 https://skia.googlesource.com/skcms.git $skcms
else
    git clone --quiet           https://skia.googlesource.com/skcms.git $skcms
    git -C $skcms checkout $1
fi

git rm --quiet    $here/LICENSE
git rm --quiet    $here/skcms.h
git rm --quiet    $here/skcms.c
git rm --quiet -r $here/src

cp    $skcms/LICENSE $here
cp    $skcms/skcms.h $here
cp    $skcms/skcms.c $here
cp -r $skcms/src     $here

git add $here
git commit -m "skcms→$(git -C $skcms log -1 --pretty=format:'%h %s')"

rm -rf $skcms
