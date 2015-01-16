#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

ImagePairSet class; see its docstring below.
"""

# System-level imports
import posixpath

# Must fix up PYTHONPATH before importing from within Skia
import rs_fixpypath  # pylint: disable=W0611

# Imports from within Skia
import column
import imagediffdb
from py.utils import gs_utils

# Keys used within dictionary representation of ImagePairSet.
# NOTE: Keep these in sync with static/constants.js
KEY__ROOT__EXTRACOLUMNHEADERS = 'extraColumnHeaders'
KEY__ROOT__EXTRACOLUMNORDER = 'extraColumnOrder'
KEY__ROOT__HEADER = 'header'
KEY__ROOT__IMAGEPAIRS = 'imagePairs'
KEY__ROOT__IMAGESETS = 'imageSets'
KEY__IMAGESETS__FIELD__BASE_URL = 'baseUrl'
KEY__IMAGESETS__FIELD__DESCRIPTION = 'description'
KEY__IMAGESETS__SET__DIFFS = 'diffs'
KEY__IMAGESETS__SET__IMAGE_A = 'imageA'
KEY__IMAGESETS__SET__IMAGE_B = 'imageB'
KEY__IMAGESETS__SET__WHITEDIFFS = 'whiteDiffs'

DEFAULT_DESCRIPTIONS = ('setA', 'setB')


class ImagePairSet(object):
  """A collection of ImagePairs, representing two arbitrary sets of images.

  These could be:
  - images generated before and after a code patch
  - expected and actual images for some tests
  - or any other pairwise set of images.
  """

  def __init__(self, diff_base_url, descriptions=None):
    """
    Args:
      diff_base_url: base URL indicating where diff images can be loaded from
      descriptions: a (string, string) tuple describing the two image sets.
          If not specified, DEFAULT_DESCRIPTIONS will be used.
    """
    self._column_header_factories = {}
    self._descriptions = descriptions or DEFAULT_DESCRIPTIONS
    self._extra_column_tallies = {}  # maps column_id -> values
                                     #                -> instances_per_value
    self._imageA_base_url = None
    self._imageB_base_url = None
    self._diff_base_url = diff_base_url

    # We build self._image_pair_objects incrementally as calls come into
    # add_image_pair(); self._image_pair_dicts is filled in lazily (so that
    # we put off asking ImageDiffDB for results as long as possible).
    self._image_pair_objects = []
    self._image_pair_dicts = None

  def add_image_pair(self, image_pair):
    """Adds an ImagePair; this may be repeated any number of times."""
    # Special handling when we add the first ImagePair...
    if not self._image_pair_objects:
      self._imageA_base_url = image_pair.imageA_base_url
      self._imageB_base_url = image_pair.imageB_base_url

    if(image_pair.imageA_base_url != self._imageA_base_url):
      raise Exception('added ImagePair with base_url "%s" instead of "%s"' % (
          image_pair.imageA_base_url, self._imageA_base_url))
    if(image_pair.imageB_base_url != self._imageB_base_url):
      raise Exception('added ImagePair with base_url "%s" instead of "%s"' % (
          image_pair.imageB_base_url, self._imageB_base_url))
    self._image_pair_objects.append(image_pair)
    extra_columns_dict = image_pair.extra_columns_dict
    if extra_columns_dict:
      for column_id, value in extra_columns_dict.iteritems():
        self._add_extra_column_value_to_summary(column_id, value)

  def set_column_header_factory(self, column_id, column_header_factory):
    """Overrides the default settings for one of the extraColumn headers.

    Args:
      column_id: string; unique ID of this column (must match a key within
          an ImagePair's extra_columns dictionary)
      column_header_factory: a ColumnHeaderFactory object
    """
    self._column_header_factories[column_id] = column_header_factory

  def get_column_header_factory(self, column_id):
    """Returns the ColumnHeaderFactory object for a particular extraColumn.

    Args:
      column_id: string; unique ID of this column (must match a key within
          an ImagePair's extra_columns dictionary)
    """
    column_header_factory = self._column_header_factories.get(column_id, None)
    if not column_header_factory:
      column_header_factory = column.ColumnHeaderFactory(header_text=column_id)
      self._column_header_factories[column_id] = column_header_factory
    return column_header_factory

  def ensure_extra_column_values_in_summary(self, column_id, values):
    """Ensure this column_id/value pair is part of the extraColumns summary.

    Args:
      column_id: string; unique ID of this column
      value: string; a possible value for this column
    """
    for value in values:
      self._add_extra_column_value_to_summary(
          column_id=column_id, value=value, addend=0)

  def _add_extra_column_value_to_summary(self, column_id, value, addend=1):
    """Records one column_id/value extraColumns pair found within an ImagePair.

    We use this information to generate tallies within the column header
    (how many instances we saw of a particular value, within a particular
    extraColumn).

    Args:
      column_id: string; unique ID of this column (must match a key within
          an ImagePair's extra_columns dictionary)
      value: string; a possible value for this column
      addend: integer; how many instances to add to the tally
    """
    known_values_for_column = self._extra_column_tallies.get(column_id, None)
    if not known_values_for_column:
      known_values_for_column = {}
      self._extra_column_tallies[column_id] = known_values_for_column
    instances_of_this_value = known_values_for_column.get(value, 0)
    instances_of_this_value += addend
    known_values_for_column[value] = instances_of_this_value

  def _column_headers_as_dict(self):
    """Returns all column headers as a dictionary."""
    asdict = {}
    for column_id, values_for_column in self._extra_column_tallies.iteritems():
      column_header_factory = self.get_column_header_factory(column_id)
      asdict[column_id] = column_header_factory.create_as_dict(
          values_for_column)
    return asdict

  def as_dict(self, column_ids_in_order=None):
    """Returns a dictionary describing this package of ImagePairs.

    Uses the KEY__* constants as keys.

    Args:
      column_ids_in_order: A list of all extracolumn IDs in the desired display
          order.  If unspecified, they will be displayed in alphabetical order.
          If specified, this list must contain all the extracolumn IDs!
          (It may contain extra column IDs; they will be ignored.)
    """
    all_column_ids = set(self._extra_column_tallies.keys())
    if column_ids_in_order == None:
      column_ids_in_order = sorted(all_column_ids)
    else:
      # Make sure the caller listed all column IDs, and throw away any extras.
      specified_column_ids = set(column_ids_in_order)
      forgotten_column_ids = all_column_ids - specified_column_ids
      assert not forgotten_column_ids, (
          'column_ids_in_order %s missing these column_ids: %s' % (
              column_ids_in_order, forgotten_column_ids))
      column_ids_in_order = [c for c in column_ids_in_order
                             if c in all_column_ids]

    key_description = KEY__IMAGESETS__FIELD__DESCRIPTION
    key_base_url = KEY__IMAGESETS__FIELD__BASE_URL
    if gs_utils.GSUtils.is_gs_url(self._imageA_base_url):
      valueA_base_url = self._convert_gs_url_to_http_url(self._imageA_base_url)
    else:
      valueA_base_url = self._imageA_base_url
    if gs_utils.GSUtils.is_gs_url(self._imageB_base_url):
      valueB_base_url = self._convert_gs_url_to_http_url(self._imageB_base_url)
    else:
      valueB_base_url = self._imageB_base_url

    # We've waited as long as we can to ask ImageDiffDB for details of the
    # image diffs, so that it has time to compute them.
    if self._image_pair_dicts == None:
      self._image_pair_dicts = [ip.as_dict() for ip in self._image_pair_objects]

    return {
        KEY__ROOT__EXTRACOLUMNHEADERS: self._column_headers_as_dict(),
        KEY__ROOT__EXTRACOLUMNORDER: column_ids_in_order,
        KEY__ROOT__IMAGEPAIRS: self._image_pair_dicts,
        KEY__ROOT__IMAGESETS: {
            KEY__IMAGESETS__SET__IMAGE_A: {
                key_description: self._descriptions[0],
                key_base_url: valueA_base_url,
            },
            KEY__IMAGESETS__SET__IMAGE_B: {
                key_description: self._descriptions[1],
                key_base_url: valueB_base_url,
            },
            KEY__IMAGESETS__SET__DIFFS: {
                key_description: 'color difference per channel',
                key_base_url: posixpath.join(
                    self._diff_base_url, imagediffdb.RGBDIFFS_SUBDIR),
            },
            KEY__IMAGESETS__SET__WHITEDIFFS: {
                key_description: 'differing pixels in white',
                key_base_url: posixpath.join(
                    self._diff_base_url, imagediffdb.WHITEDIFFS_SUBDIR),
            },
        },
    }

  @staticmethod
  def _convert_gs_url_to_http_url(gs_url):
    """Returns HTTP URL that can be used to download this Google Storage file.

    TODO(epoger): Create functionality like this within gs_utils.py instead of
    here?  See https://codereview.chromium.org/428493005/ ('create
    anyfile_utils.py for copying files between HTTP/GS/local filesystem')

    Args:
      gs_url: "gs://bucket/path" format URL
    """
    bucket, path = gs_utils.GSUtils.split_gs_url(gs_url)
    http_url = 'http://storage.cloud.google.com/' + bucket
    if path:
      http_url += '/' + path
    return http_url
