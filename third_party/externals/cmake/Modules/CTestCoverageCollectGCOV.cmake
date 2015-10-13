#.rst:
# CTestCoverageCollectGCOV
# ------------------------
#
# This module provides the function ``ctest_coverage_collect_gcov``.
# The function will run gcov on the .gcda files in a binary tree and then
# package all of the .gcov files into a tar file with a data.json that
# contains the source and build directories for CDash to use in parsing
# the coverage data. In addtion the Labels.json files for targets that
# have coverage information are also put in the tar file for CDash to
# asign the correct labels. This file can be sent to a CDash server for
# display with the
# :command:`ctest_submit(CDASH_UPLOAD)` command.
#
# .. command:: cdash_coverage_collect_gcov
#
#   ::
#
#     ctest_coverage_collect_gcov(TARBALL <tarfile>
#       [SOURCE <source_dir>][BUILD <build_dir>]
#       [GCOV_COMMAND <gcov_command>]
#       [GCOV_OPTIONS <options>...]
#       )
#
#   Run gcov and package a tar file for CDash.  The options are:
#
#   ``TARBALL <tarfile>``
#     Specify the location of the ``.tar`` file to be created for later
#     upload to CDash.  Relative paths will be interpreted with respect
#     to the top-level build directory.
#
#   ``SOURCE <source_dir>``
#     Specify the top-level source directory for the build.
#     Default is the value of :variable:`CTEST_SOURCE_DIRECTORY`.
#
#   ``BUILD <build_dir>``
#     Specify the top-level build directory for the build.
#     Default is the value of :variable:`CTEST_BINARY_DIRECTORY`.
#
#   ``GCOV_COMMAND <gcov_command>``
#     Specify the full path to the ``gcov`` command on the machine.
#     Default is the value of :variable:`CTEST_COVERAGE_COMMAND`.
#
#   ``GCOV_OPTIONS <options>...``
#     Specify options to be passed to gcov.  The ``gcov`` command
#     is run as ``gcov <options>... -o <gcov-dir> <file>.gcda``.
#     If not specified, the default option is just ``-b``.
#
#   ``QUIET``
#     Suppress non-error messages that otherwise would have been
#     printed out by this function.

