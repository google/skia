#.rst:
# BundleUtilities
# ---------------
#
# Functions to help assemble a standalone bundle application.
#
# A collection of CMake utility functions useful for dealing with .app
# bundles on the Mac and bundle-like directories on any OS.
#
# The following functions are provided by this module:
#
# ::
#
#    fixup_bundle
#    copy_and_fixup_bundle
#    verify_app
#    get_bundle_main_executable
#    get_dotapp_dir
#    get_bundle_and_executable
#    get_bundle_all_executables
#    get_item_key
#    get_item_rpaths
#    clear_bundle_keys
#    set_bundle_key_values
#    get_bundle_keys
#    copy_resolved_item_into_bundle
#    copy_resolved_framework_into_bundle
#    fixup_bundle_item
#    verify_bundle_prerequisites
#    verify_bundle_symlinks
#
# Requires CMake 2.6 or greater because it uses function, break and
# PARENT_SCOPE.  Also depends on GetPrerequisites.cmake.
#
# ::
#
#   FIXUP_BUNDLE(<app> <libs> <dirs>)
#
# Fix up a bundle in-place and make it standalone, such that it can be
# drag-n-drop copied to another machine and run on that machine as long
# as all of the system libraries are compatible.
#
# If you pass plugins to fixup_bundle as the libs parameter, you should
# install them or copy them into the bundle before calling fixup_bundle.
# The "libs" parameter is a list of libraries that must be fixed up, but
# that cannot be determined by otool output analysis.  (i.e., plugins)
#
# Gather all the keys for all the executables and libraries in a bundle,
# and then, for each key, copy each prerequisite into the bundle.  Then
# fix each one up according to its own list of prerequisites.
#
# Then clear all the keys and call verify_app on the final bundle to
# ensure that it is truly standalone.
#
# ::
#
#   COPY_AND_FIXUP_BUNDLE(<src> <dst> <libs> <dirs>)
#
# Makes a copy of the bundle <src> at location <dst> and then fixes up
# the new copied bundle in-place at <dst>...
#
# ::
#
#   VERIFY_APP(<app>)
#
# Verifies that an application <app> appears valid based on running
# analysis tools on it.  Calls "message(FATAL_ERROR" if the application
# is not verified.
#
# ::
#
#   GET_BUNDLE_MAIN_EXECUTABLE(<bundle> <result_var>)
#
# The result will be the full path name of the bundle's main executable
# file or an "error:" prefixed string if it could not be determined.
#
# ::
#
#   GET_DOTAPP_DIR(<exe> <dotapp_dir_var>)
#
# Returns the nearest parent dir whose name ends with ".app" given the
# full path to an executable.  If there is no such parent dir, then
# simply return the dir containing the executable.
#
# The returned directory may or may not exist.
#
# ::
#
#   GET_BUNDLE_AND_EXECUTABLE(<app> <bundle_var> <executable_var> <valid_var>)
#
# Takes either a ".app" directory name or the name of an executable
# nested inside a ".app" directory and returns the path to the ".app"
# directory in <bundle_var> and the path to its main executable in
# <executable_var>
#
# ::
#
#   GET_BUNDLE_ALL_EXECUTABLES(<bundle> <exes_var>)
#
# Scans the given bundle recursively for all executable files and
# accumulates them into a variable.
#
# ::
#
#   GET_ITEM_KEY(<item> <key_var>)
#
# Given a file (item) name, generate a key that should be unique
# considering the set of libraries that need copying or fixing up to
# make a bundle standalone.  This is essentially the file name including
# extension with "." replaced by "_"
#
# This key is used as a prefix for CMake variables so that we can
# associate a set of variables with a given item based on its key.
#
# ::
#
#   CLEAR_BUNDLE_KEYS(<keys_var>)
#
# Loop over the list of keys, clearing all the variables associated with
# each key.  After the loop, clear the list of keys itself.
#
# Caller of get_bundle_keys should call clear_bundle_keys when done with
# list of keys.
#
# ::
#
#   SET_BUNDLE_KEY_VALUES(<keys_var> <context> <item> <exepath> <dirs>
#                         <copyflag> [<rpaths>])
#
# Add a key to the list (if necessary) for the given item.  If added,
# also set all the variables associated with that key.
#
# ::
#
#   GET_BUNDLE_KEYS(<app> <libs> <dirs> <keys_var>)
#
# Loop over all the executable and library files within the bundle (and
# given as extra <libs>) and accumulate a list of keys representing
# them.  Set values associated with each key such that we can loop over
# all of them and copy prerequisite libs into the bundle and then do
# appropriate install_name_tool fixups.
#
# ::
#
#   COPY_RESOLVED_ITEM_INTO_BUNDLE(<resolved_item> <resolved_embedded_item>)
#
# Copy a resolved item into the bundle if necessary.  Copy is not
# necessary if the resolved_item is "the same as" the
# resolved_embedded_item.
#
# ::
#
#   COPY_RESOLVED_FRAMEWORK_INTO_BUNDLE(<resolved_item> <resolved_embedded_item>)
#
# Copy a resolved framework into the bundle if necessary.  Copy is not
# necessary if the resolved_item is "the same as" the
# resolved_embedded_item.
#
# By default, BU_COPY_FULL_FRAMEWORK_CONTENTS is not set.  If you want
# full frameworks embedded in your bundles, set
# BU_COPY_FULL_FRAMEWORK_CONTENTS to ON before calling fixup_bundle.  By
# default, COPY_RESOLVED_FRAMEWORK_INTO_BUNDLE copies the framework
# dylib itself plus the framework Resources directory.
#
# ::
#
#   FIXUP_BUNDLE_ITEM(<resolved_embedded_item> <exepath> <dirs>)
#
# Get the direct/non-system prerequisites of the resolved embedded item.
# For each prerequisite, change the way it is referenced to the value of
# the _EMBEDDED_ITEM keyed variable for that prerequisite.  (Most likely
# changing to an "@executable_path" style reference.)
#
# This function requires that the resolved_embedded_item be "inside" the
# bundle already.  In other words, if you pass plugins to fixup_bundle
# as the libs parameter, you should install them or copy them into the
# bundle before calling fixup_bundle.  The "libs" parameter is a list of
# libraries that must be fixed up, but that cannot be determined by
# otool output analysis.  (i.e., plugins)
#
# Also, change the id of the item being fixed up to its own
# _EMBEDDED_ITEM value.
#
# Accumulate changes in a local variable and make *one* call to
# install_name_tool at the end of the function with all the changes at
# once.
#
# If the BU_CHMOD_BUNDLE_ITEMS variable is set then bundle items will be
# marked writable before install_name_tool tries to change them.
#
# ::
#
#   VERIFY_BUNDLE_PREREQUISITES(<bundle> <result_var> <info_var>)
#
# Verifies that the sum of all prerequisites of all files inside the
# bundle are contained within the bundle or are "system" libraries,
# presumed to exist everywhere.
#
# ::
#
#   VERIFY_BUNDLE_SYMLINKS(<bundle> <result_var> <info_var>)
#
# Verifies that any symlinks found in the bundle point to other files
# that are already also in the bundle...  Anything that points to an
# external file causes this function to fail the verification.

