/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmIfCommand_h
#define cmIfCommand_h

#include "cmCommand.h"
#include "cmFunctionBlocker.h"

class cmIfFunctionBlocker : public cmFunctionBlocker
{
public:
  cmIfFunctionBlocker() {
    this->HasRun = false; this->ScopeDepth = 0; }
  virtual ~cmIfFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction& lff,
                                 cmMakefile &mf,
                                 cmExecutionStatus &);
  virtual bool ShouldRemove(const cmListFileFunction& lff,
                            cmMakefile &mf);

  std::vector<cmListFileArgument> Args;
  std::vector<cmListFileFunction> Functions;
  bool IsBlocking;
  bool HasRun;
  unsigned int ScopeDepth;
};

/// Starts an if block
class cmIfCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmIfCommand;
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
                           cmExecutionStatus &) { return false;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "if";}

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  // Filter the given variable definition based on policy CMP0054.
  static const char* GetDefinitionIfUnquoted(
    const cmMakefile* mf, cmExpandedCommandArgument const& argument);

  cmTypeMacro(cmIfCommand, cmCommand);
};


#endif
