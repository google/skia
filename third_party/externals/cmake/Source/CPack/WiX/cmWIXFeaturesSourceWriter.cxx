/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmWIXFeaturesSourceWriter.h"

cmWIXFeaturesSourceWriter::cmWIXFeaturesSourceWriter(cmCPackLog* logger,
  std::string const& filename):
    cmWIXSourceWriter(logger, filename)
{

}

void cmWIXFeaturesSourceWriter::CreateCMakePackageRegistryEntry(
    std::string const& package,
    std::string const& upgradeGuid)
{
  BeginElement("Component");
  AddAttribute("Id", "CM_PACKAGE_REGISTRY");
  AddAttribute("Directory", "TARGETDIR");
  AddAttribute("Guid", "*");

  std::string registryKey =
      std::string("Software\\Kitware\\CMake\\Packages\\") + package;

  BeginElement("RegistryValue");
  AddAttribute("Root", "HKLM");
  AddAttribute("Key", registryKey);
  AddAttribute("Name", upgradeGuid);
  AddAttribute("Type", "string");
  AddAttribute("Value", "[INSTALL_ROOT]");
  AddAttribute("KeyPath", "yes");
  EndElement("RegistryValue");

  EndElement("Component");
}

void cmWIXFeaturesSourceWriter::EmitFeatureForComponentGroup(
  cmCPackComponentGroup const& group)
{
  BeginElement("Feature");
  AddAttribute("Id", "CM_G_" + group.Name);

  if(group.IsExpandedByDefault)
    {
    AddAttribute("Display", "expand");
    }

  AddAttributeUnlessEmpty("Title", group.DisplayName);
  AddAttributeUnlessEmpty("Description", group.Description);

  for(std::vector<cmCPackComponentGroup*>::const_iterator
    i = group.Subgroups.begin(); i != group.Subgroups.end(); ++i)
    {
    EmitFeatureForComponentGroup(**i);
    }

  for(std::vector<cmCPackComponent*>::const_iterator
    i = group.Components.begin(); i != group.Components.end(); ++i)
    {
    EmitFeatureForComponent(**i);
    }

  EndElement("Feature");
}

void cmWIXFeaturesSourceWriter::EmitFeatureForComponent(
  cmCPackComponent const& component)
{
  BeginElement("Feature");
  AddAttribute("Id", "CM_C_" + component.Name);

  AddAttributeUnlessEmpty("Title", component.DisplayName);
  AddAttributeUnlessEmpty("Description", component.Description);

  if(component.IsRequired)
    {
    AddAttribute("Absent", "disallow");
    }

  if(component.IsHidden)
    {
    AddAttribute("Display", "hidden");
    }

  EndElement("Feature");
}

void cmWIXFeaturesSourceWriter::EmitComponentRef(std::string const& id)
{
  BeginElement("ComponentRef");
  AddAttribute("Id", id);
  EndElement("ComponentRef");
}
