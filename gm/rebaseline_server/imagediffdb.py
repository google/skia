#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Calulate differences between image pairs, and store them in a database.
"""

# System-level imports
import contextlib
import json
import logging
import os
import re
import shutil
import tempfile
import urllib

# Must fix up PYTHONPATH before importing from within Skia
import fix_pythonpath  # pylint: disable=W0611

# Imports from within Skia
import find_run_binary

SKPDIFF_BINARY = find_run_binary.find_path_to_program('skpdiff')

DEFAULT_IMAGE_SUFFIX = '.png'
DEFAULT_IMAGES_SUBDIR = 'images'

DISALLOWED_FILEPATH_CHAR_REGEX = re.compile('[^\w\-]')

RGBDIFFS_SUBDIR = 'diffs'
WHITEDIFFS_SUBDIR = 'whitediffs'

# Keys used within DiffRecord dictionary representations.
# NOTE: Keep these in sync with static/constants.js
KEY__DIFFERENCES__MAX_DIFF_PER_CHANNEL = 'maxDiffPerChannel'
KEY__DIFFERENCES__NUM_DIFF_PIXELS = 'numDifferingPixels'
KEY__DIFFERENCES__PERCENT_DIFF_PIXELS = 'percentDifferingPixels'
KEY__DIFFERENCES__PERCEPTUAL_DIFF = 'perceptualDifference'


class DiffRecord(object):
  """ Record of differences between two images. """

  def __init__(self, storage_root,
               expected_image_url, expected_image_locator,
               actual_image_url, actual_image_locator,
               expected_images_subdir=DEFAULT_IMAGES_SUBDIR,
               actual_images_subdir=DEFAULT_IMAGES_SUBDIR,
               image_suffix=DEFAULT_IMAGE_SUFFIX):
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
      expected_images_subdir: the subdirectory expected images are stored in.
      actual_images_subdir: the subdirectory actual images are stored in.
      image_suffix: the suffix of images.
    """
    expected_image_locator = _sanitize_locator(expected_image_locator)
    actual_image_locator = _sanitize_locator(actual_image_locator)

    # Download the expected/actual images, if we don't have them already.
    # TODO(rmistry): Add a parameter that just tries to use already-present
    # image files rather than downloading them.
    expected_image_file = os.path.join(
        storage_root, expected_images_subdir,
        str(expected_image_locator) + image_suffix)
    actual_image_file = os.path.join(
        storage_root, actual_images_subdir,
        str(actual_image_locator) + image_suffix)
    try:
      _download_file(expected_image_file, expected_image_url)
    except Exception:
      logging.exception('unable to download expected_image_url %s to file %s' %
                        (expected_image_url, expected_image_file))
      raise
    try:
      _download_file(actual_image_file, actual_image_url)
    except Exception:
      logging.exception('unable to download actual_image_url %s to file %s' %
                        (actual_image_url, actual_image_file))
      raise

    # Get all diff images and values from skpdiff binary.
    skpdiff_output_dir = tempfile.mkdtemp()
    try:
      skpdiff_summary_file = os.path.join(skpdiff_output_dir,
                                          'skpdiff-output.json')
      skpdiff_rgbdiff_dir = os.path.join(skpdiff_output_dir, 'rgbDiff')
      skpdiff_whitediff_dir = os.path.join(skpdiff_output_dir, 'whiteDiff')
      expected_img = os.path.join(storage_root, expected_images_subdir,
                                  str(expected_image_locator) + image_suffix)
      actual_img = os.path.join(storage_root, actual_images_subdir,
                                str(actual_image_locator) + image_suffix)

      # TODO: Call skpdiff ONCE for all image pairs, instead of calling it
      # repeatedly.  This will allow us to parallelize a lot more work.
      find_run_binary.run_command(
          [SKPDIFF_BINARY, '-p', expected_img, actual_img,
           '--jsonp', 'false',
           '--output', skpdiff_summary_file,
           '--differs', 'perceptual', 'different_pixels',
           '--rgbDiffDir', skpdiff_rgbdiff_dir,
           '--whiteDiffDir', skpdiff_whitediff_dir,
           ])

      # Get information out of the skpdiff_summary_file.
      with contextlib.closing(open(skpdiff_summary_file)) as fp:
        data = json.load(fp)

      # For now, we can assume there is only one record in the output summary,
      # since we passed skpdiff only one pair of images.
      record = data['records'][0]
      self._width = record['width']
      self._height = record['height']
      # TODO: make max_diff_per_channel a tuple instead of a list, because the
      # structure is meaningful (first element is red, second is green, etc.)
      # See http://stackoverflow.com/a/626871
      self._max_diff_per_channel = [
          record['maxRedDiff'], record['maxGreenDiff'], record['maxBlueDiff']]
      rgb_diff_path = record['rgbDiffPath']
      white_diff_path = record['whiteDiffPath']
      per_differ_stats = record['diffs']
      for stats in per_differ_stats:
        differ_name = stats['differName']
        if differ_name == 'different_pixels':
          self._num_pixels_differing = stats['pointsOfInterest']
        elif differ_name == 'perceptual':
          perceptual_similarity = stats['result']

      # skpdiff returns the perceptual similarity; convert it to get the
      # perceptual difference percentage.
      # skpdiff outputs -1 if the images are different sizes. Treat any
      # output that does not lie in [0, 1] as having 0% perceptual
      # similarity.
      if not 0 <= perceptual_similarity <= 1:
        perceptual_similarity = 0
      self._perceptual_difference = 100 - (perceptual_similarity * 100)

      # Store the rgbdiff and whitediff images generated above.
      diff_image_locator = _get_difference_locator(
          expected_image_locator=expected_image_locator,
          actual_image_locator=actual_image_locator)
      basename = str(diff_image_locator) + image_suffix
      _mkdir_unless_exists(os.path.join(storage_root, RGBDIFFS_SUBDIR))
      _mkdir_unless_exists(os.path.join(storage_root, WHITEDIFFS_SUBDIR))
      # TODO: Modify skpdiff's behavior so we can tell it exactly where to
      # write the image files into, rather than having to move them around
      # after skpdiff writes them out.
      shutil.copyfile(rgb_diff_path,
                      os.path.join(storage_root, RGBDIFFS_SUBDIR, basename))
      shutil.copyfile(white_diff_path,
                      os.path.join(storage_root, WHITEDIFFS_SUBDIR, basename))

    finally:
      shutil.rmtree(skpdiff_output_dir)

  # TODO(epoger): Use properties instead of getters throughout.
  # See http://stackoverflow.com/a/6618176
  def get_num_pixels_differing(self):
    """Returns the absolute number of pixels that differ."""
    return self._num_pixels_differing

  def get_percent_pixels_differing(self):
    """Returns the percentage of pixels that differ, as a float between
    0 and 100 (inclusive)."""
    return ((float(self._num_pixels_differing) * 100) /
            (self._width * self._height))

  def get_perceptual_difference(self):
    """Returns the perceptual difference percentage."""
    return self._perceptual_difference

  def get_max_diff_per_channel(self):
    """Returns the maximum difference between the expected and actual images
    for each R/G/B channel, as a list."""
    return self._max_diff_per_channel

  def as_dict(self):
    """Returns a dictionary representation of this DiffRecord, as needed when
    constructing the JSON representation."""
    return {
        KEY__DIFFERENCES__NUM_DIFF_PIXELS: self._num_pixels_differing,
        KEY__DIFFERENCES__PERCENT_DIFF_PIXELS:
            self.get_percent_pixels_differing(),
        KEY__DIFFERENCES__MAX_DIFF_PER_CHANNEL: self._max_diff_per_channel,
        KEY__DIFFERENCES__PERCEPTUAL_DIFF: self._perceptual_difference,
    }


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

  @property
  def storage_root(self):
    return self._storage_root

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
    expected_image_locator = _sanitize_locator(expected_image_locator)
    actual_image_locator = _sanitize_locator(actual_image_locator)
    key = (expected_image_locator, actual_image_locator)
    if not key in self._diff_dict:
      try:
        new_diff_record = DiffRecord(
            self._storage_root,
            expected_image_url=expected_image_url,
            expected_image_locator=expected_image_locator,
            actual_image_url=actual_image_url,
            actual_image_locator=actual_image_locator)
      except Exception:
        # If we can't create a real DiffRecord for this (expected, actual) pair,
        # store None and the UI will show whatever information we DO have.
        # Fixes http://skbug.com/2368 .
        logging.exception(
            'got exception while creating a DiffRecord for '
            'expected_image_url=%s , actual_image_url=%s; returning None' % (
                expected_image_url, actual_image_url))
        new_diff_record = None
      self._diff_dict[key] = new_diff_record

  def get_diff_record(self, expected_image_locator, actual_image_locator):
    """Returns the DiffRecord for this image pair.

    Raises a KeyError if we don't have a DiffRecord for this image pair.
    """
    key = (_sanitize_locator(expected_image_locator),
           _sanitize_locator(actual_image_locator))
    return self._diff_dict[key]


