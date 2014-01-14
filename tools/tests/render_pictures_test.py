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


class RenderPicturesTest(base_unittest.TestCase):

  def setUp(self):
    self._temp_dir = tempfile.mkdtemp()

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
                "input.png" : [ "bitmap-64bitMD5", 12793741875005523433 ]
                }
            }
        }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def test_tiled_no_comparison(self):
    """Generate individual tiles.

    TODO(epoger): The results of this test are currently broken!
    The summary should contain a list of tiles, but for some reason, it is
    empty."""
    input_skp_path = os.path.join(self._temp_dir, 'input.skp')
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._run_skpmaker(['--writePath', input_skp_path])
    self._run_render_pictures(['-r', input_skp_path,
                               '--bbh', 'grid', '256', '256',
                               '--mode', 'tile', '256', '256',
                               '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "actual-results" : {
            "no-comparison" : None
            }
        }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def test_untiled_no_comparison(self):
    """Run without tiles.

    TODO(epoger): The results of this test are currently broken!
    The summary should contain a single image, but for some reason, it is
    empty."""
    input_skp_path = os.path.join(self._temp_dir, 'input.skp')
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._run_skpmaker(['--writePath', input_skp_path])
    self._run_render_pictures(['-r', input_skp_path,
                               '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "actual-results" : {
            "no-comparison" : None
            }
        }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def _run_render_pictures(self, args):
    binary = self.find_path_to_program('render_pictures')
    return self.run_command([binary,
                             '--clone', '1',
                             '--config', '8888',
                             '--validate'
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
