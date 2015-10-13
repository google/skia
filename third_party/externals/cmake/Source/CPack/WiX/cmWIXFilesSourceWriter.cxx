/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014-2015 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmWIXFilesSourceWriter.h"
#include "cmWIXAccessControlList.h"

#include <cmInstalledFile.h>

#include <sys/types.h>
#include <sys/stat.h>

cmWIXFilesSourceWriter::cmWIXFilesSourceWriter(cmCPackLog* logger,
  std::string const& filename):
    cmWIXSourceWriter(logger, filename)
{

}

void cmWIXFilesSourceWriter::EmitShortcut(
  std::string const& id,
  cmWIXShortcut const& shortcut,
  std::string const& shortcutPrefix,
  size_t shortcutIndex)
{
  std::stringstream shortcutId;
  shortcutId << shortcutPrefix << id;

  if(shortcutIndex > 0)
    {
    shortcutId << "_"  << shortcutIndex;
    }

  std::string fileId = std::string("CM_F") + id;

  BeginElement("Shortcut");
  AddAttribute("Id", shortcutId.str());
  AddAttribute("Name", shortcut.label);
  std::string target = "[#" + fileId + "]";
  AddAttribute("Target", target);
  AddAttribute("WorkingDirectory", shortcut.workingDirectoryId);
  EndElement("Shortcut");
}

void cmWIXFilesSourceWriter::EmitRemoveFolder(std::string const& id)
{
  BeginElement("RemoveFolder");
  AddAttribute("Id", id);
  AddAttribute("On", "uninstall");
  EndElement("RemoveFolder");
}

void cmWIXFilesSourceWriter::EmitInstallRegistryValue(
  std::string const& registryKey,
  std::string const& cpackComponentName,
  std::string const& suffix)
{
  std::string valueName;
  if(!cpackComponentName.empty())
    {
      valueName = cpackComponentName + "_";
    }

  valueName += "installed";
  valueName += suffix;

  BeginElement("RegistryValue");
  AddAttribute("Root", "HKCU");
  AddAttribute("Key", registryKey);
  AddAttribute("Name", valueName);
  AddAttribute("Type", "integer");
  AddAttribute("Value", "1");
  AddAttribute("KeyPath", "yes");
  EndElement("RegistryValue");
}

void cmWIXFilesSourceWriter::EmitUninstallShortcut(
  std::string const& packageName)
{
  BeginElement("Shortcut");
  AddAttribute("Id", "UNINSTALL");
  AddAttribute("Name", "Uninstall " + packageName);
  AddAttribute("Description", "Uninstalls " + packageName);
  AddAttribute("Target", "[SystemFolder]msiexec.exe");
  AddAttribute("Arguments", "/x [ProductCode]");
  EndElement("Shortcut");
}

std::string cmWIXFilesSourceWriter::EmitComponentCreateFolder(
  std::string const& directoryId,
  std::string const& guid,
  cmInstalledFile const* installedFile)
{
  std::string componentId =
    std::string("CM_C_EMPTY_") + directoryId;

  BeginElement("DirectoryRef");
  AddAttribute("Id", directoryId);

  BeginElement("Component");
  AddAttribute("Id", componentId);
  AddAttribute("Guid", guid);

  BeginElement("CreateFolder");

  if(installedFile)
    {
    cmWIXAccessControlList acl(Logger, *installedFile, *this);
    acl.Apply();
    }

  EndElement("CreateFolder");
  EndElement("Component");
  EndElement("DirectoryRef");

  return componentId;
}

std::string cmWIXFilesSourceWriter::EmitComponentFile(
  std::string const& directoryId,
  std::string const& id,
  std::string const& filePath,
  cmWIXPatch &patch,
  cmInstalledFile const* installedFile)
{
  std::string componentId = std::string("CM_C") + id;
  std::string fileId = std::string("CM_F") + id;

  BeginElement("DirectoryRef");
  AddAttribute("Id", directoryId);

  BeginElement("Component");
  AddAttribute("Id", componentId);
  AddAttribute("Guid", "*");

  if(installedFile)
    {
    if(installedFile->GetPropertyAsBool("CPACK_NEVER_OVERWRITE"))
      {
      AddAttribute("NeverOverwrite", "yes");
      }
    if(installedFile->GetPropertyAsBool("CPACK_PERMANENT"))
      {
      AddAttribute("Permanent", "yes");
      }
    }

  BeginElement("File");
  AddAttribute("Id", fileId);
  AddAttribute("Source", filePath);
  AddAttribute("KeyPath", "yes");

  mode_t fileMode = 0;
  cmSystemTools::GetPermissions(filePath.c_str(), fileMode);

  if(!(fileMode & S_IWRITE))
    {
    AddAttribute("ReadOnly", "yes");
    }

  if(installedFile)
    {
    cmWIXAccessControlList acl(Logger, *installedFile, *this);
    acl.Apply();
    }

  patch.ApplyFragment(fileId, *this);
  EndElement("File");

  patch.ApplyFragment(componentId, *this);
  EndElement("Component");
  EndElement("DirectoryRef");

  return componentId;
}
