#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test download.py

TODO(epoger): Create a command to update the expected results (in
self._output_dir_expected) when appropriate.  For now, you should:
1. examine the results in self.output_dir_actual and make sure they are ok
2. rm -rf self._output_dir_expected
3. mv self.output_dir_actual self._output_dir_expected
Although, if you're using an SVN checkout, this will blow away .svn directories
within self._output_dir_expected, which wouldn't be good...

"""

# System-level imports
import os

# Must fix up PYTHONPATH before importing from within Skia
import rs_fixpypath  # pylint: disable=W0611

# Imports from within Skia
from py.utils import url_utils
import base_unittest
import download_actuals


class DownloadTest(base_unittest.TestCase):

  def test_fetch(self):
    """Tests fetch() of GM results from actual-results.json ."""
    downloader = download_actuals.Download(
        actuals_base_url=url_utils.create_filepath_url(
            os.path.join(self.input_dir, 'gm-actuals')),
        gm_actuals_root_url=url_utils.create_filepath_url(
            os.path.join(self.input_dir, 'fake-gm-imagefiles')))
    downloader.fetch(
        builder_name='Test-Android-GalaxyNexus-SGX540-Arm7-Release',
        dest_dir=self.output_dir_actual)


def main():
  base_unittest.main(DownloadTest)


if __name__ == '__main__':
  main()
