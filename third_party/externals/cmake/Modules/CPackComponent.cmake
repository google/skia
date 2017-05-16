#.rst:
# CPackComponent
# --------------
#
# Build binary and source package installers
#
# Variables concerning CPack Components
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# The CPackComponent module is the module which handles the component
# part of CPack.  See CPack module for general information about CPack.
#
# For certain kinds of binary installers (including the graphical
# installers on Mac OS X and Windows), CPack generates installers that
# allow users to select individual application components to install.
# The contents of each of the components are identified by the COMPONENT
# argument of CMake's INSTALL command.  These components can be
# annotated with user-friendly names and descriptions, inter-component
# dependencies, etc., and grouped in various ways to customize the
# resulting installer.  See the cpack_add_* commands, described below,
# for more information about component-specific installations.
#
# Component-specific installation allows users to select specific sets
# of components to install during the install process.  Installation
# components are identified by the COMPONENT argument of CMake's INSTALL
# commands, and should be further described by the following CPack
# commands:
#
# .. variable:: CPACK_COMPONENTS_ALL
#
#  The list of component to install.
#
#  The default value of this variable is computed by CPack and contains all
#  components defined by the project.  The user may set it to only include the
#  specified components.
#
# .. variable:: CPACK_<GENNAME>_COMPONENT_INSTALL
#
#  Enable/Disable component install for CPack generator <GENNAME>.
#
#  Each CPack Generator (RPM, DEB, ARCHIVE, NSIS, DMG, etc...) has a legacy
#  default behavior.  e.g.  RPM builds monolithic whereas NSIS builds
#  component.  One can change the default behavior by setting this variable to
#  0/1 or OFF/ON.
#
# .. variable:: CPACK_COMPONENTS_GROUPING
#
#  Specify how components are grouped for multi-package component-aware CPack
#  generators.
#
#  Some generators like RPM or ARCHIVE family (TGZ, ZIP, ...) generates
#  several packages files when asked for component packaging.  They group
#  the component differently depending on the value of this variable:
#
#  * ONE_PER_GROUP (default): creates one package file per component group
#  * ALL_COMPONENTS_IN_ONE : creates a single package with all (requested) component
#  * IGNORE : creates one package per component, i.e. IGNORE component group
#
#  One can specify different grouping for different CPack generator by
#  using a CPACK_PROJECT_CONFIG_FILE.
#
# .. variable:: CPACK_COMPONENT_<compName>_DISPLAY_NAME
#
#  The name to be displayed for a component.
#
# .. variable:: CPACK_COMPONENT_<compName>_DESCRIPTION
#
#  The description of a component.
#
# .. variable:: CPACK_COMPONENT_<compName>_GROUP
#
#  The group of a component.
#
# .. variable:: CPACK_COMPONENT_<compName>_DEPENDS
#
#  The dependencies (list of components) on which this component depends.
#
# .. variable:: CPACK_COMPONENT_<compName>_REQUIRED
#
#  True is this component is required.
#
# .. command:: cpack_add_component
#
# Describes a CPack installation
# component named by the COMPONENT argument to a CMake INSTALL command.
#
# ::
#
#   cpack_add_component(compname
#                       [DISPLAY_NAME name]
#                       [DESCRIPTION description]
#                       [HIDDEN | REQUIRED | DISABLED ]
#                       [GROUP group]
#                       [DEPENDS comp1 comp2 ... ]
#                       [INSTALL_TYPES type1 type2 ... ]
#                       [DOWNLOADED]
#                       [ARCHIVE_FILE filename])
#
#
#
# The cmake_add_component command describes an installation component,
# which the user can opt to install or remove as part of the graphical
# installation process.  compname is the name of the component, as
# provided to the COMPONENT argument of one or more CMake INSTALL
# commands.
#
# DISPLAY_NAME is the displayed name of the component, used in graphical
# installers to display the component name.  This value can be any
# string.
#
# DESCRIPTION is an extended description of the component, used in
# graphical installers to give the user additional information about the
# component.  Descriptions can span multiple lines using ``\n`` as the
# line separator.  Typically, these descriptions should be no more than
# a few lines long.
#
# HIDDEN indicates that this component will be hidden in the graphical
# installer, so that the user cannot directly change whether it is
# installed or not.
#
# REQUIRED indicates that this component is required, and therefore will
# always be installed.  It will be visible in the graphical installer,
# but it cannot be unselected.  (Typically, required components are
# shown greyed out).
#
# DISABLED indicates that this component should be disabled (unselected)
# by default.  The user is free to select this component for
# installation, unless it is also HIDDEN.
#
# DEPENDS lists the components on which this component depends.  If this
# component is selected, then each of the components listed must also be
# selected.  The dependency information is encoded within the installer
# itself, so that users cannot install inconsistent sets of components.
#
# GROUP names the component group of which this component is a part.  If
# not provided, the component will be a standalone component, not part
# of any component group.  Component groups are described with the
# cpack_add_component_group command, detailed below.
#
# INSTALL_TYPES lists the installation types of which this component is
# a part.  When one of these installations types is selected, this
# component will automatically be selected.  Installation types are
# described with the cpack_add_install_type command, detailed below.
#
# DOWNLOADED indicates that this component should be downloaded
# on-the-fly by the installer, rather than packaged in with the
# installer itself.  For more information, see the
# cpack_configure_downloads command.
#
# ARCHIVE_FILE provides a name for the archive file created by CPack to
# be used for downloaded components.  If not supplied, CPack will create
# a file with some name based on CPACK_PACKAGE_FILE_NAME and the name of
# the component.  See cpack_configure_downloads for more information.
#
# .. command:: cpack_add_component_group
#
# Describes a group of related CPack installation components.
#
# ::
#
#   cpack_add_component_group(groupname
#                            [DISPLAY_NAME name]
#                            [DESCRIPTION description]
#                            [PARENT_GROUP parent]
#                            [EXPANDED]
#                            [BOLD_TITLE])
#
#
#
# The cpack_add_component_group describes a group of installation
# components, which will be placed together within the listing of
# options.  Typically, component groups allow the user to
# select/deselect all of the components within a single group via a
# single group-level option.  Use component groups to reduce the
# complexity of installers with many options.  groupname is an arbitrary
# name used to identify the group in the GROUP argument of the
# cpack_add_component command, which is used to place a component in a
# group.  The name of the group must not conflict with the name of any
# component.
#
# DISPLAY_NAME is the displayed name of the component group, used in
# graphical installers to display the component group name.  This value
# can be any string.
#
# DESCRIPTION is an extended description of the component group, used in
# graphical installers to give the user additional information about the
# components within that group.  Descriptions can span multiple lines
# using ``\n`` as the line separator.  Typically, these descriptions
# should be no more than a few lines long.
#
# PARENT_GROUP, if supplied, names the parent group of this group.
# Parent groups are used to establish a hierarchy of groups, providing
# an arbitrary hierarchy of groups.
#
# EXPANDED indicates that, by default, the group should show up as
# "expanded", so that the user immediately sees all of the components
# within the group.  Otherwise, the group will initially show up as a
# single entry.
#
# BOLD_TITLE indicates that the group title should appear in bold, to
# call the user's attention to the group.
#
# .. command:: cpack_add_install_type
#
# Add a new installation type containing
# a set of predefined component selections to the graphical installer.
#
# ::
#
#   cpack_add_install_type(typename
#                          [DISPLAY_NAME name])
#
#
#
# The cpack_add_install_type command identifies a set of preselected
# components that represents a common use case for an application.  For
# example, a "Developer" install type might include an application along
# with its header and library files, while an "End user" install type
# might just include the application's executable.  Each component
# identifies itself with one or more install types via the INSTALL_TYPES
# argument to cpack_add_component.
#
# DISPLAY_NAME is the displayed name of the install type, which will
# typically show up in a drop-down box within a graphical installer.
# This value can be any string.
#
# .. command:: cpack_configure_downloads
#
# Configure CPack to download
# selected components on-the-fly as part of the installation process.
#
# ::
#
#   cpack_configure_downloads(site
#                             [UPLOAD_DIRECTORY dirname]
#                             [ALL]
#                             [ADD_REMOVE|NO_ADD_REMOVE])
#
#
#
# The cpack_configure_downloads command configures installation-time
# downloads of selected components.  For each downloadable component,
# CPack will create an archive containing the contents of that
# component, which should be uploaded to the given site.  When the user
# selects that component for installation, the installer will download
# and extract the component in place.  This feature is useful for
# creating small installers that only download the requested components,
# saving bandwidth.  Additionally, the installers are small enough that
# they will be installed as part of the normal installation process, and
# the "Change" button in Windows Add/Remove Programs control panel will
# allow one to add or remove parts of the application after the original
# installation.  On Windows, the downloaded-components functionality
# requires the ZipDLL plug-in for NSIS, available at:
#
# ::
#
#   http://nsis.sourceforge.net/ZipDLL_plug-in
#
#
#
# On Mac OS X, installers that download components on-the-fly can only
# be built and installed on system using Mac OS X 10.5 or later.
#
# The site argument is a URL where the archives for downloadable
# components will reside, e.g.,
# http://www.cmake.org/files/2.6.1/installer/ All of the archives
# produced by CPack should be uploaded to that location.
#
# UPLOAD_DIRECTORY is the local directory where CPack will create the
# various archives for each of the components.  The contents of this
# directory should be uploaded to a location accessible by the URL given
# in the site argument.  If omitted, CPack will use the directory
# CPackUploads inside the CMake binary directory to store the generated
# archives.
#
# The ALL flag indicates that all components be downloaded.  Otherwise,
# only those components explicitly marked as DOWNLOADED or that have a
# specified ARCHIVE_FILE will be downloaded.  Additionally, the ALL
# option implies ADD_REMOVE (unless NO_ADD_REMOVE is specified).
#
# ADD_REMOVE indicates that CPack should install a copy of the installer
# that can be called from Windows' Add/Remove Programs dialog (via the
# "Modify" button) to change the set of installed components.
# NO_ADD_REMOVE turns off this behavior.  This option is ignored on Mac
# OS X.

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

