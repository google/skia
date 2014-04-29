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

sys.path.append(test_variables.ANDROID_DIR)

import gyp_gen.android_framework_gyp

GYPD_SUFFIX = ".gypd"
GYP_SUFFIX = ".gyp"
GYPI_SUFFIX = ".gypi"
OTHER_SUFFIX = ".txt"

class CleanGypdTest(unittest.TestCase):

  def setUp(self):
    self.__tmp_dir = tempfile.mkdtemp()
    self.__num_files = 10
    # Fill the dir with four types of files. .gypd files should be deleted by
    # clean_gypd_files(), while the rest should be left alone.
    for i in range(self.__num_files):
      self.create_file('%s%s' % (str(i), GYPD_SUFFIX))
      self.create_file('%s%s' % (str(i), GYPI_SUFFIX))
      self.create_file('%s%s' % (str(i), GYP_SUFFIX))
      self.create_file('%s%s' % (str(i), OTHER_SUFFIX))

  def create_file(self, basename):
    """Create a file named 'basename' in self.__tmp_dir.
    """
    f = tempfile.mkstemp(dir=self.__tmp_dir)
    os.rename(f[1], os.path.join(self.__tmp_dir, basename))
    self.assert_file_exists(basename)

  def assert_file_exists(self, basename):
    """Assert that 'basename' exists in self.__tmp_dir.
    """
    full_name = os.path.join(self.__tmp_dir, basename)
    self.assertTrue(os.path.exists(full_name))

  def assert_file_does_not_exist(self, basename):
    """Assert that 'basename' does not exist in self.__tmp_dir.
    """
    full_name = os.path.join(self.__tmp_dir, basename)
    self.assertFalse(os.path.exists(full_name))

  def test_clean(self):
    """Test that clean_gypd_files() deletes .gypd files, and leaves others.
    """
    gyp_gen.android_framework_gyp.clean_gypd_files(self.__tmp_dir)
    for i in range(self.__num_files):
      self.assert_file_exists('%s%s' % (str(i), GYPI_SUFFIX))
      self.assert_file_exists('%s%s' % (str(i), GYP_SUFFIX))
      self.assert_file_exists('%s%s' % (str(i), OTHER_SUFFIX))
      # Only the GYPD files should have been deleted.
      self.assert_file_does_not_exist('%s%s' % (str(i), GYPD_SUFFIX))

  def tearDown(self):
    shutil.rmtree(self.__tmp_dir)


def main():
  loader = unittest.TestLoader()
  suite = loader.loadTestsFromTestCase(CleanGypdTest)
  unittest.TextTestRunner(verbosity=2).run(suite)

if __name__ == "__main__":
  main()
