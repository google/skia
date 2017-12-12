#! /usr/bin/env python2
# Copyright 2017 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import sys
import zlib
sys.stdout.write(zlib.decompress(sys.stdin.read()))

