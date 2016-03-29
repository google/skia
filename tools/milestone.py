#!/usr/bin/env python
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

milestone_file = 'include/core/SkMilestone.h'

usage = '''
usage:
  git fetch
  git checkout -b change_milestone origin/master
  python %s MILESTONE_NUMBER
  git add %s
  git commit -m "Update Skia milestone."
  git cl land

'''
try:
  milestone = int(sys.argv[1])
  assert milestone > 0
except (IndexError, ValueError, AssertionError):
  sys.stderr.write(usage % (sys.argv[0], milestone_file))
  exit(1)

text = '''/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SK_MILESTONE
#define SK_MILESTONE %d
#endif
'''

os.chdir(os.path.join(os.path.dirname(__file__), os.pardir))

with open(milestone_file, 'w') as o:
  o.write(text % milestone)

with open(milestone_file, 'r') as f:
  sys.stdout.write(f.read())
