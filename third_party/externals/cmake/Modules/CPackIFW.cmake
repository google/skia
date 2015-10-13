#.rst:
# CPackIFW
# --------
#
# .. _QtIFW: http://doc.qt.io/qtinstallerframework/index.html
#
# This module looks for the location of the command line utilities supplied with
# the Qt Installer Framework (QtIFW_).
#
# The module also defines several commands to control the behavior of the
# CPack ``IFW`` generator.
#
#
# Overview
# ^^^^^^^^
#
# CPack ``IFW`` generator helps you to create online and offline
# binary cross-platform installers with a graphical user interface.
#
# CPack IFW generator prepares project installation and generates configuration
# and meta information for QtIFW_ tools.
#
# The QtIFW_ provides a set of tools and utilities to create
# installers for the supported desktop Qt platforms: Linux, Microsoft Windows,
# and Mac OS X.
#
# You should also install QtIFW_ to use CPack ``IFW`` generator.
# If you don't use a default path for the installation, please set
# the used path in the variable ``QTIFWDIR``.
#
# Variables
# ^^^^^^^^^
#
# You can use the following variables to change behavior of CPack ``IFW`` generator.
#
# Debug
# """"""
#
# .. variable:: CPACK_IFW_VERBOSE
#
#  Set to ``ON`` to enable addition debug output.
#  By default is ``OFF``.
#
# Package
# """""""
#
# .. variable:: CPACK_IFW_PACKAGE_TITLE
#
#  Name of the installer as displayed on the title bar.
#  By default used :variable:`CPACK_PACKAGE_DESCRIPTION_SUMMARY`.
#
# .. variable:: CPACK_IFW_PACKAGE_PUBLISHER
#
#  Publisher of the software (as shown in the Windows Control Panel).
#  By default used :variable:`CPACK_PACKAGE_VENDOR`.
#
# .. variable:: CPACK_IFW_PRODUCT_URL
#
#  URL to a page that contains product information on your web site.
#
# .. variable:: CPACK_IFW_PACKAGE_ICON
#
#  Filename for a custom installer icon. The actual file is '.icns' (Mac OS X),
#  '.ico' (Windows). No functionality on Unix.
#
# .. variable:: CPACK_IFW_PACKAGE_WINDOW_ICON
#
#  Filename for a custom window icon in PNG format for the Installer application.
#
# .. variable:: CPACK_IFW_PACKAGE_LOGO
#
#  Filename for a logo is used as QWizard::LogoPixmap.
#
# .. variable:: CPACK_IFW_PACKAGE_START_MENU_DIRECTORY
#
#  Name of the default program group for the product in the Windows Start menu.
#
#  By default used :variable:`CPACK_IFW_PACKAGE_NAME`.
#
# .. variable:: CPACK_IFW_TARGET_DIRECTORY
#
#  Default target directory for installation.
#  By default used "@ApplicationsDir@/:variable:`CPACK_PACKAGE_INSTALL_DIRECTORY`"
#
#  You can use predefined variables.
#
# .. variable:: CPACK_IFW_ADMIN_TARGET_DIRECTORY
#
#  Default target directory for installation with administrator rights.
#
#  You can use predefined variables.
#
# .. variable:: CPACK_IFW_PACKAGE_GROUP
#
#  The group, which will be used to configure the root package
#
# .. variable:: CPACK_IFW_PACKAGE_NAME
#
#  The root package name, which will be used if configuration group is not
#  specified
#
# .. variable:: CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_NAME
#
#  Filename of the generated maintenance tool.
#  The platform-specific executable file extension is appended.
#
#  By default used QtIFW_ defaults (``maintenancetool``).
#
# .. variable:: CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_INI_FILE
#
#  Filename for the configuration of the generated maintenance tool.
#
#  By default used QtIFW_ defaults (``maintenancetool.ini``).
#
# .. variable:: CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS
#
#  Set to ``ON`` if the installation path can contain non-ASCII characters.
#
#  Is ``ON`` for QtIFW_ less 2.0 tools.
#
# .. variable:: CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH
#
#  Set to ``OFF`` if the installation path cannot contain space characters.
#
#  Is ``ON`` for QtIFW_ less 2.0 tools.
#
# .. variable:: CPACK_IFW_PACKAGE_CONTROL_SCRIPT
#
#  Filename for a custom installer control script.
#
# .. variable:: CPACK_IFW_REPOSITORIES_ALL
#
#  The list of remote repositories.
#
#  The default value of this variable is computed by CPack and contains
#  all repositories added with command :command:`cpack_ifw_add_repository`
#
# .. variable:: CPACK_IFW_DOWNLOAD_ALL
#
#  If this is ``ON`` all components will be downloaded.
#  By default is ``OFF`` or used value
#  from ``CPACK_DOWNLOAD_ALL`` if set
#
# Components
# """"""""""
#
# .. variable:: CPACK_IFW_RESOLVE_DUPLICATE_NAMES
#
#  Resolve duplicate names when installing components with groups.
#
# .. variable:: CPACK_IFW_PACKAGES_DIRECTORIES
#
#  Additional prepared packages dirs that will be used to resolve
#  dependent components.
#
# Tools
# """"""""
#
# .. variable:: CPACK_IFW_FRAMEWORK_VERSION
#
#  The version of used QtIFW_ tools.
#
# .. variable:: CPACK_IFW_BINARYCREATOR_EXECUTABLE
#
#  The path to "binarycreator" command line client.
#
#  This variable is cached and can be configured user if need.
#
# .. variable:: CPACK_IFW_REPOGEN_EXECUTABLE
#
#  The path to "repogen" command line client.
#
#  This variable is cached and can be configured user if need.
#
# Commands
# ^^^^^^^^^
#
# The module defines the following commands:
#
# --------------------------------------------------------------------------
#
# .. command:: cpack_ifw_configure_component
#
# Sets the arguments specific to the CPack IFW generator.
#
# ::
#
#   cpack_ifw_configure_component(<compname> [COMMON]
#                       [NAME <name>]
#                       [VERSION <version>]
#                       [SCRIPT <script>]
#                       [PRIORITY <priority>]
#                       [DEPENDS <com_id> ...]
#                       [LICENSES <display_name> <file_path> ...])
#
# This command should be called after cpack_add_component command.
#
# ``COMMON`` if set, then the component will be packaged and installed as part
# of a group to which it belongs.
#
# ``VERSION`` is version of component.
# By default used :variable:`CPACK_PACKAGE_VERSION`.
#
# ``SCRIPT`` is a relative or absolute path to operations script
# for this component.
#
# ``NAME`` is used to create domain-like identification for this component.
# By default used origin component name.
#
# ``PRIORITY`` is priority of the component in the tree.
#
# ``DEPENDS`` list of dependency component identifiers in QtIFW_ style.
#
# ``LICENSES`` pair of <display_name> and <file_path> of license text for this
# component. You can specify more then one license.
#
# --------------------------------------------------------------------------
#
# .. command:: cpack_ifw_configure_component_group
#
# Sets the arguments specific to the CPack IFW generator.
#
# ::
#
#   cpack_ifw_configure_component_group(<grpname>
#                       [VERSION <version>]
#                       [NAME <name>]
#                       [SCRIPT <script>]
#                       [PRIORITY <priority>]
#                       [LICENSES <display_name> <file_path> ...])
#
# This command should be called after cpack_add_component_group command.
#
# ``VERSION`` is version of component group.
# By default used :variable:`CPACK_PACKAGE_VERSION`.
#
# ``NAME`` is used to create domain-like identification for this component group.
# By default used origin component group name.
#
# ``SCRIPT`` is a relative or absolute path to operations script
# for this component group.
#
# ``PRIORITY`` is priority of the component group in the tree.
#
# ``LICENSES`` pair of <display_name> and <file_path> of license text for this
# component group. You can specify more then one license.
#
# --------------------------------------------------------------------------
#
# .. command:: cpack_ifw_add_repository
#
# Add QtIFW_ specific remote repository.
#
# ::
#
#   cpack_ifw_add_repository(<reponame> [DISABLED]
#                       URL <url>
#                       [USERNAME <username>]
#                       [PASSWORD <password>]
#                       [DISPLAY_NAME <display_name>])
#
# This macro will also add the <reponame> repository
# to a variable :variable:`CPACK_IFW_REPOSITORIES_ALL`
#
# ``DISABLED`` if set, then the repository will be disabled by default.
#
# ``URL`` is points to a list of available components.
#
# ``USERNAME`` is used as user on a protected repository.
#
# ``PASSWORD`` is password to use on a protected repository.
#
# ``DISPLAY_NAME`` is string to display instead of the URL.
#
# Example usage
# ^^^^^^^^^^^^^
#
# .. code-block:: cmake
#
#    set(CPACK_PACKAGE_NAME "MyPackage")
#    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MyPackage Installation Example")
#    set(CPACK_PACKAGE_VERSION "1.0.0") # Version of installer
#
#    include(CPack)
#    include(CPackIFW)
#
#    cpack_add_component(myapp
#        DISPLAY_NAME "MyApp"
#        DESCRIPTION "My Application")
#    cpack_ifw_configure_component(myapp
#        VERSION "1.2.3" # Version of component
#        SCRIPT "operations.qs")
#    cpack_add_component(mybigplugin
#        DISPLAY_NAME "MyBigPlugin"
#        DESCRIPTION "My Big Downloadable Plugin"
#        DOWNLOADED)
#    cpack_ifw_add_repository(myrepo
#        URL "http://example.com/ifw/repo/myapp"
#        DISPLAY_NAME "My Application Repository")
#
#
# Online installer
# ^^^^^^^^^^^^^^^^
#
# By default CPack IFW generator makes offline installer. This means that all
# components will be packaged into a binary file.
#
# To make a component downloaded, you must set the ``DOWNLOADED`` option in
# :command:`cpack_add_component`.
#
# Then you would use the command :command:`cpack_configure_downloads`.
# If you set ``ALL`` option all components will be downloaded.
#
# You also can use command :command:`cpack_ifw_add_repository` and
# variable :variable:`CPACK_IFW_DOWNLOAD_ALL` for more specific configuration.
#
# CPack IFW generator creates "repository" dir in current binary dir. You
# would copy content of this dir to specified ``site`` (``url``).
#
# See Also
# ^^^^^^^^
#
# Qt Installer Framework Manual:
#
#  Index page
#   http://doc.qt.io/qtinstallerframework/index.html
#
#  Component Scripting
#   http://doc.qt.io/qtinstallerframework/scripting.html
#
#  Predefined Variables
#   http://doc.qt.io/qtinstallerframework/scripting.html#predefined-variables
#
# Download Qt Installer Framework for you platform from Qt site:
#  http://download.qt.io/official_releases/qt-installer-framework
#

