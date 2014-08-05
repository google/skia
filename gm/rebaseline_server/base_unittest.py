#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

A wrapper around the standard Python unittest library, adding features we need
for various unittests within this directory.
"""

# System-level imports.
import os
import sys

PARENT_DIR = os.path.abspath(os.path.dirname(__file__))
TRUNK_DIR = os.path.abspath(os.path.join(PARENT_DIR, os.pardir, os.pardir))

# Import the superclass base_unittest module from the tools dir.
#
# TODO(epoger): If I don't put this at the beginning of sys.path, the import of
# tests.base_unittest fails.  That's bad.  I need to come up with a cleaner way
# of doing this... I think this will involve changing how we import the "boto"
# library in gs_utils.py, within the common repo.
TOOLS_DIR = os.path.join(TRUNK_DIR, 'tools')
if TOOLS_DIR != sys.path[0]:
  sys.path.insert(0, TOOLS_DIR)
import tests.base_unittest as superclass_module


class TestCase(superclass_module.TestCase):

  def __init__(self, *args, **kwargs):
    super(TestCase, self).__init__(*args, **kwargs)
    # Some of the tests within this package want their output validated,
    # so we declare where the expected and actual output will be.
    self._testdata_dir = os.path.join(PARENT_DIR, 'testdata')

def main(*args, **kwargs):
  superclass_module.main(*args, **kwargs)
