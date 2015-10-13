/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmPropertyDefinitionMap.h"
#include "cmSystemTools.h"
#include "cmDocumentationSection.h"

void cmPropertyDefinitionMap
::DefineProperty(const std::string& name, cmProperty::ScopeType scope,
                 const char *ShortDescription,
                 const char *FullDescription,
                 bool chain)
{
  cmPropertyDefinitionMap::iterator it = this->find(name);
  cmPropertyDefinition *prop;
  if (it == this->end())
    {
    prop = &(*this)[name];
    prop->DefineProperty(name,scope,ShortDescription, FullDescription,
                         chain);
    }
}

bool cmPropertyDefinitionMap::IsPropertyDefined(const std::string& name)
{
  cmPropertyDefinitionMap::iterator it = this->find(name);
  if (it == this->end())
    {
    return false;
    }

  return true;
}

bool cmPropertyDefinitionMap::IsPropertyChained(const std::string& name)
{
  cmPropertyDefinitionMap::iterator it = this->find(name);
  if (it == this->end())
    {
    return false;
    }

  return it->second.IsChained();
}
