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
  'copy': CMakeTargetType.custom,
  'action': CMakeTargetType.custom,
  'action_foreach': CMakeTargetType.custom,
  'bundle_data': CMakeTargetType.custom,
  'create_bundle': CMakeTargetType.custom,
}


def GetBaseName(gn_target_name):
  base_name = posixpath.basename(gn_target_name)
  sep = base_name.rfind(":")
  if sep != -1:
    base_name = base_name[sep+1:]
  return base_name


class Project(object):
  def __init__(self, project_json):
    self.targets = project_json['targets']
    build_settings = project_json['build_settings']
    self.root_path = build_settings['root_path']
    self.build_path = posixpath.join(self.root_path,
                                     build_settings['build_dir'][2:])

  def GetAbsolutePath(self, path):
    if path.startswith("//"):
      return self.root_path + "/" + path[2:]
    else:
      return path

  def GetObjectDependencies(self, gn_target_name, object_dependencies):
    dependencies = self.targets[gn_target_name].get('deps', [])
    for dependency in dependencies:
      dependency_type = self.targets[dependency].get('type', None)
      if dependency_type == 'source_set':
        object_dependencies.add(dependency)
      if dependency_type not in gn_target_types_that_absorb_objects:
        self.GetObjectDependencies(dependency, object_dependencies)

  def GetCMakeTargetName(self, gn_target_name):
    target_properties = self.targets[gn_target_name]
    output_name = target_properties.get("output_name", None)
    if output_name is None:
      output_name = GetBaseName(gn_target_name)
    output_extension = target_properties.get("output_extension", None)
    if output_extension is not None:
      output_name = posixpath.splitext(output_name)[0]
      if len(output_extension):
        output_name += "." + output_extension
    return output_name


class Target(object):
  def __init__(self, gn_target_name, project):
    self.gn_name = gn_target_name
    self.properties = project.targets[self.gn_name]
    self.cmake_name = project.GetCMakeTargetName(self.gn_name)
    self.gn_type = self.properties.get('type', None)
    self.cmake_type = cmake_target_types.get(self.gn_type, None)


def WriteAction(out, target, project, sources, synthetic_dependencies):
  outputs = []
  output_directories = set()
  for output in target.properties.get('outputs', []):
    output_abs_path = project.GetAbsolutePath(output)
    outputs.append(output_abs_path)
    output_directory = posixpath.dirname(output_abs_path)
    if output_directory:
      output_directories.add(output_directory)
  outputs_name = target.cmake_name + '__output'
  SetVariableList(out, outputs_name, outputs)

  out.write('add_custom_command(OUTPUT ')
  WriteVariable(out, outputs_name)
  out.write('\n')

  for directory in output_directories:
    out.write('  COMMAND ${CMAKE_COMMAND} -E make_directory ')
    out.write(directory)
    out.write('\n')

  out.write('  COMMAND python ')
  out.write(project.GetAbsolutePath(target.properties['script']))
  out.write(' ')
  out.write(' '.join(target.properties['args']))
  out.write('\n')

  out.write('  DEPENDS ')
  for sources_type_name in sources.values():
    WriteVariable(out, sources_type_name, ' ')
  out.write('\n')

  out.write('  WORKING_DIRECTORY ')
  out.write(project.build_path)
  out.write('\n')

  out.write('  COMMENT ')
  out.write(target.cmake_name)
  out.write('\n')

  out.write('  VERBATIM)\n')

  synthetic_dependencies.add(outputs_name)


def WriteCompilerFlags(out, target, project, sources):
  # Hack, set linker language to c if no c or cxx files present.
  if not 'c' in sources and not 'cxx' in sources:
    SetTargetProperty(out, target.cmake_name, 'LINKER_LANGUAGE', ['C'])

  # Mark uncompiled sources as uncompiled.
  if 'input' in sources:
    SetFilesProperty(out, sources['input'], 'HEADER_FILE_ONLY', ('True',), '')
  if 'other' in sources:
    SetFilesProperty(out, sources['other'], 'HEADER_FILE_ONLY', ('True',), '')

  # Mark object sources as linkable.
  if 'obj' in sources:
    SetFilesProperty(out, sources['obj'], 'EXTERNAL_OBJECT', ('True',), '')

  # TODO: 'output_name', 'output_dir', 'output_extension'
  # This includes using 'source_outputs' to direct compiler output.

  # Includes
  includes = target.properties.get('include_dirs', [])
  if includes:
    out.write('set_property(TARGET ')
    out.write(target.cmake_name)
    out.write(' APPEND PROPERTY INCLUDE_DIRECTORIES')
    for include_dir in includes:
      out.write('\n  "')
      out.write(project.GetAbsolutePath(include_dir))
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


gn_target_types_that_absorb_objects = (
  'executable',
  'loadable_module',
  'shared_library',
  'static_library'
)


