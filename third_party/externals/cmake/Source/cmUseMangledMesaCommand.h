/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmUseMangledMesaCommand_h
#define cmUseMangledMesaCommand_h

#include "cmCommand.h"

class cmUseMangledMesaCommand : public cmCommand
{
public:
  cmTypeMacro(cmUseMangledMesaCommand, cmCommand);
  virtual cmCommand* Clone() { return new cmUseMangledMesaCommand; }
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);
  virtual std::string GetName() const { return "use_mangled_mesa";}
  virtual bool IsScriptable() const { return true; }
  virtual bool IsDiscouraged() const { return true; }
protected:
  void CopyAndFullPathMesaHeader(const char* source,
                                 const char* outdir);
};

#endif
