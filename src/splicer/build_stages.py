#!/usr/bin/env python2.7
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess

cflags = '-std=c++11 -Os -fomit-frame-pointer -mavx2 -mfma -mf16c'

subprocess.check_call(['clang++'] + cflags.split() +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'stages.o'])