#=============================================================================
# Copyright 2014 Kitware, Inc.
# Copyright 2014 Konstantin Podsvirov <konstantin@podsvirov.pro>
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

#=============================================================================
# Search Qt Installer Framework tools
#=============================================================================

# Default path

set(_CPACK_IFW_PATHS
  "${QTIFWDIR}"
  "$ENV{QTIFWDIR}"
  "${QTDIR}"
  "$ENV{QTIFWDIR}")
if(WIN32)
  list(APPEND _CPACK_IFW_PATHS
    "$ENV{HOMEDRIVE}/Qt"
    "C:/Qt")
else()
  list(APPEND _CPACK_IFW_PATHS
    "$ENV{HOME}/Qt"
    "/opt/Qt")
endif()

set(_CPACK_IFW_SUFFIXES
# Common
  "bin"
# Second branch
  "QtIFW2.3.0/bin"
  "QtIFW2.2.0/bin"
  "QtIFW2.1.0/bin"
  "QtIFW2.0.0/bin"
# First branch
  "QtIFW-1.6.0/bin"
  "QtIFW-1.5.0/bin"
  "QtIFW-1.4.0/bin"
  "QtIFW-1.3.0/bin")

# Look for 'binarycreator'

find_program(CPACK_IFW_BINARYCREATOR_EXECUTABLE
  NAMES binarycreator
  PATHS ${_CPACK_IFW_PATHS}
  PATH_SUFFIXES ${_CPACK_IFW_SUFFIXES}
  DOC "QtIFW binarycreator command line client")

