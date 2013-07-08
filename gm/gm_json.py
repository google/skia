#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Schema of the JSON summary file written out by the GM tool.

This must be kept in sync with the kJsonKey_ constants in gm_expectations.cpp !
"""

__author__ = 'Elliot Poger'


# system-level imports
import json


# Key strings used in GM results JSON files (both expected-results.json and
# actual-results.json).
#
# These constants must be kept in sync with the kJsonKey_ constants in
# gm_expectations.cpp !

JSONKEY_ACTUALRESULTS = 'actual-results'
# Tests whose results failed to match expectations.
JSONKEY_ACTUALRESULTS_FAILED = 'failed'
# Tests whose results failed to match expectations, but IGNOREFAILURE causes
# us to take them less seriously.
JSONKEY_ACTUALRESULTS_FAILUREIGNORED = 'failure-ignored'
# Tests for which we do not have any expectations.  They may be new tests that
# we haven't had a chance to check in expectations for yet, or we may have
# consciously decided to leave them without expectations because we are unhappy
# with the results (although we should try to move away from that, and instead
# check in expectations with the IGNOREFAILURE flag set).
JSONKEY_ACTUALRESULTS_NOCOMPARISON = 'no-comparison'
# Tests whose results matched their expectations.
JSONKEY_ACTUALRESULTS_SUCCEEDED = 'succeeded'

JSONKEY_EXPECTEDRESULTS = 'expected-results'
# One or more [HashType/DigestValue] pairs representing valid results for this
# test.  Typically, there will just be one pair, but we allow for multiple
# expectations, and the test will pass if any one of them is matched.
JSONKEY_EXPECTEDRESULTS_ALLOWEDDIGESTS = 'allowed-digests'
# If IGNOREFAILURE is set to True, a failure of this test will be reported
# within the FAILUREIGNORED section (thus NOT causing the buildbots to go red)
# rather than the FAILED section (which WOULD cause the buildbots to go red).
JSONKEY_EXPECTEDRESULTS_IGNOREFAILURE = 'ignore-failure'

# Allowed hash types for test expectations.
JSONKEY_HASHTYPE_BITMAP_64BITMD5 = 'bitmap-64bitMD5'

def LoadFromString(file_contents):
  """Loads the JSON summary written out by the GM tool.
     Returns a dictionary keyed by the values listed as JSONKEY_ constants
     above."""
  # TODO(epoger): we should add a version number to the JSON file to ensure
  # that the writer and reader agree on the schema (raising an exception
  # otherwise).
  json_dict = json.loads(file_contents)
  return json_dict

def LoadFromFile(file_path):
  """Loads the JSON summary written out by the GM tool.
     Returns a dictionary keyed by the values listed as JSONKEY_ constants
     above."""
  file_contents = open(file_path, 'r').read()
  return LoadFromString(file_contents)

def WriteToFile(json_dict, file_path):
  """Writes the JSON summary in json_dict out to file_path."""
  with open(file_path, 'w') as outfile:
    json.dump(json_dict, outfile, sort_keys=True, indent=2)
