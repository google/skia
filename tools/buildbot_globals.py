#!/usr/bin/python

# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Provides read access to buildbot's global_variables.json .
"""

import json
import svn

_global_vars = None

class NoSuchGlobalVariable(KeyError):
  pass

def Get(var_name):
  '''Return the value associated with this name in global_variables.json.
  Raises NoSuchGlobalVariable if there is no variable with that name.'''
  global _global_vars
  if not _global_vars:
    _global_vars = json.loads(svn.Cat('http://skia.googlecode.com/svn/'
                                      'buildbot/site_config/'
                                      'global_variables.json'))
  try:
    return _global_vars[var_name]['value']
  except KeyError:
    raise NoSuchGlobalVariable(var_name)
