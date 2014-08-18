#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Calulate differences between image pairs, and store them in a database.
"""

# System-level imports
import contextlib
import errno
import json
import logging
import os
import Queue
import re
import shutil
import tempfile
import threading
import time
import urllib

# Must fix up PYTHONPATH before importing from within Skia
import rs_fixpypath  # pylint: disable=W0611

# Imports from within Skia
import find_run_binary
from py.utils import gs_utils


SKPDIFF_BINARY = find_run_binary.find_path_to_program('skpdiff')

DEFAULT_IMAGE_SUFFIX = '.png'
DEFAULT_IMAGES_SUBDIR = 'images'
# TODO(epoger): Figure out a better default number of threads; for now,
# using a conservative default value.
DEFAULT_NUM_WORKER_THREADS = 1

DISALLOWED_FILEPATH_CHAR_REGEX = re.compile('[^\w\-]')

RGBDIFFS_SUBDIR = 'diffs'
WHITEDIFFS_SUBDIR = 'whitediffs'

# Keys used within DiffRecord dictionary representations.
# NOTE: Keep these in sync with static/constants.js
KEY__DIFFERENCES__MAX_DIFF_PER_CHANNEL = 'maxDiffPerChannel'
KEY__DIFFERENCES__NUM_DIFF_PIXELS = 'numDifferingPixels'
KEY__DIFFERENCES__PERCENT_DIFF_PIXELS = 'percentDifferingPixels'
KEY__DIFFERENCES__PERCEPTUAL_DIFF = 'perceptualDifference'
KEY__DIFFERENCES__DIFF_URL = 'diffUrl'
KEY__DIFFERENCES__WHITE_DIFF_URL = 'whiteDiffUrl'

# Special values within ImageDiffDB._diff_dict
_DIFFRECORD_FAILED = 'failed'
_DIFFRECORD_PENDING = 'pending'

# How often to report tasks_queue size
QUEUE_LOGGING_GRANULARITY = 1000

# Temporary variable to keep track of how many times we download
# the same file in multiple threads.
# TODO(epoger): Delete this, once we see that the number stays close to 0.
global_file_collisions = 0


class DiffRecord(object):
  """ Record of differences between two images. """

  def __init__(self, gs, storage_root,
               expected_image_url, expected_image_locator,
               actual_image_url, actual_image_locator,
               expected_images_subdir=DEFAULT_IMAGES_SUBDIR,
               actual_images_subdir=DEFAULT_IMAGES_SUBDIR,
               image_suffix=DEFAULT_IMAGE_SUFFIX):
    """Download this pair of images (unless we already have them on local disk),
    and prepare a DiffRecord for them.

    Args:
      gs: instance of GSUtils object we can use to download images
      storage_root: root directory on local disk within which we store all
          images
      expected_image_url: file, GS, or HTTP url from which we will download the
          expected image
      expected_image_locator: a unique ID string under which we will store the
          expected image within storage_root (probably including a checksum to
          guarantee uniqueness)
      actual_image_url: file, GS, or HTTP url from which we will download the
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
    expected_image_file = os.path.join(
        storage_root, expected_images_subdir,
        str(expected_image_locator) + image_suffix)
    actual_image_file = os.path.join(
        storage_root, actual_images_subdir,
        str(actual_image_locator) + image_suffix)
    for image_file, image_url in [
        (expected_image_file, expected_image_url),
        (actual_image_file, actual_image_url)]:
      if image_file and image_url:
        try:
          _download_file(gs, image_file, image_url)
        except Exception:
          logging.exception('unable to download image_url %s to file %s' %
                            (image_url, image_file))
          raise

    # Return early if we do not need to generate diffs.
    if (expected_image_url == actual_image_url or
        not expected_image_url or not actual_image_url):
      return

    # Get all diff images and values using the skpdiff binary.
    skpdiff_output_dir = tempfile.mkdtemp()
    try:
      skpdiff_summary_file = os.path.join(skpdiff_output_dir,
                                          'skpdiff-output.json')
      skpdiff_rgbdiff_dir = os.path.join(storage_root, RGBDIFFS_SUBDIR)
      skpdiff_whitediff_dir = os.path.join(storage_root, WHITEDIFFS_SUBDIR)
      _mkdir_unless_exists(skpdiff_rgbdiff_dir)
      _mkdir_unless_exists(skpdiff_rgbdiff_dir)

      # TODO(epoger): Consider calling skpdiff ONCE for all image pairs,
      # instead of calling it separately for each image pair.
      # Pro: we'll incur less overhead from making repeated system calls,
      # spinning up the skpdiff binary, etc.
      # Con: we would have to wait until all image pairs were loaded before
      # generating any of the diffs?
      # Note(stephana): '--longnames' was added to allow for this 
      # case (multiple files at once) versus specifying output diffs 
      # directly.
      find_run_binary.run_command(
          [SKPDIFF_BINARY, '-p', expected_image_file, actual_image_file,
           '--jsonp', 'false',
           '--longnames', 'true',
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
      self._diffUrl = os.path.split(record['rgbDiffPath'])[1]
      self._whiteDiffUrl = os.path.split(record['whiteDiffPath'])[1]

      # TODO: make max_diff_per_channel a tuple instead of a list, because the
      # structure is meaningful (first element is red, second is green, etc.)
      # See http://stackoverflow.com/a/626871
      self._max_diff_per_channel = [
          record['maxRedDiff'], record['maxGreenDiff'], record['maxBlueDiff']]
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
        KEY__DIFFERENCES__DIFF_URL: self._diffUrl,
        KEY__DIFFERENCES__WHITE_DIFF_URL: self._whiteDiffUrl, 
    }



class ImageDiffDB(object):
  """ Calculates differences between image pairs, maintaining a database of
  them for download."""

  def __init__(self, storage_root, gs=None,
               num_worker_threads=DEFAULT_NUM_WORKER_THREADS):
    """
    Args:
      storage_root: string; root path within the DB will store all of its stuff
      gs: instance of GSUtils object we can use to download images
      num_worker_threads: how many threads that download images and
          generate diffs simultaneously
    """
    self._storage_root = storage_root
    self._gs = gs

    # Mechanism for reporting queue size periodically.
    self._last_queue_size_reported = None
    self._queue_size_report_lock = threading.RLock()

    # Dictionary of DiffRecords, keyed by (expected_image_locator,
    # actual_image_locator) tuples.
    # Values can also be _DIFFRECORD_PENDING, _DIFFRECORD_FAILED.
    #
    # Any thread that modifies _diff_dict must first acquire
    # _diff_dict_writelock!
    #
    # TODO(epoger): Disk is limitless, but RAM is not... so, we should probably
    # remove items from self._diff_dict if they haven't been accessed for a
    # long time.  We can always regenerate them by diffing the images we
    # previously downloaded to local disk.
    # I guess we should figure out how expensive it is to download vs diff the
    # image pairs... if diffing them is expensive too, we can write these
    # _diff_dict objects out to disk if there's too many to hold in RAM.
    # Or we could use virtual memory to handle that automatically.
    self._diff_dict = {}
    self._diff_dict_writelock = threading.RLock()

    # Set up the queue for asynchronously loading DiffRecords, and start the
    # worker threads reading from it.
    # The queue maxsize must be 0 (infinite size queue), so that asynchronous
    # calls can return as soon as possible.
    self._tasks_queue = Queue.Queue(maxsize=0)
    self._workers = []
    for i in range(num_worker_threads):
      worker = threading.Thread(target=self.worker, args=(i,))
      worker.daemon = True
      worker.start()
      self._workers.append(worker)

  def log_queue_size_if_changed(self, limit_verbosity=True):
    """Log the size of self._tasks_queue, if it has changed since the last call.

    Reports the current queue size, using log.info(), unless the queue is the
    same size as the last time we reported it.

    Args:
      limit_verbosity: if True, only log if the queue size is a multiple of
          QUEUE_LOGGING_GRANULARITY
    """
    # Acquire the lock, to synchronize access to self._last_queue_size_reported
    self._queue_size_report_lock.acquire()
    try:
      size = self._tasks_queue.qsize()
      if size == self._last_queue_size_reported:
        return
      if limit_verbosity and (size % QUEUE_LOGGING_GRANULARITY != 0):
        return
      logging.info('tasks_queue size is %d' % size)
      self._last_queue_size_reported = size
    finally:
      self._queue_size_report_lock.release()

  def worker(self, worker_num):
    """Launch a worker thread that pulls tasks off self._tasks_queue.

    Args:
      worker_num: (integer) which worker this is
    """
    while True:
      self.log_queue_size_if_changed()
      params = self._tasks_queue.get()
      key, expected_image_url, actual_image_url = params
      try:
        diff_record = DiffRecord(
            self._gs, self._storage_root,
            expected_image_url=expected_image_url,
            expected_image_locator=key[0],
            actual_image_url=actual_image_url,
            actual_image_locator=key[1])
      except Exception:
        logging.exception(
            'exception while creating DiffRecord for key %s' % str(key))
        diff_record = _DIFFRECORD_FAILED
      self._diff_dict_writelock.acquire()
      try:
        self._diff_dict[key] = diff_record
      finally:
        self._diff_dict_writelock.release()

  @property
  def storage_root(self):
    return self._storage_root

  def add_image_pair(self,
                     expected_image_url, expected_image_locator,
                     actual_image_url, actual_image_locator):
    """Asynchronously prepare a DiffRecord for a pair of images.

    This method will return quickly; calls to get_diff_record() will block
    until the DiffRecord is available (or we have given up on creating it).

    If we already have a DiffRecord for this particular image pair, no work
    will be done.

    If expected_image_url (or its locator) is None, just download actual_image.
    If actual_image_url (or its locator) is None, just download expected_image.

    Args:
      expected_image_url: file, GS, or HTTP url from which we will download the
          expected image
      expected_image_locator: a unique ID string under which we will store the
          expected image within storage_root (probably including a checksum to
          guarantee uniqueness)
      actual_image_url: file, GS, or HTTP url from which we will download the
          actual image
      actual_image_locator: a unique ID string under which we will store the
          actual image within storage_root (probably including a checksum to
          guarantee uniqueness)
    """
    expected_image_locator = _sanitize_locator(expected_image_locator)
    actual_image_locator = _sanitize_locator(actual_image_locator)
    key = (expected_image_locator, actual_image_locator)
    must_add_to_queue = False

    self._diff_dict_writelock.acquire()
    try:
      if not key in self._diff_dict:
        # If we have already requested a diff between these two images,
        # we don't need to request it again.
        must_add_to_queue = True
        self._diff_dict[key] = _DIFFRECORD_PENDING
    finally:
      self._diff_dict_writelock.release()

    if must_add_to_queue:
      self._tasks_queue.put((key, expected_image_url, actual_image_url))
      self.log_queue_size_if_changed()

  def get_diff_record(self, expected_image_locator, actual_image_locator):
    """Returns the DiffRecord for this image pair.

    This call will block until the diff record is available, or we were unable
    to generate it.

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
    key = (_sanitize_locator(expected_image_locator),
           _sanitize_locator(actual_image_locator))
    diff_record = self._diff_dict[key]

    # If we have no results yet, block until we do.
    while diff_record == _DIFFRECORD_PENDING:
      time.sleep(1)
      diff_record = self._diff_dict[key]

    # Once we have the result...
    if diff_record == _DIFFRECORD_FAILED:
      logging.error(
          'failed to create a DiffRecord for expected_image_locator=%s , '
          'actual_image_locator=%s' % (
              expected_image_locator, actual_image_locator))
      return None
    else:
      return diff_record


# Utility functions

def _download_file(gs, local_filepath, url):
  """Download a file from url to local_filepath, unless it is already there.

  Args:
    gs: instance of GSUtils object, in case the url points at Google Storage
    local_filepath: path on local disk where the image should be stored
    url: HTTP or GS URL from which we can download the image if we don't have
        it yet
  """
  global global_file_collisions
  if not os.path.exists(local_filepath):
    _mkdir_unless_exists(os.path.dirname(local_filepath))

    # First download the file contents into a unique filename, and
    # then rename that file.  That way, if multiple threads are downloading
    # the same filename at the same time, they won't interfere with each
    # other (they will both download the file, and one will "win" in the end)
    temp_filename = '%s-%d' % (local_filepath,
                               threading.current_thread().ident)
    if gs_utils.GSUtils.is_gs_url(url):
      (bucket, path) = gs_utils.GSUtils.split_gs_url(url)
      gs.download_file(source_bucket=bucket, source_path=path,
                       dest_path=temp_filename)
    else:
      with contextlib.closing(urllib.urlopen(url)) as url_handle:
        with open(temp_filename, 'wb') as file_handle:
          shutil.copyfileobj(fsrc=url_handle, fdst=file_handle)

    # Rename the file to its real filename.
    # Keep count of how many colliding downloads we encounter;
    # if it's a large number, we may want to change our download strategy
    # to minimize repeated downloads.
    if os.path.exists(local_filepath):
      global_file_collisions += 1
    else:
      os.rename(temp_filename, local_filepath)


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
    locator: string, or something that can be represented as a string.
        If None or '', it is returned without modification, because empty
        locators have a particular meaning ("there is no image for this")
  """
  if locator:
    return DISALLOWED_FILEPATH_CHAR_REGEX.sub('_', str(locator))
  else:
    return locator
