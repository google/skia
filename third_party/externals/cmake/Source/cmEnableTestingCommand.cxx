/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmEnableTestingCommand.h"
#include "cmLocalGenerator.h"

// we do this in the final pass so that we now the subdirs have all
// been defined
bool cmEnableTestingCommand::InitialPass(std::vector<std::string> const&,
                                         cmExecutionStatus &)
{
  this->Makefile->AddDefinition("CMAKE_TESTING_ENABLED","1");
  return true;
}
