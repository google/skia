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
    """Test methods on OrderedSet.
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

    # Test reset(), for a range of ranges.
    for subrange in range(RANGE):
      for i in range(subrange):
        self.__set.add(create_dummy_var(i))
      self.assertEqual(len(self.__set), subrange)
      self.__set.reset()
      self.assertEqual(len(self.__set), 0)

  def test_set(self):
    """Test OrderedSet.set().
    """
    # Create a set with dummy values.
    my_set = OrderedSet()
    RANGE = 10
    for i in range(RANGE):
      my_set.add(create_dummy_var(i))
    my_len = len(my_set)
    self.assertEqual(my_len, RANGE)

    # Copy it to another set.
    other_set = OrderedSet()
    self.assertEqual(len(other_set), 0)
    other_set.set(my_set)

    # Both sets should contain the same values, in the same order.
    iterator = iter(my_set)
    for item in other_set:
      self.assertTrue(item == iterator.next())
    with self.assertRaises(StopIteration):
      iterator.next()
    self.assertEqual(my_len, len(other_set))

    # But the sets are different. Changing one will not affect the other.
    self.assertFalse(other_set is my_set)
    other_var = 'something_else'
    other_set.add(other_var)
    self.assertEqual(my_len + 1, len(other_set))
    self.assertEqual(my_len, len(my_set))
    self.assertNotIn(other_var, my_set)


def main():
  loader = unittest.TestLoader()
  suite = loader.loadTestsFromTestCase(OrderedSetTest)
  unittest.TextTestRunner(verbosity=2).run(suite)

if __name__ == "__main__":
  main()

