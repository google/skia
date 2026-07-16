#!/usr/bin/env python
#
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
import subprocess
import sys

cc, cxx = sys.argv[1:3]

try:
  if (b'Apple' in subprocess.check_output([cc, '--version']) and
      b'Apple' in subprocess.check_output([cxx, '--version'])):
    print('true')
  else:
    print('false')
except Exception:
  # If we can't run the compiler, assume false.
  print('false')
