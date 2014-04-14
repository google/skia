#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

ImagePair class (see class docstring for details)
"""

import posixpath

# Keys used within ImagePair dictionary representations.
# NOTE: Keep these in sync with static/constants.js
KEY__DIFFERENCE_DATA = 'differenceData'
KEY__EXPECTATIONS_DATA = 'expectations'
KEY__EXTRA_COLUMN_VALUES = 'extraColumns'
KEY__IMAGE_A_URL = 'imageAUrl'
KEY__IMAGE_B_URL = 'imageBUrl'
KEY__IS_DIFFERENT = 'isDifferent'


class ImagePair(object):
  """Describes a pair of images, pixel difference info, and optional metadata.
  """

  def __init__(self, image_diff_db,
               base_url, imageA_relative_url, imageB_relative_url,
               expectations=None, extra_columns=None):
    """
    Args:
      image_diff_db: ImageDiffDB instance we use to generate/store image diffs
      base_url: base of all image URLs
      imageA_relative_url: string; URL pointing at an image, relative to
          base_url; or None, if this image is missing
      imageB_relative_url: string; URL pointing at an image, relative to
          base_url; or None, if this image is missing
      expectations: optional dictionary containing expectations-specific
          metadata (ignore-failure, bug numbers, etc.)
      extra_columns: optional dictionary containing more metadata (test name,
          builder name, etc.)
    """
    self.base_url = base_url
    self.imageA_relative_url = imageA_relative_url
    self.imageB_relative_url = imageB_relative_url
    self.expectations_dict = expectations
    self.extra_columns_dict = extra_columns
    if not imageA_relative_url or not imageB_relative_url:
      self._is_different = True
      self._diff_record = None
      self._diff_record_set = True
    elif imageA_relative_url == imageB_relative_url:
      self._is_different = False
      self._diff_record = None
      self._diff_record_set = True
    else:
      # Tell image_diff_db to add this ImagePair.
      # It will do so in a separate thread so as not to block this one;
      # when you call self.get_diff_record(), it will block until the results
      # are ready.
      image_diff_db.add_image_pair_async(
          expected_image_locator=imageA_relative_url,
          expected_image_url=posixpath.join(base_url, imageA_relative_url),
          actual_image_locator=imageB_relative_url,
          actual_image_url=posixpath.join(base_url, imageB_relative_url))
      self._image_diff_db = image_diff_db
      self._diff_record_set = False

  def get_diff_record(self):
    """Returns the DiffRecord associated with this ImagePair.

    Returns None if the images are identical, or one is missing.
    This method will block until the DiffRecord is available.
    """
    if not self._diff_record_set:
      self._diff_record = self._image_diff_db.get_diff_record(
          expected_image_locator=self.imageA_relative_url,
          actual_image_locator=self.imageB_relative_url)
      self._image_diff_db = None  # release reference, no longer needed
      if (self._diff_record and
          self._diff_record.get_num_pixels_differing() == 0):
        self._is_different = False
      else:
        self._is_different = True
      self._diff_record_set = True
    return self._diff_record

  def as_dict(self):
    """Returns a dictionary describing this ImagePair.

    Uses the KEY__* constants as keys.
    """
    asdict = {
        KEY__IMAGE_A_URL: self.imageA_relative_url,
        KEY__IMAGE_B_URL: self.imageB_relative_url,
    }
    if self.expectations_dict:
      asdict[KEY__EXPECTATIONS_DATA] = self.expectations_dict
    if self.extra_columns_dict:
      asdict[KEY__EXTRA_COLUMN_VALUES] = self.extra_columns_dict
    diff_record = self.get_diff_record()
    if diff_record and (diff_record.get_num_pixels_differing() > 0):
      asdict[KEY__DIFFERENCE_DATA] = diff_record.as_dict()
    asdict[KEY__IS_DIFFERENT] = self._is_different
    return asdict
