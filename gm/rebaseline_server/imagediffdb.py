#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Calulate differences between image pairs, and store them in a database.
"""

import contextlib
import csv
import errno
import logging
import Queue
import os
import re
import shutil
import sys
import tempfile
import time
import threading
import urllib
try:
  from PIL import Image, ImageChops
except ImportError:
  raise ImportError('Requires PIL to be installed; see '
                    + 'http://www.pythonware.com/products/pil/')

# Set the PYTHONPATH to include the tools directory.
sys.path.append(
    os.path.join(
        os.path.dirname(os.path.realpath(__file__)), os.pardir, os.pardir,
                        'tools'))
import find_run_binary

SKPDIFF_BINARY = find_run_binary.find_path_to_program('skpdiff')

DEFAULT_IMAGE_SUFFIX = '.png'
DEFAULT_IMAGES_SUBDIR = 'images'
DEFAULT_NUM_WORKERS = 8

DISALLOWED_FILEPATH_CHAR_REGEX = re.compile('[^\w\-]')

DIFFS_SUBDIR = 'diffs'
WHITEDIFFS_SUBDIR = 'whitediffs'

VALUES_PER_BAND = 256

# Keys used within DiffRecord dictionary representations.
# NOTE: Keep these in sync with static/constants.js
KEY__DIFFERENCE_DATA__MAX_DIFF_PER_CHANNEL = 'maxDiffPerChannel'
KEY__DIFFERENCE_DATA__NUM_DIFF_PIXELS = 'numDifferingPixels'
KEY__DIFFERENCE_DATA__PERCENT_DIFF_PIXELS = 'percentDifferingPixels'
KEY__DIFFERENCE_DATA__PERCEPTUAL_DIFF = 'perceptualDifference'
KEY__DIFFERENCE_DATA__WEIGHTED_DIFF = 'weightedDiffMeasure'

# Special values within ImageDiffDB._diff_dict
DIFFRECORD_FAILED = 'failed'
DIFFRECORD_PENDING = 'pending'

# TODO(epoger): Temporary(?) list to keep track of how many times we download
# the same file in multiple threads.
global_file_collisions = 0


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
    # TODO(rmistry): Add a parameter that makes _download_and_open_image raise
    # an exception if images are not found locally (instead of trying to
    # download them).
    expected_image_file = os.path.join(
        storage_root, expected_images_subdir,
        str(expected_image_locator) + image_suffix)
    actual_image_file = os.path.join(
        storage_root, actual_images_subdir,
        str(actual_image_locator) + image_suffix)
    try:
      expected_image = _download_and_open_image(
          expected_image_file, expected_image_url)
    except Exception:
      logging.exception('unable to download expected_image_url %s to file %s' %
                        (expected_image_url, expected_image_file))
      raise
    try:
      actual_image = _download_and_open_image(
          actual_image_file, actual_image_url)
    except Exception:
      logging.exception('unable to download actual_image_url %s to file %s' %
                        (actual_image_url, actual_image_file))
      raise

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

    # Calculate the perceptual difference percentage.
    skpdiff_csv_dir = tempfile.mkdtemp()
    try:
      skpdiff_csv_output = os.path.join(skpdiff_csv_dir, 'skpdiff-output.csv')
      expected_img = os.path.join(storage_root, expected_images_subdir,
                                  str(expected_image_locator) + image_suffix)
      actual_img = os.path.join(storage_root, actual_images_subdir,
                                str(actual_image_locator) + image_suffix)
      find_run_binary.run_command(
          [SKPDIFF_BINARY, '-p', expected_img, actual_img,
           '--csv', skpdiff_csv_output, '-d', 'perceptual'])
      with contextlib.closing(open(skpdiff_csv_output)) as csv_file:
        for row in csv.DictReader(csv_file):
          perceptual_similarity = float(row[' perceptual'].strip())
          if not 0 <= perceptual_similarity <= 1:
            # skpdiff outputs -1 if the images are different sizes. Treat any
            # output that does not lie in [0, 1] as having 0% perceptual
            # similarity.
            perceptual_similarity = 0
          # skpdiff returns the perceptual similarity, convert it to get the
          # perceptual difference percentage.
          self._perceptual_difference = 100 - (perceptual_similarity * 100)
    finally:
      shutil.rmtree(skpdiff_csv_dir)

    # Final touches on diff_image: use whitediff_image as an alpha mask.
    # Unchanged pixels are transparent; differing pixels are opaque.
    diff_image.putalpha(whitediff_image)

    # Store the diff and whitediff images generated above.
    diff_image_locator = _get_difference_locator(
        expected_image_locator=expected_image_locator,
        actual_image_locator=actual_image_locator)
    basename = str(diff_image_locator) + image_suffix
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

  def get_perceptual_difference(self):
    """Returns the perceptual difference percentage."""
    return self._perceptual_difference

  def get_weighted_diff_measure(self):
    """Returns a weighted measure of image diffs, as a float between 0 and 100
    (inclusive).

    TODO(epoger): Delete this function, now that we have perceptual diff?
    """
    return self._weighted_diff_measure

  def get_max_diff_per_channel(self):
    """Returns the maximum difference between the expected and actual images
    for each R/G/B channel, as a list."""
    return self._max_diff_per_channel

  def as_dict(self):
    """Returns a dictionary representation of this DiffRecord, as needed when
    constructing the JSON representation."""
    return {
        KEY__DIFFERENCE_DATA__NUM_DIFF_PIXELS: self._num_pixels_differing,
        KEY__DIFFERENCE_DATA__PERCENT_DIFF_PIXELS:
            self.get_percent_pixels_differing(),
        KEY__DIFFERENCE_DATA__WEIGHTED_DIFF: self.get_weighted_diff_measure(),
        KEY__DIFFERENCE_DATA__MAX_DIFF_PER_CHANNEL: self._max_diff_per_channel,
        KEY__DIFFERENCE_DATA__PERCEPTUAL_DIFF: self._perceptual_difference,
    }


class ImageDiffDB(object):
  """ Calculates differences between image pairs, maintaining a database of
  them for download."""

  def __init__(self, storage_root, num_workers=DEFAULT_NUM_WORKERS):
    """
    Args:
      storage_root: string; root path within the DB will store all of its stuff
      num_workers: integer; number of worker threads to spawn
    """
    self._storage_root = storage_root

    # Dictionary of DiffRecords, keyed by (expected_image_locator,
    # actual_image_locator) tuples.
    # Values can also be DIFFRECORD_PENDING, DIFFRECORD_FAILED.
    self._diff_dict = {}

    # Set up the queue for asynchronously loading DiffRecords, and start the
    # worker threads reading from it.
    self._tasks_queue = Queue.Queue(maxsize=2*num_workers)
    self._workers = []
    for i in range(num_workers):
      worker = threading.Thread(target=self.worker, args=(i,))
      worker.daemon = True
      worker.start()
      self._workers.append(worker)

  def worker(self, worker_num):
    """Launch a worker thread that pulls tasks off self._tasks_queue.

    Args:
      worker_num: (integer) which worker this is
    """
    while True:
      params = self._tasks_queue.get()
      key, expected_image_url, actual_image_url = params
      try:
        diff_record = DiffRecord(
            self._storage_root,
            expected_image_url=expected_image_url,
            expected_image_locator=key[0],
            actual_image_url=actual_image_url,
            actual_image_locator=key[1])
      except Exception:
        logging.exception(
            'exception while creating DiffRecord for key %s' % str(key))
        diff_record = DIFFRECORD_FAILED
      self._diff_dict[key] = diff_record

  def add_image_pair(self,
                     expected_image_url, expected_image_locator,
                     actual_image_url, actual_image_locator):
    """Download this pair of images (unless we already have them on local disk),
    and prepare a DiffRecord for them.

    This method will block until the images are downloaded and DiffRecord is
    available by calling get_diff_record().

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

    Raises:
      Exception if we are unable to create a DiffRecord for this image pair.
    """
    key = _generate_key(expected_image_locator, actual_image_locator)
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

  def add_image_pair_async(self,
                           expected_image_url, expected_image_locator,
                           actual_image_url, actual_image_locator):
    """Download this pair of images (unless we already have them on local disk),
    and prepare a DiffRecord for them.

    This method will return quickly; calls to get_diff_record() will block
    until the DiffRecord is available (or we have given up on creating it).

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
    key = _generate_key(expected_image_locator, actual_image_locator)
    if not key in self._diff_dict:
      # If we have already requested a diff between these two images,
      # we don't need to request it again.
      #
      # Threading note: If multiple threads called into this method with the
      # same key at the same time, there will be multiple tasks on the queue
      # with the same key.  But that's OK; they will both complete successfully,
      # and just waste a little time in the process. Nothing will break.
      self._diff_dict[key] = DIFFRECORD_PENDING
      self._tasks_queue.put((key, expected_image_url, actual_image_url))

  def get_diff_record(self, expected_image_locator, actual_image_locator):
    """Returns the DiffRecord for this image pair.

    Args:
      expected_image_locator: a unique ID string under which we will store the
          expected image within storage_root (probably including a checksum to
          guarantee uniqueness)
      actual_image_locator: a unique ID string under which we will store the
          actual image within storage_root (probably including a checksum to
          guarantee uniqueness)

    Returns the DiffRecord for this image pair, or None if we were unable to
    generate one.
    """
    key = _generate_key(expected_image_locator, actual_image_locator)
    diff_record = self._diff_dict[key]

    # If we have no results yet, block until we do.
    while diff_record == DIFFRECORD_PENDING:
      time.sleep(1)
      diff_record = self._diff_dict[key]

    # Once we have the result...
    if diff_record == DIFFRECORD_FAILED:
      logging.error(
          'failed to create a DiffRecord for expected_image_locator=%s , '
          'actual_image_locator=%s' % (
              expected_image_locator, actual_image_locator))
      return None
    else:
      return diff_record


