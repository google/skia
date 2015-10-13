/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmInstallDirectoryGenerator.h"

#include "cmTarget.h"

//----------------------------------------------------------------------------
cmInstallDirectoryGenerator
::cmInstallDirectoryGenerator(std::vector<std::string> const& dirs,
                              const char* dest,
                              const char* file_permissions,
                              const char* dir_permissions,
                              std::vector<std::string> const& configurations,
                              const char* component,
                              MessageLevel message,
                              const char* literal_args,
                              bool optional):
  cmInstallGenerator(dest, configurations, component, message),
  Directories(dirs),
  FilePermissions(file_permissions), DirPermissions(dir_permissions),
  LiteralArguments(literal_args), Optional(optional)
{
}

//----------------------------------------------------------------------------
cmInstallDirectoryGenerator
::~cmInstallDirectoryGenerator()
{
}

//----------------------------------------------------------------------------
void
cmInstallDirectoryGenerator::GenerateScriptActions(std::ostream& os,
                                                   Indent const& indent)
{
  // Write code to install the directories.
  const char* no_rename = 0;
  this->AddInstallRule(os,
                       this->Destination,
                       cmInstallType_DIRECTORY,
                       this->Directories,
                       this->Optional,
                       this->FilePermissions.c_str(),
                       this->DirPermissions.c_str(),
                       no_rename, this->LiteralArguments.c_str(),
                       indent);
}
