/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCLocaleEnvironmentScope.h"

#include "cmSystemTools.h"

#include <sstream>

cmCLocaleEnvironmentScope::cmCLocaleEnvironmentScope()
{
  this->SetEnv("LANGUAGE", "");
  this->SetEnv("LC_MESSAGES", "C");

  std::string lcAll = this->GetEnv("LC_ALL");

  if(!lcAll.empty())
    {
    this->SetEnv("LC_ALL", "");
    this->SetEnv("LC_CTYPE", lcAll);
    }
}

std::string cmCLocaleEnvironmentScope::GetEnv(std::string const& key)
{
  const char* value = cmSystemTools::GetEnv(key);
  return value ? value : std::string();
}

void cmCLocaleEnvironmentScope::SetEnv(
  std::string const& key, std::string const& value)
{
  std::string oldValue = this->GetEnv(key);

  this->EnvironmentBackup.insert(std::make_pair(key, oldValue));

  if(value.empty())
    {
    cmSystemTools::UnsetEnv(key.c_str());
    }
  else
    {
    std::stringstream tmp;
    tmp << key << "=" << value;
    cmSystemTools::PutEnv(tmp.str());
    }
}

cmCLocaleEnvironmentScope::~cmCLocaleEnvironmentScope()
{
  for(backup_map_t::const_iterator i = this->EnvironmentBackup.begin();
    i != this->EnvironmentBackup.end(); ++i)
    {
    std::stringstream tmp;
    tmp << i->first << "=" << i->second;
    cmSystemTools::PutEnv(tmp.str());
    }
}