#=============================================================================
# Copyright 2008-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# The functions defined in this file depend on the get_prerequisites function
# (and possibly others) found in:
#
get_filename_component(BundleUtilities_cmake_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${BundleUtilities_cmake_dir}/GetPrerequisites.cmake")


function(get_bundle_main_executable bundle result_var)
  set(result "error: '${bundle}/Contents/Info.plist' file does not exist")

  if(EXISTS "${bundle}/Contents/Info.plist")
    set(result "error: no CFBundleExecutable in '${bundle}/Contents/Info.plist' file")
    set(line_is_main_executable 0)
    set(bundle_executable "")

    # Read Info.plist as a list of lines:
    #
    set(eol_char "E")
    file(READ "${bundle}/Contents/Info.plist" info_plist)
    string(REPLACE ";" "\\;" info_plist "${info_plist}")
    string(REPLACE "\n" "${eol_char};" info_plist "${info_plist}")
    string(REPLACE "\r" "${eol_char};" info_plist "${info_plist}")

    # Scan the lines for "<key>CFBundleExecutable</key>" - the line after that
    # is the name of the main executable.
    #
    foreach(line ${info_plist})
      if(line_is_main_executable)
        string(REGEX REPLACE "^.*<string>(.*)</string>.*$" "\\1" bundle_executable "${line}")
        break()
      endif()

      if(line MATCHES "<key>CFBundleExecutable</key>")
        set(line_is_main_executable 1)
      endif()
    endforeach()

    if(NOT "${bundle_executable}" STREQUAL "")
      if(EXISTS "${bundle}/Contents/MacOS/${bundle_executable}")
        set(result "${bundle}/Contents/MacOS/${bundle_executable}")
      else()

        # Ultimate goal:
        # If not in "Contents/MacOS" then scan the bundle for matching files. If
        # there is only one executable file that matches, then use it, otherwise
        # it's an error...
        #
        #file(GLOB_RECURSE file_list "${bundle}/${bundle_executable}")

        # But for now, pragmatically, it's an error. Expect the main executable
        # for the bundle to be in Contents/MacOS, it's an error if it's not:
        #
        set(result "error: '${bundle}/Contents/MacOS/${bundle_executable}' does not exist")
      endif()
    endif()
  else()
    #
    # More inclusive technique... (This one would work on Windows and Linux
    # too, if a developer followed the typical Mac bundle naming convention...)
    #
    # If there is no Info.plist file, try to find an executable with the same
    # base name as the .app directory:
    #
  endif()

  set(${result_var} "${result}" PARENT_SCOPE)
endfunction()


function(get_dotapp_dir exe dotapp_dir_var)
  set(s "${exe}")

  if(s MATCHES "/.*\\.app/")
    # If there is a ".app" parent directory,
    # ascend until we hit it:
    #   (typical of a Mac bundle executable)
    #
    set(done 0)
    while(NOT ${done})
      get_filename_component(snamewe "${s}" NAME_WE)
      get_filename_component(sname "${s}" NAME)
      get_filename_component(sdir "${s}" PATH)
      set(s "${sdir}")
      if(sname MATCHES "\\.app$")
        set(done 1)
        set(dotapp_dir "${sdir}/${sname}")
      endif()
    endwhile()
  else()
    # Otherwise use a directory containing the exe
    #   (typical of a non-bundle executable on Mac, Windows or Linux)
    #
    is_file_executable("${s}" is_executable)
    if(is_executable)
      get_filename_component(sdir "${s}" PATH)
      set(dotapp_dir "${sdir}")
    else()
      set(dotapp_dir "${s}")
    endif()
  endif()


  set(${dotapp_dir_var} "${dotapp_dir}" PARENT_SCOPE)
endfunction()


function(get_bundle_and_executable app bundle_var executable_var valid_var)
  set(valid 0)

  if(EXISTS "${app}")
    # Is it a directory ending in .app?
    if(IS_DIRECTORY "${app}")
      if(app MATCHES "\\.app$")
        get_bundle_main_executable("${app}" executable)
        if(EXISTS "${app}" AND EXISTS "${executable}")
          set(${bundle_var} "${app}" PARENT_SCOPE)
          set(${executable_var} "${executable}" PARENT_SCOPE)
          set(valid 1)
          #message(STATUS "info: handled .app directory case...")
        else()
          message(STATUS "warning: *NOT* handled - .app directory case...")
        endif()
      else()
        message(STATUS "warning: *NOT* handled - directory but not .app case...")
      endif()
    else()
      # Is it an executable file?
      is_file_executable("${app}" is_executable)
      if(is_executable)
        get_dotapp_dir("${app}" dotapp_dir)
        if(EXISTS "${dotapp_dir}")
          set(${bundle_var} "${dotapp_dir}" PARENT_SCOPE)
          set(${executable_var} "${app}" PARENT_SCOPE)
          set(valid 1)
          #message(STATUS "info: handled executable file in .app dir case...")
        else()
          get_filename_component(app_dir "${app}" PATH)
          set(${bundle_var} "${app_dir}" PARENT_SCOPE)
          set(${executable_var} "${app}" PARENT_SCOPE)
          set(valid 1)
          #message(STATUS "info: handled executable file in any dir case...")
        endif()
      else()
        message(STATUS "warning: *NOT* handled - not .app dir, not executable file...")
      endif()
    endif()
  else()
    message(STATUS "warning: *NOT* handled - directory/file does not exist...")
  endif()

  if(NOT valid)
    set(${bundle_var} "error: not a bundle" PARENT_SCOPE)
    set(${executable_var} "error: not a bundle" PARENT_SCOPE)
  endif()

  set(${valid_var} ${valid} PARENT_SCOPE)
endfunction()


function(get_bundle_all_executables bundle exes_var)
  set(exes "")

  if(UNIX)
    find_program(find_cmd "find")
    mark_as_advanced(find_cmd)
  endif()

  # find command is much quicker than checking every file one by one on Unix
  # which can take long time for large bundles, and since anyway we expect
  # executable to have execute flag set we can narrow the list much quicker.
  if(find_cmd)
    execute_process(COMMAND "${find_cmd}" "${bundle}"
      -type f \( -perm -0100 -o -perm -0010 -o -perm -0001 \)
      OUTPUT_VARIABLE file_list
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    string(REPLACE "\n" ";" file_list "${file_list}")
  else()
    file(GLOB_RECURSE file_list "${bundle}/*")
  endif()

  foreach(f ${file_list})
    is_file_executable("${f}" is_executable)
    if(is_executable)
      set(exes ${exes} "${f}")
    endif()
  endforeach()

  set(${exes_var} "${exes}" PARENT_SCOPE)
endfunction()


function(get_item_rpaths item rpaths_var)
  if(APPLE)
    find_program(otool_cmd "otool")
    mark_as_advanced(otool_cmd)
  endif()

  if(otool_cmd)
    execute_process(
      COMMAND "${otool_cmd}" -l "${item}"
      OUTPUT_VARIABLE load_cmds_ov
      )
    string(REGEX REPLACE "[^\n]+cmd LC_RPATH\n[^\n]+\n[^\n]+path ([^\n]+) \\(offset[^\n]+\n" "rpath \\1\n" load_cmds_ov "${load_cmds_ov}")
    string(REGEX MATCHALL "rpath [^\n]+" load_cmds_ov "${load_cmds_ov}")
    string(REGEX REPLACE "rpath " "" load_cmds_ov "${load_cmds_ov}")
    if(load_cmds_ov)
      gp_append_unique(${rpaths_var} "${load_cmds_ov}")
    endif()
  endif()

  set(${rpaths_var} ${${rpaths_var}} PARENT_SCOPE)
endfunction()


function(get_item_key item key_var)
  get_filename_component(item_name "${item}" NAME)
  if(WIN32)
    string(TOLOWER "${item_name}" item_name)
  endif()
  string(REPLACE "." "_" ${key_var} "${item_name}")
  set(${key_var} ${${key_var}} PARENT_SCOPE)
endfunction()


function(clear_bundle_keys keys_var)
  foreach(key ${${keys_var}})
    set(${key}_ITEM PARENT_SCOPE)
    set(${key}_RESOLVED_ITEM PARENT_SCOPE)
    set(${key}_DEFAULT_EMBEDDED_PATH PARENT_SCOPE)
    set(${key}_EMBEDDED_ITEM PARENT_SCOPE)
    set(${key}_RESOLVED_EMBEDDED_ITEM PARENT_SCOPE)
    set(${key}_COPYFLAG PARENT_SCOPE)
    set(${key}_RPATHS PARENT_SCOPE)
  endforeach()
  set(${keys_var} PARENT_SCOPE)
endfunction()


function(set_bundle_key_values keys_var context item exepath dirs copyflag)
  if(ARGC GREATER 6)
    set(rpaths "${ARGV6}")
  else()
    set(rpaths "")
  endif()
  get_filename_component(item_name "${item}" NAME)

  get_item_key("${item}" key)

  list(LENGTH ${keys_var} length_before)
  gp_append_unique(${keys_var} "${key}")
  list(LENGTH ${keys_var} length_after)

  if(NOT length_before EQUAL length_after)
    gp_resolve_item("${context}" "${item}" "${exepath}" "${dirs}" resolved_item "${rpaths}")

    gp_item_default_embedded_path("${item}" default_embedded_path)

    get_item_rpaths("${resolved_item}" item_rpaths)

    if(item MATCHES "[^/]+\\.framework/")
      # For frameworks, construct the name under the embedded path from the
      # opening "${item_name}.framework/" to the closing "/${item_name}":
      #
      string(REGEX REPLACE "^.*(${item_name}.framework/.*/?${item_name}).*$" "${default_embedded_path}/\\1" embedded_item "${item}")
    else()
      # For other items, just use the same name as the original, but in the
      # embedded path:
      #
      set(embedded_item "${default_embedded_path}/${item_name}")
    endif()

    # Replace @executable_path and resolve ".." references:
    #
    string(REPLACE "@executable_path" "${exepath}" resolved_embedded_item "${embedded_item}")
    get_filename_component(resolved_embedded_item "${resolved_embedded_item}" ABSOLUTE)

    # *But* -- if we are not copying, then force resolved_embedded_item to be
    # the same as resolved_item. In the case of multiple executables in the
    # original bundle, using the default_embedded_path results in looking for
    # the resolved executable next to the main bundle executable. This is here
    # so that exes in the other sibling directories (like "bin") get fixed up
    # properly...
    #
    if(NOT copyflag)
      set(resolved_embedded_item "${resolved_item}")
    endif()

    set(${keys_var} ${${keys_var}} PARENT_SCOPE)
    set(${key}_ITEM "${item}" PARENT_SCOPE)
    set(${key}_RESOLVED_ITEM "${resolved_item}" PARENT_SCOPE)
    set(${key}_DEFAULT_EMBEDDED_PATH "${default_embedded_path}" PARENT_SCOPE)
    set(${key}_EMBEDDED_ITEM "${embedded_item}" PARENT_SCOPE)
    set(${key}_RESOLVED_EMBEDDED_ITEM "${resolved_embedded_item}" PARENT_SCOPE)
    set(${key}_COPYFLAG "${copyflag}" PARENT_SCOPE)
    set(${key}_RPATHS "${item_rpaths}" PARENT_SCOPE)
    set(${key}_RDEP_RPATHS "${rpaths}" PARENT_SCOPE)
  else()
    #message("warning: item key '${key}' already in the list, subsequent references assumed identical to first")
  endif()
endfunction()


function(get_bundle_keys app libs dirs keys_var)
  set(${keys_var} PARENT_SCOPE)

  get_bundle_and_executable("${app}" bundle executable valid)
  if(valid)
    # Always use the exepath of the main bundle executable for @executable_path
    # replacements:
    #
    get_filename_component(exepath "${executable}" PATH)

    # But do fixups on all executables in the bundle:
    #
    get_bundle_all_executables("${bundle}" exes)

    # Set keys for main executable first:
    #
    set_bundle_key_values(${keys_var} "${executable}" "${executable}" "${exepath}" "${dirs}" 0)

    # Get rpaths specified by main executable:
    #
    get_item_key("${executable}" executable_key)
    set(main_rpaths "${${executable_key}_RPATHS}")

    # For each extra lib, accumulate a key as well and then also accumulate
    # any of its prerequisites. (Extra libs are typically dynamically loaded
    # plugins: libraries that are prerequisites for full runtime functionality
    # but that do not show up in otool -L output...)
    #
    foreach(lib ${libs})
      set_bundle_key_values(${keys_var} "${lib}" "${lib}" "${exepath}" "${dirs}" 0 "${main_rpaths}")

      set(prereqs "")
      get_prerequisites("${lib}" prereqs 1 1 "${exepath}" "${dirs}" "${main_rpaths}")
      foreach(pr ${prereqs})
        set_bundle_key_values(${keys_var} "${lib}" "${pr}" "${exepath}" "${dirs}" 1 "${main_rpaths}")
      endforeach()
    endforeach()

    # For each executable found in the bundle, accumulate keys as we go.
    # The list of keys should be complete when all prerequisites of all
    # binaries in the bundle have been analyzed.
    #
    foreach(exe ${exes})
      # Main executable is scanned first above:
      #
      if(NOT "${exe}" STREQUAL "${executable}")
        # Add the exe itself to the keys:
        #
        set_bundle_key_values(${keys_var} "${exe}" "${exe}" "${exepath}" "${dirs}" 0 "${main_rpaths}")

        # Get rpaths specified by executable:
        #
        get_item_key("${exe}" exe_key)
        set(exe_rpaths "${main_rpaths}" "${${exe_key}_RPATHS}")
      else()
        set(exe_rpaths "${main_rpaths}")
      endif()

      # Add each prerequisite to the keys:
      #
      set(prereqs "")
      get_prerequisites("${exe}" prereqs 1 1 "${exepath}" "${dirs}" "${exe_rpaths}")
      foreach(pr ${prereqs})
        set_bundle_key_values(${keys_var} "${exe}" "${pr}" "${exepath}" "${dirs}" 1 "${exe_rpaths}")
      endforeach()
    endforeach()

    # Propagate values to caller's scope:
    #
    set(${keys_var} ${${keys_var}} PARENT_SCOPE)
    foreach(key ${${keys_var}})
      set(${key}_ITEM "${${key}_ITEM}" PARENT_SCOPE)
      set(${key}_RESOLVED_ITEM "${${key}_RESOLVED_ITEM}" PARENT_SCOPE)
      set(${key}_DEFAULT_EMBEDDED_PATH "${${key}_DEFAULT_EMBEDDED_PATH}" PARENT_SCOPE)
      set(${key}_EMBEDDED_ITEM "${${key}_EMBEDDED_ITEM}" PARENT_SCOPE)
      set(${key}_RESOLVED_EMBEDDED_ITEM "${${key}_RESOLVED_EMBEDDED_ITEM}" PARENT_SCOPE)
      set(${key}_COPYFLAG "${${key}_COPYFLAG}" PARENT_SCOPE)
      set(${key}_RPATHS "${${key}_RPATHS}" PARENT_SCOPE)
      set(${key}_RDEP_RPATHS "${${key}_RDEP_RPATHS}" PARENT_SCOPE)
    endforeach()
  endif()
endfunction()


function(copy_resolved_item_into_bundle resolved_item resolved_embedded_item)
  if(WIN32)
    # ignore case on Windows
    string(TOLOWER "${resolved_item}" resolved_item_compare)
    string(TOLOWER "${resolved_embedded_item}" resolved_embedded_item_compare)
  else()
    set(resolved_item_compare "${resolved_item}")
    set(resolved_embedded_item_compare "${resolved_embedded_item}")
  endif()

  if("${resolved_item_compare}" STREQUAL "${resolved_embedded_item_compare}")
    message(STATUS "warning: resolved_item == resolved_embedded_item - not copying...")
  else()
    #message(STATUS "copying COMMAND ${CMAKE_COMMAND} -E copy ${resolved_item} ${resolved_embedded_item}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${resolved_item}" "${resolved_embedded_item}")
    if(UNIX AND NOT APPLE)
      file(RPATH_REMOVE FILE "${resolved_embedded_item}")
    endif()
  endif()

endfunction()


function(copy_resolved_framework_into_bundle resolved_item resolved_embedded_item)
  if(WIN32)
    # ignore case on Windows
    string(TOLOWER "${resolved_item}" resolved_item_compare)
    string(TOLOWER "${resolved_embedded_item}" resolved_embedded_item_compare)
  else()
    set(resolved_item_compare "${resolved_item}")
    set(resolved_embedded_item_compare "${resolved_embedded_item}")
  endif()

  if("${resolved_item_compare}" STREQUAL "${resolved_embedded_item_compare}")
    message(STATUS "warning: resolved_item == resolved_embedded_item - not copying...")
  else()
    if(BU_COPY_FULL_FRAMEWORK_CONTENTS)
      # Full Framework (everything):
      get_filename_component(resolved_dir "${resolved_item}" PATH)
      get_filename_component(resolved_dir "${resolved_dir}/../.." ABSOLUTE)
      get_filename_component(resolved_embedded_dir "${resolved_embedded_item}" PATH)
      get_filename_component(resolved_embedded_dir "${resolved_embedded_dir}/../.." ABSOLUTE)
      #message(STATUS "copying COMMAND ${CMAKE_COMMAND} -E copy_directory '${resolved_dir}' '${resolved_embedded_dir}'")
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${resolved_dir}" "${resolved_embedded_dir}")
    else()
      # Framework lib itself:
      #message(STATUS "copying COMMAND ${CMAKE_COMMAND} -E copy ${resolved_item} ${resolved_embedded_item}")
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${resolved_item}" "${resolved_embedded_item}")

      # Plus Resources, if they exist:
      string(REGEX REPLACE "^(.*)/[^/]+$" "\\1/Resources" resolved_resources "${resolved_item}")
      string(REGEX REPLACE "^(.*)/[^/]+$" "\\1/Resources" resolved_embedded_resources "${resolved_embedded_item}")
      if(EXISTS "${resolved_resources}")
        #message(STATUS "copying COMMAND ${CMAKE_COMMAND} -E copy_directory '${resolved_resources}' '${resolved_embedded_resources}'")
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${resolved_resources}" "${resolved_embedded_resources}")
      endif()

      # Some frameworks e.g. Qt put Info.plist in wrong place, so when it is
      # missing in resources, copy it from other well known incorrect locations:
      if(NOT EXISTS "${resolved_resources}/Info.plist")
        # Check for Contents/Info.plist in framework root (older Qt SDK):
        string(REGEX REPLACE "^(.*)/[^/]+/[^/]+/[^/]+$" "\\1/Contents/Info.plist" resolved_info_plist "${resolved_item}")
        string(REGEX REPLACE "^(.*)/[^/]+$" "\\1/Resources/Info.plist" resolved_embedded_info_plist "${resolved_embedded_item}")
        if(EXISTS "${resolved_info_plist}")
          #message(STATUS "copying COMMAND ${CMAKE_COMMAND} -E copy_directory '${resolved_info_plist}' '${resolved_embedded_info_plist}'")
          execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${resolved_info_plist}" "${resolved_embedded_info_plist}")
        endif()
      endif()

      # Check if framework is versioned and fix it layout
      string(REGEX REPLACE "^.*/([^/]+)/[^/]+$" "\\1" resolved_embedded_version "${resolved_embedded_item}")
      string(REGEX REPLACE "^(.*)/[^/]+/[^/]+$" "\\1" resolved_embedded_versions "${resolved_embedded_item}")
      string(REGEX REPLACE "^.*/([^/]+)/[^/]+/[^/]+$" "\\1" resolved_embedded_versions_basename "${resolved_embedded_item}")
      if(resolved_embedded_versions_basename STREQUAL "Versions")
        # Ensure Current symlink points to the framework version
        if(NOT EXISTS "${resolved_embedded_versions}/Current")
          execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${resolved_embedded_version}" "${resolved_embedded_versions}/Current")
        endif()
        # Restore symlinks in framework root pointing to current framework
        # binary and resources:
        string(REGEX REPLACE "^(.*)/[^/]+/[^/]+/[^/]+$" "\\1" resolved_embedded_root "${resolved_embedded_item}")
        string(REGEX REPLACE "^.*/([^/]+)$" "\\1" resolved_embedded_item_basename "${resolved_embedded_item}")
        if(NOT EXISTS "${resolved_embedded_root}/${resolved_embedded_item_basename}")
          execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "Versions/Current/${resolved_embedded_item_basename}" "${resolved_embedded_root}/${resolved_embedded_item_basename}")
        endif()
        if(NOT EXISTS "${resolved_embedded_root}/Resources")
          execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "Versions/Current/Resources" "${resolved_embedded_root}/Resources")
        endif()
      endif()
    endif()
    if(UNIX AND NOT APPLE)
      file(RPATH_REMOVE FILE "${resolved_embedded_item}")
    endif()
  endif()

