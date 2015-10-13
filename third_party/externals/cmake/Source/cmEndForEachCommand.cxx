/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmEndForEachCommand.h"

bool cmEndForEachCommand
::InvokeInitialPass(std::vector<cmListFileArgument> const&,
                    cmExecutionStatus &)
{
  this->SetError("An ENDFOREACH command was found outside of a proper "
                 "FOREACH ENDFOREACH structure. Or its arguments did "
                 "not match the opening FOREACH command.");
  return false;
}

