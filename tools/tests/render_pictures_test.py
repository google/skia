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
    self._input_skp_dir = tempfile.mkdtemp()
    self._temp_dir = tempfile.mkdtemp()
    self.maxDiff = MAX_DIFF_LENGTH

  def tearDown(self):
    shutil.rmtree(self._input_skp_dir)
    shutil.rmtree(self._temp_dir)

  def test_tiled_whole_image(self):
    """Run render_pictures with tiles and --writeWholeImage flag."""
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._generate_skps()
    self._run_render_pictures(['-r', self._input_skp_dir,
                               '--bbh', 'grid', '256', '256',
                               '--mode', 'tile', '256', '256',
                               '--writeJsonSummaryPath', output_json_path,
                               '--writeWholeImage'])
    expected_summary_dict = {
        "actual-results" : {
            "no-comparison" : {
                # Manually verified: 640x400 red rectangle with black border
                "red.png" : [ "bitmap-64bitMD5", 11092453015575919668 ],
                # Manually verified: 640x400 green rectangle with black border
                "green.png" : [ "bitmap-64bitMD5", 8891695120562235492 ],
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def test_untiled(self):
    """Run without tiles."""
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._generate_skps()
    self._run_render_pictures(['-r', self._input_skp_dir,
                               '--writePath', self._temp_dir,
                               '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "actual-results" : {
            "no-comparison" : {
                # Manually verified: 640x400 red rectangle with black border
                "red.png" : ["bitmap-64bitMD5", 11092453015575919668],
                # Manually verified: 640x400 green rectangle with black border
                "green.png" : ["bitmap-64bitMD5", 8891695120562235492],
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def test_untiled_validate(self):
    """Same as test_untiled, but with --validate.

    TODO(epoger): This test generates undesired results!  The call
    to render_pictures should succeed, and generate the same output as
    test_untiled.
    See https://code.google.com/p/skia/issues/detail?id=2044 ('render_pictures:
    --validate fails')
    """
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._generate_skps()
    with self.assertRaises(Exception):
      self._run_render_pictures(['-r', self._input_skp_dir,
                                 '--validate',
                                 '--writePath', self._temp_dir,
                                 '--writeJsonSummaryPath', output_json_path])

  def test_untiled_without_writePath(self):
    """Same as test_untiled, but without --writePath.

    TODO(epoger): This test generates undesired results!
    See https://code.google.com/p/skia/issues/detail?id=2043 ('render_pictures:
    --writeJsonSummaryPath fails unless --writePath is specified')
    """
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._generate_skps()
    self._run_render_pictures(['-r', self._input_skp_dir,
                               '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "actual-results" : {
            "no-comparison" : None,
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def test_tiled(self):
    """Generate individual tiles."""
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._generate_skps()
    self._run_render_pictures(['-r', self._input_skp_dir,
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
                "red0.png" : ["bitmap-64bitMD5", 5815827069051002745],
                "red1.png" : ["bitmap-64bitMD5", 9323613075234140270],
                "red2.png" : ["bitmap-64bitMD5", 16670399404877552232],
                "red3.png" : ["bitmap-64bitMD5", 2507897274083364964],
                "red4.png" : ["bitmap-64bitMD5", 7325267995523877959],
                "red5.png" : ["bitmap-64bitMD5", 2181381724594493116],
                # Manually verified these 6 images, all 256x256 tiles,
                # consistent with a tiled version of the 640x400 green rect
                # with black borders.
                "green0.png" : ["bitmap-64bitMD5", 12587324416545178013],
                "green1.png" : ["bitmap-64bitMD5", 7624374914829746293],
                "green2.png" : ["bitmap-64bitMD5", 5686489729535631913],
                "green3.png" : ["bitmap-64bitMD5", 7980646035555096146],
                "green4.png" : ["bitmap-64bitMD5", 17817086664365875131],
                "green5.png" : ["bitmap-64bitMD5", 10673669813016809363],
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

  def _generate_skps(self):
    """Runs the skpmaker binary to generate files in self._input_skp_dir."""
    self._run_skpmaker(
        output_path=os.path.join(self._input_skp_dir, 'red.skp'), red=255)
    self._run_skpmaker(
        output_path=os.path.join(self._input_skp_dir, 'green.skp'), green=255)

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
    return self.run_command([binary,
                             '--red', str(red),
                             '--green', str(green),
                             '--blue', str(blue),
                             '--width', str(width),
                             '--height', str(height),
                             '--writePath', str(output_path),
                             ])

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
