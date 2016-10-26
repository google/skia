#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

# We'll read a single named list of paths from a .gni file.
gni, name = sys.argv[1:]

# The .gni files we want to read are close enough to Python syntax
# that we can use execfile() if we supply definitions for GN builtins.

def get_path_info(path, kind):
  assert kind == "abspath"
  # While we want absolute paths in GN, GYP prefers relative paths.
  return path

builtins = {
  'get_path_info': get_path_info,
}
definitions = {}
execfile(gni, builtins, definitions)

# definitions now holds all the values defined in the .gni.
paths = definitions[name]

# Perform any string substitutions.
for var in definitions:
  if type(definitions[var]) is str:
    paths = [ p.replace('$'+var, definitions[var]) for p in paths ]

# Print the path list, to be received by <!@ syntax in GYP.
for p in paths:
  print p