mark_as_advanced(CPACK_IFW_BINARYCREATOR_EXECUTABLE)

# Look for 'repogen'

find_program(CPACK_IFW_REPOGEN_EXECUTABLE
  NAMES repogen
  PATHS ${_CPACK_IFW_PATHS}
  PATH_SUFFIXES ${_CPACK_IFW_SUFFIXES}
  DOC "QtIFW repogen command line client"
  )
mark_as_advanced(CPACK_IFW_REPOGEN_EXECUTABLE)

# Look for 'installerbase'

find_program(CPACK_IFW_INSTALLERBASE_EXECUTABLE
  NAMES installerbase
  PATHS ${_CPACK_IFW_PATHS}
  PATH_SUFFIXES ${_CPACK_IFW_SUFFIXES}
  DOC "QtIFW installer executable base"
  )
mark_as_advanced(CPACK_IFW_INSTALLERBASE_EXECUTABLE)

# Look for 'devtool' (appeared in the second branch)

find_program(CPACK_IFW_DEVTOOL_EXECUTABLE
  NAMES devtool
  PATHS ${_CPACK_IFW_PATHS}
  PATH_SUFFIXES ${_CPACK_IFW_SUFFIXES}
  DOC "QtIFW devtool command line client"
  )
mark_as_advanced(CPACK_IFW_DEVTOOL_EXECUTABLE)

