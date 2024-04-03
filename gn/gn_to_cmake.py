#!/usr/bin/env python3
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
python3 gn/gn_to_cmake.py out/config/project.json

The first is recommended, as it will auto-update.
"""


import itertools
import functools
import json
import posixpath
import os
import string
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


def CMakeTargetEscape(a):
  """Escapes the string 'a' for use as a CMake target name.

  CMP0037 in CMake 3.0 restricts target names to "^[A-Za-z0-9_.:+-]+$"
  The ':' is only allowed for imported targets.
  """
  def Escape(c):
    if c in string.ascii_letters or c in string.digits or c in '_.+-':
      return c
    else:
      return '__'
  return ''.join(map(Escape, a))


def SetVariable(out, variable_name, value):
  """Sets a CMake variable."""
  out.write('set("')
  out.write(CMakeStringEscape(variable_name))
  out.write('" "')
  out.write(CMakeStringEscape(value))
  out.write('")\n')


def SetVariableList(out, variable_name, values):
  """Sets a CMake variable to a list."""
  if not values:
    return SetVariable(out, variable_name, "")
  if len(values) == 1:
    return SetVariable(out, variable_name, values[0])
  out.write('list(APPEND "')
  out.write(CMakeStringEscape(variable_name))
  out.write('"\n  "')
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


def SetCurrentTargetProperty(out, property_name, values, sep=''):
  """Given a target, sets the given property."""
  out.write('set_target_properties("${target}" PROPERTIES ')
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
  '.m': 'objc',
  '.mm': 'objcc',
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
  'group': CMakeTargetType('add_library', 'INTERFACE', None, True),
  'executable': CMakeTargetType('add_executable', None, 'RUNTIME', True),
  'loadable_module': CMakeTargetType('add_library', 'MODULE', 'LIBRARY', True),
  'shared_library': CMakeTargetType('add_library', 'SHARED', 'LIBRARY', True),
  'static_library': CMakeTargetType('add_library', 'STATIC', 'ARCHIVE', True),
  'source_set': CMakeTargetType('add_library', 'OBJECT', None, False),
  'copy': CMakeTargetType.custom,
  'action': CMakeTargetType.custom,
  'action_foreach': CMakeTargetType.custom,
  'bundle_data': CMakeTargetType.custom,
  'create_bundle': CMakeTargetType.custom,
}


def FindFirstOf(s, a):
  return min(s.find(i) for i in a if i in s)


class Project(object):
  def __init__(self, project_json):
    self.targets = project_json['targets']
    build_settings = project_json['build_settings']
    self.root_path = build_settings['root_path']
    self.build_path = self.GetAbsolutePath(build_settings['build_dir'])

  def GetAbsolutePath(self, path):
    if path.startswith('//'):
      return posixpath.join(self.root_path, path[2:])
    else:
      return path

  def GetObjectSourceDependencies(self, gn_target_name, object_dependencies):
    """All OBJECT libraries whose sources have not been absorbed."""
    dependencies = self.targets[gn_target_name].get('deps', [])
    for dependency in dependencies:
      dependency_type = self.targets[dependency].get('type', None)
      if dependency_type == 'source_set':
        object_dependencies.add(dependency)
      if dependency_type not in gn_target_types_that_absorb_objects:
        self.GetObjectSourceDependencies(dependency, object_dependencies)

  def GetObjectLibraryDependencies(self, gn_target_name, object_dependencies):
    """All OBJECT libraries whose libraries have not been absorbed."""
    dependencies = self.targets[gn_target_name].get('deps', [])
    for dependency in dependencies:
      dependency_type = self.targets[dependency].get('type', None)
      if dependency_type == 'source_set':
        object_dependencies.add(dependency)
        self.GetObjectLibraryDependencies(dependency, object_dependencies)

  def GetCMakeTargetName(self, gn_target_name):
    # See <chromium>/src/tools/gn/label.cc#Resolve
    # //base/test:test_support(//build/toolchain/win:msvc)
    path_separator = FindFirstOf(gn_target_name, (':', '('))
    location = None
    name = None
    toolchain = None
    if not path_separator:
      location = gn_target_name[2:]
    else:
      location = gn_target_name[2:path_separator]
      toolchain_separator = gn_target_name.find('(', path_separator)
      if toolchain_separator == -1:
        name = gn_target_name[path_separator + 1:]
      else:
        if toolchain_separator > path_separator:
          name = gn_target_name[path_separator + 1:toolchain_separator]
        assert gn_target_name.endswith(')')
        toolchain = gn_target_name[toolchain_separator + 1:-1]
    assert location or name

    cmake_target_name = None
    if location.endswith('/' + name):
      cmake_target_name = location
    elif location:
      cmake_target_name = location + '_' + name
    else:
      cmake_target_name = name
    if toolchain:
      cmake_target_name += '--' + toolchain
    return CMakeTargetEscape(cmake_target_name)


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
  outputs_name = '${target}__output'
  SetVariableList(out, outputs_name, outputs)

  out.write('add_custom_command(OUTPUT ')
  WriteVariable(out, outputs_name)
  out.write('\n')

  if output_directories:
    out.write('  COMMAND ${CMAKE_COMMAND} -E make_directory "')
    out.write('" "'.join(map(CMakeStringEscape, output_directories)))
    out.write('"\n')

  script = target.properties['script']
  arguments = target.properties['args']
  out.write('  COMMAND python3 "')
  out.write(CMakeStringEscape(project.GetAbsolutePath(script)))
  out.write('"')
  if arguments:
    out.write('\n    "')
    out.write('"\n    "'.join(map(CMakeStringEscape, arguments)))
    out.write('"')
  out.write('\n')

  out.write('  DEPENDS ')
  for sources_type_name in sources.values():
    WriteVariable(out, sources_type_name, ' ')
  out.write('\n')

  #TODO: CMake 3.7 is introducing DEPFILE

  out.write('  WORKING_DIRECTORY "')
  out.write(CMakeStringEscape(project.build_path))
  out.write('"\n')

  out.write('  COMMENT "Action: ${target}"\n')

  out.write('  VERBATIM)\n')

  synthetic_dependencies.add(outputs_name)


def ExpandPlaceholders(source, a):
  source_dir, source_file_part = posixpath.split(source)
  source_name_part, _ = posixpath.splitext(source_file_part)
  #TODO: {{source_gen_dir}}, {{source_out_dir}}, {{response_file_name}}
  return a.replace('{{source}}', source) \
          .replace('{{source_file_part}}', source_file_part) \
          .replace('{{source_name_part}}', source_name_part) \
          .replace('{{source_dir}}', source_dir) \
          .replace('{{source_root_relative_dir}}', source_dir)


def WriteActionForEach(out, target, project, sources, synthetic_dependencies):
  all_outputs = target.properties.get('outputs', [])
  inputs = target.properties.get('sources', [])
  # TODO: consider expanding 'output_patterns' instead.
  outputs_per_input = int(len(all_outputs) / len(inputs))
  for count, source in enumerate(inputs):
    source_abs_path = project.GetAbsolutePath(source)

    outputs = []
    output_directories = set()
    for output in all_outputs[outputs_per_input *  count:
                              outputs_per_input * (count+1)]:
      output_abs_path = project.GetAbsolutePath(output)
      outputs.append(output_abs_path)
      output_directory = posixpath.dirname(output_abs_path)
      if output_directory:
        output_directories.add(output_directory)
    outputs_name = '${target}__output_' + str(count)
    SetVariableList(out, outputs_name, outputs)

    out.write('add_custom_command(OUTPUT ')
    WriteVariable(out, outputs_name)
    out.write('\n')

    if output_directories:
      out.write('  COMMAND ${CMAKE_COMMAND} -E make_directory "')
      out.write('" "'.join(map(CMakeStringEscape, output_directories)))
      out.write('"\n')

    script = target.properties['script']
    # TODO: need to expand {{xxx}} in arguments
    arguments = target.properties['args']
    out.write('  COMMAND python3 "')
    out.write(CMakeStringEscape(project.GetAbsolutePath(script)))
    out.write('"')
    if arguments:
      out.write('\n    "')
      expand = functools.partial(ExpandPlaceholders, source_abs_path)
      out.write('"\n    "'.join(map(CMakeStringEscape, map(expand,arguments))))
      out.write('"')
    out.write('\n')

    out.write('  DEPENDS')
    if 'input' in sources:
      WriteVariable(out, sources['input'], ' ')
    out.write(' "')
    out.write(CMakeStringEscape(source_abs_path))
    out.write('"\n')

    #TODO: CMake 3.7 is introducing DEPFILE

    out.write('  WORKING_DIRECTORY "')
    out.write(CMakeStringEscape(project.build_path))
    out.write('"\n')

    out.write('  COMMENT "Action ${target} on ')
    out.write(CMakeStringEscape(source_abs_path))
    out.write('"\n')

    out.write('  VERBATIM)\n')

    synthetic_dependencies.add(outputs_name)


def WriteCopy(out, target, project, sources, synthetic_dependencies):
  inputs = target.properties.get('sources', [])
  raw_outputs = target.properties.get('outputs', [])

  # TODO: consider expanding 'output_patterns' instead.
  outputs = []
  for output in raw_outputs:
    output_abs_path = project.GetAbsolutePath(output)
    outputs.append(output_abs_path)
  outputs_name = '${target}__output'
  SetVariableList(out, outputs_name, outputs)

  out.write('add_custom_command(OUTPUT ')
  WriteVariable(out, outputs_name)
  out.write('\n')

  for src, dst in zip(inputs, outputs):
    abs_src_path = CMakeStringEscape(project.GetAbsolutePath(src))
    # CMake distinguishes between copying files and copying directories but
    # gn does not. We assume if the src has a period in its name then it is
    # a file and otherwise a directory.
    if "." in os.path.basename(abs_src_path):
      out.write('  COMMAND ${CMAKE_COMMAND} -E copy "')
    else:
      out.write('  COMMAND ${CMAKE_COMMAND} -E copy_directory "')
    out.write(abs_src_path)
    out.write('" "')
    out.write(CMakeStringEscape(dst))
    out.write('"\n')

  out.write('  DEPENDS ')
  for sources_type_name in sources.values():
    WriteVariable(out, sources_type_name, ' ')
  out.write('\n')

  out.write('  WORKING_DIRECTORY "')
  out.write(CMakeStringEscape(project.build_path))
  out.write('"\n')

  out.write('  COMMENT "Copy ${target}"\n')

  out.write('  VERBATIM)\n')

  synthetic_dependencies.add(outputs_name)


def WriteCompilerFlags(out, target, project, sources):
  # Hack, set linker language to c if no c or cxx files present.
  # However, cannot set LINKER_LANGUAGE on INTERFACE (with source files) until 3.19.
  if not 'c' in sources and not 'cxx' in sources and not target.cmake_type.modifier == "INTERFACE":
    SetCurrentTargetProperty(out, 'LINKER_LANGUAGE', ['C'])

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
    out.write('set_property(TARGET "${target}" ')
    out.write('APPEND PROPERTY INCLUDE_DIRECTORIES')
    for include_dir in includes:
      out.write('\n  "')
      out.write(project.GetAbsolutePath(include_dir))
      out.write('"')
    out.write(')\n')

  # Defines
  defines = target.properties.get('defines', [])
  if defines:
    SetCurrentTargetProperty(out, 'COMPILE_DEFINITIONS', defines, ';')

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
  cflags_objc = cflags_c[:]
  cflags_objc.extend(target.properties.get('cflags_objc', []))
  cflags_objcc = cflags_cxx[:]
  cflags_objcc.extend(target.properties.get('cflags_objcc', []))

  if 'c' in sources and not any(k in sources for k in ('asm', 'cxx', 'objc', 'objcc')):
    flags.extend(cflags_c)
  elif 'cxx' in sources and not any(k in sources for k in ('asm', 'c', 'objc', 'objcc')):
    flags.extend(cflags_cxx)
  elif 'objc' in sources and not any(k in sources for k in ('asm', 'c', 'cxx', 'objcc')):
    flags.extend(cflags_objc)
  elif 'objcc' in sources and not any(k in sources for k in ('asm', 'c', 'cxx', 'objc')):
    flags.extend(cflags_objcc)
  else:
    # TODO: This is broken, one cannot generally set properties on files,
    # as other targets may require different properties on the same files.
    if 'asm' in sources and cflags_asm:
      SetFilesProperty(out, sources['asm'], 'COMPILE_FLAGS', cflags_asm, ' ')
    if 'c' in sources and cflags_c:
      SetFilesProperty(out, sources['c'], 'COMPILE_FLAGS', cflags_c, ' ')
    if 'cxx' in sources and cflags_cxx:
      SetFilesProperty(out, sources['cxx'], 'COMPILE_FLAGS', cflags_cxx, ' ')
    if 'objc' in sources and cflags_objc:
      SetFilesProperty(out, sources['objc'], 'COMPILE_FLAGS', cflags_objc, ' ')
    if 'objcc' in sources and cflags_objcc:
      SetFilesProperty(out, sources['objcc'], 'COMPILE_FLAGS', cflags_objcc, ' ')
  if flags:
    SetCurrentTargetProperty(out, 'COMPILE_FLAGS', flags, ' ')

  # Linker flags
  ldflags = target.properties.get('ldflags', [])
  if ldflags:
    SetCurrentTargetProperty(out, 'LINK_FLAGS', ldflags, ' ')


gn_target_types_that_absorb_objects = (
  'executable',
  'loadable_module',
  'shared_library',
  'static_library'
)


def WriteSourceVariables(out, target, project):
  # gn separates the sheep from the goats based on file extensions.
  # A full separation is done here because of flag handing (see Compile flags).
  source_types = {'cxx':[], 'c':[], 'asm':[], 'objc':[], 'objcc':[],
                  'obj':[], 'obj_target':[], 'input':[], 'other':[]}

  all_sources = target.properties.get('sources', [])

  # As of cmake 3.11 add_library must have sources.
  # If there are no sources, add empty.cpp as the file to compile.
  # Unless it's an INTERFACE, which must not have sources until 3.19.
  if len(all_sources) == 0 and not target.cmake_type.modifier == "INTERFACE":
    all_sources.append(posixpath.join(project.build_path, 'empty.cpp'))

  # TODO .def files on Windows
  for source in all_sources:
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
    project.GetObjectSourceDependencies(target.gn_name, object_dependencies)
    for dependency in object_dependencies:
      cmake_dependency_name = project.GetCMakeTargetName(dependency)
      obj_target_sources = '$<TARGET_OBJECTS:' + cmake_dependency_name + '>'
      source_types['obj_target'].append(obj_target_sources)

  sources = {}
  for source_type, sources_of_type in source_types.items():
    if sources_of_type:
      sources[source_type] = '${target}__' + source_type + '_srcs'
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

  SetVariable(out, 'target', target.cmake_name)

  sources = WriteSourceVariables(out, target, project)

  synthetic_dependencies = set()
  if target.gn_type == 'action':
    WriteAction(out, target, project, sources, synthetic_dependencies)
  if target.gn_type == 'action_foreach':
    WriteActionForEach(out, target, project, sources, synthetic_dependencies)
  if target.gn_type == 'copy':
    WriteCopy(out, target, project, sources, synthetic_dependencies)

  out.write(target.cmake_type.command)
  out.write('("${target}"')
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

  libraries = set()
  nonlibraries = set()

  dependencies = set(target.properties.get('deps', []))
  # Transitive OBJECT libraries are in sources.
  # Those sources are dependent on the OBJECT library dependencies.
  # Those sources cannot bring in library dependencies.
  object_dependencies = set()
  if target.gn_type != 'source_set':
    project.GetObjectLibraryDependencies(target.gn_name, object_dependencies)
  for object_dependency in object_dependencies:
    dependencies.update(project.targets.get(object_dependency).get('deps', []))

  for dependency in dependencies:
    gn_dependency_type = project.targets.get(dependency, {}).get('type', None)
    cmake_dependency_type = cmake_target_types.get(gn_dependency_type, None)
    cmake_dependency_name = project.GetCMakeTargetName(dependency)
    if cmake_dependency_type.command != 'add_library':
      nonlibraries.add(cmake_dependency_name)
    elif cmake_dependency_type.modifier != 'OBJECT':
      if target.cmake_type.is_linkable:
        libraries.add(cmake_dependency_name)
      else:
        nonlibraries.add(cmake_dependency_name)

  # Non-library dependencies.
  if nonlibraries:
    out.write('add_dependencies("${target}"')
    for nonlibrary in nonlibraries:
      out.write('\n  "')
      out.write(nonlibrary)
      out.write('"')
    out.write(')\n')

  # Non-OBJECT library dependencies.
  combined_library_lists = [target.properties.get(key, []) for key in ['libs', 'frameworks']]
  external_libraries = list(itertools.chain(*combined_library_lists))
  if target.cmake_type.is_linkable and (external_libraries or libraries):
    library_dirs = target.properties.get('lib_dirs', [])
    if library_dirs:
      SetVariableList(out, '${target}__library_directories', library_dirs)

    system_libraries = []
    for external_library in external_libraries:
      if '/' in external_library:
        libraries.add(project.GetAbsolutePath(external_library))
      else:
        if external_library.endswith('.framework'):
          external_library = external_library[:-len('.framework')]
        system_library = 'library__' + external_library
        if library_dirs:
          system_library = system_library + '__for_${target}'
        out.write('find_library("')
        out.write(CMakeStringEscape(system_library))
        out.write('" "')
        out.write(CMakeStringEscape(external_library))
        out.write('"')
        if library_dirs:
          out.write(' PATHS "')
          WriteVariable(out, '${target}__library_directories')
          out.write('"')
        out.write(')\n')
        system_libraries.append(system_library)
    out.write('target_link_libraries("${target}"')
    if (target.cmake_type.modifier == "INTERFACE"):
      out.write(' INTERFACE')
    for library in libraries:
      out.write('\n  "')
      out.write(CMakeStringEscape(library))
      out.write('"')
    for system_library in system_libraries:
      WriteVariable(out, system_library, '\n  "')
      out.write('"')
    out.write(')\n')


def WriteProject(project):
  out = open(posixpath.join(project.build_path, 'CMakeLists.txt'), 'w+')
  extName = posixpath.join(project.build_path, 'CMakeLists.ext')
  out.write('# Generated by gn_to_cmake.py.\n')
  out.write('cmake_minimum_required(VERSION 3.7 FATAL_ERROR)\n')
  out.write('cmake_policy(VERSION 3.7)\n')
  out.write('project(Skia)\n\n')

  out.write('file(WRITE "')
  out.write(CMakeStringEscape(posixpath.join(project.build_path, "empty.cpp")))
  out.write('")\n')

  # Update the gn generated ninja build.
  # If a build file has changed, this will update CMakeLists.ext if
  # gn gen out/config --ide=json --json-ide-script=../../gn/gn_to_cmake.py
  # style was used to create this config.
  out.write('execute_process(COMMAND\n')
  out.write('  ninja -C "')
  out.write(CMakeStringEscape(project.build_path))
  out.write('" build.ninja\n')
  out.write('  RESULT_VARIABLE ninja_result)\n')
  out.write('if (ninja_result)\n')
  out.write('  message(WARNING ')
  out.write('"Regeneration failed running ninja: ${ninja_result}")\n')
  out.write('endif()\n')

  out.write('include("')
  out.write(CMakeStringEscape(extName))
  out.write('")\n')
  # This lets Clion find the emscripten header files when working with CanvasKit.
  out.write('include_directories(SYSTEM $ENV{EMSDK}/upstream/emscripten/system/include/)\n')
  out.close()

  out = open(extName, 'w+')
  out.write('# Generated by gn_to_cmake.py.\n')
  out.write('cmake_minimum_required(VERSION 3.7 FATAL_ERROR)\n')
  out.write('cmake_policy(VERSION 3.7)\n')

  # The following appears to be as-yet undocumented.
  # http://public.kitware.com/Bug/view.php?id=8392
  out.write('enable_language(ASM)\n\n')
  # ASM-ATT does not support .S files.
  # output.write('enable_language(ASM-ATT)\n')

  # Current issues with automatic re-generation:
  # The gn generated build.ninja target uses build.ninja.d
  #   but build.ninja.d does not contain the ide or gn.
  # Currently the ide is not run if the project.json file is not changed
  #   but the ide needs to be run anyway if it has itself changed.
  #   This can be worked around by deleting the project.json file.
  out.write('file(READ "')
  gn_deps_file = posixpath.join(project.build_path, 'build.ninja.d')
  out.write(CMakeStringEscape(gn_deps_file))
  out.write('" "gn_deps_file_content")\n')

  out.write('string(REGEX REPLACE "^[^:]*: " "" ')
  out.write('gn_deps_string ${gn_deps_file_content})\n')

  # One would think this would need to worry about escaped spaces
  # but gn doesn't escape spaces here (it generates invalid .d files).
  out.write('string(REPLACE " " ";" "gn_deps" ${gn_deps_string})\n')
  out.write('foreach("gn_dep" ${gn_deps})\n')
  out.write('  configure_file("')
  out.write(CMakeStringEscape(project.build_path))
  out.write('${gn_dep}" "CMakeLists.devnull" COPYONLY)\n')
  out.write('endforeach("gn_dep")\n')

  out.write('list(APPEND other_deps "')
  out.write(CMakeStringEscape(os.path.abspath(__file__)))
  out.write('")\n')
  out.write('foreach("other_dep" ${other_deps})\n')
  out.write('  configure_file("${other_dep}" "CMakeLists.devnull" COPYONLY)\n')
  out.write('endforeach("other_dep")\n')

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
