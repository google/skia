#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Test the VarsDict.
"""

import sys
import test_variables
import unittest

sys.path.append(test_variables.GYP_GEN_DIR)

import vars_dict_lib
from vars_dict_lib import OrderedSet
from vars_dict_lib import VarsDict
from vars_dict_lib import VAR_NAMES

class VarsDictTest(unittest.TestCase):
  """
  Tests for the VarsDict class.
  """

  # May not be needed.
  def setUp(self):
    self.__vars_dict = VarsDict()

  def assert_consistency(self, v_dict):
    self.assertIs(v_dict.LOCAL_CFLAGS, v_dict['LOCAL_CFLAGS'])
    self.assertIs(v_dict.LOCAL_CPPFLAGS, v_dict['LOCAL_CPPFLAGS'])
    self.assertIs(v_dict.LOCAL_SRC_FILES, v_dict['LOCAL_SRC_FILES'])
    self.assertIs(v_dict.LOCAL_SHARED_LIBRARIES,
                  v_dict['LOCAL_SHARED_LIBRARIES'])
    self.assertIs(v_dict.LOCAL_STATIC_LIBRARIES,
                  v_dict['LOCAL_STATIC_LIBRARIES'])
    self.assertIs(v_dict.LOCAL_C_INCLUDES, v_dict['LOCAL_C_INCLUDES'])
    self.assertIs(v_dict.LOCAL_EXPORT_C_INCLUDE_DIRS,
                  v_dict['LOCAL_EXPORT_C_INCLUDE_DIRS'])
    self.assertIs(v_dict.KNOWN_TARGETS, v_dict['KNOWN_TARGETS'])

  def test_creation(self):
    v_dict = VarsDict()
    # VarsDict has one entry for each label in VAR_NAMES
    self.assertEqual(len(v_dict.keys()), len(VAR_NAMES))
    for key in v_dict.keys():
      self.assertIn(key, VAR_NAMES)
      # Each entry is an empty OrderedSet
      self.assertIsNotNone(v_dict[key])
      self.assertIsInstance(v_dict[key], OrderedSet)
      self.assertEqual(len(v_dict[key]), 0)
      self.assert_consistency(v_dict)

  def test_intersection(self):
    v_dict_list = []
    RANGE = 10
    for i in range(RANGE):
      v_dict = VarsDict()
      # Add something common to each field, as well as a unique entry
      for key in v_dict.keys():
        v_dict[key].add(key.lower())
        v_dict[key].add(str(i))

      self.assert_consistency(v_dict)

      v_dict_list.append(v_dict)

    intersection = vars_dict_lib.intersect(v_dict_list)

    self.assert_consistency(intersection)

    for key in intersection.keys():
      # Each field had one common item
      self.assertEqual(len(intersection[key]), 1)
      for item in intersection[key]:
        for other_v_dict in v_dict_list:
          self.assertNotIn(item, other_v_dict[key])


def main():
  loader = unittest.TestLoader()
  suite = loader.loadTestsFromTestCase(VarsDictTest)
  unittest.TextTestRunner(verbosity=2).run(suite)

if __name__ == "__main__":
  main()

