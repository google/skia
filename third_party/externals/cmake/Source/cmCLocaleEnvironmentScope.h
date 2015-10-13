/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCLocaleEnvironmentScope_h
#define cmCLocaleEnvironmentScope_h

#include "cmStandardIncludes.h"

class cmCLocaleEnvironmentScope
{
public:
  cmCLocaleEnvironmentScope();
  ~cmCLocaleEnvironmentScope();

private:
  std::string GetEnv(std::string const& key);
  void SetEnv(std::string const& key, std::string const& value);

  typedef std::map<std::string, std::string> backup_map_t;
  backup_map_t EnvironmentBackup;
};

#endif
