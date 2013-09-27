#!/usr/bin/python

'''
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

'''
Repackage expected/actual GM results as needed by our HTML rebaseline viewer.
'''

# System-level imports
import fnmatch
import json
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
GM_DIRECTORY = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
if GM_DIRECTORY not in sys.path:
  sys.path.append(GM_DIRECTORY)
import gm_json

IMAGE_FILENAME_RE = re.compile(gm_json.IMAGE_FILENAME_PATTERN)

class Results(object):
  """ Loads actual and expected results from all builders, supplying combined
  reports as requested. """

  def __init__(self, actuals_root, expected_root):
    """
    params:
      actuals_root: root directory containing all actual-results.json files
      expected_root: root directory containing all expected-results.json files
    """
    self._actual_builder_dicts = Results._GetDictsFromRoot(actuals_root)
    self._expected_builder_dicts = Results._GetDictsFromRoot(expected_root)
    self._all_results = self._Combine()

  def GetAll(self):
    """Return results of all tests, as a list in this form:

       [
         {
           "builder": "Test-Mac10.6-MacMini4.1-GeForce320M-x86-Debug",
           "test": "bigmatrix",
           "config": "8888",
           "resultType": "failed",
           "expectedHashType": "bitmap-64bitMD5",
           "expectedHashDigest": "10894408024079689926",
           "actualHashType": "bitmap-64bitMD5",
           "actualHashDigest": "2409857384569",
         },
         ...
       ]
    """
    return self._all_results

  @staticmethod
  def _GetDictsFromRoot(root, pattern='*.json'):
    """Read all JSON dictionaries within a directory tree, returning them within
    a meta-dictionary (keyed by the builder name for each dictionary).

    params:
      root: path to root of directory tree
      pattern: which files to read within root (fnmatch-style pattern)
    """
    meta_dict = {}
    for dirpath, dirnames, filenames in os.walk(root):
      for matching_filename in fnmatch.filter(filenames, pattern):
        builder = os.path.basename(dirpath)
        if builder.endswith('-Trybot'):
          continue
        fullpath = os.path.join(dirpath, matching_filename)
        meta_dict[builder] = gm_json.LoadFromFile(fullpath)
    return meta_dict

  def _Combine(self):
    """Returns a list of all tests, across all builders, based on the
    contents of self._actual_builder_dicts and self._expected_builder_dicts .
    Returns the list in the same form needed for GetAllResults().
    """
    all_tests = []
    for builder in sorted(self._actual_builder_dicts.keys()):
      actual_results_for_this_builder = (
          self._actual_builder_dicts[builder][gm_json.JSONKEY_ACTUALRESULTS])
      for result_type in sorted(actual_results_for_this_builder.keys()):
        results_of_this_type = actual_results_for_this_builder[result_type]
        if not results_of_this_type:
          continue
        for image_name in sorted(results_of_this_type.keys()):
          actual_image = results_of_this_type[image_name]
          try:
            # TODO(epoger): assumes a single allowed digest per test
            expected_image = (
                self._expected_builder_dicts
                    [builder][gm_json.JSONKEY_EXPECTEDRESULTS]
                    [image_name][gm_json.JSONKEY_EXPECTEDRESULTS_ALLOWEDDIGESTS]
                    [0])
          except (KeyError, TypeError):
            # There are several cases in which we would expect to find
            # no expectations for a given test:
            #
            # 1. result_type == NOCOMPARISON
            #   There are no expectations for this test yet!
            #
            # 2. ignore-tests.txt
            #   If a test has been listed in ignore-tests.txt, then its status
            #   may show as FAILUREIGNORED even if it doesn't have any
            #   expectations yet.
            #
            # 3. alternate rendering mode failures (e.g. serialized)
            #   In cases like
            #   https://code.google.com/p/skia/issues/detail?id=1684
            #   ('tileimagefilter GM test failing in serialized render mode'),
            #   the gm-actuals will list a failure for the alternate
            #   rendering mode even though we don't have explicit expectations
            #   for the test (the implicit expectation is that it must
            #   render the same in all rendering modes).
            #
            # Don't log types 1 or 2, because they are common.
            # Log other types, because they are rare and we should know about
            # them, but don't throw an exception, because we need to keep our
            # tools working in the meanwhile!
            if result_type not in [
                gm_json.JSONKEY_ACTUALRESULTS_NOCOMPARISON,
                gm_json.JSONKEY_ACTUALRESULTS_FAILUREIGNORED] :
              print 'WARNING: No expectations found for test: %s' % {
                  'builder': builder,
                  'image_name': image_name,
                  'result_type': result_type,
                  }
            expected_image = [None, None]

          # If this test was recently rebaselined, it will remain in
          # the "failed" set of actuals until all the bots have
          # cycled (although the expectations have indeed been set
          # from the most recent actuals).  Treat these as successes
          # instead of failures.
          #
          # TODO(epoger): Do we need to do something similar in
          # other cases, such as when we have recently marked a test
          # as ignoreFailure but it still shows up in the "failed"
          # category?  Maybe we should not rely on the result_type
          # categories recorded within the gm_actuals AT ALL, and
          # instead evaluate the result_type ourselves based on what
          # we see in expectations vs actual checksum?
          if expected_image == actual_image:
            updated_result_type = gm_json.JSONKEY_ACTUALRESULTS_SUCCEEDED
          else:
            updated_result_type = result_type

          # TODO(epoger): For now, don't include succeeded results.
          # There are so many of them that they make the client too slow.
          if updated_result_type == gm_json.JSONKEY_ACTUALRESULTS_SUCCEEDED:
            continue

          (test, config) = IMAGE_FILENAME_RE.match(image_name).groups()
          all_tests.append({
              "builder": builder,
              "test": test,
              "config": config,
              "resultType": updated_result_type,
              "actualHashType": actual_image[0],
              "actualHashDigest": str(actual_image[1]),
              "expectedHashType": expected_image[0],
              "expectedHashDigest": str(expected_image[1]),
          })
    return all_tests
