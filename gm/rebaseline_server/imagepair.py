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
KEY__IMAGEPAIRS__DIFFERENCES = 'differenceData'
KEY__IMAGEPAIRS__EXPECTATIONS = 'expectations'
KEY__IMAGEPAIRS__EXTRACOLUMNS = 'extraColumns'
KEY__IMAGEPAIRS__IMAGE_A_URL = 'imageAUrl'
KEY__IMAGEPAIRS__IMAGE_B_URL = 'imageBUrl'
KEY__IMAGEPAIRS__IS_DIFFERENT = 'isDifferent'


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
      self.diff_record = None
    elif imageA_relative_url == imageB_relative_url:
      self._is_different = False
      self.diff_record = None
    else:
      # TODO(epoger): Rather than blocking until image_diff_db can read in
      # the image pair and generate diffs, it would be better to do it
      # asynchronously: tell image_diff_db to download a bunch of file pairs,
      # and only block later if we're still waiting for diff_records to come
      # back.
      self._is_different = True
      image_diff_db.add_image_pair(
          expected_image_locator=imageA_relative_url,
          expected_image_url=posixpath.join(base_url, imageA_relative_url),
          actual_image_locator=imageB_relative_url,
          actual_image_url=posixpath.join(base_url, imageB_relative_url))
      self.diff_record = image_diff_db.get_diff_record(
          expected_image_locator=imageA_relative_url,
          actual_image_locator=imageB_relative_url)
      if self.diff_record and self.diff_record.get_num_pixels_differing() == 0:
        self._is_different = False

  def as_dict(self):
    """Returns a dictionary describing this ImagePair.

    Uses the KEY__IMAGEPAIRS__* constants as keys.
    """
    asdict = {
        KEY__IMAGEPAIRS__IMAGE_A_URL: self.imageA_relative_url,
        KEY__IMAGEPAIRS__IMAGE_B_URL: self.imageB_relative_url,
    }
    asdict[KEY__IMAGEPAIRS__IS_DIFFERENT] = self._is_different
    if self.expectations_dict:
      asdict[KEY__IMAGEPAIRS__EXPECTATIONS] = self.expectations_dict
    if self.extra_columns_dict:
      asdict[KEY__IMAGEPAIRS__EXTRACOLUMNS] = self.extra_columns_dict
    if self.diff_record and (self.diff_record.get_num_pixels_differing() > 0):
      asdict[KEY__IMAGEPAIRS__DIFFERENCES] = self.diff_record.as_dict()
    return asdict
