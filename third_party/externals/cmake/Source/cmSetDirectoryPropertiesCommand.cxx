/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSetDirectoryPropertiesCommand.h"

#include "cmake.h"

// cmSetDirectoryPropertiesCommand
bool cmSetDirectoryPropertiesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string errors;
  bool ret =
    cmSetDirectoryPropertiesCommand::RunCommand(this->Makefile,
                                                args.begin() + 1,
                                                args.end(), errors);
  if (!ret)
    {
    this->SetError(errors);
    }
  return ret;
}

bool cmSetDirectoryPropertiesCommand
::RunCommand(cmMakefile *mf,
             std::vector<std::string>::const_iterator ait,
             std::vector<std::string>::const_iterator aitend,
             std::string &errors)
{
  for (; ait != aitend; ait += 2 )
    {
    if ( ait +1 == aitend)
      {
      errors = "Wrong number of arguments";
      return false;
      }
    const std::string& prop = *ait;
    const std::string& value = *(ait+1);
    if ( prop == "VARIABLES" )
      {
      errors =
        "Variables and cache variables should be set using SET command";
      return false;
      }
    else if ( prop == "MACROS" )
      {
      errors =
        "Commands and macros cannot be set using SET_CMAKE_PROPERTIES";
      return false;
      }
    mf->SetProperty(prop, value.c_str());
    }

  return true;
}

