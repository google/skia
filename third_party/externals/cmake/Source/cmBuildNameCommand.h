/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmBuildNameCommand_h
#define cmBuildNameCommand_h

#include "cmCommand.h"

class cmBuildNameCommand : public cmCommand
{
public:
  cmTypeMacro(cmBuildNameCommand, cmCommand);
  virtual cmCommand* Clone() { return new cmBuildNameCommand; }
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);
  virtual std::string GetName() const {return "build_name";}
  virtual bool IsScriptable() const { return true; }
  virtual bool IsDiscouraged() const { return true; }
};



#endif
