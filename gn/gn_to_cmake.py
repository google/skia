#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Usage: gn_to_cmake.py <json_file_name>

gn gen out/config --ide=json --json-ide-script=../../gn/gn_to_cmake.py

or

gn gen out/config --ide=json
python gn/gn_to_cmake.py out/config/project.json
"""

import json
import posixpath
import os
import sys

def write_project(project):

  def get_base_name(target_name):
    base_name = posixpath.basename(target_name)
    sep = base_name.rfind(":")
    if sep != -1:
      base_name = base_name[sep+1:]
    return base_name

  def get_output_name(target_name, target_properties):
    output_name = target_properties.get("output_name", None)
    if output_name is None:
      output_name = get_base_name(target_name)

    output_extension = target_properties.get("output_extension", None)
    if output_extension is not None:
      output_name = posixpath.splitext(output_name)[0]
      if len(output_extension):
        output_name += "." + output_extension

    return output_name

  def get_absolute_path(root_path, path):
    if path.startswith("//"):
      return root_path + "/" + path[2:]
    else:
      return path

  build_settings = project['build_settings']
  root_path = build_settings['root_path']
  build_path = os.path.join(root_path, build_settings['build_dir'][2:])

  out = open(os.path.join(build_path, 'CMakeLists.txt'), 'w+')
  out.write('cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)\n')
  out.write('cmake_policy(VERSION 2.8.8)\n')

  for target_name, target_properties in project['targets'].items():
    sources = target_properties.get('sources', [])
    if not sources:
      continue

    target_output_name = get_output_name(target_name, target_properties)
    out.write('\n')
    out.write('add_library(')
    out.write(target_output_name)
    out.write(' STATIC\n')
    for source in sources:
      out.write('    "')
      out.write(get_absolute_path(root_path, source))
      out.write('"\n')
    out.write(')\n')

    out.write('set_target_properties(')
    out.write(target_output_name)
    out.write(' PROPERTIES EXCLUDE_FROM_ALL "TRUE")\n')

    out.write('set_property(TARGET ')
    out.write(target_output_name)
    out.write(' APPEND PROPERTY INCLUDE_DIRECTORIES\n')
    for include_dir in target_properties.get('include_dirs', []):
      out.write('    "')
      out.write(get_absolute_path(root_path, include_dir))
      out.write('"\n')
    out.write(')\n')

    out.write('set_target_properties(')
    out.write(target_output_name)
    out.write(' PROPERTIES COMPILE_DEFINITIONS "')
    for define in target_properties.get('defines', []):
      out.write(define)
      out.write(';')
    out.write('")\n')

def main():
  if len(sys.argv) != 2:
    print('Usage: ' + sys.argv[0] + ' <json_file_name>')
    exit(1)

  json_path = sys.argv[1]
  project = None
  with open(json_path, 'r') as json_file:
    project = json.loads(json_file.read())

  write_project(project)

if __name__ == "__main__":
  main()
