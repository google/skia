#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Calulate differences between image pairs, and store them in a database.
"""

import contextlib
import logging
import os
import shutil
import urllib
try:
  from PIL import Image, ImageChops
except ImportError:
  raise ImportError('Requires PIL to be installed; see '
                    + 'http://www.pythonware.com/products/pil/')

IMAGE_SUFFIX = '.png'
IMAGE_FORMAT = 'PNG'   # must match one of the PIL image formats, listed at
                       # http://effbot.org/imagingbook/formats.htm

IMAGES_SUBDIR = 'images'
DIFFS_SUBDIR = 'diffs'
WHITEDIFFS_SUBDIR = 'whitediffs'


class DiffRecord(object):
  """ Record of differences between two images. """

  def __init__(self, storage_root,
               expected_image_url, expected_image_locator,
               actual_image_url, actual_image_locator):
    """Download this pair of images (unless we already have them on local disk),
    and prepare a DiffRecord for them.

    TODO(epoger): Make this asynchronously download images, rather than blocking
    until the images have been downloaded and processed.

    Args:
      storage_root: root directory on local disk within which we store all
          images
      expected_image_url: file or HTTP url from which we will download the
          expected image
      expected_image_locator: a unique ID string under which we will store the
          expected image within storage_root (probably including a checksum to
          guarantee uniqueness)
      actual_image_url: file or HTTP url from which we will download the
          actual image
      actual_image_locator: a unique ID string under which we will store the
          actual image within storage_root (probably including a checksum to
          guarantee uniqueness)
    """
    # Download the expected/actual images, if we don't have them already.
    expected_image = _download_and_open_image(
        os.path.join(storage_root, IMAGES_SUBDIR,
                     str(expected_image_locator) + IMAGE_SUFFIX),
        expected_image_url)
    actual_image = _download_and_open_image(
        os.path.join(storage_root, IMAGES_SUBDIR,
                     str(actual_image_locator) + IMAGE_SUFFIX),
        actual_image_url)

    # Store the diff image (absolute diff at each pixel).
    diff_image = _generate_image_diff(actual_image, expected_image)
    self._weighted_diff_measure = _calculate_weighted_diff_metric(diff_image)
    diff_image_locator = _get_difference_locator(
        expected_image_locator=expected_image_locator,
        actual_image_locator=actual_image_locator)
    diff_image_filepath = os.path.join(
        storage_root, DIFFS_SUBDIR, str(diff_image_locator) + IMAGE_SUFFIX)
    _mkdir_unless_exists(os.path.join(storage_root, DIFFS_SUBDIR))
    diff_image.save(diff_image_filepath, IMAGE_FORMAT)

    # Store the whitediff image (any differing pixels show as white).
    #
    # TODO(epoger): From http://effbot.org/imagingbook/image.htm , it seems
    # like we should be able to use im.point(function, mode) to perform both
    # the point() and convert('1') operations simultaneously, but I couldn't
    # get it to work.
    whitediff_image = (diff_image.point(lambda p: (0, 256)[p!=0])
                                 .convert('1'))
    whitediff_image_filepath = os.path.join(
        storage_root, WHITEDIFFS_SUBDIR, str(diff_image_locator) + IMAGE_SUFFIX)
    _mkdir_unless_exists(os.path.join(storage_root, WHITEDIFFS_SUBDIR))
    whitediff_image.save(whitediff_image_filepath, IMAGE_FORMAT)

    # Calculate difference metrics.
    (self._width, self._height) = diff_image.size
    self._num_pixels_differing = whitediff_image.histogram()[255]

  def get_num_pixels_differing(self):
    """Returns the absolute number of pixels that differ."""
    return self._num_pixels_differing

  def get_percent_pixels_differing(self):
    """Returns the percentage of pixels that differ, as a float between
    0 and 100 (inclusive)."""
    return ((float(self._num_pixels_differing) * 100) /
            (self._width * self._height))

  def get_weighted_diff_measure(self):
    """Returns a weighted measure of image diffs, as a float between 0 and 100
    (inclusive)."""
    return self._weighted_diff_measure


class ImageDiffDB(object):
  """ Calculates differences between image pairs, maintaining a database of
  them for download."""

  def __init__(self, storage_root):
    """
    Args:
      storage_root: string; root path within the DB will store all of its stuff
    """
    self._storage_root = storage_root

    # Dictionary of DiffRecords, keyed by (expected_image_locator,
    # actual_image_locator) tuples.
    self._diff_dict = {}

  def add_image_pair(self,
                     expected_image_url, expected_image_locator,
                     actual_image_url, actual_image_locator):
    """Download this pair of images (unless we already have them on local disk),
    and prepare a DiffRecord for them.

    TODO(epoger): Make this asynchronously download images, rather than blocking
    until the images have been downloaded and processed.
    When we do that, we should probably add a new method that will block
    until all of the images have been downloaded and processed.  Otherwise,
    we won't know when it's safe to start calling get_diff_record().
    jcgregorio notes: maybe just make ImageDiffDB thread-safe and create a
    thread-pool/worker queue at a higher level that just uses ImageDiffDB?

    Args:
      expected_image_url: file or HTTP url from which we will download the
          expected image
      expected_image_locator: a unique ID string under which we will store the
          expected image within storage_root (probably including a checksum to
          guarantee uniqueness)
      actual_image_url: file or HTTP url from which we will download the
          actual image
      actual_image_locator: a unique ID string under which we will store the
          actual image within storage_root (probably including a checksum to
          guarantee uniqueness)
    """
    key = (expected_image_locator, actual_image_locator)
    if not key in self._diff_dict:
      try:
        new_diff_record = DiffRecord(
            self._storage_root,
            expected_image_url=expected_image_url,
            expected_image_locator=expected_image_locator,
            actual_image_url=actual_image_url,
            actual_image_locator=actual_image_locator)
      except:
        logging.exception('got exception while creating new DiffRecord')
        return
      self._diff_dict[key] = new_diff_record

  def get_diff_record(self, expected_image_locator, actual_image_locator):
    """Returns the DiffRecord for this image pair.

    Raises a KeyError if we don't have a DiffRecord for this image pair.
    """
    key = (expected_image_locator, actual_image_locator)
    return self._diff_dict[key]


# Utility functions

def _calculate_weighted_diff_metric(image):
  """Given a diff image (per-channel diff at each pixel between two images),
  calculate the weighted diff metric (a stab at how different the two images
  really are).

  Args:
    image: PIL image; a per-channel diff between two images

  Returns: a weighted diff metric, as a float between 0 and 100 (inclusive).
  """
  # TODO(epoger): This is just a wild guess at an appropriate metric.
  # In the long term, we will probably use some metric generated by
  # skpdiff anyway.
  (width, height) = image.size
  maxdiff = 3 * (width * height) * 255**2
  h = image.histogram()
  assert(len(h) % 256 == 0)
  totaldiff = sum(map(lambda index,value: value * (index%256)**2,
                      range(len(h)), h))
  return float(100 * totaldiff) / maxdiff

def _generate_image_diff(image1, image2):
  """Wrapper for ImageChops.difference(image1, image2) that will handle some
  errors automatically, or at least yield more useful error messages.

  TODO(epoger): Currently, some of the images generated by the bots are RGBA
  and others are RGB.  I'm not sure why that is.  For now, to avoid confusion
  within the UI, convert all to RGB when diffing.

  Args:
    image1: a PIL image object
    image2: a PIL image object

  Returns: per-pixel diffs between image1 and image2, as a PIL image object
  """
  try:
    return ImageChops.difference(image1.convert('RGB'), image2.convert('RGB'))
  except ValueError:
    logging.error('Error diffing image1 [%s] and image2 [%s].' % (
        repr(image1), repr(image2)))
    raise

def _download_and_open_image(local_filepath, url):
  """Open the image at local_filepath; if there is no file at that path,
  download it from url to that path and then open it.

  Args:
    local_filepath: path on local disk where the image should be stored
    url: URL from which we can download the image if we don't have it yet

  Returns: a PIL image object
  """
  if not os.path.exists(local_filepath):
    _mkdir_unless_exists(os.path.dirname(local_filepath))
    with contextlib.closing(urllib.urlopen(url)) as url_handle:
      with open(local_filepath, 'wb') as file_handle:
        shutil.copyfileobj(fsrc=url_handle, fdst=file_handle)
  return _open_image(local_filepath)

def _open_image(filepath):
  """Wrapper for Image.open(filepath) that yields more useful error messages.

  Args:
    filepath: path on local disk to load image from

  Returns: a PIL image object
  """
  try:
    return Image.open(filepath)
  except IOError:
    logging.error('IOError loading image file %s' % filepath)
    raise

def _mkdir_unless_exists(path):
  """Unless path refers to an already-existing directory, create it.

  Args:
    path: path on local disk
  """
  if not os.path.isdir(path):
    os.makedirs(path)

def _get_difference_locator(expected_image_locator, actual_image_locator):
  """Returns the locator string used to look up the diffs between expected_image
  and actual_image.

  Args:
    expected_image_locator: locator string pointing at expected image
    actual_image_locator: locator string pointing at actual image

  Returns: locator where the diffs between expected and actual images can be
      found
  """
  return "%s-vs-%s" % (expected_image_locator, actual_image_locator)
