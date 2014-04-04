#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test compare_rendered_pictures.py

TODO(epoger): Create a command to update the expected results (in
self._output_dir_expected) when appropriate.  For now, you should:
1. examine the results in self._output_dir_actual and make sure they are ok
2. rm -rf self._output_dir_expected
3. mv self._output_dir_actual self._output_dir_expected
Although, if you're using an SVN checkout, this will blow away .svn directories
within self._output_dir_expected, which wouldn't be good...

"""

import os
import sys

# Imports from within Skia
import base_unittest
import compare_rendered_pictures
import results
import gm_json  # must import results first, so that gm_json will be in sys.path


class CompareRenderedPicturesTest(base_unittest.TestCase):

  def test_endToEnd(self):
    """Compare results of two render_pictures runs."""
    # TODO(epoger): Specify image_base_url pointing at the directory on local
    # disk containing our test images, so that we can actually compute pixel
    # diffs.  For now, this test attempts to download images from
    # DEFAULT_IMAGE_BASE_URL, and there aren't any there yet.
    results_obj = compare_rendered_pictures.RenderedPicturesComparisons(
        actuals_root=os.path.join(self._input_dir, 'render_pictures_output'),
        subdirs=('before_patch', 'after_patch'),
        generated_images_root=self._temp_dir,
        diff_base_url='/static/generated-images')
    results_obj.get_timestamp = mock_get_timestamp
    gm_json.WriteToFile(
        results_obj.get_packaged_results_of_type(
            results.KEY__HEADER__RESULTS_ALL),
        os.path.join(self._output_dir_actual, 'compare_rendered_pictures.json'))


def mock_get_timestamp():
  """Mock version of BaseComparisons.get_timestamp() for testing."""
  return 12345678


def main():
  base_unittest.main(CompareRenderedPicturesTest)


if __name__ == '__main__':
  main()
