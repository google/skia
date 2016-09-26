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


def CMakeStringEscape(a):
  """Escapes the string 'a' for use inside a CMake string.

  This means escaping
  '\' otherwise it may be seen as modifying the next character
  '"' otherwise it will end the string
  ';' otherwise the string becomes a list

  The following do not need to be escaped
  '#' when the lexer is in string state, this does not start a comment
  """
  return a.replace('\\', '\\\\').replace(';', '\\;').replace('"', '\\"')


def SetVariable(out, variable_name, value):
  """Sets a CMake variable."""
  out.write('set(')
  out.write(variable_name)
  out.write(' "')
  out.write(CMakeStringEscape(value))
  out.write('")\n')


def SetVariableList(out, variable_name, values):
  """Sets a CMake variable to a list."""
  if not values:
    return SetVariable(out, variable_name, "")
  if len(values) == 1:
    return SetVariable(out, variable_name, values[0])
  out.write('list(APPEND ')
  out.write(variable_name)
  out.write('\n  "')
  out.write('"\n  "'.join([CMakeStringEscape(value) for value in values]))
  out.write('")\n')


def SetFilesProperty(output, variable, property_name, values, sep):
  """Given a set of source files, sets the given property on them."""
  output.write('set_source_files_properties(')
  WriteVariable(output, variable)
  output.write(' PROPERTIES ')
  output.write(property_name)
  output.write(' "')
  for value in values:
    output.write(CMakeStringEscape(value))
    output.write(sep)
  output.write('")\n')


def SetTargetProperty(out, target_name, property_name, values, sep=''):
  """Given a target, sets the given property."""
  out.write('set_target_properties(')
  out.write(target_name)
  out.write(' PROPERTIES ')
  out.write(property_name)
  out.write(' "')
  for value in values:
    out.write(CMakeStringEscape(value))
    out.write(sep)
  out.write('")\n')


def WriteVariable(output, variable_name, prepend=None):
  if prepend:
    output.write(prepend)
  output.write('${')
  output.write(variable_name)
  output.write('}')


def GetBaseName(target_name):
  base_name = posixpath.basename(target_name)
  sep = base_name.rfind(":")
  if sep != -1:
    base_name = base_name[sep+1:]
  return base_name


def GetOutputName(target_name, target_properties):
  output_name = target_properties.get("output_name", None)
  if output_name is None:
    output_name = GetBaseName(target_name)
  output_extension = target_properties.get("output_extension", None)
  if output_extension is not None:
    output_name = posixpath.splitext(output_name)[0]
    if len(output_extension):
      output_name += "." + output_extension
  return output_name


def GetAbsolutePath(root_path, path):
  if path.startswith("//"):
    return root_path + "/" + path[2:]
  else:
    return path


# See GetSourceFileType in gn
source_file_type = {
  '.cc': 'cxx',
  '.cpp': 'cxx',
  '.cxx': 'cxx',
  '.c': 'c',
  '.s': 'asm',  #cc
  '.S': 'asm',  #cc
  '.asm': 'asm',  #cc
  '.o': 'obj',
  '.obj': 'obj',
}


class CMakeTargetType(object):
  def __init__(self, command, modifier, property_modifier, is_linkable):
    self.command = command
    self.modifier = modifier
    self.property_modifier = property_modifier
    self.is_linkable = is_linkable

# See GetStringForOutputType in gn
cmake_target_type_from_gn_target_type = {
  'unknown': CMakeTargetType('add_custom_target', 'SOURCES', None, False),
  'group': CMakeTargetType('add_custom_target', 'SOURCES', None, False),
  'executable': CMakeTargetType('add_executable', None, 'RUNTIME', True),
  'loadable_module': CMakeTargetType('add_library', 'MODULE', 'LIBRARY', True),
  'shared_library': CMakeTargetType('add_library', 'SHARED', 'LIBRARY', True),
  'static_library': CMakeTargetType('add_library', 'STATIC', 'ARCHIVE', False),
  'source_set': CMakeTargetType('add_library', 'OBJECT', 'LIBRARY', False),
  'action': CMakeTargetType('add_custom_target', 'SOURCES', None, False),
  'action_foreach': CMakeTargetType('add_custom_target', 'SOURCES', None, False),
  'bundle_data': CMakeTargetType('add_custom_target', 'SOURCES', None, False),
  'create_bundle': CMakeTargetType('add_custom_target', 'SOURCES', None, False),
}


