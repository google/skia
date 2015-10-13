.. cmake-manual-description: CMake Language Command Reference

cmake-commands(7)
*****************

.. only:: html

   .. contents::

Normal Commands
===============

These commands may be used freely in CMake projects.

.. toctree::
   :maxdepth: 1

   /command/add_compile_options
   /command/add_custom_command
   /command/add_custom_target
   /command/add_definitions
   /command/add_dependencies
   /command/add_executable
   /command/add_library
   /command/add_subdirectory
   /command/add_test
   /command/aux_source_directory
   /command/break
   /command/build_command
   /command/cmake_host_system_information
   /command/cmake_minimum_required
   /command/cmake_policy
   /command/configure_file
   /command/continue
   /command/create_test_sourcelist
   /command/define_property
   /command/elseif
   /command/else
   /command/enable_language
   /command/enable_testing
   /command/endforeach
   /command/endfunction
   /command/endif
   /command/endmacro
   /command/endwhile
   /command/execute_process
   /command/export
   /command/file
   /command/find_file
   /command/find_library
   /command/find_package
   /command/find_path
   /command/find_program
   /command/fltk_wrap_ui
   /command/foreach
   /command/function
   /command/get_cmake_property
   /command/get_directory_property
   /command/get_filename_component
   /command/get_property
   /command/get_source_file_property
   /command/get_target_property
   /command/get_test_property
   /command/if
   /command/include_directories
   /command/include_external_msproject
   /command/include_regular_expression
   /command/include
   /command/install
   /command/link_directories
   /command/link_libraries
   /command/list
   /command/load_cache
   /command/load_command
   /command/macro
   /command/mark_as_advanced
   /command/math
   /command/message
   /command/option
   /command/project
   /command/qt_wrap_cpp
   /command/qt_wrap_ui
   /command/remove_definitions
   /command/return
   /command/separate_arguments
   /command/set_directory_properties
   /command/set_property
   /command/set
   /command/set_source_files_properties
   /command/set_target_properties
   /command/set_tests_properties
   /command/site_name
   /command/source_group
   /command/string
   /command/target_compile_definitions
   /command/target_compile_features
   /command/target_compile_options
   /command/target_include_directories
   /command/target_link_libraries
   /command/target_sources
   /command/try_compile
   /command/try_run
   /command/unset
   /command/variable_watch
   /command/while

Deprecated Commands
===================

These commands are available only for compatibility with older
versions of CMake.  Do not use them in new code.

.. toctree::
   :maxdepth: 1

   /command/build_name
   /command/exec_program
   /command/export_library_dependencies
   /command/install_files
   /command/install_programs
   /command/install_targets
   /command/make_directory
   /command/output_required_files
   /command/remove
   /command/subdir_depends
   /command/subdirs
   /command/use_mangled_mesa
   /command/utility_source
   /command/variable_requires
   /command/write_file

.. _`CTest Commands`:

CTest Commands
==============

These commands are available only in ctest scripts.

.. toctree::
   :maxdepth: 1

   /command/ctest_build
   /command/ctest_configure
   /command/ctest_coverage
   /command/ctest_empty_binary_directory
   /command/ctest_memcheck
   /command/ctest_read_custom_files
   /command/ctest_run_script
   /command/ctest_sleep
   /command/ctest_start
   /command/ctest_submit
   /command/ctest_test
   /command/ctest_update
   /command/ctest_upload
