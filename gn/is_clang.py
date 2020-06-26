#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function

import subprocess
import sys
cc,cxx = sys.argv[1:3]

if (b'clang' in subprocess.check_output('%s --version' % cc, shell=True) and
    b'clang' in subprocess.check_output('%s --version' % cxx, shell=True)):
  print('true')
else:
  print('false')
