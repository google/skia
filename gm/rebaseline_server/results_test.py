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
OUTPUT_DIR_EXPECTATIONS) when appropriate.  For now, you should:
1. examine the results in OUTPUT_DIR and make sure they are ok
2. rm -rf OUTPUT_DIR_EXPECTATIONS
3. mv OUTPUT_DIR OUTPUT_DIR_EXPECTATIONS
Although, if you're using an SVN checkout, this will blow away .svn directories
within OUTPUT_DIR_EXPECTATIONS, which wouldn't be good...

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
OUTPUT_DIR = os.path.join(PARENT_DIR, 'tests', 'outputs', 'actual')
OUTPUT_DIR_EXPECTATIONS = os.path.join(
    PARENT_DIR, 'tests', 'outputs', 'expected')

class ResultsTest(unittest.TestCase):

  def setUp(self):
    self._tempdir = tempfile.mkdtemp()

  def tearDown(self):
    shutil.rmtree(self._tempdir)

  def test_gm(self):
    """Process results of a GM run with the Results object."""
    results_obj = results.Results(
        actuals_root=os.path.join(INPUT_DIR, 'gm-actuals'),
        expected_root=os.path.join(INPUT_DIR, 'gm-expectations'),
        generated_images_root=self._tempdir)
    gm_json.WriteToFile(results_obj.get_results_of_type(results.RESULTS_ALL),
                        os.path.join(OUTPUT_DIR, 'gm.json'))


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
  if not os.path.isdir(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)
  suite = unittest.TestLoader().loadTestsFromTestCase(ResultsTest)
  unittest.TextTestRunner(verbosity=2).run(suite)
  different_files = find_different_files(OUTPUT_DIR, OUTPUT_DIR_EXPECTATIONS)
  assert (not different_files), 'found differing files: %s' % different_files


if __name__ == '__main__':
  main()
