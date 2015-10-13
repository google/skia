/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmPropertyDefinition.h"
#include "cmSystemTools.h"

void cmPropertyDefinition
::DefineProperty(const std::string& name, cmProperty::ScopeType scope,
                 const char *shortDescription,
                 const char *fullDescription,
                 bool chain)
{
  this->Name = name;
  this->Scope = scope;
  this->Chained = chain;
  if (shortDescription)
    {
    this->ShortDescription = shortDescription;
    }
  if (fullDescription)
    {
    this->FullDescription = fullDescription;
    }
}

