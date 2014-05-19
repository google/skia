#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test the render_pictures binary.
"""

# System-level imports
import copy
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

# Manually verified: 640x400 red rectangle with black border
# Standard expectations will be set up in such a way that this image fails
# the comparison.
RED_WHOLEIMAGE = {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 11092453015575919668,
    "comparisonResult" : "failed",
    "filepath" : "red_skp.png",
}

# Manually verified: 640x400 green rectangle with black border
# Standard expectations will be set up in such a way that this image passes
# the comparison.
GREEN_WHOLEIMAGE = {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 8891695120562235492,
    "comparisonResult" : "succeeded",
    "filepath" : "green_skp.png",
}

# Manually verified these 6 images, all 256x256 tiles,
# consistent with a tiled version of the 640x400 red rect
# with black borders.
# Standard expectations will be set up in such a way that these images fail
# the comparison.
RED_TILES = [{
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 5815827069051002745,
    "comparisonResult" : "failed",
    "filepath" : "red_skp-tile0.png",
},{
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 9323613075234140270,
    "comparisonResult" : "failed",
    "filepath" : "red_skp-tile1.png",
}, {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 16670399404877552232,
    "comparisonResult" : "failed",
    "filepath" : "red_skp-tile2.png",
}, {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 2507897274083364964,
    "comparisonResult" : "failed",
    "filepath" : "red_skp-tile3.png",
}, {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 7325267995523877959,
    "comparisonResult" : "failed",
    "filepath" : "red_skp-tile4.png",
}, {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 2181381724594493116,
    "comparisonResult" : "failed",
    "filepath" : "red_skp-tile5.png",
}]

# Manually verified these 6 images, all 256x256 tiles,
# consistent with a tiled version of the 640x400 green rect
# with black borders.
# Standard expectations will be set up in such a way that these images pass
# the comparison.
GREEN_TILES = [{
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 12587324416545178013,
    "comparisonResult" : "succeeded",
    "filepath" : "green_skp-tile0.png",
}, {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 7624374914829746293,
    "comparisonResult" : "succeeded",
    "filepath" : "green_skp-tile1.png",
}, {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 5686489729535631913,
    "comparisonResult" : "succeeded",
    "filepath" : "green_skp-tile2.png",
}, {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 7980646035555096146,
    "comparisonResult" : "succeeded",
    "filepath" : "green_skp-tile3.png",
}, {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 17817086664365875131,
    "comparisonResult" : "succeeded",
    "filepath" : "green_skp-tile4.png",
}, {
    "checksumAlgorithm" : "bitmap-64bitMD5",
    "checksumValue" : 10673669813016809363,
    "comparisonResult" : "succeeded",
    "filepath" : "green_skp-tile5.png",
}]


def modified_dict(input_dict, modification_dict):
  """Returns a dict, with some modifications applied to it.

  Args:
    input_dict: a dictionary (which will be copied, not modified in place)
    modification_dict: a set of key/value pairs to overwrite in the dict
  """
  output_dict = input_dict.copy()
  output_dict.update(modification_dict)
  return output_dict


def modified_list_of_dicts(input_list, modification_dict):
  """Returns a list of dicts, with some modifications applied to each dict.

  Args:
    input_list: a list of dictionaries; these dicts will be copied, not
        modified in place
    modification_dict: a set of key/value pairs to overwrite in each dict
        within input_list
  """
  output_list = []
  for input_dict in input_list:
    output_dict = modified_dict(input_dict, modification_dict)
    output_list.append(output_dict)
  return output_list


class RenderPicturesTest(base_unittest.TestCase):

  def setUp(self):
    self.maxDiff = MAX_DIFF_LENGTH
    self._expectations_dir = tempfile.mkdtemp()
    self._input_skp_dir = tempfile.mkdtemp()
    # All output of render_pictures binary will go into this directory.
    self._output_dir = tempfile.mkdtemp()

  def tearDown(self):
    shutil.rmtree(self._expectations_dir)
    shutil.rmtree(self._input_skp_dir)
    shutil.rmtree(self._output_dir)

  def test_tiled_whole_image(self):
    """Run render_pictures with tiles and --writeWholeImage flag.

    TODO(epoger): This test generates undesired results!  The JSON summary
    includes both whole-image and tiled-images (as it should), but only
    whole-images are written out to disk.  See http://skbug.com/2463
    Once I fix that, I should add a similar test that exercises mismatchPath.

    TODO(epoger): I noticed that when this is run without --writePath being
    specified, this test writes red_skp.png and green_skp.png to the current
    directory.  We should fix that... if --writePath is not specified, this
    probably shouldn't write out red_skp.png and green_skp.png at all!
    See http://skbug.com/2464
    """
    output_json_path = os.path.join(self._output_dir, 'actuals.json')
    write_path_dir = self.create_empty_dir(
        path=os.path.join(self._output_dir, 'writePath'))
    self._generate_skps()
    expectations_path = self._create_expectations()
    self._run_render_pictures([
        '-r', self._input_skp_dir,
        '--bbh', 'grid', '256', '256',
        '--mode', 'tile', '256', '256',
        '--readJsonSummaryPath', expectations_path,
        '--writeJsonSummaryPath', output_json_path,
        '--writePath', write_path_dir,
        '--writeWholeImage'])
    expected_summary_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                "tiled-images": RED_TILES,
                "whole-image": RED_WHOLEIMAGE,
            },
            "green.skp": {
                "tiled-images": GREEN_TILES,
                "whole-image": GREEN_WHOLEIMAGE,
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)
    self._assert_directory_contents(
        write_path_dir, ['red_skp.png', 'green_skp.png'])

  def test_missing_tile_and_whole_image(self):
    """test_tiled_whole_image, but missing expectations for some images.
    """
    output_json_path = os.path.join(self._output_dir, 'actuals.json')
    write_path_dir = self.create_empty_dir(
        path=os.path.join(self._output_dir, 'writePath'))
    self._generate_skps()
    expectations_path = self._create_expectations(missing_some_images=True)
    self._run_render_pictures([
        '-r', self._input_skp_dir,
        '--bbh', 'grid', '256', '256',
        '--mode', 'tile', '256', '256',
        '--readJsonSummaryPath', expectations_path,
        '--writeJsonSummaryPath', output_json_path,
        '--writePath', write_path_dir,
        '--writeWholeImage'])
    modified_red_tiles = copy.deepcopy(RED_TILES)
    modified_red_tiles[5]['comparisonResult'] = 'no-comparison'
    expected_summary_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                "tiled-images": modified_red_tiles,
                "whole-image": modified_dict(
                    RED_WHOLEIMAGE, {"comparisonResult" : "no-comparison"}),
            },
            "green.skp": {
                "tiled-images": GREEN_TILES,
                "whole-image": GREEN_WHOLEIMAGE,
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def _test_untiled(self, expectations_path=None, expected_summary_dict=None,
                    additional_args=None):
    """Base for multiple tests without tiles.

    Args:
      expectations_path: path we should pass using --readJsonSummaryPath, or
          None if we should create the default expectations file
      expected_summary_dict: dict we should compare against the output actual
          results summary, or None if we should use a default comparison dict
      additional_args: array of command-line args to add when we run
          render_pictures
    """
    output_json_path = os.path.join(self._output_dir, 'actuals.json')
    write_path_dir = self.create_empty_dir(
        path=os.path.join(self._output_dir, 'writePath'))
    self._generate_skps()
    if expectations_path == None:
      expectations_path = self._create_expectations()
    args = [
        '-r', self._input_skp_dir,
        '--readJsonSummaryPath', expectations_path,
        '--writePath', write_path_dir,
        '--writeJsonSummaryPath', output_json_path,
    ]
    if additional_args:
      args.extend(additional_args)
    self._run_render_pictures(args)
    if expected_summary_dict == None:
      expected_summary_dict = {
          "header" : EXPECTED_HEADER_CONTENTS,
          "actual-results" : {
              "red.skp": {
                  "whole-image": RED_WHOLEIMAGE,
              },
              "green.skp": {
                  "whole-image": GREEN_WHOLEIMAGE,
              }
          }
      }
    self._assert_json_contents(output_json_path, expected_summary_dict)
    self._assert_directory_contents(
        write_path_dir, ['red_skp.png', 'green_skp.png'])

  def test_untiled(self):
    """Basic test without tiles."""
    self._test_untiled()

  def test_untiled_empty_expectations_file(self):
    """Same as test_untiled, but with an empty expectations file."""
    expectations_path = os.path.join(self._expectations_dir, 'empty')
    with open(expectations_path, 'w') as fh:
      pass
    expected_summary_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                "whole-image": modified_dict(
                    RED_WHOLEIMAGE, {"comparisonResult" : "no-comparison"}),
            },
            "green.skp": {
                "whole-image": modified_dict(
                    GREEN_WHOLEIMAGE, {"comparisonResult" : "no-comparison"}),
            }
        }
    }
    self._test_untiled(expectations_path=expectations_path,
                       expected_summary_dict=expected_summary_dict)

  def test_untiled_writeChecksumBasedFilenames(self):
    """Same as test_untiled, but with --writeChecksumBasedFilenames."""
    output_json_path = os.path.join(self._output_dir, 'actuals.json')
    write_path_dir = self.create_empty_dir(
        path=os.path.join(self._output_dir, 'writePath'))
    self._generate_skps()
    self._run_render_pictures(['-r', self._input_skp_dir,
                               '--writeChecksumBasedFilenames',
                               '--writePath', write_path_dir,
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
    self._assert_directory_contents(write_path_dir, ['red_skp', 'green_skp'])
    self._assert_directory_contents(
        os.path.join(write_path_dir, 'red_skp'),
        ['bitmap-64bitMD5_11092453015575919668.png'])
    self._assert_directory_contents(
        os.path.join(write_path_dir, 'green_skp'),
        ['bitmap-64bitMD5_8891695120562235492.png'])

  def test_untiled_validate(self):
    """Same as test_untiled, but with --validate."""
    self._test_untiled(additional_args=['--validate'])

  def test_untiled_without_writePath(self):
    """Same as test_untiled, but without --writePath."""
    output_json_path = os.path.join(self._output_dir, 'actuals.json')
    self._generate_skps()
    expectations_path = self._create_expectations()
    self._run_render_pictures([
        '-r', self._input_skp_dir,
        '--readJsonSummaryPath', expectations_path,
        '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                "whole-image": RED_WHOLEIMAGE,
            },
            "green.skp": {
                "whole-image": GREEN_WHOLEIMAGE,
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)

  def test_tiled(self):
    """Generate individual tiles."""
    output_json_path = os.path.join(self._output_dir, 'actuals.json')
    write_path_dir = self.create_empty_dir(
        path=os.path.join(self._output_dir, 'writePath'))
    self._generate_skps()
    expectations_path = self._create_expectations()
    self._run_render_pictures([
        '-r', self._input_skp_dir,
        '--bbh', 'grid', '256', '256',
        '--mode', 'tile', '256', '256',
        '--readJsonSummaryPath', expectations_path,
        '--writePath', write_path_dir,
        '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                "tiled-images": RED_TILES,
            },
            "green.skp": {
                "tiled-images": GREEN_TILES,
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)
    self._assert_directory_contents(
        write_path_dir,
        ['red_skp-tile0.png', 'red_skp-tile1.png', 'red_skp-tile2.png',
         'red_skp-tile3.png', 'red_skp-tile4.png', 'red_skp-tile5.png',
         'green_skp-tile0.png', 'green_skp-tile1.png', 'green_skp-tile2.png',
         'green_skp-tile3.png', 'green_skp-tile4.png', 'green_skp-tile5.png',
        ])

  def test_tiled_mismatches(self):
    """Same as test_tiled, but only write out mismatching images."""
    output_json_path = os.path.join(self._output_dir, 'actuals.json')
    mismatch_path_dir = self.create_empty_dir(
        path=os.path.join(self._output_dir, 'mismatchPath'))
    self._generate_skps()
    expectations_path = self._create_expectations()
    self._run_render_pictures([
        '-r', self._input_skp_dir,
        '--bbh', 'grid', '256', '256',
        '--mode', 'tile', '256', '256',
        '--readJsonSummaryPath', expectations_path,
        '--mismatchPath', mismatch_path_dir,
        '--writeJsonSummaryPath', output_json_path])
    expected_summary_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "actual-results" : {
            "red.skp": {
                "tiled-images": RED_TILES,
            },
            "green.skp": {
                "tiled-images": GREEN_TILES,
            }
        }
    }
    self._assert_json_contents(output_json_path, expected_summary_dict)
    self._assert_directory_contents(
        mismatch_path_dir,
        ['red_skp-tile0.png', 'red_skp-tile1.png', 'red_skp-tile2.png',
         'red_skp-tile3.png', 'red_skp-tile4.png', 'red_skp-tile5.png',
        ])

  def test_tiled_writeChecksumBasedFilenames(self):
    """Same as test_tiled, but with --writeChecksumBasedFilenames."""
    output_json_path = os.path.join(self._output_dir, 'actuals.json')
    write_path_dir = self.create_empty_dir(
        path=os.path.join(self._output_dir, 'writePath'))
    self._generate_skps()
    self._run_render_pictures(['-r', self._input_skp_dir,
                               '--bbh', 'grid', '256', '256',
                               '--mode', 'tile', '256', '256',
                               '--writeChecksumBasedFilenames',
                               '--writePath', write_path_dir,
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
    self._assert_directory_contents(write_path_dir, ['red_skp', 'green_skp'])
    self._assert_directory_contents(
        os.path.join(write_path_dir, 'red_skp'),
        ['bitmap-64bitMD5_5815827069051002745.png',
         'bitmap-64bitMD5_9323613075234140270.png',
         'bitmap-64bitMD5_16670399404877552232.png',
         'bitmap-64bitMD5_2507897274083364964.png',
         'bitmap-64bitMD5_7325267995523877959.png',
         'bitmap-64bitMD5_2181381724594493116.png'])
    self._assert_directory_contents(
        os.path.join(write_path_dir, 'green_skp'),
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

  def _create_expectations(self, missing_some_images=False,
                           rel_path='expectations.json'):
    """Creates expectations JSON file within self._expectations_dir .

    Args:
      missing_some_images: (bool) whether to remove expectations for a subset
          of the images
      rel_path: (string) relative path within self._expectations_dir to write
          the expectations into

    Returns: full path to the expectations file created.
    """
    expectations_dict = {
        "header" : EXPECTED_HEADER_CONTENTS,
        "expected-results" : {
            # red.skp: these should fail the comparison
            "red.skp": {
                "tiled-images": modified_list_of_dicts(
                    RED_TILES, {'checksumValue': 11111}),
                "whole-image": modified_dict(
                    RED_WHOLEIMAGE, {'checksumValue': 22222}),
            },
            # green.skp: these should pass the comparison
            "green.skp": {
                "tiled-images": GREEN_TILES,
                "whole-image": GREEN_WHOLEIMAGE,
            }
        }
    }
    if missing_some_images:
      del expectations_dict['expected-results']['red.skp']['whole-image']
      del expectations_dict['expected-results']['red.skp']['tiled-images'][-1]
    path = os.path.join(self._expectations_dir, rel_path)
    with open(path, 'w') as fh:
      json.dump(expectations_dict, fh)
    return path

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
    prettyprinted_expected_dict = json.dumps(expected_dict, sort_keys=True,
                                             indent=2)
    with open(json_path, 'r') as fh:
      prettyprinted_json_dict = json.dumps(json.load(fh), sort_keys=True,
                                           indent=2)
    self.assertMultiLineEqual(prettyprinted_expected_dict,
                              prettyprinted_json_dict)


def main():
  base_unittest.main(RenderPicturesTest)


if __name__ == '__main__':
  main()
