#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Repackage expected/actual GM results as needed by our HTML rebaseline viewer.
"""

# System-level imports
import fnmatch
import os
import re
import sys

# Imports from within Skia
#
# We need to add the 'gm' directory, so that we can import gm_json.py within
# that directory.  That script allows us to parse the actual-results.json file
# written out by the GM tool.
# Make sure that the 'gm' dir is in the PYTHONPATH, but add it at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
PARENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
GM_DIRECTORY = os.path.dirname(PARENT_DIRECTORY)
if GM_DIRECTORY not in sys.path:
  sys.path.append(GM_DIRECTORY)
import gm_json
import imagepairset

# Keys used to link an image to a particular GM test.
# NOTE: Keep these in sync with static/constants.js
REBASELINE_SERVER_SCHEMA_VERSION_NUMBER = 2
KEY__EXPECTATIONS__BUGS = gm_json.JSONKEY_EXPECTEDRESULTS_BUGS
KEY__EXPECTATIONS__IGNOREFAILURE = gm_json.JSONKEY_EXPECTEDRESULTS_IGNOREFAILURE
KEY__EXPECTATIONS__REVIEWED = gm_json.JSONKEY_EXPECTEDRESULTS_REVIEWED
KEY__EXTRACOLUMN__BUILDER = 'builder'
KEY__EXTRACOLUMN__CONFIG = 'config'
KEY__EXTRACOLUMN__RESULT_TYPE = 'resultType'
KEY__EXTRACOLUMN__TEST = 'test'
KEY__HEADER = 'header'
KEY__HEADER__DATAHASH = 'dataHash'
KEY__HEADER__IS_EDITABLE = 'isEditable'
KEY__HEADER__IS_EXPORTED = 'isExported'
KEY__HEADER__IS_STILL_LOADING = 'resultsStillLoading'
KEY__HEADER__RESULTS_ALL = 'all'
KEY__HEADER__RESULTS_FAILURES = 'failures'
KEY__HEADER__SCHEMA_VERSION = 'schemaVersion'
KEY__HEADER__TIME_NEXT_UPDATE_AVAILABLE = 'timeNextUpdateAvailable'
KEY__HEADER__TIME_UPDATED = 'timeUpdated'
KEY__HEADER__TYPE = 'type'
KEY__NEW_IMAGE_URL = 'newImageUrl'
KEY__RESULT_TYPE__FAILED = gm_json.JSONKEY_ACTUALRESULTS_FAILED
KEY__RESULT_TYPE__FAILUREIGNORED = gm_json.JSONKEY_ACTUALRESULTS_FAILUREIGNORED
KEY__RESULT_TYPE__NOCOMPARISON = gm_json.JSONKEY_ACTUALRESULTS_NOCOMPARISON
KEY__RESULT_TYPE__SUCCEEDED = gm_json.JSONKEY_ACTUALRESULTS_SUCCEEDED

IMAGE_FILENAME_RE = re.compile(gm_json.IMAGE_FILENAME_PATTERN)
IMAGE_FILENAME_FORMATTER = '%s_%s.png'  # pass in (testname, config)

# Ignore expectations/actuals for builders matching any of these patterns.
# This allows us to ignore builders for which we don't maintain expectations
# (trybots, Valgrind, ASAN, TSAN), and avoid problems like
# https://code.google.com/p/skia/issues/detail?id=2036 ('rebaseline_server
# produces error when trying to add baselines for ASAN/TSAN builders')
SKIP_BUILDERS_PATTERN_LIST = [re.compile(p) for p in [
    '.*-Trybot', '.*Valgrind.*', '.*TSAN.*', '.*ASAN.*']]

DEFAULT_ACTUALS_DIR = '.gm-actuals'
DEFAULT_GENERATED_IMAGES_ROOT = os.path.join(
    PARENT_DIRECTORY, '.generated-images')


class BaseComparisons(object):
  """Base class for generating summary of comparisons between two image sets.
  """

  def __init__(self):
    raise NotImplementedError('cannot instantiate the abstract base class')

  def get_results_of_type(self, results_type):
    """Return results of some/all tests (depending on 'results_type' parameter).

    Args:
      results_type: string describing which types of results to include; must
          be one of the RESULTS_* constants

    Results are returned in a dictionary as output by ImagePairSet.as_dict().
    """
    return self._results[results_type]

  def get_packaged_results_of_type(self, results_type, reload_seconds=None,
                                   is_editable=False, is_exported=True):
    """Package the results of some/all tests as a complete response_dict.

    Args:
      results_type: string indicating which set of results to return;
          must be one of the RESULTS_* constants
      reload_seconds: if specified, note that new results may be available once
          these results are reload_seconds old
      is_editable: whether clients are allowed to submit new baselines
      is_exported: whether these results are being made available to other
          network hosts
    """
    response_dict = self._results[results_type]
    time_updated = self.get_timestamp()
    response_dict[KEY__HEADER] = {
        KEY__HEADER__SCHEMA_VERSION: (
            REBASELINE_SERVER_SCHEMA_VERSION_NUMBER),

        # Timestamps:
        # 1. when this data was last updated
        # 2. when the caller should check back for new data (if ever)
        KEY__HEADER__TIME_UPDATED: time_updated,
        KEY__HEADER__TIME_NEXT_UPDATE_AVAILABLE: (
            (time_updated+reload_seconds) if reload_seconds else None),

        # The type we passed to get_results_of_type()
        KEY__HEADER__TYPE: results_type,

        # Hash of dataset, which the client must return with any edits--
        # this ensures that the edits were made to a particular dataset.
        KEY__HEADER__DATAHASH: str(hash(repr(
            response_dict[imagepairset.KEY__IMAGEPAIRS]))),

        # Whether the server will accept edits back.
        KEY__HEADER__IS_EDITABLE: is_editable,

        # Whether the service is accessible from other hosts.
        KEY__HEADER__IS_EXPORTED: is_exported,
    }
    return response_dict

  def get_timestamp(self):
    """Return the time at which this object was created, in seconds past epoch
    (UTC).
    """
    return self._timestamp

  @staticmethod
  def _ignore_builder(builder):
    """Returns True if this builder matches any of SKIP_BUILDERS_PATTERN_LIST.

    Args:
      builder: name of this builder, as a string

    Returns:
      True if we should ignore expectations and actuals for this builder.
    """
    for pattern in SKIP_BUILDERS_PATTERN_LIST:
      if pattern.match(builder):
        return True
    return False

  @staticmethod
  def _read_dicts_from_root(root, pattern='*.json'):
    """Read all JSON dictionaries within a directory tree.

    Args:
      root: path to root of directory tree
      pattern: which files to read within root (fnmatch-style pattern)

    Returns:
      A meta-dictionary containing all the JSON dictionaries found within
      the directory tree, keyed by the builder name of each dictionary.

    Raises:
      IOError if root does not refer to an existing directory
    """
    if not os.path.isdir(root):
      raise IOError('no directory found at path %s' % root)
    meta_dict = {}
    for dirpath, dirnames, filenames in os.walk(root):
      for matching_filename in fnmatch.filter(filenames, pattern):
        builder = os.path.basename(dirpath)
        if BaseComparisons._ignore_builder(builder):
          continue
        fullpath = os.path.join(dirpath, matching_filename)
        meta_dict[builder] = gm_json.LoadFromFile(fullpath)
    return meta_dict

  @staticmethod
  def _create_relative_url(hashtype_and_digest, test_name):
    """Returns the URL for this image, relative to GM_ACTUALS_ROOT_HTTP_URL.

    If we don't have a record of this image, returns None.

    Args:
      hashtype_and_digest: (hash_type, hash_digest) tuple, or None if we
          don't have a record of this image
      test_name: string; name of the GM test that created this image
    """
    if not hashtype_and_digest:
      return None
    return gm_json.CreateGmRelativeUrl(
        test_name=test_name,
        hash_type=hashtype_and_digest[0],
        hash_digest=hashtype_and_digest[1])

  @staticmethod
  def combine_subdicts(input_dict):
    """ Flatten out a dictionary structure by one level.

    Input:
      {
        "failed" : {
          "changed.png" : [ "bitmap-64bitMD5", 8891695120562235492 ],
        },
        "no-comparison" : {
          "unchanged.png" : [ "bitmap-64bitMD5", 11092453015575919668 ],
        }
      }

    Output:
      {
        "changed.png" : [ "bitmap-64bitMD5", 8891695120562235492 ],
        "unchanged.png" : [ "bitmap-64bitMD5", 11092453015575919668 ],
      }

    If this would result in any repeated keys, it will raise an Exception.
    """
    output_dict = {}
    for key, subdict in input_dict.iteritems():
      for subdict_key, subdict_value in subdict.iteritems():
        if subdict_key in output_dict:
          raise Exception('duplicate key %s in combine_subdicts' % subdict_key)
        output_dict[subdict_key] = subdict_value
    return output_dict
