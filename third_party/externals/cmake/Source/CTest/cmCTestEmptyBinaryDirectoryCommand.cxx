/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestEmptyBinaryDirectoryCommand.h"

#include "cmCTestScriptHandler.h"

bool cmCTestEmptyBinaryDirectoryCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() != 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  if ( !cmCTestScriptHandler::EmptyBinaryDirectory(args[0].c_str()) )
    {
    std::ostringstream ostr;
    ostr << "problem removing the binary directory: " << args[0];
    this->SetError(ostr.str());
    return false;
    }

  return true;
}


