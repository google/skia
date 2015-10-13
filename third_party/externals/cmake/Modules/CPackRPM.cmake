#.rst:
# CPackRPM
# --------
#
# The builtin (binary) CPack RPM generator (Unix only)
#
# Variables specific to CPack RPM generator
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# CPackRPM may be used to create RPM package using CPack.  CPackRPM is a
# CPack generator thus it uses the CPACK_XXX variables used by CPack :
# http://www.cmake.org/Wiki/CMake:CPackConfiguration
#
# However CPackRPM has specific features which are controlled by the
# specifics CPACK_RPM_XXX variables.  CPackRPM is a component aware
# generator so when CPACK_RPM_COMPONENT_INSTALL is ON some more
# CPACK_RPM_<ComponentName>_XXXX variables may be used in order to have
# component specific values.  Note however that <componentName> refers
# to the **grouping name**.  This may be either a component name or a
# component GROUP name.  Usually those vars correspond to RPM spec file
# entities, one may find information about spec files here
# http://www.rpm.org/wiki/Docs.  You'll find a detailed usage of
# CPackRPM on the wiki:
#
# ::
#
#   http://www.cmake.org/Wiki/CMake:CPackPackageGenerators#RPM_.28Unix_Only.29
#
# However as a handy reminder here comes the list of specific variables:
#
# .. variable:: CPACK_RPM_PACKAGE_SUMMARY
#               CPACK_RPM_<component>_PACKAGE_SUMMARY
#
#  The RPM package summary.
#
#  * Mandatory : YES
#  * Default   : CPACK_PACKAGE_DESCRIPTION_SUMMARY
#
# .. variable:: CPACK_RPM_PACKAGE_NAME
#
#  The RPM package name.
#
#  * Mandatory : YES
#  * Default   : CPACK_PACKAGE_NAME
#
# .. variable:: CPACK_RPM_PACKAGE_VERSION
#
#  The RPM package version.
#
#  * Mandatory : YES
#  * Default   : CPACK_PACKAGE_VERSION
#
# .. variable:: CPACK_RPM_PACKAGE_ARCHITECTURE
#               CPACK_RPM_<component>_PACKAGE_ARCHITECTURE
#
#  The RPM package architecture.
#
#  * Mandatory : YES
#  * Default   : Native architecture output by "uname -m"
#
#  This may be set to "noarch" if you know you are building a noarch package.
#
# .. variable:: CPACK_RPM_PACKAGE_RELEASE
#
#  The RPM package release.
#
#  * Mandatory : YES
#  * Default   : 1
#
#  This is the numbering of the RPM package itself, i.e. the version of the
#  packaging and not the version of the content (see
#  CPACK_RPM_PACKAGE_VERSION). One may change the default value if the
#  previous packaging was buggy and/or you want to put here a fancy Linux
#  distro specific numbering.
#
# .. variable:: CPACK_RPM_PACKAGE_LICENSE
#
#  The RPM package license policy.
#
#  * Mandatory : YES
#  * Default   : "unknown"
#
# .. variable:: CPACK_RPM_PACKAGE_GROUP
#
#  The RPM package group.
#
#  * Mandatory : YES
#  * Default   : "unknown"
#
# .. variable:: CPACK_RPM_PACKAGE_VENDOR
#
#  The RPM package vendor.
#
#  * Mandatory : YES
#  * Default   : CPACK_PACKAGE_VENDOR if set or "unknown"
#
# .. variable:: CPACK_RPM_PACKAGE_URL
#
#  The projects URL.
#
#  * Mandatory : NO
#  * Default   : -
#
# .. variable:: CPACK_RPM_PACKAGE_DESCRIPTION
#               CPACK_RPM_<component>_PACKAGE_DESCRIPTION
#
#  RPM package description.
#
#  * Mandatory : YES
#  * Default : CPACK_COMPONENT_<compName>_DESCRIPTION (component based installers
#    only) if set, CPACK_PACKAGE_DESCRIPTION_FILE if set or "no package
#    description available"
#
# .. variable:: CPACK_RPM_COMPRESSION_TYPE
#
#  RPM compression type.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to override RPM compression type to be used to build the
#  RPM. For example some Linux distribution now default to lzma or xz
#  compression whereas older cannot use such RPM.  Using this one can enforce
#  compression type to be used.  Possible value are: lzma, xz, bzip2 and gzip.
#
# .. variable:: CPACK_RPM_PACKAGE_REQUIRES
#
#  RPM spec requires field.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to set RPM dependencies (requires).  Note that you must enclose
#  the complete requires string between quotes, for example::
#
#   set(CPACK_RPM_PACKAGE_REQUIRES "python >= 2.5.0, cmake >= 2.8")
#
#  The required package list of an RPM file could be printed with::
#
#   rpm -qp --requires file.rpm
#
# .. variable:: CPACK_RPM_PACKAGE_REQUIRES_PRE
#
#  RPM spec requires(pre) field.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to set RPM preinstall dependencies (requires(pre)).  Note that you must enclose
#  the complete requires string between quotes, for example::
#
#   set(CPACK_RPM_PACKAGE_REQUIRES_PRE "shadow-utils, initscripts")
#
# .. variable:: CPACK_RPM_PACKAGE_REQUIRES_POST
#
#  RPM spec requires(post) field.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to set RPM postinstall dependencies (requires(post)).  Note that you must enclose
#  the complete requires string between quotes, for example::
#
#   set(CPACK_RPM_PACKAGE_REQUIRES_POST "shadow-utils, initscripts")
#
#
# .. variable:: CPACK_RPM_PACKAGE_REQUIRES_POSTUN
#
#  RPM spec requires(postun) field.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to set RPM postuninstall dependencies (requires(postun)).  Note that you must enclose
#  the complete requires string between quotes, for example::
#
#   set(CPACK_RPM_PACKAGE_REQUIRES_POSTUN "shadow-utils, initscripts")
#
#
# .. variable:: CPACK_RPM_PACKAGE_REQUIRES_PREUN
#
#  RPM spec requires(preun) field.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to set RPM preuninstall dependencies (requires(preun)).  Note that you must enclose
#  the complete requires string between quotes, for example::
#
#   set(CPACK_RPM_PACKAGE_REQUIRES_PREUN "shadow-utils, initscripts")
#
# .. variable:: CPACK_RPM_PACKAGE_SUGGESTS
#
#  RPM spec suggest field.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to set weak RPM dependencies (suggests).  Note that you must
#  enclose the complete requires string between quotes.
#
# .. variable:: CPACK_RPM_PACKAGE_PROVIDES
#
#  RPM spec provides field.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to set RPM dependencies (provides).  The provided package list
#  of an RPM file could be printed with::
#
#   rpm -qp --provides file.rpm
#
# .. variable:: CPACK_RPM_PACKAGE_OBSOLETES
#
#  RPM spec obsoletes field.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to set RPM packages that are obsoleted by this one.
#
# .. variable:: CPACK_RPM_PACKAGE_RELOCATABLE
#
#  build a relocatable RPM.
#
#  * Mandatory : NO
#  * Default   : CPACK_PACKAGE_RELOCATABLE
#
#  If this variable is set to TRUE or ON CPackRPM will try
#  to build a relocatable RPM package. A relocatable RPM may
#  be installed using::
#
#   rpm --prefix or --relocate
#
#  in order to install it at an alternate place see rpm(8).  Note that
#  currently this may fail if CPACK_SET_DESTDIR is set to ON.  If
#  CPACK_SET_DESTDIR is set then you will get a warning message but if there
#  is file installed with absolute path you'll get unexpected behavior.
#
# .. variable:: CPACK_RPM_SPEC_INSTALL_POST
#
#  * Mandatory : NO
#  * Default   : -
#  * Deprecated: YES
#
#  This way of specifying post-install script is deprecated, use
#  CPACK_RPM_POST_INSTALL_SCRIPT_FILE.
#  May be used to set an RPM post-install command inside the spec file.
#  For example setting it to "/bin/true" may be used to prevent
#  rpmbuild to strip binaries.
#
# .. variable:: CPACK_RPM_SPEC_MORE_DEFINE
#
#  RPM extended spec definitions lines.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to add any %define lines to the generated spec file.
#
# .. variable:: CPACK_RPM_PACKAGE_DEBUG
#
#  Toggle CPackRPM debug output.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be set when invoking cpack in order to trace debug information
#  during CPack RPM run. For example you may launch CPack like this::
#
#   cpack -D CPACK_RPM_PACKAGE_DEBUG=1 -G RPM
#
# .. variable:: CPACK_RPM_USER_BINARY_SPECFILE
#
#  A user provided spec file.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be set by the user in order to specify a USER binary spec file
#  to be used by CPackRPM instead of generating the file.
#  The specified file will be processed by configure_file( @ONLY).
#  One can provide a component specific file by setting
#  CPACK_RPM_<componentName>_USER_BINARY_SPECFILE.
#
# .. variable:: CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE
#
#  Spec file template.
#
#  * Mandatory : NO
#  * Default   : -
#
#  If set CPack will generate a template for USER specified binary
#  spec file and stop with an error. For example launch CPack like this::
#
#   cpack -D CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE=1 -G RPM
#
#  The user may then use this file in order to hand-craft is own
#  binary spec file which may be used with CPACK_RPM_USER_BINARY_SPECFILE.
#
# .. variable:: CPACK_RPM_PRE_INSTALL_SCRIPT_FILE
#               CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to embed a pre (un)installation script in the spec file.
#  The refered script file(s) will be read and directly
#  put after the %pre or %preun section
#  If CPACK_RPM_COMPONENT_INSTALL is set to ON the (un)install script for
#  each component can be overridden with
#  CPACK_RPM_<COMPONENT>_PRE_INSTALL_SCRIPT_FILE and
#  CPACK_RPM_<COMPONENT>_PRE_UNINSTALL_SCRIPT_FILE.
#  One may verify which scriptlet has been included with::
#
#   rpm -qp --scripts  package.rpm
#
# .. variable:: CPACK_RPM_POST_INSTALL_SCRIPT_FILE
#               CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to embed a post (un)installation script in the spec file.
#  The refered script file(s) will be read and directly
#  put after the %post or %postun section.
#  If CPACK_RPM_COMPONENT_INSTALL is set to ON the (un)install script for
#  each component can be overridden with
#  CPACK_RPM_<COMPONENT>_POST_INSTALL_SCRIPT_FILE and
#  CPACK_RPM_<COMPONENT>_POST_UNINSTALL_SCRIPT_FILE.
#  One may verify which scriptlet has been included with::
#
#   rpm -qp --scripts  package.rpm
#
# .. variable:: CPACK_RPM_USER_FILELIST
#               CPACK_RPM_<COMPONENT>_USER_FILELIST
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to explicitly specify %(<directive>) file line
#  in the spec file. Like %config(noreplace) or any other directive
#  that be found in the %files section. Since CPackRPM is generating
#  the list of files (and directories) the user specified files of
#  the CPACK_RPM_<COMPONENT>_USER_FILELIST list will be removed from
#  the generated list.
#
# .. variable:: CPACK_RPM_CHANGELOG_FILE
#
#  RPM changelog file.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to embed a changelog in the spec file.
#  The refered file will be read and directly put after the %changelog
#  section.
#
# .. variable:: CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST
#
#  list of path to be excluded.
#
#  * Mandatory : NO
#  * Default   : /etc /etc/init.d /usr /usr/share /usr/share/doc /usr/bin /usr/lib /usr/lib64 /usr/include
#
#  May be used to exclude path (directories or files) from the auto-generated
#  list of paths discovered by CPack RPM. The defaut value contains a
#  reasonable set of values if the variable is not defined by the user. If the
#  variable is defined by the user then CPackRPM will NOT any of the default
#  path.  If you want to add some path to the default list then you can use
#  CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION variable.
#
# .. variable:: CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
#
#  additional list of path to be excluded.
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to add more exclude path (directories or files) from the initial
#  default list of excluded paths. See CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST.
#
# .. variable:: CPACK_RPM_RELOCATION_PATHS
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to specify more than one relocation path per relocatable RPM.
#  Variable contains a list of relocation paths that if relative are prefixed
#  by the value of CPACK_RPM_<COMPONENT>_PACKAGE_PREFIX or by the value of
#  CPACK_PACKAGING_INSTALL_PREFIX if the component version is not provided.
#  Variable is not component based as its content can be used to set a different
#  path prefix for e.g. binary dir and documentation dir at the same time.
#  Only prefixes that are required by a certain component are added to that
#  component - component must contain at least one file/directory/symbolic link
#  with CPACK_RPM_RELOCATION_PATHS prefix for a certain relocation path
#  to be added. Package will not contain any relocation paths if there are no
#  files/directories/symbolic links on any of the provided prefix locations.
#  Packages that either do not contain any relocation paths or contain
#  files/directories/symbolic links that are outside relocation paths print
#  out an AUTHOR_WARNING that RPM will be partially relocatable.
#
# .. variable:: CPACK_RPM_<COMPONENT>_PACKAGE_PREFIX
#
#  * Mandatory : NO
#  * Default   : CPACK_PACKAGING_INSTALL_PREFIX
#
#  May be used to set per component CPACK_PACKAGING_INSTALL_PREFIX for
#  relocatable RPM packages.
#
# .. variable:: CPACK_RPM_NO_INSTALL_PREFIX_RELOCATION
#               CPACK_RPM_NO_<COMPONENT>_INSTALL_PREFIX_RELOCATION
#
#  * Mandatory : NO
#  * Default   : CPACK_PACKAGING_INSTALL_PREFIX or CPACK_RPM_<COMPONENT>_PACKAGE_PREFIX
#                are treated as one of relocation paths
#
#  May be used to remove CPACK_PACKAGING_INSTALL_PREFIX and CPACK_RPM_<COMPONENT>_PACKAGE_PREFIX
#  from relocatable RPM prefix paths.
#
# .. variable:: CPACK_RPM_ADDITIONAL_MAN_DIRS
#
#  * Mandatory : NO
#  * Default   : -
#
#  May be used to set additional man dirs that could potentially be compressed
#  by brp-compress RPM macro. Variable content must be a list of regular
#  expressions that point to directories containing man files or to man files
#  directly. Note that in order to compress man pages a path must also be
#  present in brp-compress RPM script and that brp-compress script must be
#  added to RPM configuration by the operating system.
#
#  Regular expressions that are added by default were taken from brp-compress
#  RPM macro:
#
#  - /usr/man/man.*
#  - /usr/man/.*/man.*
#  - /usr/info.*
#  - /usr/share/man/man.*
#  - /usr/share/man/.*/man.*
#  - /usr/share/info.*
#  - /usr/kerberos/man.*
#  - /usr/X11R6/man/man.*
#  - /usr/lib/perl5/man/man.*
#  - /usr/share/doc/.*/man/man.*
#  - /usr/lib/.*/man/man.*
#
# Packaging of Symbolic Links
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# CPackRPM supports packaging of symbolic links::
#
#   execute_process(COMMAND ${CMAKE_COMMAND}
#     -E create_symlink <relative_path_location> <symlink_name>)
#   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/<symlink_name>
#     DESTINATION <symlink_location> COMPONENT libraries)
#
# Symbolic links will be optimized (paths will be shortened if possible)
# before being added to the package or if multiple relocation paths are
# detected, a post install symlink relocation script will be generated.
#
# Symbolic links may point to locations that are not packaged by the same
# package (either a different component or even not packaged at all) but
# those locations will be treated as if they were a part of the package
# while determining if symlink should be either created or present in a
# post install script - depending on relocation paths.
#
# Currenty there are a few limitations though:
#
# * Only symbolic links with relative path can be packaged.
#
# * For component based packaging component interdependency is not checked
#   when processing symbolic links. Symbolic links pointing to content of
#   a different component are treated the same way as if pointing to location
#   that will not be packaged.
#
# * Symbolic links pointing to a location through one or more intermediate
#   symbolic links will not be handled differently - if the intermediate
#   symbolic link(s) is also on a relocatable path, relocating it during
#   package installation may cause initial symbolic link to point to an
#   invalid location.

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
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

