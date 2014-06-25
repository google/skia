# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Miscellaneous utilities."""


import re


class ReSearch(object):
  """A collection of static methods for regexing things."""

  @staticmethod
  def search_within_stream(input_stream, pattern, default=None):
    """Search for regular expression in a file-like object.

    Opens a file for reading and searches line by line for a match to
    the regex and returns the parenthesized group named return for the
    first match.  Does not search across newlines.

    For example:
        pattern = '^root(:[^:]*){4}:(?P<return>[^:]*)'
        with open('/etc/passwd', 'r') as stream:
            return search_within_file(stream, pattern)
    should return root's home directory (/root on my system).

    Args:
        input_stream: file-like object to be read
        pattern: (string) to be passed to re.compile
        default: what to return if no match

    Returns:
        A string or whatever default is
    """
    pattern_object = re.compile(pattern)
    for line in input_stream:
      match = pattern_object.search(line)
      if match:
        return match.group('return')
    return default

  @staticmethod
  def search_within_string(input_string, pattern, default=None):
    """Search for regular expression in a string.

    Args:
        input_string: (string) to be searched
        pattern: (string) to be passed to re.compile
        default: what to return if no match

    Returns:
        A string or whatever default is
    """
    match = re.search(pattern, input_string)
    return match.group('return') if match else default

