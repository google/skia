
#=============================================================================
# Copyright 2002-2014 Kitware, Inc.
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
include(Compiler/GNU)

macro(__compiler_qcc lang)
  __compiler_gnu(${lang})

  # http://www.qnx.com/developers/docs/6.4.0/neutrino/utilities/q/qcc.html#examples
  set(CMAKE_${lang}_COMPILE_OPTIONS_TARGET "-V")

  set(CMAKE_INCLUDE_SYSTEM_FLAG_${lang} "-Wp,-isystem,")
  set(CMAKE_DEPFILE_FLAGS_${lang} "-Wc,-MMD,<DEPFILE>,-MT,<OBJECT>,-MF,<DEPFILE>")
endmacro()