endfunction()


function(fixup_bundle_item resolved_embedded_item exepath dirs)
  # This item's key is "ikey":
  #
  get_item_key("${resolved_embedded_item}" ikey)

  # Ensure the item is "inside the .app bundle" -- it should not be fixed up if
  # it is not in the .app bundle... Otherwise, we'll modify files in the build
  # tree, or in other varied locations around the file system, with our call to
  # install_name_tool. Make sure that doesn't happen here:
  #
  get_dotapp_dir("${exepath}" exe_dotapp_dir)
  string(LENGTH "${exe_dotapp_dir}/" exe_dotapp_dir_length)
  string(LENGTH "${resolved_embedded_item}" resolved_embedded_item_length)
  set(path_too_short 0)
  set(is_embedded 0)
  if(${resolved_embedded_item_length} LESS ${exe_dotapp_dir_length})
    set(path_too_short 1)
  endif()
  if(NOT path_too_short)
    string(SUBSTRING "${resolved_embedded_item}" 0 ${exe_dotapp_dir_length} item_substring)
    if("${exe_dotapp_dir}/" STREQUAL "${item_substring}")
      set(is_embedded 1)
    endif()
  endif()
  if(NOT is_embedded)
    message("  exe_dotapp_dir/='${exe_dotapp_dir}/'")
    message("  item_substring='${item_substring}'")
    message("  resolved_embedded_item='${resolved_embedded_item}'")
    message("")
    message("Install or copy the item into the bundle before calling fixup_bundle.")
    message("Or maybe there's a typo or incorrect path in one of the args to fixup_bundle?")
    message("")
    message(FATAL_ERROR "cannot fixup an item that is not in the bundle...")
  endif()

  set(rpaths "${${ikey}_RPATHS}" "${${ikey}_RDEP_RPATHS}")

  set(prereqs "")
  get_prerequisites("${resolved_embedded_item}" prereqs 1 0 "${exepath}" "${dirs}" "${rpaths}")

  set(changes "")

  foreach(pr ${prereqs})
    # Each referenced item's key is "rkey" in the loop:
    #
    get_item_key("${pr}" rkey)

    if(NOT "${${rkey}_EMBEDDED_ITEM}" STREQUAL "")
      set(changes ${changes} "-change" "${pr}" "${${rkey}_EMBEDDED_ITEM}")
    else()
      message("warning: unexpected reference to '${pr}'")
    endif()
  endforeach()

  if(BU_CHMOD_BUNDLE_ITEMS)
    execute_process(COMMAND chmod u+w "${resolved_embedded_item}")
  endif()

  # Only if install_name_tool supports -delete_rpath:
  #
  execute_process(COMMAND install_name_tool
    OUTPUT_VARIABLE install_name_tool_usage
    ERROR_VARIABLE  install_name_tool_usage
    )
  if(install_name_tool_usage MATCHES ".*-delete_rpath.*")
    foreach(rpath ${${ikey}_RPATHS})
      set(changes ${changes} -delete_rpath "${rpath}")
    endforeach()
  endif()

  if(${ikey}_EMBEDDED_ITEM)
    set(changes ${changes} -id "${${ikey}_EMBEDDED_ITEM}")
  endif()

  # Change this item's id and all of its references in one call
  # to install_name_tool:
  #
  if(changes)
    set(cmd install_name_tool ${changes} "${resolved_embedded_item}")
    execute_process(COMMAND ${cmd} RESULT_VARIABLE install_name_tool_result)
    if(NOT install_name_tool_result EQUAL 0)
      string(REPLACE ";" "' '" msg "'${cmd}'")
      message(FATAL_ERROR "Command failed:\n ${msg}")
    endif()
  endif()
