#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Utility to confirm that a JSON summary written by GM contains no failures.

Usage:
  python confirm_no_failures_in_json.py <filename>
"""

__author__ = 'Elliot Poger'


import json
import sys


# These constants must be kept in sync with the kJsonKey_ constants in
# gm_expectations.cpp !
JSONKEY_ACTUALRESULTS = 'actual-results'
JSONKEY_ACTUALRESULTS_FAILED = 'failed'

# This is the same indent level as used by jsoncpp, just for consistency.
JSON_INDENTLEVEL = 3


def Assert(filepath):
  """Raises an exception if the JSON summary at filepath contains any failed
  tests, or if we were unable to read the JSON summary."""
  failed_tests = GetFailedTests(filepath)
  if failed_tests:
    raise Exception('JSON file %s contained these test failures...\n%s' % (
        filepath, json.dumps(failed_tests, indent=JSON_INDENTLEVEL)))


def GetFailedTests(filepath):
  """Returns the dictionary of failed tests from the JSON file at filepath."""
  json_dict = json.load(open(filepath))
  actual_results = json_dict[JSONKEY_ACTUALRESULTS]
  return actual_results[JSONKEY_ACTUALRESULTS_FAILED]


if '__main__' == __name__:
  if len(sys.argv) != 2:
    raise Exception('usage: %s <input-json-filepath>' % sys.argv[0])
  Assert(sys.argv[1])
