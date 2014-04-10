#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test results.py

"""

# Imports from within Skia
import base_unittest
import results


class ResultsTest(base_unittest.TestCase):

  def test_ignore_builder(self):
    """Test _ignore_builder()."""
    results_obj = results.BaseComparisons()
    self.assertEqual(results_obj._ignore_builder('SomethingTSAN'), True)
    self.assertEqual(results_obj._ignore_builder('Something-Trybot'), True)
    self.assertEqual(results_obj._ignore_builder(
        'Test-Ubuntu12-ShuttleA-GTX660-x86-Release'), False)
    results_obj.set_skip_builders_pattern_list(['.*TSAN.*', '.*GTX660.*'])
    self.assertEqual(results_obj._ignore_builder('SomethingTSAN'), True)
    self.assertEqual(results_obj._ignore_builder('Something-Trybot'), False)
    self.assertEqual(results_obj._ignore_builder(
        'Test-Ubuntu12-ShuttleA-GTX660-x86-Release'), True)
    results_obj.set_skip_builders_pattern_list(None)
    self.assertEqual(results_obj._ignore_builder('SomethingTSAN'), False)
    self.assertEqual(results_obj._ignore_builder('Something-Trybot'), False)
    self.assertEqual(results_obj._ignore_builder(
        'Test-Ubuntu12-ShuttleA-GTX660-x86-Release'), False)
    results_obj.set_match_builders_pattern_list(['.*TSAN'])
    self.assertEqual(results_obj._ignore_builder('SomethingTSAN'), False)
    self.assertEqual(results_obj._ignore_builder('Something-Trybot'), True)
    self.assertEqual(results_obj._ignore_builder(
        'Test-Ubuntu12-ShuttleA-GTX660-x86-Release'), True)

  def test_combine_subdicts_typical(self):
    """Test combine_subdicts() with no merge conflicts. """
    input_dict = {
      "failed" : {
        "changed.png" : [ "bitmap-64bitMD5", 8891695120562235492 ],
      },
      "no-comparison" : {
        "unchanged.png" : [ "bitmap-64bitMD5", 11092453015575919668 ],
      }
    }
    expected_output_dict = {
      "changed.png" : [ "bitmap-64bitMD5", 8891695120562235492 ],
      "unchanged.png" : [ "bitmap-64bitMD5", 11092453015575919668 ],
    }
    actual_output_dict = results.BaseComparisons.combine_subdicts(
        input_dict=input_dict)
    self.assertEqual(actual_output_dict, expected_output_dict)

  def test_combine_subdicts_with_merge_conflict(self):
    """Test combine_subdicts() with a merge conflict. """
    input_dict = {
      "failed" : {
        "changed.png" : [ "bitmap-64bitMD5", 8891695120562235492 ],
      },
      "no-comparison" : {
        "changed.png" : [ "bitmap-64bitMD5", 11092453015575919668 ],
      }
    }
    with self.assertRaises(Exception):
      actual_output_dict = results.BaseComparisons.combine_subdicts(
          input_dict=input_dict)


def main():
  base_unittest.main(ResultsTest)


if __name__ == '__main__':
  main()