# Author: Eric Noulard with the help of Alexander Neundorf.

function(cpack_rpm_prepare_relocation_paths)
  # set appropriate prefix, remove possible trailing slash and convert backslashes to slashes
  if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_PREFIX)
    file(TO_CMAKE_PATH "${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_PREFIX}" PATH_PREFIX)
  else()
    file(TO_CMAKE_PATH "${CPACK_PACKAGING_INSTALL_PREFIX}" PATH_PREFIX)
  endif()

  set(RPM_RELOCATION_PATHS "${CPACK_RPM_RELOCATION_PATHS}")
  list(REMOVE_DUPLICATES RPM_RELOCATION_PATHS)

  # set base path prefix
  if(EXISTS "${WDIR}/${PATH_PREFIX}")
    if(NOT CPACK_RPM_NO_INSTALL_PREFIX_RELOCATION AND
       NOT CPACK_RPM_NO_${CPACK_RPM_PACKAGE_COMPONENT}_INSTALL_PREFIX_RELOCATION)
      set(TMP_RPM_PREFIXES "${TMP_RPM_PREFIXES}Prefix: ${PATH_PREFIX}\n")
      list(APPEND RPM_USED_PACKAGE_PREFIXES "${PATH_PREFIX}")

      if(CPACK_RPM_PACKAGE_DEBUG)
        message("CPackRPM:Debug: removing '${PATH_PREFIX}' from relocation paths")
      endif()
    endif()
  endif()

  # set other path prefixes
  foreach(RELOCATION_PATH ${RPM_RELOCATION_PATHS})
    if(IS_ABSOLUTE "${RELOCATION_PATH}")
      set(PREPARED_RELOCATION_PATH "${RELOCATION_PATH}")
    else()
      set(PREPARED_RELOCATION_PATH "${PATH_PREFIX}/${RELOCATION_PATH}")
    endif()

    if(EXISTS "${WDIR}/${PREPARED_RELOCATION_PATH}")
      set(TMP_RPM_PREFIXES "${TMP_RPM_PREFIXES}Prefix: ${PREPARED_RELOCATION_PATH}\n")
      list(APPEND RPM_USED_PACKAGE_PREFIXES "${PREPARED_RELOCATION_PATH}")
    endif()
  endforeach()

  # warn about all the paths that are not relocatable
  cmake_policy(PUSH)
    # Tell file(GLOB_RECURSE) not to follow directory symlinks
    # even if the project does not set this policy to NEW.
    cmake_policy(SET CMP0009 NEW)
    file(GLOB_RECURSE FILE_PATHS_ "${WDIR}/*")
  cmake_policy(POP)
  foreach(TMP_PATH ${FILE_PATHS_})
    string(LENGTH "${WDIR}" WDIR_LEN)
    string(SUBSTRING "${TMP_PATH}" ${WDIR_LEN} -1 TMP_PATH)
    unset(TMP_PATH_FOUND_)

    foreach(RELOCATION_PATH ${RPM_USED_PACKAGE_PREFIXES})
      file(RELATIVE_PATH REL_PATH_ "${RELOCATION_PATH}" "${TMP_PATH}")
      string(SUBSTRING "${REL_PATH_}" 0 2 PREFIX_)

      if(NOT "${PREFIX_}" STREQUAL "..")
        set(TPM_PATH_FOUND_ TRUE)
        break()
      endif()
    endforeach()

    if(NOT TPM_PATH_FOUND_)
      message(AUTHOR_WARNING "CPackRPM:Warning: Path ${TMP_PATH} is not on one of the relocatable paths! Package will be partially relocatable.")
    endif()
  endforeach()

  set(RPM_USED_PACKAGE_PREFIXES "${RPM_USED_PACKAGE_PREFIXES}" PARENT_SCOPE)
  set(TMP_RPM_PREFIXES "${TMP_RPM_PREFIXES}" PARENT_SCOPE)
endfunction()

