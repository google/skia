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
source_file_types = {
  '.cc': 'cxx',
  '.cpp': 'cxx',
  '.cxx': 'cxx',
  '.c': 'c',
  '.s': 'asm',
  '.S': 'asm',
  '.asm': 'asm',
  '.o': 'obj',
  '.obj': 'obj',
}


class CMakeTargetType(object):
  def __init__(self, command, modifier, property_modifier, is_linkable):
    self.command = command
    self.modifier = modifier
    self.property_modifier = property_modifier
    self.is_linkable = is_linkable
CMakeTargetType.custom = CMakeTargetType('add_custom_target', 'SOURCES',
                                         None, False)

# See GetStringForOutputType in gn
cmake_target_types = {
  'unknown': CMakeTargetType.custom,
  'group': CMakeTargetType.custom,
  'executable': CMakeTargetType('add_executable', None, 'RUNTIME', True),
  'loadable_module': CMakeTargetType('add_library', 'MODULE', 'LIBRARY', True),
  'shared_library': CMakeTargetType('add_library', 'SHARED', 'LIBRARY', True),
  'static_library': CMakeTargetType('add_library', 'STATIC', 'ARCHIVE', False),
  'source_set': CMakeTargetType('add_library', 'OBJECT', None, False),
  'action': CMakeTargetType.custom,
  'action_foreach': CMakeTargetType.custom,
  'bundle_data': CMakeTargetType.custom,
  'create_bundle': CMakeTargetType.custom,
}


class Target(object):
  def __init__(self, gn_name, targets):
    self.gn_name = gn_name
    self.properties = targets[self.gn_name]
    self.cmake_name = GetOutputName(self.gn_name, self.properties)
    self.gn_type = self.properties.get('type', None)
    self.cmake_type = cmake_target_types.get(self.gn_type, None)


def WriteCompilerFlags(out, target, targets, root_path, sources):
  # Hack, set linker language to c if no c or cxx files present.
  if not 'c' in sources and not 'cxx' in sources:
    SetTargetProperty(out, target.cmake_name, 'LINKER_LANGUAGE', ['C'])

  # Mark uncompiled sources as uncompiled.
  if 'other' in sources:
    out.write('set_source_files_properties(')
    WriteVariable(out, sources['other'], '')
    out.write(' PROPERTIES HEADER_FILE_ONLY "TRUE")\n')

  # Mark object sources as linkable.
  if 'obj' in sources:
    out.write('set_source_files_properties(')
    WriteVariable(out, sources['obj'], '')
    out.write(' PROPERTIES EXTERNAL_OBJECT "TRUE")\n')

  # TODO: 'output_name', 'output_dir', 'output_extension'

  # Includes
  includes = target.properties.get('include_dirs', [])
  if includes:
    out.write('set_property(TARGET ')
    out.write(target.cmake_name)
    out.write(' APPEND PROPERTY INCLUDE_DIRECTORIES')
    for include_dir in includes:
      out.write('\n  "')
      out.write(GetAbsolutePath(root_path, include_dir))
      out.write('"')
    out.write(')\n')

  # Defines
  defines = target.properties.get('defines', [])
  if defines:
    SetTargetProperty(out, target.cmake_name,
                      'COMPILE_DEFINITIONS', defines, ';')

  # Compile flags
  # "arflags", "asmflags", "cflags",
  # "cflags_c", "clfags_cc", "cflags_objc", "clfags_objcc"
  # CMake does not have per target lang compile flags.
  # TODO: $<$<COMPILE_LANGUAGE:CXX>:cflags_cc style generator expression.
  #       http://public.kitware.com/Bug/view.php?id=14857
  flags = []
  flags.extend(target.properties.get('cflags', []))
  cflags_asm = target.properties.get('asmflags', [])
  cflags_c = target.properties.get('cflags_c', [])
  cflags_cxx = target.properties.get('cflags_cc', [])
  if 'c' in sources and not any(k in sources for k in ('asm', 'cxx')):
    flags.extend(cflags_c)
  elif 'cxx' in sources and not any(k in sources for k in ('asm', 'c')):
    flags.extend(cflags_cxx)
  else:
    # TODO: This is broken, one cannot generally set properties on files,
    # as other targets may require different properties on the same files.
    if 'asm' in sources and cflags_asm:
      SetFilesProperty(out, sources['asm'], 'COMPILE_FLAGS', cflags_asm, ' ')
    if 'c' in sources and cflags_c:
      SetFilesProperty(out, sources['c'], 'COMPILE_FLAGS', cflags_c, ' ')
    if 'cxx' in sources and cflags_cxx:
      SetFilesProperty(out, sources['cxx'], 'COMPILE_FLAGS', cflags_cxx, ' ')
  if flags:
    SetTargetProperty(out, target.cmake_name, 'COMPILE_FLAGS', flags, ' ')

  # Linker flags
  ldflags = target.properties.get('ldflags', [])
  if ldflags:
    SetTargetProperty(out, target.cmake_name, 'LINK_FLAGS', ldflags, ' ')


