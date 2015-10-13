/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallType_h
#define cmInstallType_h

/**
 * Enumerate types known to file(INSTALL).
 */
enum cmInstallType
{
  cmInstallType_EXECUTABLE,
  cmInstallType_STATIC_LIBRARY,
  cmInstallType_SHARED_LIBRARY,
  cmInstallType_MODULE_LIBRARY,
  cmInstallType_FILES,
  cmInstallType_PROGRAMS,
  cmInstallType_DIRECTORY
};

#endif
