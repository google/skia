/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmVersion_h
#define cmVersion_h

#include "cmStandardIncludes.h"

/** \class cmVersion
 * \brief Helper class for providing CMake and CTest version information.
 *
 * Finds all version related information.
 */
class cmVersion
{
public:
  /**
   * Return major and minor version numbers for cmake.
   */
  static unsigned int GetMajorVersion();
  static unsigned int GetMinorVersion();
  static unsigned int GetPatchVersion();
  static unsigned int GetTweakVersion();
  static const char* GetCMakeVersion();
};

/* Encode with room for up to 1000 minor releases between major releases
   and to encode dates until the year 10000 in the patch level.  */
#define CMake_VERSION_ENCODE__BASE cmIML_INT_UINT64_C(100000000)
#define CMake_VERSION_ENCODE(major, minor, patch) \
  ((((major) * 1000u) * CMake_VERSION_ENCODE__BASE) + \
   (((minor) % 1000u) * CMake_VERSION_ENCODE__BASE) + \
   (((patch)          % CMake_VERSION_ENCODE__BASE)))

#endif