# Utility functions

def _download_file(local_filepath, url):
  """Download a file from url to local_filepath, unless it is already there.

  Args:
    local_filepath: path on local disk where the image should be stored
    url: URL from which we can download the image if we don't have it yet
  """
  if not os.path.exists(local_filepath):
    _mkdir_unless_exists(os.path.dirname(local_filepath))
    with contextlib.closing(urllib.urlopen(url)) as url_handle:
      with open(local_filepath, 'wb') as file_handle:
        shutil.copyfileobj(fsrc=url_handle, fdst=file_handle)


def _mkdir_unless_exists(path):
  """Unless path refers to an already-existing directory, create it.

  Args:
    path: path on local disk
  """
  if not os.path.isdir(path):
    os.makedirs(path)


def _sanitize_locator(locator):
  """Returns a sanitized version of a locator (one in which we know none of the
  characters will have special meaning in filenames).

  Args:
    locator: string, or something that can be represented as a string
  """
  return DISALLOWED_FILEPATH_CHAR_REGEX.sub('_', str(locator))


def _get_difference_locator(expected_image_locator, actual_image_locator):
  """Returns the locator string used to look up the diffs between expected_image
  and actual_image.

  We must keep this function in sync with getImageDiffRelativeUrl() in
  static/loader.js

  Args:
    expected_image_locator: locator string pointing at expected image
    actual_image_locator: locator string pointing at actual image

  Returns: already-sanitized locator where the diffs between expected and
      actual images can be found
  """
  return "%s-vs-%s" % (_sanitize_locator(expected_image_locator),
                       _sanitize_locator(actual_image_locator))