def GetObjectDependencies(object_dependencies, target_name, targets):
  dependencies = targets[target_name].get('deps', [])
  for dependency in dependencies:
    if targets[dependency].get('type', None) == 'source_set':
      object_dependencies.add(dependency)
      GetObjectDependencies(object_dependencies, dependency, targets)


def WriteSourceVariables(out, target, targets, root_path):
  raw_sources = target.properties.get('sources', [])

  # gn separates the sheep from the goats based on file extensions.
  # A full separation is done here because of flag handing (see Compile flags).
  source_types = {'cxx':[], 'c':[], 'asm':[],
                  'obj':[], 'obj_target':[], 'other':[]}
  for source in raw_sources:
    _, ext = posixpath.splitext(source)
    source_abs_path = GetAbsolutePath(root_path, source)
    source_types[source_file_types.get(ext, 'other')].append(source_abs_path)

  # OBJECT library dependencies need to be listed as sources.
  # Only executables and non-OBJECT libraries may reference an OBJECT library.
  # https://gitlab.kitware.com/cmake/cmake/issues/14778
  if target.cmake_type.modifier != 'OBJECT':
    object_dependencies = set()
    GetObjectDependencies(object_dependencies, target.gn_name, targets)
    for dependency in object_dependencies:
      cmake_dependency_name = GetOutputName(dependency, targets[dependency])
      obj_target_sources = '$<TARGET_OBJECTS:' + cmake_dependency_name + '>'
      source_types['obj_target'].append(obj_target_sources)

  sources = {}
  for source_type, sources_of_type in source_types.items():
    if sources_of_type:
      sources[source_type] = target.cmake_name + '__' + source_type + '_srcs'
      SetVariableList(out, sources[source_type], sources_of_type)
  return sources


def WriteTarget(out, target_name, root_path, targets):
  out.write('\n#')
  out.write(target_name)
  out.write('\n')

  target = Target(target_name, targets)

  if target.cmake_type is None:
    print ('Target %s has unknown target type %s, skipping.' %
          (        target_name,               target.gn_type ) )
    return

  sources = WriteSourceVariables(out, target, targets, root_path)

  out.write(target.cmake_type.command)
  out.write('(')
  out.write(target.cmake_name)
  if target.cmake_type.modifier is not None:
    out.write(' ')
    out.write(target.cmake_type.modifier)
  for sources_type_name in sources.values():
    WriteVariable(out, sources_type_name, ' ')
  out.write(')\n')

  if target.cmake_type.command != 'add_custom_target':
    WriteCompilerFlags(out, target, targets, root_path, sources)

  dependencies = target.properties.get('deps', [])
  libraries = []
  nonlibraries = []
  for dependency in dependencies:
    gn_dependency_type = targets.get(dependency, {}).get('type', None)
    cmake_dependency_type = cmake_target_types.get(gn_dependency_type, None)
    if cmake_dependency_type.command != 'add_library':
      nonlibraries.append(dependency)
    elif cmake_dependency_type.modifier != 'OBJECT':
      libraries.append(GetOutputName(dependency, targets[dependency]))

  # Non-library dependencies.
  if nonlibraries:
    out.write('add_dependencies(')
    out.write(target.cmake_name)
    out.write('\n')
    for nonlibrary in nonlibraries:
      out.write('  ')
      out.write(GetOutputName(nonlibrary, targets[nonlibrary]))
      out.write('\n')
    out.write(')\n')

  # Non-OBJECT library dependencies.
  external_libraries = target.properties.get('libs', [])
  if target.cmake_type.is_linkable and (external_libraries or libraries):
    system_libraries = []
    for external_library in external_libraries:
      if '/' in external_library:
        libraries.append(GetAbsolutePath(root_path, external_library))
      else:
        if external_library.endswith('.framework'):
          external_library = external_library[:-len('.framework')]
        system_library = external_library + '__library'
        out.write('find_library (')
        out.write(system_library)
        out.write(' ')
        out.write(external_library)
        out.write(')\n')
        system_libraries.append(system_library)
    out.write('target_link_libraries(')
    out.write(target.cmake_name)
    for library in libraries:
      out.write('\n  "')
      out.write(CMakeStringEscape(library))
      out.write('"')
    for system_library in system_libraries:
      WriteVariable(out, system_library, '\n  ')
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