# Define var in order to avoid multiple inclusion
if(NOT CPackComponent_CMake_INCLUDED)
set(CPackComponent_CMake_INCLUDED 1)

# Argument-parsing macro from http://www.cmake.org/Wiki/CMakeMacroParseArguments
macro(cpack_parse_arguments prefix arg_names option_names)
  set(${prefix}_DEFAULT_ARGS)
  foreach(arg_name ${arg_names})
    set(${prefix}_${arg_name})
  endforeach()
  foreach(option ${option_names})
    set(${prefix}_${option} FALSE)
  endforeach()

  set(current_arg_name DEFAULT_ARGS)
  set(current_arg_list)
  foreach(arg ${ARGN})
    set(larg_names ${arg_names})
    list(FIND larg_names "${arg}" is_arg_name)
    if (is_arg_name GREATER -1)
      set(${prefix}_${current_arg_name} ${current_arg_list})
      set(current_arg_name ${arg})
      set(current_arg_list)
    else ()
      set(loption_names ${option_names})
      list(FIND loption_names "${arg}" is_option)
      if (is_option GREATER -1)
        set(${prefix}_${arg} TRUE)
      else ()
        set(current_arg_list ${current_arg_list} ${arg})
      endif ()
    endif ()
  endforeach()
  set(${prefix}_${current_arg_name} ${current_arg_list})
