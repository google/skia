#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test the render_pictures binary.
"""

# System-level imports
import json
import os
import shutil
import tempfile

# Imports from within Skia
import base_unittest

# Maximum length of text diffs to show when tests fail
MAX_DIFF_LENGTH = 30000


class RenderPicturesTest(base_unittest.TestCase):

  def setUp(self):
    self._temp_dir = tempfile.mkdtemp()
    self.maxDiff = MAX_DIFF_LENGTH

  def tearDown(self):
    shutil.rmtree(self._temp_dir)

  def test_tiled_whole_image_no_comparison(self):
    """Run render_pictures with tiles and --writeWholeImage flag."""
    input_skp_path = os.path.join(self._temp_dir, 'input.skp')
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._run_skpmaker(['--writePath', input_skp_path])
    self._run_render_pictures(['-r', input_skp_path,
                               '--bbh', 'grid', '256', '256',
                               '--mode', 'tile', '256', '256',
                               '--writeJsonSummaryPath', output_json_path,
                               '--writeWholeImage'])
    expected_summary_dict = {
        "actual-results" : {
            "no-comparison" : {
                # Manually verified: 640x400 red rectangle with black border
                "input.png" : [ "bitmap-64bitMD5", 11092453015575919668 ]
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def test_untiled_no_comparison(self):
    """Run without tiles."""
    input_skp_path = os.path.join(self._temp_dir, 'input.skp')
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._run_skpmaker(['--writePath', input_skp_path])
    self._run_render_pictures(['-r', input_skp_path,
                               '--writePath', self._temp_dir,
                               '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "actual-results" : {
            "no-comparison" : {
                # Manually verified: 640x400 red rectangle with black border
                "input.png" : ["bitmap-64bitMD5", 11092453015575919668],
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def test_validate(self):
    """Same as test_untiled_no_comparison, but with --validate.

    TODO(epoger): This test generates undesired results!  The call
    to render_pictures should succeed, and generate the same output as
    test_untiled_no_comparison.
    See https://code.google.com/p/skia/issues/detail?id=2044 ('render_pictures:
    --validate fails')
    """
    input_skp_path = os.path.join(self._temp_dir, 'input.skp')
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._run_skpmaker(['--writePath', input_skp_path])
    with self.assertRaises(Exception):
      self._run_render_pictures(['-r', input_skp_path,
                                 '--validate',
                                 '--writePath', self._temp_dir,
                                 '--writeJsonSummaryPath', output_json_path])

  def test_without_writePath(self):
    """Same as test_untiled_no_comparison, but without --writePath.

    TODO(epoger): This test generates undesired results!
    See https://code.google.com/p/skia/issues/detail?id=2043 ('render_pictures:
    --writeJsonSummaryPath fails unless --writePath is specified')
    """
    input_skp_path = os.path.join(self._temp_dir, 'input.skp')
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._run_skpmaker(['--writePath', input_skp_path])
    self._run_render_pictures(['-r', input_skp_path,
                               '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "actual-results" : {
            "no-comparison" : None,
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def test_tiled_no_comparison(self):
    """Generate individual tiles."""
    input_skp_path = os.path.join(self._temp_dir, 'input.skp')
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._run_skpmaker(['--writePath', input_skp_path])
    self._run_render_pictures(['-r', input_skp_path,
                               '--bbh', 'grid', '256', '256',
                               '--mode', 'tile', '256', '256',
                               '--writePath', self._temp_dir,
                               '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "actual-results" : {
            "no-comparison" : {
                # Manually verified these 6 images, all 256x256 tiles,
                # consistent with a tiled version of the 640x400 red rect
                # with black borders.
                "input0.png" : ["bitmap-64bitMD5", 5815827069051002745],
                "input1.png" : ["bitmap-64bitMD5", 9323613075234140270],
                "input2.png" : ["bitmap-64bitMD5", 16670399404877552232],
                "input3.png" : ["bitmap-64bitMD5", 2507897274083364964],
                "input4.png" : ["bitmap-64bitMD5", 7325267995523877959],
                "input5.png" : ["bitmap-64bitMD5", 2181381724594493116],
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def _run_render_pictures(self, args):
    binary = self.find_path_to_program('render_pictures')
    return self.run_command([binary,
                             '--clone', '1',
                             '--config', '8888',
                             ] + args)

  def _run_skpmaker(self, args):
    binary = self.find_path_to_program('skpmaker')
    return self.run_command([binary,
                             '--red', '255',
                             '--green', '0',
                             '--blue', '0',
                             '--width', '640',
                             '--height', '400',
                             ] + args)

  def _assert_json_contents(self, json_path, expected_dict):
    """Asserts that contents of a JSON file are identical to expected_dict.

    Args:
      json_path: Path to a JSON file.
      expected_dict: Dictionary indicating the expected contents of the JSON
                     file.

    Raises:
      AssertionError: contents of the JSON file are not identical to
                      expected_dict.
    """
    file_contents = open(json_path, 'r').read()
    actual_dict = json.loads(file_contents)
    self.assertEqual(actual_dict, expected_dict)


def main():
  base_unittest.main(RenderPicturesTest)


if __name__ == '__main__':
  main()
