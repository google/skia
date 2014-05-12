#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

ColumnHeaderFactory class (see class docstring for details)
"""

# Keys used within dictionary representation of each column header.
# NOTE: Keep these in sync with static/constants.js
KEY__EXTRACOLUMNHEADERS__HEADER_TEXT = 'headerText'
KEY__EXTRACOLUMNHEADERS__HEADER_URL = 'headerUrl'
KEY__EXTRACOLUMNHEADERS__IS_FILTERABLE = 'isFilterable'
KEY__EXTRACOLUMNHEADERS__IS_SORTABLE = 'isSortable'
KEY__EXTRACOLUMNHEADERS__VALUES_AND_COUNTS = 'valuesAndCounts'


class ColumnHeaderFactory(object):
  """Factory which assembles the header for a single column of data."""

  def __init__(self, header_text, header_url=None,
               is_filterable=True, is_sortable=True,
               include_values_and_counts=True):
    """
    Args:
      header_text: string; text the client should display within column header.
      header_url: string; target URL if user clicks on column header.
          If None, nothing to click on.
      is_filterable: boolean; whether client should allow filtering on this
          column.
      is_sortable: boolean; whether client should allow sorting on this column.
      include_values_and_counts: boolean; whether the set of values found
          within this column, and their counts, should be available for the
          client to display.
    """
    self._header_text = header_text
    self._header_url = header_url
    self._is_filterable = is_filterable
    self._is_sortable = is_sortable
    self._include_values_and_counts = include_values_and_counts

  def create_as_dict(self, values_and_counts_dict=None):
    """Creates the header for this column, in dictionary form.

    Creates the header for this column in dictionary form, as needed when
    constructing the JSON representation.  Uses the KEY__EXTRACOLUMNHEADERS__*
    constants as keys.

    Args:
      values_and_counts_dict: dictionary mapping each possible column value
          to its count (how many entries in the column have this value), or
          None if this information is not available.
    """
    asdict = {
        KEY__EXTRACOLUMNHEADERS__HEADER_TEXT: self._header_text,
        KEY__EXTRACOLUMNHEADERS__IS_FILTERABLE: self._is_filterable,
        KEY__EXTRACOLUMNHEADERS__IS_SORTABLE: self._is_sortable,
    }
    if self._header_url:
      asdict[KEY__EXTRACOLUMNHEADERS__HEADER_URL] = self._header_url
    if self._include_values_and_counts and values_and_counts_dict:
      asdict[KEY__EXTRACOLUMNHEADERS__VALUES_AND_COUNTS] = sorted(
          values_and_counts_dict.items())
    return asdict