function(cpack_rpm_prepare_content_list)
  # get files list
  cmake_policy(PUSH)
    cmake_policy(SET CMP0009 NEW)
    file(GLOB_RECURSE CPACK_RPM_INSTALL_FILES LIST_DIRECTORIES true RELATIVE "${WDIR}" "${WDIR}/*")
  cmake_policy(POP)
  set(CPACK_RPM_INSTALL_FILES "/${CPACK_RPM_INSTALL_FILES}")
  string(REPLACE ";" ";/" CPACK_RPM_INSTALL_FILES "${CPACK_RPM_INSTALL_FILES}")

  # if we are creating a relocatable package, omit parent directories of
  # CPACK_RPM_PACKAGE_PREFIX. This is achieved by building a "filter list"
  # which is passed to the find command that generates the content-list
  if(CPACK_RPM_PACKAGE_RELOCATABLE)
    # get a list of the elements in CPACK_RPM_PACKAGE_PREFIXES that are
    # destinct parent paths of other relocation paths and remove the
    # final element (so the install-prefix dir itself is not omitted
    # from the RPM's content-list)
    list(SORT RPM_USED_PACKAGE_PREFIXES)
    set(_DISTINCT_PATH "NOT_SET")
    foreach(_RPM_RELOCATION_PREFIX ${RPM_USED_PACKAGE_PREFIXES})
      if(NOT "${_RPM_RELOCATION_PREFIX}" MATCHES "${_DISTINCT_PATH}/.*")
        set(_DISTINCT_PATH "${_RPM_RELOCATION_PREFIX}")

        string(REPLACE "/" ";" _CPACK_RPM_PACKAGE_PREFIX_ELEMS " ${_RPM_RELOCATION_PREFIX}")
        list(REMOVE_AT _CPACK_RPM_PACKAGE_PREFIX_ELEMS -1)
        unset(_TMP_LIST)
        # Now generate all of the parent dirs of the relocation path
        foreach(_PREFIX_PATH_ELEM ${_CPACK_RPM_PACKAGE_PREFIX_ELEMS})
          list(APPEND _TMP_LIST "${_PREFIX_PATH_ELEM}")
          string(REPLACE ";" "/" _OMIT_DIR "${_TMP_LIST}")
          separate_arguments(_OMIT_DIR)
          list(APPEND _RPM_DIRS_TO_OMIT ${_OMIT_DIR})
        endforeach()
      endif()
    endforeach()
  endif()

  if(CPACK_RPM_PACKAGE_DEBUG)
    message("CPackRPM:Debug: Initial list of path to OMIT in RPM: ${_RPM_DIRS_TO_OMIT}")
  endif()

  if(NOT DEFINED CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST)
    set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST /etc /etc/init.d /usr /usr/share /usr/share/doc /usr/bin /usr/lib /usr/lib64 /usr/include)
    if(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION)
      message("CPackRPM:Debug: Adding ${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION} to builtin omit list.")
      list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST "${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION}")
    endif()
  endif()

  if(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST)
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST= ${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST}")
    endif()
    list(APPEND _RPM_DIRS_TO_OMIT ${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST})
  endif()

  if(CPACK_RPM_PACKAGE_DEBUG)
    message("CPackRPM:Debug: Final list of path to OMIT in RPM: ${_RPM_DIRS_TO_OMIT}")
  endif()

  list(REMOVE_ITEM CPACK_RPM_INSTALL_FILES ${_RPM_DIRS_TO_OMIT})

  # add man paths that will be compressed
  # (copied from /usr/lib/rpm/brp-compress - script that does the actual
  # compressing)
  list(APPEND MAN_LOCATIONS "/usr/man/man.*" "/usr/man/.*/man.*" "/usr/info.*"
    "/usr/share/man/man.*" "/usr/share/man/.*/man.*" "/usr/share/info.*"
    "/usr/kerberos/man.*" "/usr/X11R6/man/man.*" "/usr/lib/perl5/man/man.*"
    "/usr/share/doc/.*/man/man.*" "/usr/lib/.*/man/man.*")

  if(CPACK_RPM_ADDITIONAL_MAN_DIRS)
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: CPACK_RPM_ADDITIONAL_MAN_DIRS= ${CPACK_RPM_ADDITIONAL_MAN_DIRS}")
    endif()
    list(APPEND MAN_LOCATIONS ${CPACK_RPM_ADDITIONAL_MAN_DIRS})
  endif()

  foreach(PACK_LOCATION IN LISTS CPACK_RPM_INSTALL_FILES)
    foreach(MAN_LOCATION IN LISTS MAN_LOCATIONS)
      # man pages are files inside a certain location
      if(PACK_LOCATION MATCHES "${MAN_LOCATION}/"
        AND NOT IS_DIRECTORY "${WDIR}${PACK_LOCATION}"
        AND NOT IS_SYMLINK "${WDIR}${PACK_LOCATION}")
        list(FIND CPACK_RPM_INSTALL_FILES "${PACK_LOCATION}" INDEX)
        # insert file location that covers compressed man pages
        # even if using a wildcard causes duplicates as those are
        # handled by RPM and we still keep the same file list
        # in spec file - wildcard only represents file type (e.g. .gz)
        list(INSERT CPACK_RPM_INSTALL_FILES ${INDEX} "${PACK_LOCATION}*")
        # remove file location that doesn't cover compressed man pages
        math(EXPR INDEX ${INDEX}+1)
        list(REMOVE_AT CPACK_RPM_INSTALL_FILES ${INDEX})

        break()
      endif()
    endforeach()
  endforeach()

  set(CPACK_RPM_INSTALL_FILES "${CPACK_RPM_INSTALL_FILES}" PARENT_SCOPE)
endfunction()

function(cpack_rpm_symlink_get_relocation_prefixes LOCATION PACKAGE_PREFIXES RETURN_VARIABLE)
  foreach(PKG_PREFIX IN LISTS PACKAGE_PREFIXES)
    string(REGEX MATCH "^${PKG_PREFIX}/.*" FOUND_ "${LOCATION}")
    if(FOUND_)
      list(APPEND TMP_PREFIXES "${PKG_PREFIX}")
    endif()
  endforeach()

  set(${RETURN_VARIABLE} "${TMP_PREFIXES}" PARENT_SCOPE)
endfunction()

function(cpack_rpm_symlink_create_relocation_script PACKAGE_PREFIXES)
  list(LENGTH PACKAGE_PREFIXES LAST_INDEX)
  set(SORTED_PACKAGE_PREFIXES "${PACKAGE_PREFIXES}")
  list(SORT SORTED_PACKAGE_PREFIXES)
  list(REVERSE SORTED_PACKAGE_PREFIXES)
  math(EXPR LAST_INDEX ${LAST_INDEX}-1)

  foreach(SYMLINK_INDEX RANGE ${LAST_INDEX})
    list(GET SORTED_PACKAGE_PREFIXES ${SYMLINK_INDEX} SRC_PATH)
    list(FIND PACKAGE_PREFIXES "${SRC_PATH}" SYMLINK_INDEX) # reverse magic
    string(LENGTH "${SRC_PATH}" SRC_PATH_LEN)

    set(PARTS_CNT 0)
    set(SCRIPT_PART "if [ \"$RPM_INSTALL_PREFIX${SYMLINK_INDEX}\" != \"${SRC_PATH}\" ]; then\n")

    # both paths relocated
    foreach(POINT_INDEX RANGE ${LAST_INDEX})
      list(GET SORTED_PACKAGE_PREFIXES ${POINT_INDEX} POINT_PATH)
      list(FIND PACKAGE_PREFIXES "${POINT_PATH}" POINT_INDEX) # reverse magic
      string(LENGTH "${POINT_PATH}" POINT_PATH_LEN)

      if(_RPM_RELOCATION_SCRIPT_${SYMLINK_INDEX}_${POINT_INDEX})
        if("${SYMLINK_INDEX}" EQUAL "${POINT_INDEX}")
          set(INDENT "")
        else()
          set(SCRIPT_PART "${SCRIPT_PART}  if [ \"$RPM_INSTALL_PREFIX${POINT_INDEX}\" != \"${POINT_PATH}\" ]; then\n")
          set(INDENT "  ")
        endif()

        foreach(RELOCATION_NO IN LISTS _RPM_RELOCATION_SCRIPT_${SYMLINK_INDEX}_${POINT_INDEX})
          math(EXPR PARTS_CNT ${PARTS_CNT}+1)

          math(EXPR RELOCATION_INDEX ${RELOCATION_NO}-1)
          list(GET _RPM_RELOCATION_SCRIPT_PAIRS ${RELOCATION_INDEX} RELOCATION_SCRIPT_PAIR)
          string(FIND "${RELOCATION_SCRIPT_PAIR}" ":" SPLIT_INDEX)

          math(EXPR SRC_PATH_END ${SPLIT_INDEX}-${SRC_PATH_LEN})
          string(SUBSTRING ${RELOCATION_SCRIPT_PAIR} ${SRC_PATH_LEN} ${SRC_PATH_END} SYMLINK_)

          math(EXPR POINT_PATH_START ${SPLIT_INDEX}+1+${POINT_PATH_LEN})
          string(SUBSTRING ${RELOCATION_SCRIPT_PAIR} ${POINT_PATH_START} -1 POINT_)

          set(SCRIPT_PART "${SCRIPT_PART}  ${INDENT}if [ -z \"$CPACK_RPM_RELOCATED_SYMLINK_${RELOCATION_INDEX}\" ]; then\n")
          set(SCRIPT_PART "${SCRIPT_PART}    ${INDENT}ln -s \"$RPM_INSTALL_PREFIX${POINT_INDEX}${POINT_}\" \"$RPM_INSTALL_PREFIX${SYMLINK_INDEX}${SYMLINK_}\"\n")
          set(SCRIPT_PART "${SCRIPT_PART}    ${INDENT}CPACK_RPM_RELOCATED_SYMLINK_${RELOCATION_INDEX}=true\n")
          set(SCRIPT_PART "${SCRIPT_PART}  ${INDENT}fi\n")
        endforeach()

        if(NOT "${SYMLINK_INDEX}" EQUAL "${POINT_INDEX}")
          set(SCRIPT_PART "${SCRIPT_PART}  fi\n")
        endif()
      endif()
    endforeach()

    # source path relocated
    if(_RPM_RELOCATION_SCRIPT_${SYMLINK_INDEX}_X)
      foreach(RELOCATION_NO IN LISTS _RPM_RELOCATION_SCRIPT_${SYMLINK_INDEX}_X)
        math(EXPR PARTS_CNT ${PARTS_CNT}+1)

        math(EXPR RELOCATION_INDEX ${RELOCATION_NO}-1)
        list(GET _RPM_RELOCATION_SCRIPT_PAIRS ${RELOCATION_INDEX} RELOCATION_SCRIPT_PAIR)
        string(FIND "${RELOCATION_SCRIPT_PAIR}" ":" SPLIT_INDEX)

        math(EXPR SRC_PATH_END ${SPLIT_INDEX}-${SRC_PATH_LEN})
        string(SUBSTRING ${RELOCATION_SCRIPT_PAIR} ${SRC_PATH_LEN} ${SRC_PATH_END} SYMLINK_)

        math(EXPR POINT_PATH_START ${SPLIT_INDEX}+1)
        string(SUBSTRING ${RELOCATION_SCRIPT_PAIR} ${POINT_PATH_START} -1 POINT_)

        set(SCRIPT_PART "${SCRIPT_PART}  if [ -z \"$CPACK_RPM_RELOCATED_SYMLINK_${RELOCATION_INDEX}\" ]; then\n")
        set(SCRIPT_PART "${SCRIPT_PART}    ln -s \"${POINT_}\" \"$RPM_INSTALL_PREFIX${SYMLINK_INDEX}${SYMLINK_}\"\n")
        set(SCRIPT_PART "${SCRIPT_PART}    CPACK_RPM_RELOCATED_SYMLINK_${RELOCATION_INDEX}=true\n")
        set(SCRIPT_PART "${SCRIPT_PART}  fi\n")
      endforeach()
    endif()

    if(PARTS_CNT)
      set(SCRIPT "${SCRIPT_PART}")
      set(SCRIPT "${SCRIPT}fi\n")
    endif()
  endforeach()

  # point path relocated
  foreach(POINT_INDEX RANGE ${LAST_INDEX})
    list(GET SORTED_PACKAGE_PREFIXES ${POINT_INDEX} POINT_PATH)
    list(FIND PACKAGE_PREFIXES "${POINT_PATH}" POINT_INDEX) # reverse magic
    string(LENGTH "${POINT_PATH}" POINT_PATH_LEN)

    if(_RPM_RELOCATION_SCRIPT_X_${POINT_INDEX})
      set(SCRIPT "${SCRIPT}if [ \"$RPM_INSTALL_PREFIX${POINT_INDEX}\" != \"${POINT_PATH}\" ]; then\n")

      foreach(RELOCATION_NO IN LISTS _RPM_RELOCATION_SCRIPT_X_${POINT_INDEX})
        math(EXPR RELOCATION_INDEX ${RELOCATION_NO}-1)
        list(GET _RPM_RELOCATION_SCRIPT_PAIRS ${RELOCATION_INDEX} RELOCATION_SCRIPT_PAIR)
        string(FIND "${RELOCATION_SCRIPT_PAIR}" ":" SPLIT_INDEX)

        string(SUBSTRING ${RELOCATION_SCRIPT_PAIR} 0 ${SPLIT_INDEX} SYMLINK_)

        math(EXPR POINT_PATH_START ${SPLIT_INDEX}+1+${POINT_PATH_LEN})
        string(SUBSTRING ${RELOCATION_SCRIPT_PAIR} ${POINT_PATH_START} -1 POINT_)

        set(SCRIPT "${SCRIPT}  if [ -z \"$CPACK_RPM_RELOCATED_SYMLINK_${RELOCATION_INDEX}\" ]; then\n")
        set(SCRIPT "${SCRIPT}    ln -s \"$RPM_INSTALL_PREFIX${POINT_INDEX}${POINT_}\" \"${SYMLINK_}\"\n")
        set(SCRIPT "${SCRIPT}    CPACK_RPM_RELOCATED_SYMLINK_${RELOCATION_INDEX}=true\n")
        set(SCRIPT "${SCRIPT}  fi\n")
      endforeach()

      set(SCRIPT "${SCRIPT}fi\n")
    endif()
  endforeach()

  # no path relocated
  if(_RPM_RELOCATION_SCRIPT_X_X)
    foreach(RELOCATION_NO IN LISTS _RPM_RELOCATION_SCRIPT_X_X)
      math(EXPR RELOCATION_INDEX ${RELOCATION_NO}-1)
      list(GET _RPM_RELOCATION_SCRIPT_PAIRS ${RELOCATION_INDEX} RELOCATION_SCRIPT_PAIR)
      string(FIND "${RELOCATION_SCRIPT_PAIR}" ":" SPLIT_INDEX)

      string(SUBSTRING ${RELOCATION_SCRIPT_PAIR} 0 ${SPLIT_INDEX} SYMLINK_)

      math(EXPR POINT_PATH_START ${SPLIT_INDEX}+1)
      string(SUBSTRING ${RELOCATION_SCRIPT_PAIR} ${POINT_PATH_START} -1 POINT_)

      set(SCRIPT "${SCRIPT}if [ -z \"$CPACK_RPM_RELOCATED_SYMLINK_${RELOCATION_INDEX}\" ]; then\n")
      set(SCRIPT "${SCRIPT}  ln -s \"${POINT_}\" \"${SYMLINK_}\"\n")
      set(SCRIPT "${SCRIPT}fi\n")
    endforeach()
  endif()

  set(RPM_SYMLINK_POSTINSTALL "${SCRIPT}" PARENT_SCOPE)
