/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmVersion.h"

#include "cmVersionMacros.h"

unsigned int cmVersion::GetMajorVersion() { return CMake_VERSION_MAJOR; }
unsigned int cmVersion::GetMinorVersion() { return CMake_VERSION_MINOR; }
unsigned int cmVersion::GetPatchVersion() { return CMake_VERSION_PATCH; }
unsigned int cmVersion::GetTweakVersion() { return 0; }

const char* cmVersion::GetCMakeVersion()
{
  return CMake_VERSION;
}
