/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestReadCustomFilesCommand.h"
#include "cmCTest.h"

bool cmCTestReadCustomFilesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if (args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::vector<std::string>::const_iterator dit;
  for ( dit = args.begin(); dit != args.end(); ++ dit )
    {
    this->CTest->ReadCustomConfigurationFileTree(dit->c_str(),
      this->Makefile);
    }

  return true;
}


