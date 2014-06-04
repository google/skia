#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test download.py

TODO(epoger): Create a command to update the expected results (in
self._output_dir_expected) when appropriate.  For now, you should:
1. examine the results in self._output_dir_actual and make sure they are ok
2. rm -rf self._output_dir_expected
3. mv self._output_dir_actual self._output_dir_expected
Although, if you're using an SVN checkout, this will blow away .svn directories
within self._output_dir_expected, which wouldn't be good...

"""

# System-level imports
import os
import shutil
import tempfile
import urllib

# Imports from within Skia
import base_unittest
import download_actuals


class DownloadTest(base_unittest.TestCase):

  def test_fetch(self):
    """Tests fetch() of GM results from actual-results.json ."""
    downloader = download_actuals.Download(
        actuals_base_url=download_actuals.create_filepath_url(
            os.path.join(self._input_dir, 'gm-actuals')),
        gm_actuals_root_url=download_actuals.create_filepath_url(
            os.path.join(self._input_dir, 'fake-gm-imagefiles')))
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


def main():
  base_unittest.main(DownloadTest)


if __name__ == '__main__':
  main()
