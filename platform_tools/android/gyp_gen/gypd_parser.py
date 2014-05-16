#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Functions for parsing the gypd output from gyp.
"""


import os


def parse_dictionary(var_dict, d, current_target_name, dest_dir):
  """Helper function to get the meaningful entries in a dictionary.

  Parse dictionary d, and store unique relevant entries in var_dict.
  Recursively parses internal dictionaries and files that are referenced.
  When parsing the 'libraries' list from gyp, entries in the form
  '-l<name>' get assigned to var_dict.LOCAL_SHARED_LIBRARIES as 'lib<name>',
  and entries in the form '[lib]<name>.a' get assigned to
  var_dict.LOCAL_STATIC_LIBRARIES as 'lib<name>'.

  Args:
    var_dict: VarsDict object for storing the results of the parsing.
    d: Dictionary object to parse.
    current_target_name: The current target being parsed. If this dictionary
      is a target, this will be its entry 'target_name'. Otherwise, this will
      be the name of the target which contains this dictionary.
    dest_dir: Destination for the eventual Android.mk that will be created from
      this parse, relative to Skia trunk. Used to determine path for source
      files.
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
    # relative to dest_dir.
    rel_source = os.path.relpath(source, os.pardir)
    rel_source = os.path.relpath(rel_source, dest_dir)
    var_dict.LOCAL_SRC_FILES.add(rel_source)

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
    parse_gypd(var_dict, sub_path, dest_dir, sub_targets)

  if 'default_configuration' in d:
    config_name = d['default_configuration']
    # default_configuration is meaningless without configurations
    assert('configurations' in d)
    config = d['configurations'][config_name]
    parse_dictionary(var_dict, config, current_target_name, dest_dir)

  for flag in d.get('cflags', []):
    var_dict.LOCAL_CFLAGS.add(flag)
  for flag in d.get('cflags_cc', []):
    var_dict.LOCAL_CPPFLAGS.add(flag)

  for include in d.get('include_dirs', []):
    if include.startswith('external'):
      # This path is relative to the Android root. Leave it alone.
      rel_include = include
    else:
      # As with source, the input path will be relative to gyp/, but Android
      # wants relative to dest_dir.
      rel_include = os.path.relpath(include, os.pardir)
      rel_include = os.path.relpath(rel_include, dest_dir)
      # No need to include the base directory.
      if rel_include is os.curdir:
        continue
      rel_include = os.path.join('$(LOCAL_PATH)', rel_include)

    # Remove a trailing slash, if present.
    if rel_include.endswith('/'):
      rel_include = rel_include[:-1]
    var_dict.LOCAL_C_INCLUDES.add(rel_include)
    # For the top level, libskia, include directories should be exported.
    # FIXME (scroggo): Do not hard code this.
    if current_target_name == 'libskia':
      var_dict.LOCAL_EXPORT_C_INCLUDE_DIRS.add(rel_include)

  for define in d.get('defines', []):
    var_dict.DEFINES.add(define)


def parse_gypd(var_dict, path, dest_dir, desired_targets=None):
  """Parse a gypd file.

  Open a file that consists of python dictionaries representing build targets.
  Parse those dictionaries using parse_dictionary. Recursively parses
  referenced files.

  Args:
    var_dict: VarsDict object for storing the result of the parse.
    path: Path to gypd file.
    dest_dir: Destination for the eventual Android.mk that will be created from
      this parse, relative to Skia trunk. Used to determine path for source
      files and include directories.
    desired_targets: List of targets to be parsed from this file. If empty,
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

    parse_dictionary(var_dict, target, target_name, dest_dir)