endfunction()


function(fixup_bundle app libs dirs)
  message(STATUS "fixup_bundle")
  message(STATUS "  app='${app}'")
  message(STATUS "  libs='${libs}'")
  message(STATUS "  dirs='${dirs}'")

  get_bundle_and_executable("${app}" bundle executable valid)
  if(valid)
    get_filename_component(exepath "${executable}" PATH)

    message(STATUS "fixup_bundle: preparing...")
    get_bundle_keys("${app}" "${libs}" "${dirs}" keys)

    message(STATUS "fixup_bundle: copying...")
    list(LENGTH keys n)
    math(EXPR n ${n}*2)

    set(i 0)
    foreach(key ${keys})
      math(EXPR i ${i}+1)
      if(${${key}_COPYFLAG})
        message(STATUS "${i}/${n}: copying '${${key}_RESOLVED_ITEM}'")
      else()
        message(STATUS "${i}/${n}: *NOT* copying '${${key}_RESOLVED_ITEM}'")
      endif()

      set(show_status 0)
      if(show_status)
        message(STATUS "key='${key}'")
        message(STATUS "item='${${key}_ITEM}'")
        message(STATUS "resolved_item='${${key}_RESOLVED_ITEM}'")
        message(STATUS "default_embedded_path='${${key}_DEFAULT_EMBEDDED_PATH}'")
        message(STATUS "embedded_item='${${key}_EMBEDDED_ITEM}'")
        message(STATUS "resolved_embedded_item='${${key}_RESOLVED_EMBEDDED_ITEM}'")
        message(STATUS "copyflag='${${key}_COPYFLAG}'")
        message(STATUS "")
      endif()

      if(${${key}_COPYFLAG})
        set(item "${${key}_ITEM}")
        if(item MATCHES "[^/]+\\.framework/")
          copy_resolved_framework_into_bundle("${${key}_RESOLVED_ITEM}"
            "${${key}_RESOLVED_EMBEDDED_ITEM}")
        else()
          copy_resolved_item_into_bundle("${${key}_RESOLVED_ITEM}"
            "${${key}_RESOLVED_EMBEDDED_ITEM}")
        endif()
      endif()
    endforeach()

    message(STATUS "fixup_bundle: fixing...")
    foreach(key ${keys})
      math(EXPR i ${i}+1)
      if(APPLE)
        message(STATUS "${i}/${n}: fixing up '${${key}_RESOLVED_EMBEDDED_ITEM}'")
        fixup_bundle_item("${${key}_RESOLVED_EMBEDDED_ITEM}" "${exepath}" "${dirs}")
      else()
        message(STATUS "${i}/${n}: fix-up not required on this platform '${${key}_RESOLVED_EMBEDDED_ITEM}'")
      endif()
    endforeach()

    message(STATUS "fixup_bundle: cleaning up...")
    clear_bundle_keys(keys)

    message(STATUS "fixup_bundle: verifying...")
    verify_app("${app}")
  else()
    message(SEND_ERROR "error: fixup_bundle: not a valid bundle")
  endif()

  message(STATUS "fixup_bundle: done")
