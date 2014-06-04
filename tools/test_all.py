#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Run all unittests within this directory tree, recursing into subdirectories.
"""

import os
import unittest

from tests import skimage_self_test


def main():
  # First, run any tests that cannot be automatically discovered (because
  # they don't use Python's unittest framework).
  skimage_self_test.main()

  # Now discover/run all tests that use Python's unittest framework.
  suite = unittest.TestLoader().discover(os.path.dirname(__file__),
                                         pattern='*_test.py')
  results = unittest.TextTestRunner(verbosity=2).run(suite)
  print repr(results)
  if not results.wasSuccessful():
    raise Exception('failed one or more unittests')

if __name__ == '__main__':
  main()
