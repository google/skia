/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmWIXAccessControlList.h"

#include <CPack/cmCPackGenerator.h>

#include <cmSystemTools.h>

cmWIXAccessControlList::cmWIXAccessControlList(
      cmCPackLog *logger,
      cmInstalledFile const& installedFile,
      cmWIXSourceWriter &sourceWriter):
        Logger(logger),
        InstalledFile(installedFile),
        SourceWriter(sourceWriter)
{

}

bool cmWIXAccessControlList::Apply()
{
  std::vector<std::string> entries;
  this->InstalledFile.GetPropertyAsList("CPACK_WIX_ACL", entries);

  for(size_t i = 0; i < entries.size(); ++i)
    {
    this->CreatePermissionElement(entries[i]);
    }

  return true;
}

void cmWIXAccessControlList::CreatePermissionElement(
  std::string const& entry)
{
  std::string::size_type pos = entry.find('=');
  if(pos == std::string::npos)
    {
    this->ReportError(entry, "Did not find mandatory '='");
    return;
    }

  std::string user_and_domain = entry.substr(0, pos);
  std::string permission_string = entry.substr(pos + 1);

  pos = user_and_domain.find('@');
  std::string user;
  std::string domain;
  if(pos != std::string::npos)
    {
    user = user_and_domain.substr(0, pos);
    domain = user_and_domain.substr(pos + 1);
    }
  else
    {
    user = user_and_domain;
    }

  std::vector<std::string> permissions =
    cmSystemTools::tokenize(permission_string, ",");

  this->SourceWriter.BeginElement("Permission");
  this->SourceWriter.AddAttribute("User", user);
  if(domain.size())
    {
    this->SourceWriter.AddAttribute("Domain", domain);
    }
  for(size_t i = 0; i < permissions.size(); ++i)
    {
    this->EmitBooleanAttribute(entry,
      cmSystemTools::TrimWhitespace(permissions[i]));
    }
  this->SourceWriter.EndElement("Permission");
}

void cmWIXAccessControlList::ReportError(
  std::string const& entry,
  std::string const& message)
{
  cmCPackLogger(cmCPackLog::LOG_ERROR,
    "Failed processing ACL entry '" << entry <<
    "': " << message << std::endl);
}

bool cmWIXAccessControlList::IsBooleanAttribute(std::string const& name)
{
  static const char* validAttributes[] =
  {
    "Append",
    "ChangePermission",
    "CreateChild",
    "CreateFile",
    "CreateLink",
    "CreateSubkeys",
    "Delete",
    "DeleteChild",
    "EnumerateSubkeys",
    "Execute",
    "FileAllRights",
    "GenericAll",
    "GenericExecute",
    "GenericRead",
    "GenericWrite",
    "Notify",
    "Read",
    "ReadAttributes",
    "ReadExtendedAttributes",
    "ReadPermission",
    "SpecificRightsAll",
    "Synchronize",
    "TakeOwnership",
    "Traverse",
    "Write",
    "WriteAttributes",
    "WriteExtendedAttributes",
    0
  };

  size_t i = 0;
  while(validAttributes[i])
    {
    if(name == validAttributes[i++]) return true;
    }

  return false;
}

void cmWIXAccessControlList::EmitBooleanAttribute(
  std::string const& entry, std::string const& name)
{
  if(!this->IsBooleanAttribute(name))
    {
    std::stringstream message;
    message << "Unknown boolean attribute '" << name << "'";
    this->ReportError(entry, message.str());
    }

  this->SourceWriter.AddAttribute(name, "yes");
}