endfunction()


function(copy_and_fixup_bundle src dst libs dirs)
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${src}" "${dst}")
  fixup_bundle("${dst}" "${libs}" "${dirs}")
endfunction()


function(verify_bundle_prerequisites bundle result_var info_var)
  set(result 1)
  set(info "")
  set(count 0)

  get_bundle_main_executable("${bundle}" main_bundle_exe)

  get_bundle_all_executables("${bundle}" file_list)
  foreach(f ${file_list})
      get_filename_component(exepath "${f}" PATH)
      math(EXPR count "${count} + 1")

      message(STATUS "executable file ${count}: ${f}")

      set(prereqs "")
      get_prerequisites("${f}" prereqs 1 1 "${exepath}" "")

      # On the Mac,
      # "embedded" and "system" prerequisites are fine... anything else means
      # the bundle's prerequisites are not verified (i.e., the bundle is not
      # really "standalone")
      #
      # On Windows (and others? Linux/Unix/...?)
      # "local" and "system" prereqs are fine...
      #
      set(external_prereqs "")

      foreach(p ${prereqs})
        set(p_type "")
        gp_file_type("${f}" "${p}" p_type)

        if(APPLE)
          if(NOT "${p_type}" STREQUAL "embedded" AND NOT "${p_type}" STREQUAL "system")
            set(external_prereqs ${external_prereqs} "${p}")
          endif()
        else()
          if(NOT "${p_type}" STREQUAL "local" AND NOT "${p_type}" STREQUAL "system")
            set(external_prereqs ${external_prereqs} "${p}")
          endif()
        endif()
      endforeach()

      if(external_prereqs)
        # Found non-system/somehow-unacceptable prerequisites:
        set(result 0)
        set(info ${info} "external prerequisites found:\nf='${f}'\nexternal_prereqs='${external_prereqs}'\n")
      endif()
  endforeach()

  if(result)
    set(info "Verified ${count} executable files in '${bundle}'")
  endif()

  set(${result_var} "${result}" PARENT_SCOPE)
  set(${info_var} "${info}" PARENT_SCOPE)