# Utility functions

def _calculate_weighted_diff_metric(histogram, num_pixels):
  """Given the histogram of a diff image (per-channel diff at each
  pixel between two images), calculate the weighted diff metric (a
  stab at how different the two images really are).

  TODO(epoger): Delete this function, now that we have perceptual diff?

  Args:
    histogram: PIL histogram of a per-channel diff between two images
    num_pixels: integer; the total number of pixels in the diff image

  Returns: a weighted diff metric, as a float between 0 and 100 (inclusive).
  """
  # TODO(epoger): As a wild guess at an appropriate metric, weight each
  # different pixel by the square of its delta value.  (The more different
  # a pixel is from its expectation, the more we care about it.)
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
  global global_file_collisions
  if not os.path.exists(local_filepath):
    _mkdir_unless_exists(os.path.dirname(local_filepath))
    with contextlib.closing(urllib.urlopen(url)) as url_handle:

      # First download the file contents into a unique filename, and
      # then rename that file.  That way, if multiple threads are downloading
      # the same filename at the same time, they won't interfere with each
      # other (they will both download the file, and one will "win" in the end)
      temp_filename = '%s-%d' % (local_filepath,
                                 threading.current_thread().ident)
      with open(temp_filename, 'wb') as file_handle:
        shutil.copyfileobj(fsrc=url_handle, fdst=file_handle)

      # Keep count of how many colliding downloads we encounter;
      # if it's a large number, we may want to change our download strategy
      # to minimize repeated downloads.
      if os.path.exists(local_filepath):
        global_file_collisions += 1
      else:
        os.rename(temp_filename, local_filepath)

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
    # If we are unable to load an image from the file, delete it from disk
    # and we will try to fetch it again next time.  Fixes http://skbug.com/2247
    logging.error('IOError loading image file %s ; deleting it.' % filepath)
    os.remove(filepath)
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
  try:
    os.makedirs(path)
  except OSError as e:
    if e.errno == errno.EEXIST:
      pass


def _sanitize_locator(locator):
  """Returns a sanitized version of a locator (one in which we know none of the
  characters will have special meaning in filenames).

  Args:
    locator: string, or something that can be represented as a string
  """
  return DISALLOWED_FILEPATH_CHAR_REGEX.sub('_', str(locator))


def _generate_key(expected_image_locator, actual_image_locator):
  """Returns a key suitable for looking up this image pair.

  Args:
    expected_image_locator: a unique ID string under which we will store the
        expected image within storage_root (probably including a checksum to
        guarantee uniqueness)
    actual_image_locator: a unique ID string under which we will store the
        actual image within storage_root (probably including a checksum to
        guarantee uniqueness)
  """
  return (_sanitize_locator(expected_image_locator),
          _sanitize_locator(actual_image_locator))


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
