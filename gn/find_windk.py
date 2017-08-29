#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

msvc = int(sys.argv[1])

editions = [
  'Community',
  'Professional',
  'Enterprise',
]

if msvc == 2015:
  # just assume it is installed to the default location
  print 'C:/Program Files (x86)/Microsoft Visual Studio 14.0'
else:
  # try each of the editions
  root = 'C:/Program Files (x86)/Microsoft Visual Studio/2017/'
  for edition in editions:
    if os.path.exists(root + edition):
      print root + edition
      sys.exit(0)