endfunction()


function(verify_bundle_symlinks bundle result_var info_var)
  set(result 1)
  set(info "")
  set(count 0)

  # TODO: implement this function for real...
  # Right now, it is just a stub that verifies unconditionally...

  set(${result_var} "${result}" PARENT_SCOPE)
  set(${info_var} "${info}" PARENT_SCOPE)
endfunction()


function(verify_app app)
  set(verified 0)
  set(info "")

  get_bundle_and_executable("${app}" bundle executable valid)

  message(STATUS "===========================================================================")
  message(STATUS "Analyzing app='${app}'")
  message(STATUS "bundle='${bundle}'")
  message(STATUS "executable='${executable}'")
  message(STATUS "valid='${valid}'")

  # Verify that the bundle does not have any "external" prerequisites:
  #
  verify_bundle_prerequisites("${bundle}" verified info)
  message(STATUS "verified='${verified}'")
  message(STATUS "info='${info}'")
  message(STATUS "")

  if(verified)
    # Verify that the bundle does not have any symlinks to external files:
    #
    verify_bundle_symlinks("${bundle}" verified info)
    message(STATUS "verified='${verified}'")
    message(STATUS "info='${info}'")
    message(STATUS "")
  endif()

  if(NOT verified)
    message(FATAL_ERROR "error: verify_app failed")
  endif()
endfunction()
