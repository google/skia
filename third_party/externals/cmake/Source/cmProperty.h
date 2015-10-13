/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmProperty_h
#define cmProperty_h

#include "cmStandardIncludes.h"

class cmProperty
{
public:
  enum ScopeType { TARGET, SOURCE_FILE, DIRECTORY, GLOBAL, CACHE,
                   TEST, VARIABLE, CACHED_VARIABLE, INSTALL };

  // set this property
  void Set(const std::string& name, const char *value);

  // append to this property
  void Append(const std::string& name, const char *value,
              bool asString = false);

  // get the value
  const char *GetValue() const;

  // construct with the value not set
  cmProperty() { this->ValueHasBeenSet = false; }

protected:
  std::string Name;
  std::string Value;
  bool ValueHasBeenSet;
};

#endif
