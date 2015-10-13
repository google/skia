
#=============================================================================
# Copyright 2015 Geoffrey Viola <geoffrey.viola@asirobots.com>
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

#Setup Greenhills MULTI specific compilation information
find_path(GHS_INT_DIRECTORY INTEGRITY.ld PATHS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\GreenHillsSoftware6433c345;InstallLocation]" #int1122
  "C:/ghs/int1122"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\GreenHillsSoftware289b6625;InstallLocation]" #int1104
  "C:/ghs/int1104"
  DOC "Path to integrity directory"
  )
set(GHS_OS_DIR ${GHS_INT_DIRECTORY} CACHE PATH "OS directory")
set(GHS_PRIMARY_TARGET "arm_integrity.tgt" CACHE STRING "target for compilation")
set(GHS_BSP_NAME "simarm" CACHE STRING "BSP name")
set(GHS_CUSTOMIZATION "" CACHE FILEPATH "optional GHS customization")
mark_as_advanced(GHS_CUSTOMIZATION)
set(GHS_GPJ_MACROS "" CACHE STRING "optional GHS macros generated in the .gpjs for legacy reasons")
mark_as_advanced(GHS_GPJ_MACROS)
