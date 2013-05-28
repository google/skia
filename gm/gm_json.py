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


# These constants must be kept in sync with the kJsonKey_ constants in
# gm_expectations.cpp !
JSONKEY_ACTUALRESULTS = 'actual-results'
JSONKEY_ACTUALRESULTS_FAILED = 'failed'
JSONKEY_ACTUALRESULTS_FAILUREIGNORED = 'failure-ignored'
JSONKEY_ACTUALRESULTS_NOCOMPARISON = 'no-comparison'
JSONKEY_ACTUALRESULTS_SUCCEEDED = 'succeeded'

def Load(filepath):
  """Loads the JSON summary written out by the GM tool.
     Returns a dictionary keyed by the values listed as JSONKEY_ constants
     above."""
  # In the future, we should add a version number to the JSON file to ensure
  # that the writer and reader agree on the schema (raising an exception
  # otherwise).
  json_dict = json.load(open(filepath))
  return json_dict
