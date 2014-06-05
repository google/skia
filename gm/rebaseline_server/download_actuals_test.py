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
import fix_pythonpath  # must do this first
from pyutils import url_utils
import base_unittest
import download_actuals


class DownloadTest(base_unittest.TestCase):

  def test_fetch(self):
    """Tests fetch() of GM results from actual-results.json ."""
    downloader = download_actuals.Download(
        actuals_base_url=url_utils.create_filepath_url(
            os.path.join(self._input_dir, 'gm-actuals')),
        gm_actuals_root_url=url_utils.create_filepath_url(
            os.path.join(self._input_dir, 'fake-gm-imagefiles')))
    downloader.fetch(
        builder_name='Test-Android-GalaxyNexus-SGX540-Arm7-Release',
        dest_dir=self._output_dir_actual)


def main():
  base_unittest.main(DownloadTest)


if __name__ == '__main__':
  main()
