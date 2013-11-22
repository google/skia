#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test imagediffdb.py

TODO(epoger): Modify to use Python's unittest framework.
"""

# System-level imports
import logging

# Local imports
import imagediffdb


IMAGE_URL_BASE = 'http://chromium-skia-gm.commondatastorage.googleapis.com/gm/bitmap-64bitMD5/'

def main():
  logging.basicConfig(level=logging.INFO)

  # params for each self-test:
  # 0. expected image locator
  # 1. expected image URL
  # 2. actual image locator
  # 3. actual image URL
  # 4. expected percent_pixels_differing (as a string, to 4 decimal places)
  # 5. expected weighted_diff_measure (as a string, to 4 decimal places)
  # 6. expected max_diff_per_channel
  selftests = [
      [
          '16206093933823793653',
          IMAGE_URL_BASE + 'arcofzorro/16206093933823793653.png',
          '13786535001616823825',
          IMAGE_URL_BASE + 'arcofzorro/13786535001616823825.png',
          '0.0662', '0.0113', [255, 255, 247],
      ],
      [
          '10552995703607727960',
          IMAGE_URL_BASE + 'gradients_degenerate_2pt/10552995703607727960.png',
          '11198253335583713230',
          IMAGE_URL_BASE + 'gradients_degenerate_2pt/11198253335583713230.png',
          '100.0000', '66.6667', [255, 0, 255],
      ],
  ]

  # Add all image pairs to the database
  db = imagediffdb.ImageDiffDB('/tmp/ImageDiffDB')
  for selftest in selftests:
    retval = db.add_image_pair(
        expected_image_locator=selftest[0], expected_image_url=selftest[1],
        actual_image_locator=selftest[2],   actual_image_url=selftest[3])

  # Fetch each image pair from the database
  for selftest in selftests:
    record = db.get_diff_record(expected_image_locator=selftest[0],
                                actual_image_locator=selftest[2])
    assert (('%.4f' % record.get_percent_pixels_differing()) == selftest[4])
    assert (('%.4f' % record.get_weighted_diff_measure()) == selftest[5])
    assert (record.get_max_diff_per_channel() == selftest[6])
  logging.info("Self-test completed successfully!")


if __name__ == '__main__':
  main()
