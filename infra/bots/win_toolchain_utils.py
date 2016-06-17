#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Utilities for manipulating the win_toolchain.json file."""


import json
import os
import stat


PLACEHOLDER = '<(TOOLCHAIN_BASE_DIR)'


def _replace_prefix(val, before, after):
  """Replace the given prefix with the given string."""
  if val.startswith(before):
    return val.replace(before, after, 1)
  return val


def _replace(val, before, after):
  """Replace occurrences of one string with another within the data."""
  if isinstance(val, basestring):
    return _replace_prefix(val, before, after)
  elif isinstance(val, (list, tuple)):
    return [_replace(elem, before, after) for elem in val]
  elif isinstance(val, dict):
    return {_replace(k, before, after):
            _replace(v, before, after) for k, v in val.iteritems()}
  raise Exception('Cannot replace variable: %s' % val)


def _replace_in_file(filename, before, after):
  """Replace occurrences of one string with another within the file."""
  # Make the file writeable, or the below won't work.
  os.chmod(filename, stat.S_IWRITE)

  with open(filename) as f:
    contents = json.load(f)
  new_contents = _replace(contents, before, after)
  with open(filename, 'w') as f:
    json.dump(new_contents, f)


def abstract(win_toolchain_json, old_path):
  """Replace absolute paths in win_toolchain.json with placeholders."""
  _replace_in_file(win_toolchain_json, old_path, PLACEHOLDER)


def resolve(win_toolchain_json, new_path):
  """Replace placeholders in win_toolchain.json with absolute paths."""
  _replace_in_file(win_toolchain_json, PLACEHOLDER, new_path)
