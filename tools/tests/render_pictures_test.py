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

EXPECTED_HEADER_CONTENTS = {
    "type" : "ChecksummedImages",
    "revision" : 1,
}


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
    # TODO(epoger): I noticed that when this is run without --writePath being
    # specified, this test writes red_skp.png and green_skp.png to the current
    # directory.  We should fix that... if --writePath is not specified, this
    # probably shouldn't write out red_skp.png and green_skp.png at all!
    self._run_render_pictures(['-r', self._input_skp_dir,
                               '--bbh', 'grid', '256', '256',
                               '--mode', 'tile', '256', '256',
                               '--writeJsonSummaryPath', output_json_path,
                               '--writePath', self._temp_dir,
                               '--writeWholeImage'])
    expected_summary_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                # Manually verified: 640x400 red rectangle with black border
                "whole-image": {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 11092453015575919668,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp.png",
                },
            },
            "green.skp": {
                # Manually verified: 640x400 green rectangle with black border
                "whole-image": {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 8891695120562235492,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp.png",
                },
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)
    self._assert_directory_contents(
        self._temp_dir, ['red_skp.png', 'green_skp.png', 'output.json'])

  def test_untiled(self):
    """Run without tiles."""
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._generate_skps()
    self._run_render_pictures(['-r', self._input_skp_dir,
                               '--writePath', self._temp_dir,
                               '--writeJsonSummaryPath', output_json_path])
    # TODO(epoger): These expectations are the same as for above unittest.
    # Define the expectations once, and share them.
    expected_summary_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                # Manually verified: 640x400 red rectangle with black border
                "whole-image": {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 11092453015575919668,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp.png",
                },
            },
            "green.skp": {
                # Manually verified: 640x400 green rectangle with black border
                "whole-image": {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 8891695120562235492,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp.png",
                },
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)
    self._assert_directory_contents(
        self._temp_dir, ['red_skp.png', 'green_skp.png', 'output.json'])

  def test_untiled_writeChecksumBasedFilenames(self):
    """Same as test_untiled, but with --writeChecksumBasedFilenames."""
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._generate_skps()
    self._run_render_pictures(['-r', self._input_skp_dir,
                               '--writeChecksumBasedFilenames',
                               '--writePath', self._temp_dir,
                               '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                # Manually verified: 640x400 red rectangle with black border
                "whole-image": {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 11092453015575919668,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp/bitmap-64bitMD5_11092453015575919668.png",
                },
            },
            "green.skp": {
                # Manually verified: 640x400 green rectangle with black border
                "whole-image": {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 8891695120562235492,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp/bitmap-64bitMD5_8891695120562235492.png",
                },
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)
    self._assert_directory_contents(self._temp_dir, [
        'red_skp', 'green_skp', 'output.json'])
    self._assert_directory_contents(
        os.path.join(self._temp_dir, 'red_skp'),
        ['bitmap-64bitMD5_11092453015575919668.png'])
    self._assert_directory_contents(
        os.path.join(self._temp_dir, 'green_skp'),
        ['bitmap-64bitMD5_8891695120562235492.png'])

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
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : None,
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
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                # Manually verified these 6 images, all 256x256 tiles,
                # consistent with a tiled version of the 640x400 red rect
                # with black borders.
                "tiled-images": [{
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 5815827069051002745,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp-tile0.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 9323613075234140270,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp-tile1.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 16670399404877552232,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp-tile2.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 2507897274083364964,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp-tile3.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 7325267995523877959,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp-tile4.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 2181381724594493116,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp-tile5.png",
                }],
            },
            "green.skp": {
                # Manually verified these 6 images, all 256x256 tiles,
                # consistent with a tiled version of the 640x400 green rect
                # with black borders.
                "tiled-images": [{
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 12587324416545178013,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp-tile0.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 7624374914829746293,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp-tile1.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 5686489729535631913,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp-tile2.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 7980646035555096146,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp-tile3.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 17817086664365875131,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp-tile4.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 10673669813016809363,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp-tile5.png",
                }],
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)
    self._assert_directory_contents(
        self._temp_dir,
        ['red_skp-tile0.png', 'red_skp-tile1.png', 'red_skp-tile2.png',
         'red_skp-tile3.png', 'red_skp-tile4.png', 'red_skp-tile5.png',
         'green_skp-tile0.png', 'green_skp-tile1.png', 'green_skp-tile2.png',
         'green_skp-tile3.png', 'green_skp-tile4.png', 'green_skp-tile5.png',
         'output.json'])

  def test_tiled_writeChecksumBasedFilenames(self):
    """Same as test_tiled, but with --writeChecksumBasedFilenames."""
    output_json_path = os.path.join(self._temp_dir, 'output.json')
    self._generate_skps()
    self._run_render_pictures(['-r', self._input_skp_dir,
                               '--bbh', 'grid', '256', '256',
                               '--mode', 'tile', '256', '256',
                               '--writeChecksumBasedFilenames',
                               '--writePath', self._temp_dir,
                               '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                # Manually verified these 6 images, all 256x256 tiles,
                # consistent with a tiled version of the 640x400 red rect
                # with black borders.
                "tiled-images": [{
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 5815827069051002745,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp/bitmap-64bitMD5_5815827069051002745.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 9323613075234140270,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp/bitmap-64bitMD5_9323613075234140270.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 16670399404877552232,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp/bitmap-64bitMD5_16670399404877552232.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 2507897274083364964,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp/bitmap-64bitMD5_2507897274083364964.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 7325267995523877959,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp/bitmap-64bitMD5_7325267995523877959.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 2181381724594493116,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "red_skp/bitmap-64bitMD5_2181381724594493116.png",
                }],
            },
            "green.skp": {
                # Manually verified these 6 images, all 256x256 tiles,
                # consistent with a tiled version of the 640x400 green rect
                # with black borders.
                "tiled-images": [{
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 12587324416545178013,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp/bitmap-64bitMD5_12587324416545178013.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 7624374914829746293,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp/bitmap-64bitMD5_7624374914829746293.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 5686489729535631913,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp/bitmap-64bitMD5_5686489729535631913.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 7980646035555096146,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp/bitmap-64bitMD5_7980646035555096146.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 17817086664365875131,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp/bitmap-64bitMD5_17817086664365875131.png",
                }, {
                    "checksumAlgorithm" : "bitmap-64bitMD5",
                    "checksumValue" : 10673669813016809363,
                    "comparisonResult" : "no-comparison",
                    "filepath" : "green_skp/bitmap-64bitMD5_10673669813016809363.png",
                }],
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)
    self._assert_directory_contents(self._temp_dir, [
        'red_skp', 'green_skp', 'output.json'])
    self._assert_directory_contents(
        os.path.join(self._temp_dir, 'red_skp'),
        ['bitmap-64bitMD5_5815827069051002745.png',
         'bitmap-64bitMD5_9323613075234140270.png',
         'bitmap-64bitMD5_16670399404877552232.png',
         'bitmap-64bitMD5_2507897274083364964.png',
         'bitmap-64bitMD5_7325267995523877959.png',
         'bitmap-64bitMD5_2181381724594493116.png'])
    self._assert_directory_contents(
        os.path.join(self._temp_dir, 'green_skp'),
        ['bitmap-64bitMD5_12587324416545178013.png',
         'bitmap-64bitMD5_7624374914829746293.png',
         'bitmap-64bitMD5_5686489729535631913.png',
         'bitmap-64bitMD5_7980646035555096146.png',
         'bitmap-64bitMD5_17817086664365875131.png',
         'bitmap-64bitMD5_10673669813016809363.png'])

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

  def _assert_directory_contents(self, dir_path, expected_filenames):
    """Asserts that files found in a dir are identical to expected_filenames.

    Args:
      dir_path: Path to a directory on local disk.
      expected_filenames: Set containing the expected filenames within the dir.

    Raises:
      AssertionError: contents of the directory are not identical to
                      expected_filenames.
    """
    self.assertEqual(set(os.listdir(dir_path)), set(expected_filenames))


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
