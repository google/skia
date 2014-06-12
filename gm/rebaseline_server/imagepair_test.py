#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test imagepair.py
"""

# System-level imports
import shutil
import tempfile
import unittest

# Local imports
import imagediffdb
import imagepair


IMG_URL_BASE = 'http://chromium-skia-gm.commondatastorage.googleapis.com/gm/bitmap-64bitMD5/'


class ImagePairTest(unittest.TestCase):

  def setUp(self):
    self._temp_dir = tempfile.mkdtemp()
    self.maxDiff = None

  def tearDown(self):
    shutil.rmtree(self._temp_dir)

  def shortDescription(self):
    """Tells unittest framework to not print docstrings for test cases."""
    return None

  def test_endToEnd(self):
    """Tests ImagePair, using a real ImageDiffDB to download real images.

    TODO(epoger): Either in addition to or instead of this end-to-end test,
    we should perform some tests using either:
    1. a mock ImageDiffDB, or
    2. a real ImageDiffDB that doesn't hit Google Storage looking for input
       image files (maybe a file:// IMG_URL_BASE)
    """
    # params for each self-test:
    #
    # inputs:
    #  0. imageA_relative_URL
    #  1. imageB_relative_URL
    #  2. expectations dict
    #  3. extra_columns dict
    # expected output:
    #  4. expected result of ImagePair.as_dict()
    selftests = [
        [
            # inputs:
            'arcofzorro/16206093933823793653.png',
            'arcofzorro/16206093933823793653.png',
            None,
            {
                'builder': 'MyBuilder',
                'test': 'MyTest',
            },
            # expected output:
            {
                'extraColumns': {
                    'builder': 'MyBuilder',
                    'test': 'MyTest',
                },
                'imageAUrl': 'arcofzorro/16206093933823793653.png',
                'imageBUrl': 'arcofzorro/16206093933823793653.png',
                'isDifferent': False,
            },
        ],

        [
            # inputs:
            'arcofzorro/16206093933823793653.png',
            'arcofzorro/13786535001616823825.png',
            None,
            None,
            # expected output:
            {
                'differenceData': {
                    'maxDiffPerChannel': [255, 255, 247],
                    'numDifferingPixels': 662,
                    'percentDifferingPixels': 0.0662,
                    'perceptualDifference': 0.06620000000000914,
                },
                'imageAUrl': 'arcofzorro/16206093933823793653.png',
                'imageBUrl': 'arcofzorro/13786535001616823825.png',
                'isDifferent': True,
            },
        ],

        [
            # inputs:
            'gradients_degenerate_2pt/10552995703607727960.png',
            'gradients_degenerate_2pt/11198253335583713230.png',
            {
                'ignoreFailure': True,
                'bugs': [1001, 1002],
            },
            {
                'builder': 'MyBuilder',
                'test': 'MyTest',
            },
            # expected output:
            {
                'differenceData': {
                    'maxDiffPerChannel': [255, 0, 255],
                    'numDifferingPixels': 102400,
                    'percentDifferingPixels': 100.00,
                    'perceptualDifference': 100.00,
                },
                'expectations': {
                    'bugs': [1001, 1002],
                    'ignoreFailure': True,
                },
                'extraColumns': {
                    'builder': 'MyBuilder',
                    'test': 'MyTest',
                },
                'imageAUrl':
                    'gradients_degenerate_2pt/10552995703607727960.png',
                'imageBUrl':
                    'gradients_degenerate_2pt/11198253335583713230.png',
                'isDifferent': True,
            },
        ],

        # Test fix for http://skbug.com/2368 -- how do we handle an ImagePair
        # missing one of its images?
        [
            # inputs:
            'arcofzorro/16206093933823793653.png',
            'nonexistentDir/111111.png',
            {
                'ignoreFailure': True,
                'bugs': [1001, 1002],
            },
            {
                'builder': 'MyBuilder',
                'test': 'MyTest',
            },
            # expected output:
            {
                'expectations': {
                    'bugs': [1001, 1002],
                    'ignoreFailure': True,
                },
                'extraColumns': {
                    'builder': 'MyBuilder',
                    'test': 'MyTest',
                },
                'imageAUrl': 'arcofzorro/16206093933823793653.png',
                'imageBUrl': 'nonexistentDir/111111.png',
                'isDifferent': True,
            },
        ],
    ]

    db = imagediffdb.ImageDiffDB(self._temp_dir)
    for selftest in selftests:
      image_pair = imagepair.ImagePair(
          image_diff_db=db,
          base_url=IMG_URL_BASE,
          imageA_relative_url=selftest[0],
          imageB_relative_url=selftest[1],
          expectations=selftest[2],
          extra_columns=selftest[3])
      self.assertEqual(image_pair.as_dict(), selftest[4])


def main():
  suite = unittest.TestLoader().loadTestsFromTestCase(ImagePairTest)
  unittest.TextTestRunner(verbosity=2).run(suite)


if __name__ == '__main__':
  main()