#=============================================================================
# Copyright 2014-2015 Kitware, Inc.
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
include(CMakeParseArguments)
function(ctest_coverage_collect_gcov)
  set(options QUIET)
  set(oneValueArgs TARBALL SOURCE BUILD GCOV_COMMAND)
  set(multiValueArgs GCOV_OPTIONS)
  cmake_parse_arguments(GCOV  "${options}" "${oneValueArgs}"
    "${multiValueArgs}" "" ${ARGN} )
  if(NOT DEFINED GCOV_TARBALL)
    message(FATAL_ERROR
      "TARBALL must be specified. for ctest_coverage_collect_gcov")
  endif()
  if(NOT DEFINED GCOV_SOURCE)
    set(source_dir "${CTEST_SOURCE_DIRECTORY}")
  else()
    set(source_dir "${GCOV_SOURCE}")
  endif()
  if(NOT DEFINED GCOV_BUILD)
    set(binary_dir "${CTEST_BINARY_DIRECTORY}")
  else()
    set(binary_dir "${GCOV_BUILD}")
  endif()
  if(NOT DEFINED GCOV_GCOV_COMMAND)
    set(gcov_command "${CTEST_COVERAGE_COMMAND}")
  else()
    set(gcov_command "${GCOV_GCOV_COMMAND}")
  endif()
  # run gcov on each gcda file in the binary tree
  set(gcda_files)
  set(label_files)
  # look for gcda files in the target directories
  # could do a glob from the top of the binary tree but
  # this will be faster and only look where the files will be
  file(STRINGS "${binary_dir}/CMakeFiles/TargetDirectories.txt" target_dirs
       ENCODING UTF-8)
  foreach(target_dir ${target_dirs})
    file(GLOB_RECURSE gfiles RELATIVE ${binary_dir} "${target_dir}/*.gcda")
    list(LENGTH gfiles len)
    # if we have gcda files then also grab the labels file for that target
    if(${len} GREATER 0)
      file(GLOB_RECURSE lfiles RELATIVE ${binary_dir}
        "${target_dir}/Labels.json")
      list(APPEND gcda_files ${gfiles})
      list(APPEND label_files ${lfiles})
    endif()
  endforeach()
  # return early if no coverage files were found
  list(LENGTH gcda_files len)
  if(len EQUAL 0)
    if (NOT GCOV_QUIET)
      message("ctest_coverage_collect_gcov: No .gcda files found, "
        "ignoring coverage request.")
    endif()
    return()
  endif()
  # setup the dir for the coverage files
  set(coverage_dir "${binary_dir}/Testing/CoverageInfo")
  file(MAKE_DIRECTORY  "${coverage_dir}")
  # call gcov on each .gcda file
  foreach (gcda_file ${gcda_files})
    # get the directory of the gcda file
    get_filename_component(gcda_file ${binary_dir}/${gcda_file} ABSOLUTE)
    get_filename_component(gcov_dir ${gcda_file} DIRECTORY)
    # run gcov, this will produce the .gcov file in the current
    # working directory
    if(NOT DEFINED GCOV_GCOV_OPTIONS)
      set(GCOV_GCOV_OPTIONS -b)
    endif()
    execute_process(COMMAND
      ${gcov_command} ${GCOV_GCOV_OPTIONS} -o ${gcov_dir} ${gcda_file}
      OUTPUT_VARIABLE out
      RESULT_VARIABLE res
      WORKING_DIRECTORY ${coverage_dir})
  endforeach()
  if(NOT "${res}" EQUAL 0)
    if (NOT GCOV_QUIET)
      message(STATUS "Error running gcov: ${res} ${out}")
    endif()
  endif()
  # create json file with project information
  file(WRITE ${coverage_dir}/data.json
    "{
    \"Source\": \"${source_dir}\",
    \"Binary\": \"${binary_dir}\"
}")
  # collect the gcov files
  set(unfiltered_gcov_files)
  file(GLOB_RECURSE unfiltered_gcov_files RELATIVE ${binary_dir} "${coverage_dir}/*.gcov")

  set(gcov_files)
  foreach(gcov_file ${unfiltered_gcov_files})
    file(STRINGS ${binary_dir}/${gcov_file} first_line LIMIT_COUNT 1 ENCODING UTF-8)

    set(is_excluded false)
    if(first_line MATCHES "^        -:    0:Source:(.*)$")
      set(source_file ${CMAKE_MATCH_1})
    elseif(NOT GCOV_QUIET)
      message(STATUS "Could not determine source file corresponding to: ${gcov_file}")
    endif()

    foreach(exclude_entry ${CTEST_CUSTOM_COVERAGE_EXCLUDE})
      if(source_file MATCHES "${exclude_entry}")
        set(is_excluded true)

        if(NOT GCOV_QUIET)
          message("Excluding coverage for: ${source_file} which matches ${exclude_entry}")
        endif()

        break()
      endif()
    endforeach()

    if(NOT is_excluded)
      list(APPEND gcov_files ${gcov_file})
    endif()
  endforeach()

  # tar up the coverage info with the same date so that the md5
  # sum will be the same for the tar file independent of file time
  # stamps
  string(REPLACE ";" "\n" gcov_files "${gcov_files}")
  string(REPLACE ";" "\n" label_files "${label_files}")
  file(WRITE "${coverage_dir}/coverage_file_list.txt"
    "${gcov_files}
${coverage_dir}/data.json
${label_files}
")

  if (GCOV_QUIET)
    set(tar_opts "cfj")
  else()
    set(tar_opts "cvfj")
  endif()

  execute_process(COMMAND
    ${CMAKE_COMMAND} -E tar ${tar_opts} ${GCOV_TARBALL}
    "--mtime=1970-01-01 0:0:0 UTC"
    "--format=gnutar"
    --files-from=${coverage_dir}/coverage_file_list.txt
    WORKING_DIRECTORY ${binary_dir})
endfunction()
