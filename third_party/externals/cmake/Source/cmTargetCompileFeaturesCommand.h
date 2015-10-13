/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTargetCompileFeaturesCommand_h
#define cmTargetCompileFeaturesCommand_h

#include "cmTargetPropCommandBase.h"

class cmTargetCompileFeaturesCommand : public cmTargetPropCommandBase
{
  virtual cmCommand* Clone()
    {
    return new cmTargetCompileFeaturesCommand;
    }

  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  virtual std::string GetName() const { return "target_compile_features";}

  cmTypeMacro(cmTargetCompileFeaturesCommand, cmTargetPropCommandBase);

private:
  virtual void HandleImportedTarget(const std::string &tgt);
  virtual void HandleMissingTarget(const std::string &name);

  virtual bool HandleDirectContent(cmTarget *tgt,
                                   const std::vector<std::string> &content,
                                   bool prepend, bool system);
  virtual std::string Join(const std::vector<std::string> &content);
};

#endif