endfunction()

function(cpack_rpm_symlink_add_for_relocation_script PACKAGE_PREFIXES SYMLINK SYMLINK_RELOCATION_PATHS POINT POINT_RELOCATION_PATHS)
  list(LENGTH SYMLINK_RELOCATION_PATHS SYMLINK_PATHS_COUTN)
  list(LENGTH POINT_RELOCATION_PATHS POINT_PATHS_COUNT)

  list(APPEND _RPM_RELOCATION_SCRIPT_PAIRS "${SYMLINK}:${POINT}")
  list(LENGTH _RPM_RELOCATION_SCRIPT_PAIRS PAIR_NO)

  if(SYMLINK_PATHS_COUTN)
    foreach(SYMLINK_RELOC_PATH IN LISTS SYMLINK_RELOCATION_PATHS)
      list(FIND PACKAGE_PREFIXES "${SYMLINK_RELOC_PATH}" SYMLINK_INDEX)

      # source path relocated
      list(APPEND _RPM_RELOCATION_SCRIPT_${SYMLINK_INDEX}_X "${PAIR_NO}")
      list(APPEND RELOCATION_VARS "_RPM_RELOCATION_SCRIPT_${SYMLINK_INDEX}_X")

      foreach(POINT_RELOC_PATH IN LISTS POINT_RELOCATION_PATHS)
        list(FIND PACKAGE_PREFIXES "${POINT_RELOC_PATH}" POINT_INDEX)

        # both paths relocated
        list(APPEND _RPM_RELOCATION_SCRIPT_${SYMLINK_INDEX}_${POINT_INDEX} "${PAIR_NO}")
        list(APPEND RELOCATION_VARS "_RPM_RELOCATION_SCRIPT_${SYMLINK_INDEX}_${POINT_INDEX}")

        # point path relocated
        list(APPEND _RPM_RELOCATION_SCRIPT_X_${POINT_INDEX} "${PAIR_NO}")
        list(APPEND RELOCATION_VARS "_RPM_RELOCATION_SCRIPT_X_${POINT_INDEX}")
      endforeach()
    endforeach()
  elseif(POINT_PATHS_COUNT)
    foreach(POINT_RELOC_PATH IN LISTS POINT_RELOCATION_PATHS)
      list(FIND PACKAGE_PREFIXES "${POINT_RELOC_PATH}" POINT_INDEX)

      # point path relocated
      list(APPEND _RPM_RELOCATION_SCRIPT_X_${POINT_INDEX} "${PAIR_NO}")
      list(APPEND RELOCATION_VARS "_RPM_RELOCATION_SCRIPT_X_${POINT_INDEX}")
    endforeach()
  endif()

  # no path relocated
  list(APPEND _RPM_RELOCATION_SCRIPT_X_X "${PAIR_NO}")
  list(APPEND RELOCATION_VARS "_RPM_RELOCATION_SCRIPT_X_X")

  # place variables into parent scope
  foreach(VAR IN LISTS RELOCATION_VARS)
    set(${VAR} "${${VAR}}" PARENT_SCOPE)
  endforeach()
  set(_RPM_RELOCATION_SCRIPT_PAIRS "${_RPM_RELOCATION_SCRIPT_PAIRS}" PARENT_SCOPE)
  set(REQUIRES_SYMLINK_RELOCATION_SCRIPT "true" PARENT_SCOPE)
  set(DIRECTIVE "%ghost " PARENT_SCOPE)
endfunction()

function(cpack_rpm_prepare_install_files INSTALL_FILES_LIST WDIR PACKAGE_PREFIXES IS_RELOCATABLE)
  # Prepend directories in ${CPACK_RPM_INSTALL_FILES} with %dir
  # This is necessary to avoid duplicate files since rpmbuild does
  # recursion on its own when encountering a pathname which is a directory
  # which is not flagged as %dir
  string(STRIP "${INSTALL_FILES_LIST}" INSTALL_FILES_LIST)
  string(REPLACE "\n" ";" INSTALL_FILES_LIST
                          "${INSTALL_FILES_LIST}")
  string(REPLACE "\"" "" INSTALL_FILES_LIST
                          "${INSTALL_FILES_LIST}")
  string(LENGTH "${WDIR}" WDR_LEN_)

  list(SORT INSTALL_FILES_LIST) # make file order consistent on all platforms

  foreach(F IN LISTS INSTALL_FILES_LIST)
    unset(DIRECTIVE)

    if(IS_SYMLINK "${WDIR}/${F}")
      if(IS_RELOCATABLE)
        # check that symlink has relocatable format
        get_filename_component(SYMLINK_LOCATION_ "${WDIR}/${F}" DIRECTORY)
        execute_process(COMMAND ls -la "${WDIR}/${F}"
                  WORKING_DIRECTORY "${WDIR}"
                  OUTPUT_VARIABLE SYMLINK_POINT_
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

        string(FIND "${SYMLINK_POINT_}" "->" SYMLINK_POINT_INDEX_ REVERSE)
        math(EXPR SYMLINK_POINT_INDEX_ ${SYMLINK_POINT_INDEX_}+3)
        string(LENGTH "${SYMLINK_POINT_}" SYMLINK_POINT_LENGTH_)

        # get destination path
        string(SUBSTRING "${SYMLINK_POINT_}" ${SYMLINK_POINT_INDEX_} ${SYMLINK_POINT_LENGTH_} SYMLINK_POINT_)

        # check if path is relative or absolute
        string(SUBSTRING "${SYMLINK_POINT_}" 0 1 SYMLINK_IS_ABSOLUTE_)

        if(${SYMLINK_IS_ABSOLUTE_} STREQUAL "/")
          # prevent absolute paths from having /../ or /./ section inside of them
          get_filename_component(SYMLINK_POINT_ "${SYMLINK_POINT_}" ABSOLUTE)
        else()
          # handle relative path
          get_filename_component(SYMLINK_POINT_ "${SYMLINK_LOCATION_}/${SYMLINK_POINT_}" ABSOLUTE)
        endif()

        string(SUBSTRING "${SYMLINK_POINT_}" ${WDR_LEN_} -1 SYMLINK_POINT_WD_)

        cpack_rpm_symlink_get_relocation_prefixes("${F}" "${PACKAGE_PREFIXES}" "SYMLINK_RELOCATIONS")
        cpack_rpm_symlink_get_relocation_prefixes("${SYMLINK_POINT_WD_}" "${PACKAGE_PREFIXES}" "POINT_RELOCATIONS")

        list(LENGTH SYMLINK_RELOCATIONS SYMLINK_RELOCATIONS_COUNT)
        list(LENGTH POINT_RELOCATIONS POINT_RELOCATIONS_COUNT)

        if(SYMLINK_RELOCATIONS_COUNT AND POINT_RELOCATIONS_COUNT)
          # find matching
          foreach(SYMLINK_RELOCATION_PREFIX IN LISTS SYMLINK_RELOCATIONS)
            list(FIND POINT_RELOCATIONS "${SYMLINK_RELOCATION_PREFIX}" FOUND_INDEX)
            if(NOT ${FOUND_INDEX} EQUAL -1)
              break()
            endif()
          endforeach()

          if(NOT ${FOUND_INDEX} EQUAL -1)
            # symlinks have the same subpath
            if(${SYMLINK_RELOCATIONS_COUNT} EQUAL 1 AND ${POINT_RELOCATIONS_COUNT} EQUAL 1)
              # permanent symlink
              get_filename_component(SYMLINK_LOCATION_ "${F}" DIRECTORY)
              file(RELATIVE_PATH FINAL_PATH_ ${SYMLINK_LOCATION_} ${SYMLINK_POINT_WD_})
              execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink "${FINAL_PATH_}" "${WDIR}/${F}")
            else()
              # relocation subpaths
              cpack_rpm_symlink_add_for_relocation_script("${PACKAGE_PREFIXES}" "${F}" "${SYMLINK_RELOCATIONS}"
                  "${SYMLINK_POINT_WD_}" "${POINT_RELOCATIONS}")
            endif()
          else()
            # not on the same relocation path
            cpack_rpm_symlink_add_for_relocation_script("${PACKAGE_PREFIXES}" "${F}" "${SYMLINK_RELOCATIONS}"
                "${SYMLINK_POINT_WD_}" "${POINT_RELOCATIONS}")
          endif()
        elseif(POINT_RELOCATIONS_COUNT)
          # point is relocatable
          cpack_rpm_symlink_add_for_relocation_script("${PACKAGE_PREFIXES}" "${F}" "${SYMLINK_RELOCATIONS}"
              "${SYMLINK_POINT_WD_}" "${POINT_RELOCATIONS}")
        else()
          # is not relocatable or points to non relocatable path - permanent symlink
          execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink "${SYMLINK_POINT_WD_}" "${WDIR}/${F}")
        endif()
      endif()
    elseif(IS_DIRECTORY "${WDIR}/${F}")
      set(DIRECTIVE "%dir ")
    endif()

    set(INSTALL_FILES "${INSTALL_FILES}${DIRECTIVE}\"${F}\"\n")
  endforeach()

  if(REQUIRES_SYMLINK_RELOCATION_SCRIPT)
    cpack_rpm_symlink_create_relocation_script("${PACKAGE_PREFIXES}")
  endif()

  set(RPM_SYMLINK_POSTINSTALL "${RPM_SYMLINK_POSTINSTALL}" PARENT_SCOPE)
  set(CPACK_RPM_INSTALL_FILES "${INSTALL_FILES}" PARENT_SCOPE)
