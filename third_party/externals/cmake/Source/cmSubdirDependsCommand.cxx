/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSubdirDependsCommand.h"

bool cmSubdirDependsCommand::InitialPass(std::vector<std::string> const& ,
                                         cmExecutionStatus &)
{
  this->Disallowed(cmPolicies::CMP0029,
    "The subdir_depends command should not be called; see CMP0029.");
  return true;
}