def WriteCompilerFlags(out, cmake_target_name, target_properties, root_path, source_type_names):
  # Hack, set linker language to c if no c or cxx files present.
  if not 'c' in source_type_names and not 'cxx' in source_type_names:
    SetTargetProperty(out, cmake_target_name, 'LINKER_LANGUAGE', ['C'])

  # Mark uncompiled sources as uncompiled.
  if 'other' in source_type_names:
    out.write('set_source_files_properties(')
    WriteVariable(out, source_type_names['other'], '')
    out.write(' PROPERTIES HEADER_FILE_ONLY "TRUE")\n')

  # Mark object sources as linkable.
  if 'obj' in source_type_names:
    out.write('set_source_files_properties(')
    WriteVariable(out, source_type_names['obj'], '')
    out.write(' PROPERTIES EXTERNAL_OBJECT "TRUE")\n')

  # TODO: 'output_name', 'output_dir', 'output_extension'

  # Includes
  out.write('set_property(TARGET ')
  out.write(cmake_target_name)
  out.write(' APPEND PROPERTY INCLUDE_DIRECTORIES')
  for include_dir in target_properties.get('include_dirs', []):
    out.write('\n  "')
    out.write(GetAbsolutePath(root_path, include_dir))
    out.write('"')
  out.write(')\n')

  # Defines
  defines = target_properties.get('defines', [])
  if defines:
    SetTargetProperty(out, cmake_target_name, 'COMPILE_DEFINITIONS', defines, ';')

  # Compile flags
  # "arflags", "asmflags", "cflags", "cflags_c", "clfags_cc", "cflags_objc", "clfags_objcc"
  # CMake does not have per target lang compile flags.
  # TODO: $<$<COMPILE_LANGUAGE:CXX>:cflags_cc style generator expression.
  #       http://public.kitware.com/Bug/view.php?id=14857
  flags = []
  flags.extend(target_properties.get('cflags', []))
  cflags_asm = target_properties.get('asmflags', [])
  cflags_c = target_properties.get('cflags_c', [])
  cflags_cxx = target_properties.get('cflags_cc', [])
  if 'c' in source_type_names and not ('asm' in source_type_names or 'cxx' in source_type_names):
    flags.extend(cflags_c)
  elif 'cxx' in source_type_names and not ('asm' in source_type_names or 'c' in source_type_names):
    flags.extend(cflags_cxx)
  else:
    # TODO: This is broken, one cannot generally set properties on files,
    # as other targets may require different properties on the same files.
    if 'asm' in source_type_names and cflags_asm:
      SetFilesProperty(out, source_type_names['asm'], 'COMPILE_FLAGS', cflags_asm, ' ')
    if 'c' in source_type_names and cflags_c:
      SetFilesProperty(out, source_type_names['c'], 'COMPILE_FLAGS', cflags_c, ' ')
    if 'cxx' in source_type_names and cflags_cxx:
      SetFilesProperty(out, source_type_names['cxx'], 'COMPILE_FLAGS', cflags_cxx, ' ')
  SetTargetProperty(out, cmake_target_name, 'COMPILE_FLAGS', flags, ' ')

  # Linker flags
  ldflags = target_properties.get('ldflags', [])
  if ldflags:
    SetTargetProperty(out, cmake_target_name, 'LINK_FLAGS', ldflags, ' ')


def GetObjectDependencies(object_dependencies, target_name, targets):
  dependencies = targets[target_name].get('deps', [])
  for dependency in dependencies:
    if targets[dependency].get('type', None) == 'source_set':
      object_dependencies.add(dependency)
      GetObjectDependencies(object_dependencies, dependency, targets)


