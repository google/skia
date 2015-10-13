/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmElseCommand.h"

bool cmElseCommand::InitialPass(std::vector<std::string> const&,
                                cmExecutionStatus &)
{
  this->SetError("An ELSE command was found outside of a proper "
                 "IF ENDIF structure. Or its arguments did not match "
                 "the opening IF command.");
  return false;
}
