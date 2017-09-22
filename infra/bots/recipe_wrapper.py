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
recipe_path = os.path.abspath(os.path.join(
    os.getcwd(), os.pardir, os.pardir, os.pardir, 'recipe_bundle'))
path = path + os.pathsep + recipe_path
os.environ['PATH'] = path

recipes_base = 'recipes'
which = 'which'
if os.name == 'nt':
  recipes_base = 'recipes.bat'
  which = 'where'

print 'PATH=%s' % os.environ['PATH']
print subprocess.check_output([which, recipes_base])

args = [recipes_base] + sys.argv[1:]
subprocess.check_call(args)
