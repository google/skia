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

IMAGES_SUBDIR = 'images'
DIFFS_SUBDIR = 'diffs'
WHITEDIFFS_SUBDIR = 'whitediffs'

VALUES_PER_BAND = 256


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

    # Generate the diff image (absolute diff at each pixel) and
    # max_diff_per_channel.
    diff_image = _generate_image_diff(actual_image, expected_image)
    diff_histogram = diff_image.histogram()
    (diff_width, diff_height) = diff_image.size
    self._weighted_diff_measure = _calculate_weighted_diff_metric(
        diff_histogram, diff_width * diff_height)
    self._max_diff_per_channel = _max_per_band(diff_histogram)

    # Generate the whitediff image (any differing pixels show as white).
    # This is tricky, because when you convert color images to grayscale or
    # black & white in PIL, it has its own ideas about thresholds.
    # We have to force it: if a pixel has any color at all, it's a '1'.
    bands = diff_image.split()
    graydiff_image = ImageChops.lighter(ImageChops.lighter(
        bands[0], bands[1]), bands[2])
    whitediff_image = (graydiff_image.point(lambda p: p > 0 and VALUES_PER_BAND)
                                     .convert('1', dither=Image.NONE))

    # Final touches on diff_image: use whitediff_image as an alpha mask.
    # Unchanged pixels are transparent; differing pixels are opaque.
    diff_image.putalpha(whitediff_image)

    # Store the diff and whitediff images generated above.
    diff_image_locator = _get_difference_locator(
        expected_image_locator=expected_image_locator,
        actual_image_locator=actual_image_locator)
    basename = str(diff_image_locator) + IMAGE_SUFFIX
    _save_image(diff_image, os.path.join(
        storage_root, DIFFS_SUBDIR, basename))
    _save_image(whitediff_image, os.path.join(
        storage_root, WHITEDIFFS_SUBDIR, basename))

    # Calculate difference metrics.
    (self._width, self._height) = diff_image.size
    self._num_pixels_differing = (
        whitediff_image.histogram()[VALUES_PER_BAND - 1])

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

  def get_max_diff_per_channel(self):
    """Returns the maximum difference between the expected and actual images
    for each R/G/B channel, as a list."""
    return self._max_diff_per_channel


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

def _calculate_weighted_diff_metric(histogram, num_pixels):
  """Given the histogram of a diff image (per-channel diff at each
  pixel between two images), calculate the weighted diff metric (a
  stab at how different the two images really are).

  Args:
    histogram: PIL histogram of a per-channel diff between two images
    num_pixels: integer; the total number of pixels in the diff image

  Returns: a weighted diff metric, as a float between 0 and 100 (inclusive).
  """
  # TODO(epoger): As a wild guess at an appropriate metric, weight each
  # different pixel by the square of its delta value.  (The more different
  # a pixel is from its expectation, the more we care about it.)
  # In the long term, we will probably use some metric generated by
  # skpdiff anyway.
  assert(len(histogram) % VALUES_PER_BAND == 0)
  num_bands = len(histogram) / VALUES_PER_BAND
  max_diff = num_pixels * num_bands * (VALUES_PER_BAND - 1)**2
  total_diff = 0
  for index in xrange(len(histogram)):
    total_diff += histogram[index] * (index % VALUES_PER_BAND)**2
  return float(100 * total_diff) / max_diff

def _max_per_band(histogram):
  """Given the histogram of an image, return the maximum value of each band
  (a.k.a. "color channel", such as R/G/B) across the entire image.

  Args:
    histogram: PIL histogram

  Returns the maximum value of each band within the image histogram, as a list.
  """
  max_per_band = []
  assert(len(histogram) % VALUES_PER_BAND == 0)
  num_bands = len(histogram) / VALUES_PER_BAND
  for band in xrange(num_bands):
    # Assuming that VALUES_PER_BAND is 256...
    #  the 'R' band makes up indices 0-255 in the histogram,
    #  the 'G' band makes up indices 256-511 in the histogram,
    #  etc.
    min_index = band * VALUES_PER_BAND
    index = min_index + VALUES_PER_BAND
    while index > min_index:
      index -= 1
      if histogram[index] > 0:
        max_per_band.append(index - min_index)
        break
  return max_per_band

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

def _save_image(image, filepath, format='PNG'):
  """Write an image to disk, creating any intermediate directories as needed.

  Args:
    image: a PIL image object
    filepath: path on local disk to write image to
    format: one of the PIL image formats, listed at
            http://effbot.org/imagingbook/formats.htm
  """
  _mkdir_unless_exists(os.path.dirname(filepath))
  image.save(filepath, format)

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
