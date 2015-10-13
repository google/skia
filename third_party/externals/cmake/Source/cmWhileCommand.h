/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmWhileCommand_h
#define cmWhileCommand_h

#include "cmCommand.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"

class cmWhileFunctionBlocker : public cmFunctionBlocker
{
public:
  cmWhileFunctionBlocker(cmMakefile* mf);
  ~cmWhileFunctionBlocker();
  virtual bool IsFunctionBlocked(const cmListFileFunction& lff,
                                 cmMakefile &mf,
                                 cmExecutionStatus &);
  virtual bool ShouldRemove(const cmListFileFunction& lff, cmMakefile &mf);

  std::vector<cmListFileArgument> Args;
  std::vector<cmListFileFunction> Functions;
private:
  cmMakefile* Makefile;
  int Depth;
};

/// \brief Starts a while loop
class cmWhileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmWhileCommand;
    }

  /**
   * This overrides the default InvokeInitialPass implementation.
   * It records the arguments before expansion.
   */
  virtual bool InvokeInitialPass(const std::vector<cmListFileArgument>& args,
                                 cmExecutionStatus &);

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const&,
                           cmExecutionStatus &) { return false; }

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "while";}

  cmTypeMacro(cmWhileCommand, cmCommand);
};


#endif
