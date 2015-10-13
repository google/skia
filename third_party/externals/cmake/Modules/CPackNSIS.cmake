#.rst:
# CPackNSIS
# ---------
#
# CPack NSIS generator specific options
#
# Variables specific to CPack NSIS generator
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# The following variables are specific to the graphical installers built
# on Windows using the Nullsoft Installation System.
#
# .. variable:: CPACK_NSIS_INSTALL_ROOT
#
#  The default installation directory presented to the end user by the NSIS
#  installer is under this root dir. The full directory presented to the end
#  user is: ${CPACK_NSIS_INSTALL_ROOT}/${CPACK_PACKAGE_INSTALL_DIRECTORY}
#
# .. variable:: CPACK_NSIS_MUI_ICON
#
#  An icon filename.  The name of a ``*.ico`` file used as the main icon for the
#  generated install program.
#
# .. variable:: CPACK_NSIS_MUI_UNIICON
#
#  An icon filename.  The name of a ``*.ico`` file used as the main icon for the
#  generated uninstall program.
#
# .. variable:: CPACK_NSIS_INSTALLER_MUI_ICON_CODE
#
#  undocumented.
#
# .. variable:: CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS
#
#  Extra NSIS commands that will be added to the beginning of the install
#  Section, before your install tree is available on the target system.
#
# .. variable:: CPACK_NSIS_EXTRA_INSTALL_COMMANDS
#
#  Extra NSIS commands that will be added to the end of the install Section,
#  after your install tree is available on the target system.
#
# .. variable:: CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
#
#  Extra NSIS commands that will be added to the uninstall Section, before
#  your install tree is removed from the target system.
#
# .. variable:: CPACK_NSIS_COMPRESSOR
#
#  The arguments that will be passed to the NSIS SetCompressor command.
#
# .. variable:: CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL
#
#  Ask about uninstalling previous versions first.  If this is set to "ON",
#  then an installer will look for previous installed versions and if one is
#  found, ask the user whether to uninstall it before proceeding with the
#  install.
#
# .. variable:: CPACK_NSIS_MODIFY_PATH
#
#  Modify PATH toggle.  If this is set to "ON", then an extra page will appear
#  in the installer that will allow the user to choose whether the program
#  directory should be added to the system PATH variable.
#
# .. variable:: CPACK_NSIS_DISPLAY_NAME
#
#  The display name string that appears in the Windows Add/Remove Program
#  control panel
#
# .. variable:: CPACK_NSIS_PACKAGE_NAME
#
#  The title displayed at the top of the installer.
#
# .. variable:: CPACK_NSIS_INSTALLED_ICON_NAME
#
#  A path to the executable that contains the installer icon.
#
# .. variable:: CPACK_NSIS_HELP_LINK
#
#  URL to a web site providing assistance in installing your application.
#
# .. variable:: CPACK_NSIS_URL_INFO_ABOUT
#
#  URL to a web site providing more information about your application.
#
# .. variable:: CPACK_NSIS_CONTACT
#
#  Contact information for questions and comments about the installation
#  process.
#
# .. variable:: CPACK_NSIS_CREATE_ICONS_EXTRA
#
#  Additional NSIS commands for creating start menu shortcuts.
#
# .. variable:: CPACK_NSIS_DELETE_ICONS_EXTRA
#
#  Additional NSIS commands to uninstall start menu shortcuts.
#
# .. variable:: CPACK_NSIS_EXECUTABLES_DIRECTORY
#
#  Creating NSIS start menu links assumes that they are in 'bin' unless this
#  variable is set.  For example, you would set this to 'exec' if your
#  executables are in an exec directory.
#
# .. variable:: CPACK_NSIS_MUI_FINISHPAGE_RUN
#
#  Specify an executable to add an option to run on the finish page of the
#  NSIS installer.
#
# .. variable:: CPACK_NSIS_MENU_LINKS
#
#  Specify links in [application] menu.  This should contain a list of pair
#  "link" "link name". The link may be an URL or a path relative to
#  installation prefix.  Like::
#
#   set(CPACK_NSIS_MENU_LINKS
#       "doc/cmake-@CMake_VERSION_MAJOR@.@CMake_VERSION_MINOR@/cmake.html"
#       "CMake Help" "http://www.cmake.org" "CMake Web Site")
#

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

#FIXME we should put NSIS specific code here
#FIXME but I'm not doing it because I'm not able to test it...
