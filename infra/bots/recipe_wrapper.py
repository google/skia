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
print 'PATH=%s' % os.environ['PATH']

recipes_base = 'recipes'
if os.name == 'nt':
  recipes_base = 'recipes.bat'
recipes = os.path.join(recipe_path, recipes_base)
args = [recipes] + sys.argv[1:]
subprocess.check_call(args)
