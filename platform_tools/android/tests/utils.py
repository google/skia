#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Common code for tests.
"""
import filecmp
import os

EXPECTATIONS_DIR = os.path.join(os.path.dirname(__file__), 'expectations')

def compare_to_expectation(actual_name, expectation_name, assert_true,
                           msg=None):
  """Check that a generated file matches its expectation in EXPECTATIONS_DIR.

  Assert that the generated file and expectation file are identical.

  Args:
      actual_name: Full path to the test file.
      expectation_name: Basename of the expectations file within which
          to compare. The file is expected to be in
          platform_tools/android/tests/expectations.
      assert_true: function for asserting a statement is True

      Args:
          condition: statement to check for True.
          msg: message to print if the files are not equal.

      msg: Message to pass to assert_true.
  """
  full_expectations_path = os.path.join(EXPECTATIONS_DIR, expectation_name)
  assert_true(filecmp.cmp(actual_name, full_expectations_path), msg)
