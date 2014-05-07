#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

A wrapper around the standard Python unittest library, adding features we need
for various unittests within this directory.
"""

import filecmp
import os
import shutil
import tempfile
import unittest

PARENT_DIR = os.path.dirname(os.path.realpath(__file__))
TRUNK_DIR = os.path.dirname(os.path.dirname(PARENT_DIR))
TESTDATA_DIR = os.path.join(PARENT_DIR, 'testdata')
OUTPUT_DIR_ACTUAL = os.path.join(TESTDATA_DIR, 'outputs', 'actual')
OUTPUT_DIR_EXPECTED = os.path.join(TESTDATA_DIR, 'outputs', 'expected')


class TestCase(unittest.TestCase):

  def setUp(self):
    self._input_dir = os.path.join(TESTDATA_DIR, 'inputs')
    self._output_dir_actual   = os.path.join(OUTPUT_DIR_ACTUAL, self.id())
    self._output_dir_expected = os.path.join(OUTPUT_DIR_EXPECTED, self.id())
    create_empty_dir(self._output_dir_actual)
    self._temp_dir = tempfile.mkdtemp()

  def tearDown(self):
    shutil.rmtree(self._temp_dir)
    if os.path.exists(self._output_dir_expected):
      different_files = find_different_files(self._output_dir_actual,
                                             self._output_dir_expected)
      # Maybe we should move this assert elsewhere?  It's unusual to see an
      # assert within tearDown(), but my thinking was:
      # 1. Every test case will have some collection of output files that need
      #    to be validated.
      # 2. So put that validation within tearDown(), which will be called after
      #    every test case!
      #
      # I have confirmed that the test really does fail if this assert is
      # triggered.
      #
      # Ravi notes: if somebody later comes along and adds cleanup code below
      # this assert, then if tests fail, the artifacts will not be cleaned up.
      assert (not different_files), \
        ('found differing files:\n' +
         '\n'.join(['tkdiff %s %s &' % (
             os.path.join(self._output_dir_actual, basename),
             os.path.join(self._output_dir_expected, basename))
                    for basename in different_files]))

  def shortDescription(self):
    """Tell unittest framework to not print docstrings for test cases."""
    return None

  def find_path_to_program(self, program):
    """Returns path to an existing program binary.

    Args:
      program: Basename of the program to find (e.g., 'render_pictures').

    Returns:
      Absolute path to the program binary, as a string.

    Raises:
      Exception: unable to find the program binary.
    """
    possible_paths = [os.path.join(TRUNK_DIR, 'out', 'Release', program),
                      os.path.join(TRUNK_DIR, 'out', 'Debug', program),
                      os.path.join(TRUNK_DIR, 'out', 'Release',
                                   program + '.exe'),
                      os.path.join(TRUNK_DIR, 'out', 'Debug',
                                   program + '.exe')]
    for try_path in possible_paths:
      if os.path.isfile(try_path):
        return try_path
    raise Exception('cannot find %s in paths %s; maybe you need to '
                    'build %s?' % (program, possible_paths, program))


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


def main(test_case_class):
  """Run the unit tests within the given class."""
  suite = unittest.TestLoader().loadTestsFromTestCase(test_case_class)
  results = unittest.TextTestRunner(verbosity=2).run(suite)