def WriteSourceVariables(out, target, project):
  # gn separates the sheep from the goats based on file extensions.
  # A full separation is done here because of flag handing (see Compile flags).
  source_types = {'cxx':[], 'c':[], 'asm':[],
                  'obj':[], 'obj_target':[], 'input':[], 'other':[]}

  # TODO .def files on Windows
  for source in target.properties.get('sources', []):
    _, ext = posixpath.splitext(source)
    source_abs_path = project.GetAbsolutePath(source)
    source_types[source_file_types.get(ext, 'other')].append(source_abs_path)

  for input_path in target.properties.get('inputs', []):
    input_abs_path = project.GetAbsolutePath(input_path)
    source_types['input'].append(input_abs_path)

  # OBJECT library dependencies need to be listed as sources.
  # Only executables and non-OBJECT libraries may reference an OBJECT library.
  # https://gitlab.kitware.com/cmake/cmake/issues/14778
  if target.gn_type in gn_target_types_that_absorb_objects:
    object_dependencies = set()
    project.GetObjectDependencies(target.gn_name, object_dependencies)
    for dependency in object_dependencies:
      cmake_dependency_name = project.GetCMakeTargetName(dependency)
      obj_target_sources = '$<TARGET_OBJECTS:' + cmake_dependency_name + '>'
      source_types['obj_target'].append(obj_target_sources)

  sources = {}
  for source_type, sources_of_type in source_types.items():
    if sources_of_type:
      sources[source_type] = target.cmake_name + '__' + source_type + '_srcs'
      SetVariableList(out, sources[source_type], sources_of_type)
  return sources


def WriteTarget(out, target, project):
  out.write('\n#')
  out.write(target.gn_name)
  out.write('\n')

  if target.cmake_type is None:
    print ('Target %s has unknown target type %s, skipping.' %
          (        target.gn_name,            target.gn_type ) )
    return

  sources = WriteSourceVariables(out, target, project)

  synthetic_dependencies = set()
  if target.gn_type == 'action':
    WriteAction(out, target, project, sources, synthetic_dependencies)

  out.write(target.cmake_type.command)
  out.write('(')
  out.write(target.cmake_name)
  if target.cmake_type.modifier is not None:
    out.write(' ')
    out.write(target.cmake_type.modifier)
  for sources_type_name in sources.values():
    WriteVariable(out, sources_type_name, ' ')
  if synthetic_dependencies:
    out.write(' DEPENDS')
    for synthetic_dependencie in synthetic_dependencies:
      WriteVariable(out, synthetic_dependencie, ' ')
  out.write(')\n')

  if target.cmake_type.command != 'add_custom_target':
    WriteCompilerFlags(out, target, project, sources)

  dependencies = target.properties.get('deps', [])
  libraries = []
  nonlibraries = []
  for dependency in dependencies:
    gn_dependency_type = project.targets.get(dependency, {}).get('type', None)
    cmake_dependency_type = cmake_target_types.get(gn_dependency_type, None)
    cmake_dependency_name = project.GetCMakeTargetName(dependency)
    if cmake_dependency_type.command != 'add_library':
      nonlibraries.append(cmake_dependency_name)
    elif cmake_dependency_type.modifier != 'OBJECT':
      if target.cmake_type.is_linkable:
        libraries.append(cmake_dependency_name)
      else:
        nonlibraries.append(cmake_dependency_name)

  # Non-library dependencies.
  if nonlibraries:
    out.write('add_dependencies(')
    out.write(target.cmake_name)
    for nonlibrary in nonlibraries:
      out.write('\n  ')
      out.write(nonlibrary)
    out.write(')\n')

  # Non-OBJECT library dependencies.
  external_libraries = target.properties.get('libs', [])
  if target.cmake_type.is_linkable and (external_libraries or libraries):
    system_libraries = []
    for external_library in external_libraries:
      if '/' in external_library:
        libraries.append(project.GetAbsolutePath(external_library))
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
  out = open(posixpath.join(project.build_path, 'CMakeLists.txt'), 'w+')
  out.write('cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)\n')
  out.write('cmake_policy(VERSION 2.8.8)\n')

  # The following appears to be as-yet undocumented.
  # http://public.kitware.com/Bug/view.php?id=8392
  out.write('enable_language(ASM)\n')
  # ASM-ATT does not support .S files.
  # output.write('enable_language(ASM-ATT)\n')

  for target_name in project.targets.keys():
    out.write('\n')
    WriteTarget(out, Target(target_name, project), project)


def main():
  if len(sys.argv) != 2:
    print('Usage: ' + sys.argv[0] + ' <json_file_name>')
    exit(1)

  json_path = sys.argv[1]
  project = None
  with open(json_path, 'r') as json_file:
    project = json.loads(json_file.read())

  WriteProject(Project(project))


if __name__ == "__main__":
  main()
