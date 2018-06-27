#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

editions = [
  'Community',
  'Professional',
  'Enterprise',
]

# try each of the editions
root = 'C:/Program Files (x86)/Microsoft Visual Studio/2017/'
for edition in editions:
  if os.path.exists(root + edition):
    print root + edition
    sys.exit(0)