def WriteTarget(out, target_name, root_path, targets):
  out.write('\n#')
  out.write(target_name)
  out.write('\n')

  target_properties = targets[target_name]
  cmake_target_name = GetOutputName(target_name, target_properties)

  raw_sources = target_properties.get('sources', [])

  gn_target_type = target_properties.get('type', None)
  cmake_target_type = cmake_target_type_from_gn_target_type.get(gn_target_type, None)
  if cmake_target_type is None:
    print ('Target %s has unknown target type %s, skipping.' %
          (        target_name,               gn_target_type  ) )
    return

  # gn separates the sheep from the goats based on file extensions.
  # A full separation is done here because of flag handing (see Compile flags).
  sources = {'cxx':[], 'c':[], 'asm':[], 'obj':[], 'obj_target':[], 'other':[]}
  for source in raw_sources:
    _, ext = posixpath.splitext(source)
    source_abs_path = GetAbsolutePath(root_path, source)
    sources[source_file_type.get(ext, 'other')].append(source_abs_path)

  # OBJECT library dependencies need to be listed as sources.
  # Only executables and non-OBJECT libraries may reference an OBJECT library.
  # https://gitlab.kitware.com/cmake/cmake/issues/14778
  if cmake_target_type.modifier != 'OBJECT':
    object_dependencies = set()
    GetObjectDependencies(object_dependencies, target_name, targets)
    for dependency in object_dependencies:
      cmake_dependency_name = GetOutputName(dependency, targets[dependency])
      sources['obj_target'].append('$<TARGET_OBJECTS:' + cmake_dependency_name + '>')

  source_type_names = {}
  for source_type, sources_of_type in sources.items():
    if sources_of_type:
      source_type_names[source_type] = cmake_target_name + '__' + source_type + '_srcs'
      SetVariableList(out, source_type_names[source_type], sources_of_type)

  out.write(cmake_target_type.command)
  out.write('(')
  out.write(cmake_target_name)
  if cmake_target_type.modifier is not None:
    out.write(' ')
    out.write(cmake_target_type.modifier)
  for sources_type_name in source_type_names.values():
    WriteVariable(out, sources_type_name, ' ')
  out.write(')\n')

  if cmake_target_type.command != 'add_custom_target':
    WriteCompilerFlags(out, cmake_target_name, target_properties, root_path, source_type_names)

  dependencies = target_properties.get('deps', [])
  libraries = []
  nonlibraries = []
  for dependency in dependencies:
    gn_dependency_type = targets.get(dependency, {}).get('type', None)
    cmake_dependency_type = cmake_target_type_from_gn_target_type.get(gn_dependency_type, None)
    if cmake_dependency_type.command != 'add_library':
      nonlibraries.append(dependency)
    elif cmake_dependency_type.modifier != 'OBJECT':
      libraries.append(GetOutputName(dependency, targets[dependency]))

  # Non-library dependencies.
  if nonlibraries:
    out.write('add_dependencies(')
    out.write(cmake_target_name)
    out.write('\n')
    for nonlibrary in nonlibraries:
      out.write('  ')
      out.write(GetOutputName(nonlibrary, targets[nonlibrary]))
      out.write('\n')
    out.write(')\n')

  # Non-OBJECT library dependencies.
  external_libraries = target_properties.get('libs', [])
  if cmake_target_type.is_linkable and (external_libraries or libraries):
    for external_library in external_libraries:
      out.write('find_library (')
      out.write(external_library)
      out.write('__library ')
      out.write(external_library)
      out.write(')\n')
    out.write('target_link_libraries(')
    out.write(cmake_target_name)
    for library in libraries:
      out.write('\n  ')
      out.write(library)
    for external_library in external_libraries:
      WriteVariable(out, external_library + '__library', '\n  ')
    out.write(')\n')


def WriteProject(project):
  build_settings = project['build_settings']
  root_path = build_settings['root_path']
  build_path = os.path.join(root_path, build_settings['build_dir'][2:])

  out = open(os.path.join(build_path, 'CMakeLists.txt'), 'w+')
  out.write('cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)\n')
  out.write('cmake_policy(VERSION 2.8.8)\n')

  # The following appears to be as-yet undocumented.
  # http://public.kitware.com/Bug/view.php?id=8392
  out.write('enable_language(ASM)\n')
  # ASM-ATT does not support .S files.
  # output.write('enable_language(ASM-ATT)\n')

  targets = project['targets']
  for target_name in targets.keys():
    out.write('\n')
    WriteTarget(out, target_name, root_path, targets)


def main():
  if len(sys.argv) != 2:
    print('Usage: ' + sys.argv[0] + ' <json_file_name>')
    exit(1)

  json_path = sys.argv[1]
  project = None
  with open(json_path, 'r') as json_file:
    project = json.loads(json_file.read())

  WriteProject(project)


if __name__ == "__main__":
  main()