endmacro()

# Macro that appends a SET command for the given variable name (var)
# to the macro named strvar, but only if the variable named "var"
# has been defined. The string will eventually be appended to a CPack
# configuration file.
macro(cpack_append_variable_set_command var strvar)
  if (DEFINED ${var})
    set(${strvar} "${${strvar}}set(${var}")
    foreach(APPENDVAL ${${var}})
      set(${strvar} "${${strvar}} ${APPENDVAL}")
    endforeach()
    set(${strvar} "${${strvar}})\n")
  endif ()
endmacro()

# Macro that appends a SET command for the given variable name (var)
# to the macro named strvar, but only if the variable named "var"
# has been defined and is a string. The string will eventually be
# appended to a CPack configuration file.
macro(cpack_append_string_variable_set_command var strvar)
  if (DEFINED ${var})
    list(LENGTH ${var} CPACK_APP_VALUE_LEN)
    if(${CPACK_APP_VALUE_LEN} EQUAL 1)
      set(${strvar} "${${strvar}}set(${var} \"${${var}}\")\n")
    endif()
  endif ()
endmacro()

# Macro that appends a SET command for the given variable name (var)
# to the macro named strvar, but only if the variable named "var"
# has been set to true. The string will eventually be
# appended to a CPack configuration file.
macro(cpack_append_option_set_command var strvar)
  if (${var})
    list(LENGTH ${var} CPACK_APP_VALUE_LEN)
    if(${CPACK_APP_VALUE_LEN} EQUAL 1)
      set(${strvar} "${${strvar}}set(${var} TRUE)\n")
    endif()
  endif ()
