#!/usr/bin/env python
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import subprocess
import sys


GIT = 'git.bat' if sys.platform == 'win32' else 'git'


def git(*args):
  '''Run the given Git command, return the output.'''
  return subprocess.check_output([GIT]+list(args))
