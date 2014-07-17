#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

A wrapper around the standard Python unittest library, adding features we need
for various unittests within this directory.

TODO(epoger): Move this into the common repo for broader use?  Or at least in
a more common place within the Skia repo?
"""

import errno
import filecmp
import os
import shutil
import tempfile
import unittest

TRUNK_DIR = os.path.abspath(os.path.join(
    os.path.dirname(__file__), os.pardir, os.pardir))


class TestCase(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super(TestCase, self).__init__(*args, **kwargs)
    # Subclasses should override this default value if they want their output
    # to be automatically compared against expectations (see setUp and tearDown)
    self._testdata_dir = None

  def setUp(self):
    """Called before each test."""
    # Get the name of this test, in such a way that it will be consistent
    # regardless of the directory it is run from (throw away package names,
    # if any).
    self._test_name = '.'.join(self.id().split('.')[-3:])

    self._temp_dir = tempfile.mkdtemp()
    if self._testdata_dir:
      self.create_empty_dir(self.output_dir_actual)

  def tearDown(self):
    """Called after each test."""
    shutil.rmtree(self._temp_dir)
    if self._testdata_dir and os.path.exists(self.output_dir_expected):
      different_files = _find_different_files(self.output_dir_actual,
                                              self.output_dir_expected)
      # Don't add any cleanup code below this assert!
      # Then if tests fail, the artifacts will not be cleaned up.
      assert (not different_files), \
        ('found differing files:\n' +
         '\n'.join(['tkdiff %s %s &' % (
             os.path.join(self.output_dir_actual, basename),
             os.path.join(self.output_dir_expected, basename))
                    for basename in different_files]))

  @property
  def temp_dir(self):
    return self._temp_dir

  @property
  def input_dir(self):
    assert self._testdata_dir, 'self._testdata_dir must be set'
    return os.path.join(self._testdata_dir, 'inputs')

  @property
  def output_dir_actual(self):
    assert self._testdata_dir, 'self._testdata_dir must be set'
    return os.path.join(
        self._testdata_dir, 'outputs', 'actual', self._test_name)

  @property
  def output_dir_expected(self):
    assert self._testdata_dir, 'self._testdata_dir must be set'
    return os.path.join(
        self._testdata_dir, 'outputs', 'expected', self._test_name)

  def shortDescription(self):
    """Tell unittest framework to not print docstrings for test cases."""
    return None

  def create_empty_dir(self, path):
    """Creates an empty directory at path and returns path.

    Args:
      path: path on local disk
    """
    # Delete the old one, if any.
    if os.path.isdir(path):
      shutil.rmtree(path=path, ignore_errors=True)
    elif os.path.lexists(path):
      os.remove(path)

    # Create the new one.
    try:
      os.makedirs(path)
    except OSError as exc:
      # Guard against race condition (somebody else is creating the same dir)
      if exc.errno != errno.EEXIST:
        raise
    return path


def _find_different_files(dir1, dir2, ignore_subtree_names=None):
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
    differing_files.extend(_find_different_files(
        os.path.join(dir1, common_dir), os.path.join(dir2, common_dir)))
  return differing_files


def main(test_case_class):
  """Run the unit tests within the given class.

  Raises an Exception if any of those tests fail (in case we are running in the
  context of run_all.py, which depends on that Exception to signal failures).
  """
  suite = unittest.TestLoader().loadTestsFromTestCase(test_case_class)
  results = unittest.TextTestRunner(verbosity=2).run(suite)
  if not results.wasSuccessful():
    raise Exception('failed unittest %s' % test_case_class)
