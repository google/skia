#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Adds [trunk]/gm and [trunk]/tools to PYTHONPATH, if they aren't already there.
"""

import os
import sys

TRUNK_DIRECTORY = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
GM_DIRECTORY = os.path.join(TRUNK_DIRECTORY, 'gm')
TOOLS_DIRECTORY = os.path.join(TRUNK_DIRECTORY, 'tools')
if GM_DIRECTORY not in sys.path:
  sys.path.append(GM_DIRECTORY)
if TOOLS_DIRECTORY not in sys.path:
  sys.path.append(TOOLS_DIRECTORY)
