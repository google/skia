#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Utility to display a summary of JSON-format GM results, and exit with
a nonzero errorcode if there were non-ignored failures in the GM results.

Usage:
  python display_json_results.py <filename>

TODO(epoger): We may want to add flags to set the following:
- which error types cause a nonzero return code
- maximum number of tests to list for any one ResultAccumulator
  (to keep the output reasonably short)
"""

__author__ = 'Elliot Poger'


# system-level imports
import sys

# local imports
import gm_json


class ResultAccumulator(object):
  """Object that accumulates results of a given type, and can generate a
     summary upon request."""

  def __init__(self, name, do_list, do_fail):
    """name: name of the category this result type falls into
       do_list: whether to list all of the tests with this results type
       do_fail: whether to return with nonzero exit code if there are any
                results of this type
    """
    self._name = name
    self._do_list = do_list
    self._do_fail = do_fail
    self._testnames = []

  def AddResult(self, testname):
    """Adds a result of this particular type.
       testname: (string) name of the test"""
    self._testnames.append(testname)

  def ShouldSignalFailure(self):
    """Returns true if this result type is serious (self._do_fail is True)
       and there were any results of this type."""
    if self._do_fail and self._testnames:
      return True
    else:
      return False

  def GetSummaryLine(self):
    """Returns a single-line string summary of all results added to this
       accumulator so far."""
    summary = ''
    if self._do_fail:
      summary += '[*] '
    else:
      summary += '[ ] '
    summary += str(len(self._testnames))
    summary += ' '
    summary += self._name
    if self._do_list:
      summary += ': '
      for testname in self._testnames:
        summary += testname
        summary += ' '
    return summary


def Display(filepath):
  """Displays a summary of the results in a JSON file.
     Returns True if the results are free of any significant failures.
     filepath: (string) path to JSON file"""

  # Map labels within the JSON file to the ResultAccumulator for each label.
  results_map = {
    gm_json.JSONKEY_ACTUALRESULTS_FAILED:
        ResultAccumulator(name='ExpectationsMismatch',
                          do_list=True, do_fail=True),
    gm_json.JSONKEY_ACTUALRESULTS_FAILUREIGNORED:
        ResultAccumulator(name='IgnoredExpectationsMismatch',
                          do_list=True, do_fail=False),
    gm_json.JSONKEY_ACTUALRESULTS_NOCOMPARISON:
        ResultAccumulator(name='MissingExpectations',
                          do_list=False, do_fail=False),
    gm_json.JSONKEY_ACTUALRESULTS_SUCCEEDED:
        ResultAccumulator(name='Passed',
                          do_list=False, do_fail=False),
  }

  success = True
  json_dict = gm_json.LoadFromFile(filepath)
  actual_results = json_dict[gm_json.JSONKEY_ACTUALRESULTS]
  for label, accumulator in results_map.iteritems():
    results = actual_results[label]
    if results:
      for result in results:
        accumulator.AddResult(result)
    print accumulator.GetSummaryLine()
    if accumulator.ShouldSignalFailure():
      success = False
  print '(results marked with [*] will cause nonzero return value)'
  return success


if '__main__' == __name__:
  if len(sys.argv) != 2:
    raise Exception('usage: %s <input-json-filepath>' % sys.argv[0])
  sys.exit(0 if Display(sys.argv[1]) else 1)
