#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Test OrderedSet.
"""

import sys
import test_variables
import unittest

sys.path.append(test_variables.GYP_GEN_DIR)

from vars_dict_lib import OrderedSet


def create_dummy_var(i):
  return 'dummy_var' + str(i)


class OrderedSetTest(unittest.TestCase):

  def setUp(self):
    self.__set = OrderedSet()

  def test_methods(self):
    """
    Test methods on OrderedSet.
    """
    RANGE = 10
    for i in range(RANGE):
      dummy_var = create_dummy_var(i)
      # Add to the list. This should succeed.
      self.__set.add(dummy_var)
      self.assertEqual(len(self.__set), i+1)
      self.assertTrue(dummy_var in self.__set)
      self.assertEqual(self.__set[i], dummy_var)

      # Now attempt to add it again. This should fail.
      self.__set.add(dummy_var)
      self.assertEqual(len(self.__set), i+1)
      self.assertEqual(self.__set[i], dummy_var)

    # Test iterator.
    counter = 0
    for set_member in self.__set:
      self.assertEqual(create_dummy_var(counter), set_member)
      counter += 1
    self.assertEqual(counter, len(self.__set))

    # Now test removal.
    for i in range(RANGE):
      dummy_var = create_dummy_var(i)
      self.__set.remove(dummy_var)
      self.assertEqual(len(self.__set), RANGE-i-1)
      self.assertFalse(dummy_var in self.__set)

def main():
  loader = unittest.TestLoader()
  suite = loader.loadTestsFromTestCase(OrderedSetTest)
  unittest.TextTestRunner(verbosity=2).run(suite)

if __name__ == "__main__":
  main()

