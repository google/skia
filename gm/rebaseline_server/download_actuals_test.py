#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test download.py

TODO(epoger): Create a command to update the expected results (in
OUTPUT_DIR_EXPECTED) when appropriate.  For now, you should:
1. examine the results in OUTPUT_DIR_ACTUAL and make sure they are ok
2. rm -rf OUTPUT_DIR_EXPECTED
3. mv OUTPUT_DIR_ACTUAL OUTPUT_DIR_EXPECTED
Although, if you're using an SVN checkout, this will blow away .svn directories
within OUTPUT_DIR_EXPECTED, which wouldn't be good...

"""

# System-level imports
import filecmp
import os
import shutil
import tempfile
import unittest
import urllib

# Imports from within Skia
import download_actuals

PARENT_DIR = os.path.dirname(os.path.realpath(__file__))
INPUT_DIR = os.path.join(PARENT_DIR, 'tests', 'inputs')
OUTPUT_DIR_ACTUAL   = os.path.join(PARENT_DIR, 'tests', 'outputs', 'actual')
OUTPUT_DIR_EXPECTED = os.path.join(PARENT_DIR, 'tests', 'outputs', 'expected')


class DownloadTest(unittest.TestCase):

  def setUp(self):
    self._temp_dir = tempfile.mkdtemp()
    self._output_dir_actual   = os.path.join(OUTPUT_DIR_ACTUAL, self.id())
    self._output_dir_expected = os.path.join(OUTPUT_DIR_EXPECTED, self.id())
    create_empty_dir(self._output_dir_actual)

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
        ('found differing files between actual dir %s and expected dir %s: %s' %
         (self._output_dir_actual, self._output_dir_expected, different_files))

  def shortDescription(self):
    """Tell unittest framework to not print docstrings for test cases."""
    return None

  def test_fetch(self):
    """Tests fetch() of GM results from actual-results.json ."""
    downloader = download_actuals.Download(
        actuals_base_url=download_actuals.create_filepath_url(
            os.path.join(INPUT_DIR, 'gm-actuals')),
        gm_actuals_root_url=download_actuals.create_filepath_url(
            os.path.join(INPUT_DIR, 'fake-gm-imagefiles')))
    downloader.fetch(
        builder_name='Test-Android-GalaxyNexus-SGX540-Arm7-Release',
        dest_dir=self._output_dir_actual)

  def test_create_filepath_url(self):
    """Tests create_filepath_url(). """
    with self.assertRaises(Exception):
      url_or_path.create_filepath_url('http://1.2.3.4/path')
    # Pass absolute filepath.
    self.assertEquals(
        download_actuals.create_filepath_url(
            '%sdir%sfile' % (os.path.sep, os.path.sep)),
        'file:///dir/file')
    # Pass relative filepath.
    self.assertEquals(
        download_actuals.create_filepath_url(os.path.join('dir', 'file')),
        'file://%s/dir/file' % urllib.pathname2url(os.getcwd()))

  def test_copy_contents(self):
    """Tests copy_contents(). """
    contents = 'these are the contents'
    tempdir_path = tempfile.mkdtemp()
    try:
      source_path = os.path.join(tempdir_path, 'source')
      source_url = download_actuals.create_filepath_url(source_path)
      with open(source_path, 'w') as source_handle:
        source_handle.write(contents)
      dest_path = os.path.join(tempdir_path, 'new_subdir', 'dest')
      # Destination subdir does not exist, so copy_contents() should fail
      # if create_subdirs_if_needed is False.
      with self.assertRaises(Exception):
        download_actuals.copy_contents(source_url=source_url,
                                       dest_path=dest_path,
                                       create_subdirs_if_needed=False)
      # If create_subdirs_if_needed is True, it should work.
      download_actuals.copy_contents(source_url=source_url,
                                     dest_path=dest_path,
                                     create_subdirs_if_needed=True)
      self.assertEquals(open(dest_path).read(), contents)
    finally:
      shutil.rmtree(tempdir_path)


# TODO(epoger): create_empty_dir(), find_different_files(), etc. should be
# extracted from this file to some common location, where they can be shared
# with results_test.py and other users.

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
  suite = unittest.TestLoader().loadTestsFromTestCase(DownloadTest)
  unittest.TextTestRunner(verbosity=2).run(suite)


if __name__ == '__main__':
  main()
