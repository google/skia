#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test results.py

TODO(epoger): Launch this (and other unittests within this dir) automatically
on the housekeeper bot, but first make sure it works properly after having been
checked out (from both git and svn)

TODO(epoger): Create a command to update the expected results (in
OUTPUT_DIR_EXPECTED) when appropriate.  For now, you should:
1. examine the results in OUTPUT_DIR_ACTUAL and make sure they are ok
2. rm -rf OUTPUT_DIR_EXPECTED
3. mv OUTPUT_DIR_ACTUAL OUTPUT_DIR_EXPECTED
Although, if you're using an SVN checkout, this will blow away .svn directories
within OUTPUT_DIR_EXPECTED, which wouldn't be good...

"""

import filecmp
import os
import shutil
import sys
import tempfile
import unittest

# Imports from within Skia
import results
import gm_json  # must import results first, so that gm_json will be in sys.path

PARENT_DIR = os.path.dirname(os.path.realpath(__file__))
INPUT_DIR = os.path.join(PARENT_DIR, 'tests', 'inputs')
OUTPUT_DIR_ACTUAL   = os.path.join(PARENT_DIR, 'tests', 'outputs', 'actual')
OUTPUT_DIR_EXPECTED = os.path.join(PARENT_DIR, 'tests', 'outputs', 'expected')


class ResultsTest(unittest.TestCase):

  def setUp(self):
    self._temp_dir = tempfile.mkdtemp()
    self._output_dir_actual   = os.path.join(OUTPUT_DIR_ACTUAL, self.id())
    self._output_dir_expected = os.path.join(OUTPUT_DIR_EXPECTED, self.id())
    create_empty_dir(self._output_dir_actual)

  def tearDown(self):
    shutil.rmtree(self._temp_dir)
    different_files = find_different_files(self._output_dir_actual,
                                           self._output_dir_expected)
    # Maybe we should move this assert elsewhere?  It's unusual to see an
    # assert within tearDown(), but my thinking was:
    # 1. Every test case will have some collection of output files that need to
    #    be validated.
    # 2. So put that validation within tearDown(), which will be called after
    #    every test case!
    #
    # I have confirmed that the test really does fail if this assert is
    # triggered.
    #
    # Ravi notes: if somebody later comes along and adds cleanup code below the
    # assert, then if tests fail, the artifacts will not be cleaned up.
    assert (not different_files), \
      ('found differing files between actual dir %s and expected dir %s: %s' %
       (self._output_dir_actual, self._output_dir_expected, different_files))

  def test_gm(self):
    """Process results of a GM run with the Results object."""
    results_obj = results.Results(
        actuals_root=os.path.join(INPUT_DIR, 'gm-actuals'),
        expected_root=os.path.join(INPUT_DIR, 'gm-expectations'),
        generated_images_root=self._temp_dir)
    gm_json.WriteToFile(results_obj.get_results_of_type(results.RESULTS_ALL),
                        os.path.join(self._output_dir_actual, 'gm.json'))


def create_empty_dir(path):
  """Create an empty directory at the given path."""
  if os.path.isdir(path):
    shutil.rmtree(path)
  elif os.path.lexists(path):
    os.remove(path)
  os.makedirs(path)


def find_different_files(dir1, dir2, ignore_subtree_names=None):
  """Returns a list of any files that differ between the directory trees rooted
  at dir1 and dir2.

  Args:
    dir1: root of a directory tree; if nonexistent, will raise OSError
    dir2: root of another directory tree; if nonexistent, will raise OSError
    ignore_subtree_names: list of subtree directory names to ignore;
          defaults to ['.svn'], so all SVN files are ignores

  TODO(epoger): include the dirname within each filename (not just the
  basename), to make it easier to locate any differences
  """
  differing_files = []
  if ignore_subtree_names is None:
    ignore_subtree_names = ['.svn']
  dircmp = filecmp.dircmp(dir1, dir2, ignore=ignore_subtree_names)
  differing_files.extend(dircmp.left_only)
  differing_files.extend(dircmp.right_only)
  differing_files.extend(dircmp.common_funny)
  differing_files.extend(dircmp.diff_files)
  differing_files.extend(dircmp.funny_files)
  for common_dir in dircmp.common_dirs:
    differing_files.extend(find_different_files(
        os.path.join(dir1, common_dir), os.path.join(dir2, common_dir)))
  return differing_files


def main():
  suite = unittest.TestLoader().loadTestsFromTestCase(ResultsTest)
  unittest.TextTestRunner(verbosity=2).run(suite)


if __name__ == '__main__':
  main()