#
## Next code is included only once
#

if(NOT CPackIFW_CMake_INCLUDED)
set(CPackIFW_CMake_INCLUDED 1)

#=============================================================================
# Framework version
#=============================================================================

if(CPACK_IFW_INSTALLERBASE_EXECUTABLE AND CPACK_IFW_DEVTOOL_EXECUTABLE)
  execute_process(COMMAND
    "${CPACK_IFW_INSTALLERBASE_EXECUTABLE}" --framework-version
    OUTPUT_VARIABLE CPACK_IFW_FRAMEWORK_VERSION)
  if(CPACK_IFW_FRAMEWORK_VERSION)
    string(REPLACE " " ""
      CPACK_IFW_FRAMEWORK_VERSION "${CPACK_IFW_FRAMEWORK_VERSION}")
    string(REPLACE "\t" ""
      CPACK_IFW_FRAMEWORK_VERSION "${CPACK_IFW_FRAMEWORK_VERSION}")
    string(REPLACE "\n" ""
      CPACK_IFW_FRAMEWORK_VERSION "${CPACK_IFW_FRAMEWORK_VERSION}")
    if(CPACK_IFW_VERBOSE)
      message(STATUS "Found QtIFW ${CPACK_IFW_FRAMEWORK_VERSION} version")
    endif()
  endif()
endif()

#=============================================================================
# Macro definition
#=============================================================================

# Macro definition based on CPackComponent

if(NOT CPackComponent_CMake_INCLUDED)
    include(CPackComponent)
endif()

if(NOT __CMAKE_PARSE_ARGUMENTS_INCLUDED)
    include(CMakeParseArguments)
endif()

