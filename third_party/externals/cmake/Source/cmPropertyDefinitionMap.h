/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmPropertyDefinitionMap_h
#define cmPropertyDefinitionMap_h

#include "cmPropertyDefinition.h"

class cmDocumentationSection;

class cmPropertyDefinitionMap :
public std::map<std::string,cmPropertyDefinition>
{
public:
  // define the property
  void DefineProperty(const std::string& name, cmProperty::ScopeType scope,
                      const char *ShortDescription,
                      const char *FullDescription,
                      bool chain);

  // has a named property been defined
  bool IsPropertyDefined(const std::string& name);

  // is a named property set to chain
  bool IsPropertyChained(const std::string& name);
};

#endif

