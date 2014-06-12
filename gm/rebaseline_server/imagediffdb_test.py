#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test imagediffdb.py
"""

# System-level imports
import logging
import shutil
import tempfile
import unittest

# Local imports
import imagediffdb


IMG_URL_BASE = ('http://chromium-skia-gm.commondatastorage.googleapis.com/gm/'
                'bitmap-64bitMD5/')


class ImageDiffDbTest(unittest.TestCase):

  def setUp(self):
    self._temp_dir = tempfile.mkdtemp()
    self.maxDiff = None

  def tearDown(self):
    shutil.rmtree(self._temp_dir)

  def shortDescription(self):
    """Tell unittest framework to not print docstrings for test cases."""
    return None

  def test_sanitize_locator(self):
    """Test _sanitize_locator()."""
    self.assertEqual(imagediffdb._sanitize_locator('simple'), 'simple')
    self.assertEqual(imagediffdb._sanitize_locator(1234), '1234')
    self.assertEqual(imagediffdb._sanitize_locator('one/two'),  'one_two')
    self.assertEqual(imagediffdb._sanitize_locator('one\\two'), 'one_two')
    self.assertEqual(imagediffdb._sanitize_locator('one_two'),  'one_two')

  def test_simple(self):
    """Test ImageDiffDB, downloading real known images from Google Storage.

    TODO(epoger): Instead of hitting Google Storage, we should read image
    files from local disk using a file:// IMG_URL_BASE.
    """
    # params for each self-test:
    # 0. expected image locator
    # 1. expected image URL
    # 2. actual image locator
    # 3. actual image URL
    # 4. expected percent_pixels_differing (as a string, to 4 decimal places)
    # 5. expected perceptual difference (as a string, to 4 decimal places)
    # 6. expected max_diff_per_channel
    selftests = [
        [
            'arcofzorro/16206093933823793653',
            IMG_URL_BASE + 'arcofzorro/16206093933823793653.png',
            'arcofzorro/13786535001616823825',
            IMG_URL_BASE + 'arcofzorro/13786535001616823825.png',
            '0.0662', '0.0662', [255, 255, 247],
        ],
        [
            'gradients_degenerate_2pt/10552995703607727960',
            IMG_URL_BASE + 'gradients_degenerate_2pt/10552995703607727960.png',
            'gradients_degenerate_2pt/11198253335583713230',
            IMG_URL_BASE + 'gradients_degenerate_2pt/11198253335583713230.png',
            '100.0000', '100.0000', [255, 0, 255],
        ],
    ]

    # Add all image pairs to the database
    db = imagediffdb.ImageDiffDB(self._temp_dir)
    for selftest in selftests:
      retval = db.add_image_pair(
          expected_image_locator=selftest[0], expected_image_url=selftest[1],
          actual_image_locator=selftest[2],   actual_image_url=selftest[3])

    # Fetch each image pair from the database
    for selftest in selftests:
      record = db.get_diff_record(expected_image_locator=selftest[0],
                                  actual_image_locator=selftest[2])
      self.assertEqual('%.4f' % record.get_percent_pixels_differing(),
                       selftest[4])
      self.assertEqual('%.4f' % record.get_perceptual_difference(), selftest[5])
      self.assertEqual(record.get_max_diff_per_channel(), selftest[6])


def main():
  suite = unittest.TestLoader().loadTestsFromTestCase(ImageDiffDbTest)
  unittest.TextTestRunner(verbosity=2).run(suite)


if __name__ == '__main__':
  main()