endmacro()

# Macro that adds a component to the CPack installer
macro(cpack_add_component compname)
  string(TOUPPER ${compname} _CPACK_ADDCOMP_UNAME)
  cpack_parse_arguments(CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}
    "DISPLAY_NAME;DESCRIPTION;GROUP;DEPENDS;INSTALL_TYPES;ARCHIVE_FILE"
    "HIDDEN;REQUIRED;DISABLED;DOWNLOADED"
    ${ARGN}
    )

  if (CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_DOWNLOADED)
    set(_CPACK_ADDCOMP_STR "\n# Configuration for downloaded component \"${compname}\"\n")
  else ()
    set(_CPACK_ADDCOMP_STR "\n# Configuration for component \"${compname}\"\n")
  endif ()

  if(NOT CPACK_MONOLITHIC_INSTALL)
    # If the user didn't set CPACK_COMPONENTS_ALL explicitly, update the
    # value of CPACK_COMPONENTS_ALL in the configuration file. This will
    # take care of any components that have been added after the CPack
    # moduled was included.
    if(NOT CPACK_COMPONENTS_ALL_SET_BY_USER)
      get_cmake_property(_CPACK_ADDCOMP_COMPONENTS COMPONENTS)
      set(_CPACK_ADDCOMP_STR "${_CPACK_ADDCOMP_STR}\nSET(CPACK_COMPONENTS_ALL")
      foreach(COMP ${_CPACK_ADDCOMP_COMPONENTS})
       set(_CPACK_ADDCOMP_STR "${_CPACK_ADDCOMP_STR} ${COMP}")
      endforeach()
      set(_CPACK_ADDCOMP_STR "${_CPACK_ADDCOMP_STR})\n")
    endif()
  endif()

  cpack_append_string_variable_set_command(
    CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_DISPLAY_NAME
    _CPACK_ADDCOMP_STR)
  cpack_append_string_variable_set_command(
    CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_DESCRIPTION
    _CPACK_ADDCOMP_STR)
  cpack_append_variable_set_command(
    CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_GROUP
    _CPACK_ADDCOMP_STR)
  cpack_append_variable_set_command(
    CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_DEPENDS
    _CPACK_ADDCOMP_STR)
  cpack_append_variable_set_command(
    CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_INSTALL_TYPES
    _CPACK_ADDCOMP_STR)
  cpack_append_string_variable_set_command(
    CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_ARCHIVE_FILE
    _CPACK_ADDCOMP_STR)
  cpack_append_option_set_command(
    CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_HIDDEN
    _CPACK_ADDCOMP_STR)
  cpack_append_option_set_command(
    CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_REQUIRED
    _CPACK_ADDCOMP_STR)
  cpack_append_option_set_command(
    CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_DISABLED
    _CPACK_ADDCOMP_STR)
  cpack_append_option_set_command(
    CPACK_COMPONENT_${_CPACK_ADDCOMP_UNAME}_DOWNLOADED
    _CPACK_ADDCOMP_STR)
  # Backward compatibility issue.
  # Write to config iff the macros is used after CPack.cmake has been
  # included, other it's not necessary because the variables
  # will be encoded by cpack_encode_variables.
  if(CPack_CMake_INCLUDED)
    file(APPEND "${CPACK_OUTPUT_CONFIG_FILE}" "${_CPACK_ADDCOMP_STR}")
  endif()
endmacro()

