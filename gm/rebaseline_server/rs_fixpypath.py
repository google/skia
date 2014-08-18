#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Adds possibly-needed directories to PYTHONPATH, if they aren't already there.
"""

import os
import sys

TRUNK_DIRECTORY = os.path.abspath(os.path.join(
    os.path.dirname(__file__), os.pardir, os.pardir))
for subdir in ['common', 'gm', 'tools']:
  fullpath = os.path.join(TRUNK_DIRECTORY, subdir)
  if fullpath not in sys.path:
    sys.path.append(fullpath)
