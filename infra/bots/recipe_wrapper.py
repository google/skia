#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Wrapper around recipes.py which fixes the environment."""


# TODO(borenet): Remove this file when it is no longer needed.


import os
import subprocess
import sys


path = os.environ.get('PATH')
path = path + os.pathsep + os.getcwd()
os.environ['PATH'] = path
args = [sys.executable, 'recipes.py'] + sys.argv[1:]
subprocess.check_output(args)
