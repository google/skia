#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys
cc,cxx = sys.argv[1:3]

if ('clang' in subprocess.check_output('%s --version' % cc, shell=True) and
    'clang' in subprocess.check_output('%s --version' % cxx, shell=True)):
  print 'true'
else:
  print 'false'

