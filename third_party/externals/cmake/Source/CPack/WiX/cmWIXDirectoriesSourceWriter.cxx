/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmWIXDirectoriesSourceWriter.h"

cmWIXDirectoriesSourceWriter::cmWIXDirectoriesSourceWriter(cmCPackLog* logger,
  std::string const& filename):
    cmWIXSourceWriter(logger, filename)
{

}

void cmWIXDirectoriesSourceWriter::EmitStartMenuFolder(
  std::string const& startMenuFolder)
{
  BeginElement("Directory");
  AddAttribute("Id", "ProgramMenuFolder");

  BeginElement("Directory");
  AddAttribute("Id", "PROGRAM_MENU_FOLDER");
  AddAttribute("Name", startMenuFolder);
  EndElement("Directory");

  EndElement("Directory");
}

void cmWIXDirectoriesSourceWriter::EmitDesktopFolder()
{
  BeginElement("Directory");
  AddAttribute("Id", "DesktopFolder");
  AddAttribute("Name", "Desktop");
  EndElement("Directory");
}

void cmWIXDirectoriesSourceWriter::EmitStartupFolder()
{
  BeginElement("Directory");
  AddAttribute("Id", "StartupFolder");
  AddAttribute("Name", "Startup");
  EndElement("Directory");
}

size_t cmWIXDirectoriesSourceWriter::BeginInstallationPrefixDirectory(
  std::string const& programFilesFolderId,
  std::string const& installRootString)
{
  BeginElement("Directory");
  AddAttribute("Id", programFilesFolderId);

  std::vector<std::string> installRoot;

  cmSystemTools::SplitPath(installRootString.c_str(), installRoot);

  if(!installRoot.empty() && installRoot.back().empty())
    {
    installRoot.pop_back();
    }

  for(size_t i = 1; i < installRoot.size(); ++i)
    {
    BeginElement("Directory");

    if(i == installRoot.size() - 1)
      {
      AddAttribute("Id", "INSTALL_ROOT");
      }
    else
      {
      std::stringstream tmp;
      tmp << "INSTALL_PREFIX_" << i;
      AddAttribute("Id", tmp.str());
      }

    AddAttribute("Name", installRoot[i]);
  }

  return installRoot.size();
}

void cmWIXDirectoriesSourceWriter::EndInstallationPrefixDirectory(size_t size)
{
  for(size_t i = 0; i < size; ++i)
    {
    EndElement("Directory");
    }
}
