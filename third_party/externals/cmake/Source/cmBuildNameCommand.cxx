/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmBuildNameCommand.h"

#include <cmsys/RegularExpression.hxx>

// cmBuildNameCommand
bool cmBuildNameCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(this->Disallowed(cmPolicies::CMP0036,
      "The build_name command should not be called; see CMP0036."))
    { return true; }
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  const char* cacheValue = this->Makefile->GetDefinition(args[0]);
  if(cacheValue)
    {
    // do we need to correct the value?
    cmsys::RegularExpression reg("[()/]");
    if (reg.find(cacheValue))
      {
      std::string cv = cacheValue;
      cmSystemTools::ReplaceString(cv,"/", "_");
      cmSystemTools::ReplaceString(cv,"(", "_");
      cmSystemTools::ReplaceString(cv,")", "_");
      this->Makefile->AddCacheDefinition(args[0],
                                     cv.c_str(),
                                     "Name of build.",
                                     cmState::STRING);
      }
    return true;
    }


  std::string buildname = "WinNT";
  if(this->Makefile->GetDefinition("UNIX"))
    {
    buildname = "";
    cmSystemTools::RunSingleCommand("uname -a", &buildname, &buildname);
    if(!buildname.empty())
      {
      std::string RegExp = "([^ ]*) [^ ]* ([^ ]*) ";
      cmsys::RegularExpression reg( RegExp.c_str() );
      if(reg.find(buildname.c_str()))
        {
        buildname = reg.match(1) + "-" + reg.match(2);
        }
      }
    }
  std::string compiler = "${CMAKE_CXX_COMPILER}";
  this->Makefile->ExpandVariablesInString ( compiler );
  buildname += "-";
  buildname += cmSystemTools::GetFilenameName(compiler);
  cmSystemTools::ReplaceString(buildname,
                               "/", "_");
  cmSystemTools::ReplaceString(buildname,
                               "(", "_");
  cmSystemTools::ReplaceString(buildname,
                               ")", "_");

  this->Makefile->AddCacheDefinition(args[0],
                                 buildname.c_str(),
                                 "Name of build.",
                                 cmState::STRING);
  return true;
}

