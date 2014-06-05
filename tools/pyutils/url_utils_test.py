#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test url_utils.py
"""

# System-level imports
import os
import shutil
import tempfile
import unittest
import urllib

# Imports from within Skia
import url_utils


class UrlUtilsTest(unittest.TestCase):

  def test_create_filepath_url(self):
    """Tests create_filepath_url(). """
    with self.assertRaises(Exception):
      url_utils.create_filepath_url('http://1.2.3.4/path')
    # Pass absolute filepath.
    self.assertEquals(
        url_utils.create_filepath_url(
            '%sdir%sfile' % (os.path.sep, os.path.sep)),
        'file:///dir/file')
    # Pass relative filepath.
    self.assertEquals(
        url_utils.create_filepath_url(os.path.join('dir', 'file')),
        'file://%s/dir/file' % urllib.pathname2url(os.getcwd()))

  def test_copy_contents(self):
    """Tests copy_contents(). """
    contents = 'these are the contents'
    tempdir_path = tempfile.mkdtemp()
    try:
      source_path = os.path.join(tempdir_path, 'source')
      source_url = url_utils.create_filepath_url(source_path)
      with open(source_path, 'w') as source_handle:
        source_handle.write(contents)
      dest_path = os.path.join(tempdir_path, 'new_subdir', 'dest')
      # Destination subdir does not exist, so copy_contents() should fail
      # if create_subdirs_if_needed is False.
      with self.assertRaises(Exception):
        url_utils.copy_contents(source_url=source_url,
                                dest_path=dest_path,
                                create_subdirs_if_needed=False)
      # If create_subdirs_if_needed is True, it should work.
      url_utils.copy_contents(source_url=source_url,
                              dest_path=dest_path,
                              create_subdirs_if_needed=True)
      self.assertEquals(open(dest_path).read(), contents)
    finally:
      shutil.rmtree(tempdir_path)
