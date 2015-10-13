/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmWIXAccessControlList_h
#define cmWIXAccessControlList_h

#include <cmInstalledFile.h>
#include <CPack/cmCPackLog.h>

#include "cmWIXSourceWriter.h"

class cmWIXAccessControlList
{
public:
  cmWIXAccessControlList(
        cmCPackLog *logger,
        cmInstalledFile const& installedFile,
        cmWIXSourceWriter &sourceWriter);

  bool Apply();

private:
  void CreatePermissionElement(std::string const& entry);

  void ReportError(std::string const& entry, std::string const& message);

  bool IsBooleanAttribute(std::string const& name);

  void EmitBooleanAttribute(
    std::string const& entry, std::string const& name);

  cmCPackLog* Logger;
  cmInstalledFile const& InstalledFile;
  cmWIXSourceWriter &SourceWriter;
};

#endif
