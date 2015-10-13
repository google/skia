/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmWIXDirectoriesSourceWriter_h
#define cmWIXDirectoriesSourceWriter_h

#include "cmWIXSourceWriter.h"

#include <CPack/cmCPackGenerator.h>

#include <string>

/** \class cmWIXDirectoriesSourceWriter
 * \brief Helper class to generate directories.wxs
 */
class cmWIXDirectoriesSourceWriter : public cmWIXSourceWriter
{
public:
  cmWIXDirectoriesSourceWriter(cmCPackLog* logger,
    std::string const& filename);

  void EmitStartMenuFolder(std::string const& startMenuFolder);

  void EmitDesktopFolder();

  void EmitStartupFolder();

  size_t BeginInstallationPrefixDirectory(
      std::string const& programFilesFolderId,
      std::string const& installRootString);

  void EndInstallationPrefixDirectory(size_t size);
};

#endif