# Resolve full filename for script file
macro(_cpack_ifw_resolve_script _variable)
  set(_ifw_script_macro ${_variable})
  set(_ifw_script_file ${${_ifw_script_macro}})
  if(DEFINED ${_ifw_script_macro})
    get_filename_component(${_ifw_script_macro} ${_ifw_script_file} ABSOLUTE)
    set(_ifw_script_file ${${_ifw_script_macro}})
    if(NOT EXISTS ${_ifw_script_file})
      message(WARNING "CPack IFW: script file \"${_ifw_script_file}\" is not exists")
      set(${_ifw_script_macro})
    endif()
  endif()
endmacro()

# Resolve full path to lisense file
macro(_cpack_ifw_resolve_lisenses _variable)
  if(${_variable})
    set(_ifw_license_file FALSE)
    set(_ifw_licenses_fix)
    foreach(_ifw_licenses_arg ${${_variable}})
      if(_ifw_license_file)
        get_filename_component(_ifw_licenses_arg "${_ifw_licenses_arg}" ABSOLUTE)
        set(_ifw_license_file FALSE)
      else()
        set(_ifw_license_file TRUE)
      endif()
      list(APPEND _ifw_licenses_fix "${_ifw_licenses_arg}")
    endforeach(_ifw_licenses_arg)
    set(${_variable} "${_ifw_licenses_fix}")
  endif()
endmacro()

# Macro for configure component
macro(cpack_ifw_configure_component compname)

  string(TOUPPER ${compname} _CPACK_IFWCOMP_UNAME)

  set(_IFW_OPT COMMON)
  set(_IFW_ARGS VERSION SCRIPT NAME PRIORITY)
  set(_IFW_MULTI_ARGS DEPENDS LICENSES)
  cmake_parse_arguments(CPACK_IFW_COMPONENT_${_CPACK_IFWCOMP_UNAME} "${_IFW_OPT}" "${_IFW_ARGS}" "${_IFW_MULTI_ARGS}" ${ARGN})

  _cpack_ifw_resolve_script(CPACK_IFW_COMPONENT_${_CPACK_IFWCOMP_UNAME}_SCRIPT)
  _cpack_ifw_resolve_lisenses(CPACK_IFW_COMPONENT_${_CPACK_IFWCOMP_UNAME}_LICENSES)

  set(_CPACK_IFWCOMP_STR "\n# Configuration for IFW component \"${compname}\"\n")

  foreach(_IFW_ARG_NAME ${_IFW_OPT})
  cpack_append_option_set_command(
    CPACK_IFW_COMPONENT_${_CPACK_IFWCOMP_UNAME}_${_IFW_ARG_NAME}
    _CPACK_IFWCOMP_STR)
  endforeach()

  foreach(_IFW_ARG_NAME ${_IFW_ARGS})
  cpack_append_string_variable_set_command(
    CPACK_IFW_COMPONENT_${_CPACK_IFWCOMP_UNAME}_${_IFW_ARG_NAME}
    _CPACK_IFWCOMP_STR)
  endforeach()

  foreach(_IFW_ARG_NAME ${_IFW_MULTI_ARGS})
  cpack_append_variable_set_command(
    CPACK_IFW_COMPONENT_${_CPACK_IFWCOMP_UNAME}_${_IFW_ARG_NAME}
    _CPACK_IFWCOMP_STR)
  endforeach()

  if(CPack_CMake_INCLUDED)
    file(APPEND "${CPACK_OUTPUT_CONFIG_FILE}" "${_CPACK_IFWCOMP_STR}")
  endif()

endmacro()

