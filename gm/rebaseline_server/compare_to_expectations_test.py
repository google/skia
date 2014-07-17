#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test compare_to_expectations.py

TODO(epoger): Create a command to update the expected results (in
self._output_dir_expected) when appropriate.  For now, you should:
1. examine the results in self.output_dir_actual and make sure they are ok
2. rm -rf self._output_dir_expected
3. mv self.output_dir_actual self._output_dir_expected
Although, if you're using an SVN checkout, this will blow away .svn directories
within self._output_dir_expected, which wouldn't be good...

"""

import os

# Imports from within Skia
import base_unittest
import compare_to_expectations
import imagediffdb
import results
import gm_json  # must import results first, so that gm_json will be in sys.path


class CompareToExpectationsTest(base_unittest.TestCase):

  def test_gm(self):
    """Process results of a GM run with the ExpectationComparisons object."""
    image_diff_db = imagediffdb.ImageDiffDB(storage_root=self.temp_dir)
    results_obj = compare_to_expectations.ExpectationComparisons(
        image_diff_db=image_diff_db,
        actuals_root=os.path.join(self.input_dir, 'gm-actuals'),
        expected_root=os.path.join(self.input_dir, 'gm-expectations'),
        diff_base_url='/static/generated-images')
    results_obj.get_timestamp = mock_get_timestamp
    gm_json.WriteToFile(
        results_obj.get_packaged_results_of_type(
            results.KEY__HEADER__RESULTS_ALL),
        os.path.join(self.output_dir_actual, 'gm.json'))


def mock_get_timestamp():
  """Mock version of BaseComparisons.get_timestamp() for testing."""
  return 12345678


def main():
  base_unittest.main(CompareToExpectationsTest)


if __name__ == '__main__':
  main()