endfunction()

if(CMAKE_BINARY_DIR)
  message(FATAL_ERROR "CPackRPM.cmake may only be used by CPack internally.")
endif()

if(NOT UNIX)
  message(FATAL_ERROR "CPackRPM.cmake may only be used under UNIX.")
endif()

function(cpack_rpm_generate_package)
  # rpmbuild is the basic command for building RPM package
  # it may be a simple (symbolic) link to rpm command.
  find_program(RPMBUILD_EXECUTABLE rpmbuild)

  # Check version of the rpmbuild tool this would be easier to
  # track bugs with users and CPackRPM debug mode.
  # We may use RPM version in order to check for available version dependent features
  if(RPMBUILD_EXECUTABLE)
    execute_process(COMMAND ${RPMBUILD_EXECUTABLE} --version
                    OUTPUT_VARIABLE _TMP_VERSION
                    ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^.* " ""
           RPMBUILD_EXECUTABLE_VERSION
           ${_TMP_VERSION})
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: rpmbuild version is <${RPMBUILD_EXECUTABLE_VERSION}>")
    endif()
  endif()

  if(NOT RPMBUILD_EXECUTABLE)
    message(FATAL_ERROR "RPM package requires rpmbuild executable")
  endif()

  # Display lsb_release output if DEBUG mode enable
  # This will help to diagnose problem with CPackRPM
  # because we will know on which kind of Linux we are
  if(CPACK_RPM_PACKAGE_DEBUG)
    find_program(LSB_RELEASE_EXECUTABLE lsb_release)
    if(LSB_RELEASE_EXECUTABLE)
      execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -a
                      OUTPUT_VARIABLE _TMP_LSB_RELEASE_OUTPUT
                      ERROR_QUIET
                      OUTPUT_STRIP_TRAILING_WHITESPACE)
      string(REGEX REPLACE "\n" ", "
             LSB_RELEASE_OUTPUT
             ${_TMP_LSB_RELEASE_OUTPUT})
    else ()
      set(LSB_RELEASE_OUTPUT "lsb_release not installed/found!")
    endif()
    message("CPackRPM:Debug: LSB_RELEASE  = ${LSB_RELEASE_OUTPUT}")
  endif()

  # We may use RPM version in the future in order
  # to shut down warning about space in buildtree
  # some recent RPM version should support space in different places.
  # not checked [yet].
  if(CPACK_TOPLEVEL_DIRECTORY MATCHES ".* .*")
    message(FATAL_ERROR "${RPMBUILD_EXECUTABLE} can't handle paths with spaces, use a build directory without spaces for building RPMs.")
  endif()

  # If rpmbuild is found
  # we try to discover alien since we may be on non RPM distro like Debian.
  # In this case we may try to to use more advanced features
  # like generating RPM directly from DEB using alien.
  # FIXME feature not finished (yet)
  find_program(ALIEN_EXECUTABLE alien)
  if(ALIEN_EXECUTABLE)
    message(STATUS "alien found, we may be on a Debian based distro.")
  endif()

  # Are we packaging components ?
  if(CPACK_RPM_PACKAGE_COMPONENT)
    set(CPACK_RPM_PACKAGE_COMPONENT_PART_NAME "-${CPACK_RPM_PACKAGE_COMPONENT}")
    string(TOUPPER ${CPACK_RPM_PACKAGE_COMPONENT} CPACK_RPM_PACKAGE_COMPONENT_UPPER)
  else()
    set(CPACK_RPM_PACKAGE_COMPONENT_PART_NAME "")
  endif()

  set(WDIR "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}${CPACK_RPM_PACKAGE_COMPONENT_PART_PATH}")

  #
  # Use user-defined RPM specific variables value
  # or generate reasonable default value from
  # CPACK_xxx generic values.
  # The variables comes from the needed (mandatory or not)
  # values found in the RPM specification file aka ".spec" file.
  # The variables which may/should be defined are:
  #

  # CPACK_RPM_PACKAGE_SUMMARY (mandatory)

  #Check for component summary first.
  #If not set, it will use regular package summary logic.
  if(CPACK_RPM_PACKAGE_COMPONENT)
    if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_SUMMARY)
      set(CPACK_RPM_PACKAGE_SUMMARY ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_SUMMARY})
    endif()
  endif()

  if(NOT CPACK_RPM_PACKAGE_SUMMARY)
    if(CPACK_PACKAGE_DESCRIPTION_SUMMARY)
      set(CPACK_RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
    else()
      # if neither var is defined lets use the name as summary
      string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_RPM_PACKAGE_SUMMARY)
    endif()
  endif()

  # CPACK_RPM_PACKAGE_NAME (mandatory)
  if(NOT CPACK_RPM_PACKAGE_NAME)
    string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_RPM_PACKAGE_NAME)
  endif()

  # CPACK_RPM_PACKAGE_VERSION (mandatory)
  if(NOT CPACK_RPM_PACKAGE_VERSION)
    if(NOT CPACK_PACKAGE_VERSION)
      message(FATAL_ERROR "RPM package requires a package version")
    endif()
    set(CPACK_RPM_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
  endif()
  # Replace '-' in version with '_'
  # '-' character is  an Illegal RPM version character
  # it is illegal because it is used to separate
  # RPM "Version" from RPM "Release"
  string(REPLACE "-" "_" CPACK_RPM_PACKAGE_VERSION ${CPACK_RPM_PACKAGE_VERSION})

  # CPACK_RPM_PACKAGE_ARCHITECTURE (mandatory)
  if(NOT CPACK_RPM_PACKAGE_ARCHITECTURE)
    execute_process(COMMAND uname "-m"
                    OUTPUT_VARIABLE CPACK_RPM_PACKAGE_ARCHITECTURE
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
  else()
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: using user-specified build arch = ${CPACK_RPM_PACKAGE_ARCHITECTURE}")
    endif()
  endif()

  set(_CPACK_RPM_PACKAGE_ARCHITECTURE ${CPACK_RPM_PACKAGE_ARCHITECTURE})

  #prefer component architecture
  if(CPACK_RPM_PACKAGE_COMPONENT)
    if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_ARCHITECTURE)
      set(_CPACK_RPM_PACKAGE_ARCHITECTURE ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_ARCHITECTURE})
      if(CPACK_RPM_PACKAGE_DEBUG)
        message("CPackRPM:Debug: using component build arch = ${_CPACK_RPM_PACKAGE_ARCHITECTURE}")
      endif()
    endif()
  endif()
  if(${_CPACK_RPM_PACKAGE_ARCHITECTURE} STREQUAL "noarch")
    set(TMP_RPM_BUILDARCH "Buildarch: ${_CPACK_RPM_PACKAGE_ARCHITECTURE}")
  else()
    set(TMP_RPM_BUILDARCH "")
  endif()

  # CPACK_RPM_PACKAGE_RELEASE
  # The RPM release is the numbering of the RPM package ITSELF
  # this is the version of the PACKAGING and NOT the version
  # of the CONTENT of the package.
  # You may well need to generate a new RPM package release
  # without changing the version of the packaged software.
  # This is the case when the packaging is buggy (not) the software :=)
  # If not set, 1 is a good candidate
  if(NOT CPACK_RPM_PACKAGE_RELEASE)
    set(CPACK_RPM_PACKAGE_RELEASE 1)
  endif()

  # CPACK_RPM_PACKAGE_LICENSE
  if(NOT CPACK_RPM_PACKAGE_LICENSE)
    set(CPACK_RPM_PACKAGE_LICENSE "unknown")
  endif()

  # CPACK_RPM_PACKAGE_GROUP
  if(NOT CPACK_RPM_PACKAGE_GROUP)
    set(CPACK_RPM_PACKAGE_GROUP "unknown")
  endif()

  # CPACK_RPM_PACKAGE_VENDOR
  if(NOT CPACK_RPM_PACKAGE_VENDOR)
    if(CPACK_PACKAGE_VENDOR)
      set(CPACK_RPM_PACKAGE_VENDOR "${CPACK_PACKAGE_VENDOR}")
    else()
      set(CPACK_RPM_PACKAGE_VENDOR "unknown")
    endif()
  endif()

  # CPACK_RPM_PACKAGE_SOURCE
  # The name of the source tarball in case we generate a source RPM

  # CPACK_RPM_PACKAGE_DESCRIPTION
  # The variable content may be either
  #   - explicitly given by the user or
  #   - filled with the content of CPACK_PACKAGE_DESCRIPTION_FILE
  #     if it is defined
  #   - set to a default value
  #

  #Check for a component description first.
  #If not set, it will use regular package description logic.
  if(CPACK_RPM_PACKAGE_COMPONENT)
    if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_DESCRIPTION)
      set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_DESCRIPTION})
    elseif(CPACK_COMPONENT_${CPACK_RPM_PACKAGE_COMPONENT_UPPER}_DESCRIPTION)
      set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_COMPONENT_${CPACK_RPM_PACKAGE_COMPONENT_UPPER}_DESCRIPTION})
    endif()
  endif()

  if(NOT CPACK_RPM_PACKAGE_DESCRIPTION)
    if(CPACK_PACKAGE_DESCRIPTION_FILE)
      file(READ ${CPACK_PACKAGE_DESCRIPTION_FILE} CPACK_RPM_PACKAGE_DESCRIPTION)
    else ()
      set(CPACK_RPM_PACKAGE_DESCRIPTION "no package description available")
    endif ()
  endif ()

  # CPACK_RPM_COMPRESSION_TYPE
  #
  if (CPACK_RPM_COMPRESSION_TYPE)
     if(CPACK_RPM_PACKAGE_DEBUG)
       message("CPackRPM:Debug: User Specified RPM compression type: ${CPACK_RPM_COMPRESSION_TYPE}")
     endif()
     if(CPACK_RPM_COMPRESSION_TYPE STREQUAL "lzma")
       set(CPACK_RPM_COMPRESSION_TYPE_TMP "%define _binary_payload w9.lzdio")
     endif()
     if(CPACK_RPM_COMPRESSION_TYPE STREQUAL "xz")
       set(CPACK_RPM_COMPRESSION_TYPE_TMP "%define _binary_payload w7.xzdio")
     endif()
     if(CPACK_RPM_COMPRESSION_TYPE STREQUAL "bzip2")
       set(CPACK_RPM_COMPRESSION_TYPE_TMP "%define _binary_payload w9.bzdio")
     endif()
     if(CPACK_RPM_COMPRESSION_TYPE STREQUAL "gzip")
       set(CPACK_RPM_COMPRESSION_TYPE_TMP "%define _binary_payload w9.gzdio")
     endif()
  else()
     set(CPACK_RPM_COMPRESSION_TYPE_TMP "")
  endif()

  if(CPACK_PACKAGE_RELOCATABLE OR CPACK_RPM_PACKAGE_RELOCATABLE)
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: Trying to build a relocatable package")
    endif()
    if(CPACK_SET_DESTDIR AND (NOT CPACK_SET_DESTDIR STREQUAL "I_ON"))
      message("CPackRPM:Warning: CPACK_SET_DESTDIR is set (=${CPACK_SET_DESTDIR}) while requesting a relocatable package (CPACK_RPM_PACKAGE_RELOCATABLE is set): this is not supported, the package won't be relocatable.")
      set(CPACK_RPM_PACKAGE_RELOCATABLE FALSE)
    else()
      set(CPACK_RPM_PACKAGE_PREFIX ${CPACK_PACKAGING_INSTALL_PREFIX}) # kept for back compatibility (provided external RPM spec files)
      cpack_rpm_prepare_relocation_paths()
      set(CPACK_RPM_PACKAGE_RELOCATABLE TRUE)
    endif()
  endif()

  # Check if additional fields for RPM spec header are given
  # There may be some COMPONENT specific variables as well
  # If component specific var is not provided we use the global one
  # for each component
  foreach(_RPM_SPEC_HEADER URL REQUIRES SUGGESTS PROVIDES OBSOLETES PREFIX CONFLICTS AUTOPROV AUTOREQ AUTOREQPROV REQUIRES_PRE REQUIRES_POST REQUIRES_PREUN REQUIRES_POSTUN)
      if(CPACK_RPM_PACKAGE_DEBUG)
        message("CPackRPM:Debug: processing ${_RPM_SPEC_HEADER}")
      endif()
      if(CPACK_RPM_PACKAGE_COMPONENT)
          if(DEFINED CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_${_RPM_SPEC_HEADER})
              if(CPACK_RPM_PACKAGE_DEBUG)
                message("CPackRPM:Debug: using CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_${_RPM_SPEC_HEADER}")
              endif()
              set(CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_${_RPM_SPEC_HEADER}})
          else()
              if(DEFINED CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER})
                if(CPACK_RPM_PACKAGE_DEBUG)
                  message("CPackRPM:Debug: CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_${_RPM_SPEC_HEADER} not defined")
                  message("CPackRPM:Debug: using CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}")
                endif()
                set(CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP ${CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}})
              endif()
          endif()
      else()
          if(DEFINED CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER})
            if(CPACK_RPM_PACKAGE_DEBUG)
              message("CPackRPM:Debug: using CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}")
            endif()
            set(CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP ${CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}})
          endif()
      endif()

    # Treat the RPM Spec keyword iff it has been properly defined
    if(DEFINED CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP)
      # Transform NAME --> Name e.g. PROVIDES --> Provides
      # The Upper-case first letter and lowercase tail is the
      # appropriate value required in the final RPM spec file.
      string(SUBSTRING ${_RPM_SPEC_HEADER} 1 -1 _PACKAGE_HEADER_TAIL)
      string(TOLOWER "${_PACKAGE_HEADER_TAIL}" _PACKAGE_HEADER_TAIL)
      string(SUBSTRING ${_RPM_SPEC_HEADER} 0 1 _PACKAGE_HEADER_NAME)
      set(_PACKAGE_HEADER_NAME "${_PACKAGE_HEADER_NAME}${_PACKAGE_HEADER_TAIL}")
      # The following keywords require parentheses around the "pre" or "post" suffix in the final RPM spec file.
      set(SCRIPTS_REQUIREMENTS_LIST REQUIRES_PRE REQUIRES_POST REQUIRES_PREUN REQUIRES_POSTUN)
      list(FIND SCRIPTS_REQUIREMENTS_LIST ${_RPM_SPEC_HEADER} IS_SCRIPTS_REQUIREMENT_FOUND)
      if(NOT ${IS_SCRIPTS_REQUIREMENT_FOUND} EQUAL -1)
        string(REPLACE "_" "(" _PACKAGE_HEADER_NAME "${_PACKAGE_HEADER_NAME}")
        set(_PACKAGE_HEADER_NAME "${_PACKAGE_HEADER_NAME})")
      endif()
      if(CPACK_RPM_PACKAGE_DEBUG)
        message("CPackRPM:Debug: User defined ${_PACKAGE_HEADER_NAME}:\n ${CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP}")
      endif()
      set(TMP_RPM_${_RPM_SPEC_HEADER} "${_PACKAGE_HEADER_NAME}: ${CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP}")
      unset(CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP)
    endif()
  endforeach()

  # CPACK_RPM_SPEC_INSTALL_POST
  # May be used to define a RPM post intallation script
  # for example setting it to "/bin/true" may prevent
  # rpmbuild from stripping binaries.
  if(CPACK_RPM_SPEC_INSTALL_POST)
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: User defined CPACK_RPM_SPEC_INSTALL_POST = ${CPACK_RPM_SPEC_INSTALL_POST}")
    endif()
    set(TMP_RPM_SPEC_INSTALL_POST "%define __spec_install_post ${CPACK_RPM_SPEC_INSTALL_POST}")
  endif()

  # CPACK_RPM_POST_INSTALL_SCRIPT_FILE (or CPACK_RPM_<COMPONENT>_POST_INSTALL_SCRIPT_FILE)
  # CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE (or CPACK_RPM_<COMPONENT>_POST_UNINSTALL_SCRIPT_FILE)
  # May be used to embed a post (un)installation script in the spec file.
  # The refered script file(s) will be read and directly
  # put after the %post or %postun section
  if(CPACK_RPM_PACKAGE_COMPONENT)
    if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_POST_INSTALL_SCRIPT_FILE)
      set(CPACK_RPM_POST_INSTALL_READ_FILE ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_POST_INSTALL_SCRIPT_FILE})
    else()
      set(CPACK_RPM_POST_INSTALL_READ_FILE ${CPACK_RPM_POST_INSTALL_SCRIPT_FILE})
    endif()
    if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_POST_UNINSTALL_SCRIPT_FILE)
      set(CPACK_RPM_POST_UNINSTALL_READ_FILE ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_POST_UNINSTALL_SCRIPT_FILE})
    else()
      set(CPACK_RPM_POST_UNINSTALL_READ_FILE ${CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE})
    endif()
  else()
    set(CPACK_RPM_POST_INSTALL_READ_FILE ${CPACK_RPM_POST_INSTALL_SCRIPT_FILE})
    set(CPACK_RPM_POST_UNINSTALL_READ_FILE ${CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE})
  endif()

  # Handle post-install file if it has been specified
  if(CPACK_RPM_POST_INSTALL_READ_FILE)
    if(EXISTS ${CPACK_RPM_POST_INSTALL_READ_FILE})
      file(READ ${CPACK_RPM_POST_INSTALL_READ_FILE} CPACK_RPM_SPEC_POSTINSTALL)
    else()
      message("CPackRPM:Warning: CPACK_RPM_POST_INSTALL_SCRIPT_FILE <${CPACK_RPM_POST_INSTALL_READ_FILE}> does not exists - ignoring")
    endif()
  else()
    # reset SPEC var value if no post install file has been specified
    # (either globally or component-wise)
    set(CPACK_RPM_SPEC_POSTINSTALL "")
  endif()

  # Handle post-uninstall file if it has been specified
  if(CPACK_RPM_POST_UNINSTALL_READ_FILE)
    if(EXISTS ${CPACK_RPM_POST_UNINSTALL_READ_FILE})
      file(READ ${CPACK_RPM_POST_UNINSTALL_READ_FILE} CPACK_RPM_SPEC_POSTUNINSTALL)
    else()
      message("CPackRPM:Warning: CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE <${CPACK_RPM_POST_UNINSTALL_READ_FILE}> does not exists - ignoring")
    endif()
  else()
    # reset SPEC var value if no post uninstall file has been specified
    # (either globally or component-wise)
    set(CPACK_RPM_SPEC_POSTUNINSTALL "")
  endif()

  # CPACK_RPM_PRE_INSTALL_SCRIPT_FILE (or CPACK_RPM_<COMPONENT>_PRE_INSTALL_SCRIPT_FILE)
  # CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE (or CPACK_RPM_<COMPONENT>_PRE_UNINSTALL_SCRIPT_FILE)
  # May be used to embed a pre (un)installation script in the spec file.
  # The refered script file(s) will be read and directly
  # put after the %pre or %preun section
  if(CPACK_RPM_PACKAGE_COMPONENT)
    if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_INSTALL_SCRIPT_FILE)
      set(CPACK_RPM_PRE_INSTALL_READ_FILE ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_INSTALL_SCRIPT_FILE})
    else()
      set(CPACK_RPM_PRE_INSTALL_READ_FILE ${CPACK_RPM_PRE_INSTALL_SCRIPT_FILE})
    endif()
    if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_UNINSTALL_SCRIPT_FILE)
      set(CPACK_RPM_PRE_UNINSTALL_READ_FILE ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_UNINSTALL_SCRIPT_FILE})
    else()
      set(CPACK_RPM_PRE_UNINSTALL_READ_FILE ${CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE})
    endif()
  else()
    set(CPACK_RPM_PRE_INSTALL_READ_FILE ${CPACK_RPM_PRE_INSTALL_SCRIPT_FILE})
    set(CPACK_RPM_PRE_UNINSTALL_READ_FILE ${CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE})
  endif()

  # Handle pre-install file if it has been specified
  if(CPACK_RPM_PRE_INSTALL_READ_FILE)
    if(EXISTS ${CPACK_RPM_PRE_INSTALL_READ_FILE})
      file(READ ${CPACK_RPM_PRE_INSTALL_READ_FILE} CPACK_RPM_SPEC_PREINSTALL)
    else()
      message("CPackRPM:Warning: CPACK_RPM_PRE_INSTALL_SCRIPT_FILE <${CPACK_RPM_PRE_INSTALL_READ_FILE}> does not exists - ignoring")
    endif()
  else()
    # reset SPEC var value if no pre-install file has been specified
    # (either globally or component-wise)
    set(CPACK_RPM_SPEC_PREINSTALL "")
  endif()

  # Handle pre-uninstall file if it has been specified
  if(CPACK_RPM_PRE_UNINSTALL_READ_FILE)
    if(EXISTS ${CPACK_RPM_PRE_UNINSTALL_READ_FILE})
      file(READ ${CPACK_RPM_PRE_UNINSTALL_READ_FILE} CPACK_RPM_SPEC_PREUNINSTALL)
    else()
      message("CPackRPM:Warning: CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE <${CPACK_RPM_PRE_UNINSTALL_READ_FILE}> does not exists - ignoring")
    endif()
  else()
    # reset SPEC var value if no pre-uninstall file has been specified
    # (either globally or component-wise)
    set(CPACK_RPM_SPEC_PREUNINSTALL "")
  endif()

  # CPACK_RPM_CHANGELOG_FILE
  # May be used to embed a changelog in the spec file.
  # The refered file will be read and directly put after the %changelog section
  if(CPACK_RPM_CHANGELOG_FILE)
    if(EXISTS ${CPACK_RPM_CHANGELOG_FILE})
      file(READ ${CPACK_RPM_CHANGELOG_FILE} CPACK_RPM_SPEC_CHANGELOG)
    else()
      message(SEND_ERROR "CPackRPM:Warning: CPACK_RPM_CHANGELOG_FILE <${CPACK_RPM_CHANGELOG_FILE}> does not exists - ignoring")
    endif()
  else()
    set(CPACK_RPM_SPEC_CHANGELOG "* Sun Jul 4 2010 Eric Noulard <eric.noulard@gmail.com> - ${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}\n  Generated by CPack RPM (no Changelog file were provided)")
  endif()

  # CPACK_RPM_SPEC_MORE_DEFINE
  # This is a generated spec rpm file spaceholder
  if(CPACK_RPM_SPEC_MORE_DEFINE)
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: User defined more define spec line specified:\n ${CPACK_RPM_SPEC_MORE_DEFINE}")
    endif()
  endif()

  # Now we may create the RPM build tree structure
  set(CPACK_RPM_ROOTDIR "${CPACK_TOPLEVEL_DIRECTORY}")
  message(STATUS "CPackRPM:Debug: Using CPACK_RPM_ROOTDIR=${CPACK_RPM_ROOTDIR}")
  # Prepare RPM build tree
  file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR})
  file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/tmp)
  file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/BUILD)
  file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/RPMS)
  file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/SOURCES)
  file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/SPECS)
  file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/SRPMS)

  #set(CPACK_RPM_FILE_NAME "${CPACK_RPM_PACKAGE_NAME}-${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}-${_CPACK_RPM_PACKAGE_ARCHITECTURE}.rpm")
  set(CPACK_RPM_FILE_NAME "${CPACK_OUTPUT_FILE_NAME}")
  # it seems rpmbuild can't handle spaces in the path
  # neither escaping (as below) nor putting quotes around the path seem to help
  #string(REGEX REPLACE " " "\\\\ " CPACK_RPM_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}")
  set(CPACK_RPM_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}")

  cpack_rpm_prepare_content_list()

  # In component case, put CPACK_ABSOLUTE_DESTINATION_FILES_<COMPONENT>
  #                   into CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL
  #         otherwise, put CPACK_ABSOLUTE_DESTINATION_FILES
  # This must be done BEFORE the CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL handling
  if(CPACK_RPM_PACKAGE_COMPONENT)
    if(CPACK_ABSOLUTE_DESTINATION_FILES)
     set(COMPONENT_FILES_TAG "CPACK_ABSOLUTE_DESTINATION_FILES_${CPACK_RPM_PACKAGE_COMPONENT}")
     set(CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL "${${COMPONENT_FILES_TAG}}")
     if(CPACK_RPM_PACKAGE_DEBUG)
       message("CPackRPM:Debug: Handling Absolute Destination Files: <${CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL}>")
       message("CPackRPM:Debug: in component = ${CPACK_RPM_PACKAGE_COMPONENT}")
     endif()
    endif()
  else()
    if(CPACK_ABSOLUTE_DESTINATION_FILES)
      set(CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL "${CPACK_ABSOLUTE_DESTINATION_FILES}")
    endif()
  endif()

  # In component case, set CPACK_RPM_USER_FILELIST_INTERNAL with CPACK_RPM_<COMPONENT>_USER_FILELIST.
  if(CPACK_RPM_PACKAGE_COMPONENT)
    if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_USER_FILELIST)
      set(CPACK_RPM_USER_FILELIST_INTERNAL ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_USER_FILELIST})
      if(CPACK_RPM_PACKAGE_DEBUG)
        message("CPackRPM:Debug: Handling User Filelist: <${CPACK_RPM_USER_FILELIST_INTERNAL}>")
        message("CPackRPM:Debug: in component = ${CPACK_RPM_PACKAGE_COMPONENT}")
      endif()
    else()
      set(CPACK_RPM_USER_FILELIST_INTERNAL "")
    endif()
  else()
    if(CPACK_RPM_USER_FILELIST)
      set(CPACK_RPM_USER_FILELIST_INTERNAL "${CPACK_RPM_USER_FILELIST}")
    else()
      set(CPACK_RPM_USER_FILELIST_INTERNAL "")
    endif()
  endif()

  # Handle user specified file line list in CPACK_RPM_USER_FILELIST_INTERNAL
  # Remove those files from CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL
  #                      or CPACK_RPM_INSTALL_FILES,
  # hence it must be done before these auto-generated lists are processed.
  if(CPACK_RPM_USER_FILELIST_INTERNAL)
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: Handling User Filelist: <${CPACK_RPM_USER_FILELIST_INTERNAL}>")
    endif()

    # Create CMake list from CPACK_RPM_INSTALL_FILES
    string(STRIP "${CPACK_RPM_INSTALL_FILES}" CPACK_RPM_INSTALL_FILES_LIST)
    string(REPLACE "\n" ";" CPACK_RPM_INSTALL_FILES_LIST
                            "${CPACK_RPM_INSTALL_FILES_LIST}")
    string(REPLACE "\"" "" CPACK_RPM_INSTALL_FILES_LIST
                            "${CPACK_RPM_INSTALL_FILES_LIST}")

    set(CPACK_RPM_USER_INSTALL_FILES "")
    foreach(F IN LISTS CPACK_RPM_USER_FILELIST_INTERNAL)
      string(REGEX REPLACE "%[A-Za-z0-9\(\),-]* " "" F_PATH ${F})
      string(REGEX MATCH "%[A-Za-z0-9\(\),-]*" F_PREFIX ${F})

      if(CPACK_RPM_PACKAGE_DEBUG)
        message("CPackRPM:Debug: F_PREFIX=<${F_PREFIX}>, F_PATH=<${F_PATH}>")
      endif()
      if(F_PREFIX)
        set(F_PREFIX "${F_PREFIX} ")
      endif()
      # Rebuild the user list file
      set(CPACK_RPM_USER_INSTALL_FILES "${CPACK_RPM_USER_INSTALL_FILES}${F_PREFIX}\"${F_PATH}\"\n")

      # Remove from CPACK_RPM_INSTALL_FILES and CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL
      list(REMOVE_ITEM CPACK_RPM_INSTALL_FILES_LIST ${F_PATH})
      # ABSOLUTE destination files list may not exists at all
      if (CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL)
        list(REMOVE_ITEM CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL ${F_PATH})
      endif()

    endforeach()

    # Rebuild CPACK_RPM_INSTALL_FILES
    set(CPACK_RPM_INSTALL_FILES "")
    foreach(F IN LISTS CPACK_RPM_INSTALL_FILES_LIST)
      set(CPACK_RPM_INSTALL_FILES "${CPACK_RPM_INSTALL_FILES}\"${F}\"\n")
    endforeach()
  else()
    set(CPACK_RPM_USER_INSTALL_FILES "")
  endif()

  if (CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL)
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: Handling Absolute Destination Files: ${CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL}")
    endif()
    # Remove trailing space
    string(STRIP "${CPACK_RPM_INSTALL_FILES}" CPACK_RPM_INSTALL_FILES_LIST)
    # Transform endline separated - string into CMake List
    string(REPLACE "\n" ";" CPACK_RPM_INSTALL_FILES_LIST "${CPACK_RPM_INSTALL_FILES_LIST}")
    # Remove unecessary quotes
    string(REPLACE "\"" "" CPACK_RPM_INSTALL_FILES_LIST "${CPACK_RPM_INSTALL_FILES_LIST}")
    # Remove ABSOLUTE install file from INSTALL FILE LIST
    list(REMOVE_ITEM CPACK_RPM_INSTALL_FILES_LIST ${CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL})
    # Rebuild INSTALL_FILES
    set(CPACK_RPM_INSTALL_FILES "")
    foreach(F IN LISTS CPACK_RPM_INSTALL_FILES_LIST)
      set(CPACK_RPM_INSTALL_FILES "${CPACK_RPM_INSTALL_FILES}\"${F}\"\n")
    endforeach()
    # Build ABSOLUTE_INSTALL_FILES
    set(CPACK_RPM_ABSOLUTE_INSTALL_FILES "")
    foreach(F IN LISTS CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL)
      set(CPACK_RPM_ABSOLUTE_INSTALL_FILES "${CPACK_RPM_ABSOLUTE_INSTALL_FILES}%config \"${F}\"\n")
    endforeach()
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: CPACK_RPM_ABSOLUTE_INSTALL_FILES=${CPACK_RPM_ABSOLUTE_INSTALL_FILES}")
      message("CPackRPM:Debug: CPACK_RPM_INSTALL_FILES=${CPACK_RPM_INSTALL_FILES}")
    endif()
  else()
    # reset vars in order to avoid leakage of value(s) from one component to another
    set(CPACK_RPM_ABSOLUTE_INSTALL_FILES "")
  endif()

  # Prepare install files
  cpack_rpm_prepare_install_files(
      "${CPACK_RPM_INSTALL_FILES}"
      "${WDIR}"
      "${RPM_USED_PACKAGE_PREFIXES}"
      "${CPACK_RPM_PACKAGE_RELOCATABLE}"
    )

  # The name of the final spec file to be used by rpmbuild
  set(CPACK_RPM_BINARY_SPECFILE "${CPACK_RPM_ROOTDIR}/SPECS/${CPACK_RPM_PACKAGE_NAME}${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.spec")

  # Print out some debug information if we were asked for that
  if(CPACK_RPM_PACKAGE_DEBUG)
     message("CPackRPM:Debug: CPACK_TOPLEVEL_DIRECTORY          = ${CPACK_TOPLEVEL_DIRECTORY}")
     message("CPackRPM:Debug: CPACK_TOPLEVEL_TAG                = ${CPACK_TOPLEVEL_TAG}")
     message("CPackRPM:Debug: CPACK_TEMPORARY_DIRECTORY         = ${CPACK_TEMPORARY_DIRECTORY}")
     message("CPackRPM:Debug: CPACK_OUTPUT_FILE_NAME            = ${CPACK_OUTPUT_FILE_NAME}")
     message("CPackRPM:Debug: CPACK_OUTPUT_FILE_PATH            = ${CPACK_OUTPUT_FILE_PATH}")
     message("CPackRPM:Debug: CPACK_PACKAGE_FILE_NAME           = ${CPACK_PACKAGE_FILE_NAME}")
     message("CPackRPM:Debug: CPACK_RPM_BINARY_SPECFILE         = ${CPACK_RPM_BINARY_SPECFILE}")
     message("CPackRPM:Debug: CPACK_PACKAGE_INSTALL_DIRECTORY   = ${CPACK_PACKAGE_INSTALL_DIRECTORY}")
     message("CPackRPM:Debug: CPACK_TEMPORARY_PACKAGE_FILE_NAME = ${CPACK_TEMPORARY_PACKAGE_FILE_NAME}")
  endif()

  #
  # USER generated/provided spec file handling.
  #

  # We can have a component specific spec file.
  if(CPACK_RPM_PACKAGE_COMPONENT AND CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_USER_BINARY_SPECFILE)
    set(CPACK_RPM_USER_BINARY_SPECFILE ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_USER_BINARY_SPECFILE})
  endif()

  # We should generate a USER spec file template:
  #  - either because the user asked for it : CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE
  #  - or the user did not provide one : NOT CPACK_RPM_USER_BINARY_SPECFILE
  if(CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE OR NOT CPACK_RPM_USER_BINARY_SPECFILE)
     file(WRITE ${CPACK_RPM_BINARY_SPECFILE}.in
      "# -*- rpm-spec -*-
BuildRoot:      \@CPACK_RPM_DIRECTORY\@/\@CPACK_PACKAGE_FILE_NAME\@\@CPACK_RPM_PACKAGE_COMPONENT_PART_PATH\@
Summary:        \@CPACK_RPM_PACKAGE_SUMMARY\@
Name:           \@CPACK_RPM_PACKAGE_NAME\@\@CPACK_RPM_PACKAGE_COMPONENT_PART_NAME\@
Version:        \@CPACK_RPM_PACKAGE_VERSION\@
Release:        \@CPACK_RPM_PACKAGE_RELEASE\@
License:        \@CPACK_RPM_PACKAGE_LICENSE\@
Group:          \@CPACK_RPM_PACKAGE_GROUP\@
Vendor:         \@CPACK_RPM_PACKAGE_VENDOR\@
\@TMP_RPM_URL\@
\@TMP_RPM_REQUIRES\@
\@TMP_RPM_REQUIRES_PRE\@
\@TMP_RPM_REQUIRES_POST\@
\@TMP_RPM_REQUIRES_PREUN\@
\@TMP_RPM_REQUIRES_POSTUN\@
\@TMP_RPM_PROVIDES\@
\@TMP_RPM_OBSOLETES\@
\@TMP_RPM_CONFLICTS\@
\@TMP_RPM_AUTOPROV\@
\@TMP_RPM_AUTOREQ\@
\@TMP_RPM_AUTOREQPROV\@
\@TMP_RPM_BUILDARCH\@
\@TMP_RPM_PREFIXES\@

%define _rpmdir \@CPACK_RPM_DIRECTORY\@
%define _rpmfilename \@CPACK_RPM_FILE_NAME\@
%define _unpackaged_files_terminate_build 0
%define _topdir \@CPACK_RPM_DIRECTORY\@
\@TMP_RPM_SPEC_INSTALL_POST\@
\@CPACK_RPM_SPEC_MORE_DEFINE\@
\@CPACK_RPM_COMPRESSION_TYPE_TMP\@

%description
\@CPACK_RPM_PACKAGE_DESCRIPTION\@

# This is a shortcutted spec file generated by CMake RPM generator
# we skip _install step because CPack does that for us.
# We do only save CPack installed tree in _prepr
# and then restore it in build.
%prep
mv $RPM_BUILD_ROOT \"\@CPACK_TOPLEVEL_DIRECTORY\@/tmpBBroot\"

#p build

%install
if [ -e $RPM_BUILD_ROOT ];
then
  rm -rf $RPM_BUILD_ROOT
fi
mv \"\@CPACK_TOPLEVEL_DIRECTORY\@/tmpBBroot\" $RPM_BUILD_ROOT

%clean

%post
\@RPM_SYMLINK_POSTINSTALL\@
\@CPACK_RPM_SPEC_POSTINSTALL\@

%postun
\@CPACK_RPM_SPEC_POSTUNINSTALL\@

%pre
\@CPACK_RPM_SPEC_PREINSTALL\@

%preun
\@CPACK_RPM_SPEC_PREUNINSTALL\@

%files
%defattr(-,root,root,-)
\@CPACK_RPM_INSTALL_FILES\@
\@CPACK_RPM_ABSOLUTE_INSTALL_FILES\@
\@CPACK_RPM_USER_INSTALL_FILES\@

%changelog
\@CPACK_RPM_SPEC_CHANGELOG\@
")
    # Stop here if we were asked to only generate a template USER spec file
    # The generated file may then be used as a template by user who wants
    # to customize their own spec file.
    if(CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE)
       message(FATAL_ERROR "CPackRPM: STOP here Generated USER binary spec file templare is: ${CPACK_RPM_BINARY_SPECFILE}.in")
    endif()
  endif()

  # After that we may either use a user provided spec file
  # or generate one using appropriate variables value.
  if(CPACK_RPM_USER_BINARY_SPECFILE)
    # User may have specified SPECFILE just use it
    message("CPackRPM: Will use USER specified spec file: ${CPACK_RPM_USER_BINARY_SPECFILE}")
    # The user provided file is processed for @var replacement
    configure_file(${CPACK_RPM_USER_BINARY_SPECFILE} ${CPACK_RPM_BINARY_SPECFILE} @ONLY)
  else()
    # No User specified spec file, will use the generated spec file
    message("CPackRPM: Will use GENERATED spec file: ${CPACK_RPM_BINARY_SPECFILE}")
    # Note the just created file is processed for @var replacement
    configure_file(${CPACK_RPM_BINARY_SPECFILE}.in ${CPACK_RPM_BINARY_SPECFILE} @ONLY)
  endif()

  if(RPMBUILD_EXECUTABLE)
    # Now call rpmbuild using the SPECFILE
    execute_process(
      COMMAND "${RPMBUILD_EXECUTABLE}" -bb
              --define "_topdir ${CPACK_RPM_DIRECTORY}"
              --buildroot "${CPACK_RPM_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}${CPACK_RPM_PACKAGE_COMPONENT_PART_PATH}"
              --target "${_CPACK_RPM_PACKAGE_ARCHITECTURE}"
              "${CPACK_RPM_BINARY_SPECFILE}"
      WORKING_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}${CPACK_RPM_PACKAGE_COMPONENT_PART_PATH}"
      RESULT_VARIABLE CPACK_RPMBUILD_EXEC_RESULT
      ERROR_FILE "${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.err"
      OUTPUT_FILE "${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.out")
    if(CPACK_RPM_PACKAGE_DEBUG OR CPACK_RPMBUILD_EXEC_RESULT)
      file(READ ${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.err RPMBUILDERR)
      file(READ ${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.out RPMBUILDOUT)
      message("CPackRPM:Debug: You may consult rpmbuild logs in: ")
      message("CPackRPM:Debug:    - ${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.err")
      message("CPackRPM:Debug: *** ${RPMBUILDERR} ***")
      message("CPackRPM:Debug:    - ${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.out")
      message("CPackRPM:Debug: *** ${RPMBUILDOUT} ***")
    endif()
  else()
    if(ALIEN_EXECUTABLE)
      message(FATAL_ERROR "RPM packaging through alien not done (yet)")
    endif()
  endif()
endfunction()

cpack_rpm_generate_package()