# Macro for configure group
macro(cpack_ifw_configure_component_group grpname)

  string(TOUPPER ${grpname} _CPACK_IFWGRP_UNAME)

  set(_IFW_OPT)
  set(_IFW_ARGS NAME VERSION SCRIPT PRIORITY)
  set(_IFW_MULTI_ARGS LICENSES)
  cmake_parse_arguments(CPACK_IFW_COMPONENT_GROUP_${_CPACK_IFWGRP_UNAME} "${_IFW_OPT}" "${_IFW_ARGS}" "${_IFW_MULTI_ARGS}" ${ARGN})

  _cpack_ifw_resolve_script(CPACK_IFW_COMPONENT_GROUP_${_CPACK_IFWGRP_UNAME}_SCRIPT)
  _cpack_ifw_resolve_lisenses(CPACK_IFW_COMPONENT_GROUP_${_CPACK_IFWGRP_UNAME}_LICENSES)

  set(_CPACK_IFWGRP_STR "\n# Configuration for IFW component group \"${grpname}\"\n")

  foreach(_IFW_ARG_NAME ${_IFW_ARGS})
  cpack_append_string_variable_set_command(
    CPACK_IFW_COMPONENT_GROUP_${_CPACK_IFWGRP_UNAME}_${_IFW_ARG_NAME}
    _CPACK_IFWGRP_STR)
  endforeach()

  foreach(_IFW_ARG_NAME ${_IFW_MULTI_ARGS})
  cpack_append_variable_set_command(
    CPACK_IFW_COMPONENT_GROUP_${_CPACK_IFWGRP_UNAME}_${_IFW_ARG_NAME}
    _CPACK_IFWGRP_STR)
  endforeach()

  if(CPack_CMake_INCLUDED)
    file(APPEND "${CPACK_OUTPUT_CONFIG_FILE}" "${_CPACK_IFWGRP_STR}")
  endif()
endmacro()

# Macro for adding repository
macro(cpack_ifw_add_repository reponame)

  string(TOUPPER ${reponame} _CPACK_IFWREPO_UNAME)

  set(_IFW_OPT DISABLED)
  set(_IFW_ARGS URL USERNAME PASSWORD DISPLAY_NAME)
  set(_IFW_MULTI_ARGS)
  cmake_parse_arguments(CPACK_IFW_REPOSITORY_${_CPACK_IFWREPO_UNAME} "${_IFW_OPT}" "${_IFW_ARGS}" "${_IFW_MULTI_ARGS}" ${ARGN})

  set(_CPACK_IFWREPO_STR "\n# Configuration for IFW repository \"${reponame}\"\n")

  foreach(_IFW_ARG_NAME ${_IFW_OPT})
  cpack_append_option_set_command(
    CPACK_IFW_REPOSITORY_${_CPACK_IFWREPO_UNAME}_${_IFW_ARG_NAME}
    _CPACK_IFWREPO_STR)
  endforeach()

  foreach(_IFW_ARG_NAME ${_IFW_ARGS})
  cpack_append_string_variable_set_command(
    CPACK_IFW_REPOSITORY_${_CPACK_IFWREPO_UNAME}_${_IFW_ARG_NAME}
    _CPACK_IFWREPO_STR)
  endforeach()

  foreach(_IFW_ARG_NAME ${_IFW_MULTI_ARGS})
  cpack_append_variable_set_command(
    CPACK_IFW_REPOSITORY_${_CPACK_IFWREPO_UNAME}_${_IFW_ARG_NAME}
    _CPACK_IFWREPO_STR)
  endforeach()

  list(APPEND CPACK_IFW_REPOSITORIES_ALL ${reponame})
  set(_CPACK_IFWREPO_STR "${_CPACK_IFWREPO_STR}list(APPEND CPACK_IFW_REPOSITORIES_ALL ${reponame})\n")

  if(CPack_CMake_INCLUDED)
    file(APPEND "${CPACK_OUTPUT_CONFIG_FILE}" "${_CPACK_IFWREPO_STR}")
  endif()

endmacro()

# Resolve package control script
_cpack_ifw_resolve_script(CPACK_IFW_PACKAGE_CONTROL_SCRIPT)

endif() # NOT CPackIFW_CMake_INCLUDED
