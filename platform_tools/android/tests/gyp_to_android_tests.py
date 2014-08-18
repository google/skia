#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Test gyp_to_android.py
"""

import os
import shutil
import sys
import tempfile
import test_variables
import unittest

# Path to gyp_to_android
sys.path.append(test_variables.BIN_DIR)

import gyp_to_android



class AndroidMkCreationTest(unittest.TestCase):

  def setUp(self):
    # Create a temporary directory for storing the output (Android.mk)
    self.__tmp_dir = tempfile.mkdtemp()

  def test_create(self):
    gyp_to_android.main(self.__tmp_dir)

    # Now there should be a file named 'Android.mk' inside __tmp_dir
    path_to_android_mk = os.path.join(self.__tmp_dir,
                                      test_variables.ANDROID_MK)
    self.assertTrue(os.path.exists(path_to_android_mk))

  def tearDown(self):
    # Remove self.__tmp_dir, which is no longer needed.
    shutil.rmtree(self.__tmp_dir)


def main():
  loader = unittest.TestLoader()
  suite = loader.loadTestsFromTestCase(AndroidMkCreationTest)
  unittest.TextTestRunner(verbosity=2).run(suite)

if __name__ == "__main__":
  main()
