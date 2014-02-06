#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Functions for parsing the gypd output from gyp.
"""

import vars_dict_lib

def parse_dictionary(var_dict, d, current_target_name):
  """
  Helper function to get the meaningful entries in a dictionary.
  @param var_dict VarsDict object for storing the results of the parsing.
  @param d Dictionary object to parse.
  @param current_target_name The current target being parsed. If this
                             dictionary is a target, this will be its entry
                             'target_name'. Otherwise, this will be the name of
                             the target which contains this dictionary.
  """
  for source in d.get('sources', []):
    # Compare against a lowercase version, in case files are named .H or .GYPI
    lowercase_source = source.lower()
    if lowercase_source.endswith('.h'):
      # Android.mk does not need the header files.
      continue
    if lowercase_source.endswith('gypi'):
      # The gypi files are included in sources, but the sources they included
      # are also included. No need to parse them again.
      continue
    # The path is relative to the gyp folder, but Android wants the path
    # relative to the root.
    source = source.replace('../src', 'src', 1)
    var_dict.LOCAL_SRC_FILES.add(source)

  for lib in d.get('libraries', []):
    if lib.endswith('.a'):
      # Remove the '.a'
      lib = lib[:-2]
      # Add 'lib', if necessary
      if not lib.startswith('lib'):
        lib = 'lib' + lib
      var_dict.LOCAL_STATIC_LIBRARIES.add(lib)
    else:
      # lib will be in the form of '-l<name>'. Change it to 'lib<name>'
      lib = lib.replace('-l', 'lib', 1)
      var_dict.LOCAL_SHARED_LIBRARIES.add(lib)

  for dependency in d.get('dependencies', []):
    # Each dependency is listed as
    #   <path_to_file>:<target>#target
    li = dependency.split(':')
    assert(len(li) <= 2 and len(li) >= 1)
    sub_targets = []
    if len(li) == 2 and li[1] != '*':
      sub_targets.append(li[1].split('#')[0])
    sub_path = li[0]
    assert(sub_path.endswith('.gyp'))
    # Although the original reference is to a .gyp, parse the corresponding
    # gypd file, which was constructed by gyp.
    sub_path = sub_path + 'd'
    parse_gypd(var_dict, sub_path, sub_targets)

  if 'default_configuration' in d:
    config_name = d['default_configuration']
    # default_configuration is meaningless without configurations
    assert('configurations' in d)
    config = d['configurations'][config_name]
    parse_dictionary(var_dict, config, current_target_name)

  for flag in d.get('cflags', []):
    var_dict.LOCAL_CFLAGS.add(flag)
  for flag in d.get('cflags_cc', []):
    var_dict.LOCAL_CPPFLAGS.add(flag)

  for include in d.get('include_dirs', []):
    # The input path will be relative to gyp/, but Android wants relative to
    # LOCAL_PATH
    include = include.replace('..', '$(LOCAL_PATH)', 1)
    # Remove a trailing slash, if present.
    if include.endswith('/'):
      include = include[:-1]
    var_dict.LOCAL_C_INCLUDES.add(include)
    # For the top level, libskia, include directories should be exported.
    if current_target_name == 'libskia':
      var_dict.LOCAL_EXPORT_C_INCLUDE_DIRS.add(include)

  for define in d.get('defines', []):
    var_dict.LOCAL_CFLAGS.add('-D' + define)


def parse_gypd(var_dict, path, desired_targets=None):
  """
  Parse a gypd file.
  @param var_dict VarsDict object for storing the result of the parse.
  @param path Path to gypd file.
  @param desired_targets List of targets to be parsed from this file. If empty,
                         parse all targets.
  """
  d = {}
  with open(path, 'r') as f:
    # Read the entire file as a dictionary
    d = eval(f.read())

  # The gypd file is structured such that the top level dictionary has an entry
  # named 'targets'
  for target in d['targets']:
    target_name = target['target_name']
    if target_name in var_dict.KNOWN_TARGETS:
      # Avoid circular dependencies
      continue
    if desired_targets and target_name not in desired_targets:
      # Our caller does not depend on this one
      continue
    # Add it to our known targets so we don't parse it again
    var_dict.KNOWN_TARGETS.add(target_name)

    parse_dictionary(var_dict, target, target_name)

