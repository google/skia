#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections
import types

class OrderedSet(object):
  """
  Ordered set of unique items that supports addition and removal.
  """

  def __init__(self):
    self.__li = []

  def add(self, item):
    """
    Add item, if it is not already in the set.
    @param item The item to add.
    """
    if item not in self.__li:
      self.__li.append(item)

  def __contains__(self, item):
    """
    Whether the set contains item.
    @param item The item to search for in the set.
    @return bool Whether the item is in the set.
    """
    return item in self.__li

  def __iter__(self):
    """
    Iterator for the set.
    """
    return self.__li.__iter__()

  def remove(self, item):
    """
    Remove item from the set.
    @param item Item to be removed.
    """
    return self.__li.remove(item)

  def __len__(self):
    """
    Number of items in the set.
    """
    return len(self.__li)

  def __getitem__(self, index):
    """
    Return item at index.
    """
    return self.__li[index]

VAR_NAMES = ['LOCAL_CFLAGS',
             'LOCAL_CPPFLAGS',
             'LOCAL_SRC_FILES',
             'LOCAL_SHARED_LIBRARIES',
             'LOCAL_STATIC_LIBRARIES',
             'LOCAL_C_INCLUDES',
             'LOCAL_EXPORT_C_INCLUDE_DIRS',
             'KNOWN_TARGETS']

class VarsDict(collections.namedtuple('VarsDict', VAR_NAMES)):
  """
  Custom class for storing the arguments to Android.mk variables. Can be
  treated as a dictionary with fixed keys.
  """

  __slots__ = ()

  def __new__(cls):
    lists = []
    # TODO (scroggo): Is there a better way add N items?
    for __unused__ in range(len(VAR_NAMES)):
      lists.append(OrderedSet())
    return tuple.__new__(cls, lists)

  def keys(self):
    """
    Return the field names as strings.
    """
    return self._fields

  def __getitem__(self, index):
    """
    Return an item, indexed by a number or a string.
    """
    if type(index) == types.IntType:
      # Treat the index as an array index into a tuple.
      return tuple.__getitem__(self, index)
    if type(index) == types.StringType:
      # Treat the index as a key into a dictionary.
      return eval('self.%s' % index)
    return None


def intersect(var_dict_list):
  """
  Find the intersection of a list of VarsDicts and trim each input to its
  unique entries.
  @param var_dict_list list of VarsDicts. WARNING: each VarsDict will be
                       modified in place, to remove the common elements!
  @return VarsDict containing list entries common to all VarsDicts in
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

