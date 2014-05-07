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
import subprocess
import sys

# Imports from within Skia
import base_unittest
import compare_rendered_pictures
import results
import gm_json  # must import results first, so that gm_json will be in sys.path


class CompareRenderedPicturesTest(base_unittest.TestCase):

  def test_endToEnd(self):
    """Generate two sets of SKPs, run render_pictures over both, and compare
    the results."""
    self._generate_skps_and_run_render_pictures(
        subdir='before_patch', skpdict={
            'changed.skp': 200,
            'unchanged.skp': 100,
            'only-in-before.skp': 128,
        })
    self._generate_skps_and_run_render_pictures(
        subdir='after_patch', skpdict={
            'changed.skp': 201,
            'unchanged.skp': 100,
            'only-in-after.skp': 128,
        })

    results_obj = compare_rendered_pictures.RenderedPicturesComparisons(
        actuals_root=self._temp_dir,
        subdirs=('before_patch', 'after_patch'),
        generated_images_root=self._temp_dir,
        diff_base_url='/static/generated-images')
    results_obj.get_timestamp = mock_get_timestamp

    gm_json.WriteToFile(
        results_obj.get_packaged_results_of_type(
            results.KEY__HEADER__RESULTS_ALL),
        os.path.join(self._output_dir_actual, 'compare_rendered_pictures.json'))

  def _generate_skps_and_run_render_pictures(self, subdir, skpdict):
    """Generate SKPs and run render_pictures on them.

    Args:
      subdir: subdirectory (within self._temp_dir) to write all files into
      skpdict: {skpname: redvalue} dictionary describing the SKP files to render
    """
    out_path = os.path.join(self._temp_dir, subdir)
    os.makedirs(out_path)
    for skpname, redvalue in skpdict.iteritems():
      self._run_skpmaker(
          output_path=os.path.join(out_path, skpname), red=redvalue)

    # TODO(epoger): Add --mode tile 256 256 --writeWholeImage to the unittest,
    # and fix its result!  (imageURLs within whole-image entries are wrong when
    # I tried adding that)
    binary = self.find_path_to_program('render_pictures')
    return subprocess.check_output([
        binary,
        '--clone', '1',
        '--config', '8888',
        '-r', out_path,
        '--writeChecksumBasedFilenames',
        '--writeJsonSummaryPath', os.path.join(out_path, 'summary.json'),
        '--writePath', out_path])

  def _run_skpmaker(self, output_path, red=0, green=0, blue=0,
                    width=640, height=400):
    """Runs the skpmaker binary to generate SKP with known characteristics.

    Args:
      output_path: Filepath to write the SKP into.
      red: Value of red color channel in image, 0-255.
      green: Value of green color channel in image, 0-255.
      blue: Value of blue color channel in image, 0-255.
      width: Width of canvas to create.
      height: Height of canvas to create.
    """
    binary = self.find_path_to_program('skpmaker')
    return subprocess.check_output([
        binary,
        '--red', str(red),
        '--green', str(green),
        '--blue', str(blue),
        '--width', str(width),
        '--height', str(height),
        '--writePath', str(output_path)])

def mock_get_timestamp():
  """Mock version of BaseComparisons.get_timestamp() for testing."""
  return 12345678


def main():
  base_unittest.main(CompareRenderedPicturesTest)


if __name__ == '__main__':
  main()
