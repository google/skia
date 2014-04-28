#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections
import types

# The goal of this class is to store a set of unique items in the order in
# which they are inserted. This is important for the final makefile, where
# we want to make sure the image decoders are in a particular order. See
# images.gyp for more information.
class OrderedSet(object):
  """Ordered set of unique items that supports addition and removal.

  Retains the order in which items are inserted.
  """

  def __init__(self):
    self.__ordered_set = []

  def add(self, item):
    """Add item, if it is not already in the set.

    item is appended to the end if it is not already in the set.

    Args:
      item: The item to add.
    """
    if item not in self.__ordered_set:
      self.__ordered_set.append(item)

  def __contains__(self, item):
    """Whether the set contains item.

    Args:
      item: The item to search for in the set.

    Returns:
      bool: Whether the item is in the set.
    """
    return item in self.__ordered_set

  def __iter__(self):
    """Iterator for the set.
    """
    return self.__ordered_set.__iter__()

  def remove(self, item):
    """
    Remove item from the set.

    Args:
      item: Item to be removed.

    Raises:
      ValueError if item is not in the set.
    """
    self.__ordered_set.remove(item)

  def __len__(self):
    """Number of items in the set.
    """
    return len(self.__ordered_set)

  def __getitem__(self, index):
    """Return item at index.
    """
    return self.__ordered_set[index]

  def reset(self):
    """Reset to empty.
    """
    self.__ordered_set = []

  def set(self, other):
    """Replace this ordered set with another.

    Args:
      other: OrderedSet to replace this one. After this call, this OrderedSet
        will contain exactly the same elements as other.
    """
    self.__ordered_set = list(other.__ordered_set)

VAR_NAMES = ['LOCAL_CFLAGS',
             'LOCAL_CPPFLAGS',
             'LOCAL_SRC_FILES',
             'LOCAL_SHARED_LIBRARIES',
             'LOCAL_STATIC_LIBRARIES',
             'LOCAL_C_INCLUDES',
             'LOCAL_EXPORT_C_INCLUDE_DIRS',
             'DEFINES',
             'KNOWN_TARGETS',
             # These are not parsed by gyp, but set manually.
             'LOCAL_MODULE_TAGS',
             'LOCAL_MODULE']

class VarsDict(collections.namedtuple('VarsDict', VAR_NAMES)):
  """Custom class for storing the arguments to Android.mk variables.

  Can also be treated as a dictionary with fixed keys.
  """

  __slots__ = ()

  def __new__(cls):
    lists = []
    # TODO (scroggo): Is there a better way add N items?
    for __unused__ in range(len(VAR_NAMES)):
      lists.append(OrderedSet())
    return tuple.__new__(cls, lists)

  def keys(self):
    """Return the field names as strings.
    """
    return self._fields

  def __getitem__(self, index):
    """Return an item, indexed by a number or a string.
    """
    if type(index) == types.IntType:
      # Treat the index as an array index into a tuple.
      return tuple.__getitem__(self, index)
    if type(index) == types.StringType:
      # Treat the index as a key into a dictionary.
      return eval('self.%s' % index)
    return None


def intersect(var_dict_list):
  """Compute intersection of VarsDicts.

  Find the intersection of a list of VarsDicts and trim each input to its
  unique entries.

  Args:
    var_dict_list: list of VarsDicts. WARNING: each VarsDict will be
      modified in place, to remove the common elements!
  Returns:
    VarsDict containing list entries common to all VarsDicts in
      var_dict_list
  """
  intersection = VarsDict()
  # First VarsDict
  var_dict_a = var_dict_list[0]
  # The rest.
  other_var_dicts = var_dict_list[1:]

  for key in var_dict_a.keys():
    # Copy A's list, so we can continue iterating after modifying the original.
    a_list = list(var_dict_a[key])
    for item in a_list:
      # If item is in all lists, add to intersection, and remove from all.
      in_all_lists = True
      for var_dict in other_var_dicts:
        if not item in var_dict[key]:
          in_all_lists = False
          break
      if in_all_lists:
        intersection[key].add(item)
        for var_dict in var_dict_list:
          var_dict[key].remove(item)
  return intersection