# Macro that adds a component group to the CPack installer
macro(cpack_add_component_group grpname)
  string(TOUPPER ${grpname} CPACK_ADDGRP_UNAME)
  cpack_parse_arguments(CPACK_COMPONENT_GROUP_${CPACK_ADDGRP_UNAME}
    "DISPLAY_NAME;DESCRIPTION;PARENT_GROUP"
    "EXPANDED;BOLD_TITLE"
    ${ARGN}
    )

  set(CPACK_ADDGRP_STR "\n# Configuration for component group \"${grpname}\"\n")
  cpack_append_string_variable_set_command(
    CPACK_COMPONENT_GROUP_${CPACK_ADDGRP_UNAME}_DISPLAY_NAME
    CPACK_ADDGRP_STR)
  cpack_append_string_variable_set_command(
    CPACK_COMPONENT_GROUP_${CPACK_ADDGRP_UNAME}_DESCRIPTION
    CPACK_ADDGRP_STR)
  cpack_append_string_variable_set_command(
    CPACK_COMPONENT_GROUP_${CPACK_ADDGRP_UNAME}_PARENT_GROUP
    CPACK_ADDGRP_STR)
  cpack_append_option_set_command(
    CPACK_COMPONENT_GROUP_${CPACK_ADDGRP_UNAME}_EXPANDED
    CPACK_ADDGRP_STR)
  cpack_append_option_set_command(
    CPACK_COMPONENT_GROUP_${CPACK_ADDGRP_UNAME}_BOLD_TITLE
    CPACK_ADDGRP_STR)
  # Backward compatibility issue.
  # Write to config iff the macros is used after CPack.cmake has been
  # included, other it's not necessary because the variables
  # will be encoded by cpack_encode_variables.
  if(CPack_CMake_INCLUDED)
    file(APPEND "${CPACK_OUTPUT_CONFIG_FILE}" "${CPACK_ADDGRP_STR}")
  endif()
endmacro()

# Macro that adds an installation type to the CPack installer
macro(cpack_add_install_type insttype)
  string(TOUPPER ${insttype} CPACK_INSTTYPE_UNAME)
  cpack_parse_arguments(CPACK_INSTALL_TYPE_${CPACK_INSTTYPE_UNAME}
    "DISPLAY_NAME"
    ""
    ${ARGN}
    )

  set(CPACK_INSTTYPE_STR
    "\n# Configuration for installation type \"${insttype}\"\n")
  set(CPACK_INSTTYPE_STR
    "${CPACK_INSTTYPE_STR}list(APPEND CPACK_ALL_INSTALL_TYPES ${insttype})\n")
  cpack_append_string_variable_set_command(
    CPACK_INSTALL_TYPE_${CPACK_INSTTYPE_UNAME}_DISPLAY_NAME
    CPACK_INSTTYPE_STR)
  # Backward compatibility issue.
  # Write to config iff the macros is used after CPack.cmake has been
  # included, other it's not necessary because the variables
  # will be encoded by cpack_encode_variables.
  if(CPack_CMake_INCLUDED)
    file(APPEND "${CPACK_OUTPUT_CONFIG_FILE}" "${CPACK_INSTTYPE_STR}")
  endif()
endmacro()

macro(cpack_configure_downloads site)
  cpack_parse_arguments(CPACK_DOWNLOAD
    "UPLOAD_DIRECTORY"
    "ALL;ADD_REMOVE;NO_ADD_REMOVE"
    ${ARGN}
    )

  set(CPACK_CONFIG_DL_STR
    "\n# Downloaded components configuration\n")
  set(CPACK_UPLOAD_DIRECTORY ${CPACK_DOWNLOAD_UPLOAD_DIRECTORY})
  set(CPACK_DOWNLOAD_SITE ${site})
  cpack_append_string_variable_set_command(
    CPACK_DOWNLOAD_SITE
    CPACK_CONFIG_DL_STR)
  cpack_append_string_variable_set_command(
    CPACK_UPLOAD_DIRECTORY
    CPACK_CONFIG_DL_STR)
  cpack_append_option_set_command(
    CPACK_DOWNLOAD_ALL
    CPACK_CONFIG_DL_STR)
  if (${CPACK_DOWNLOAD_ALL} AND NOT ${CPACK_DOWNLOAD_NO_ADD_REMOVE})
    set(CPACK_DOWNLOAD_ADD_REMOVE ON)
  endif ()
  set(CPACK_ADD_REMOVE ${CPACK_DOWNLOAD_ADD_REMOVE})
  cpack_append_option_set_command(
    CPACK_ADD_REMOVE
    CPACK_CONFIG_DL_STR)
  # Backward compatibility issue.
  # Write to config iff the macros is used after CPack.cmake has been
  # included, other it's not necessary because the variables
  # will be encoded by cpack_encode_variables.
  if(CPack_CMake_INCLUDED)
    file(APPEND "${CPACK_OUTPUT_CONFIG_FILE}" "${CPACK_CONFIG_DL_STR}")
  endif()
endmacro()
endif()
