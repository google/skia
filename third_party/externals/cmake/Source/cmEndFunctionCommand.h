/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmEndFunctionCommand_h
#define cmEndFunctionCommand_h

#include "cmCommand.h"

/** \class cmEndFunctionCommand
 * \brief ends an if block
 *
 * cmEndFunctionCommand ends an if block
 */
class cmEndFunctionCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmEndFunctionCommand;
    }

  /**
   * Override cmCommand::InvokeInitialPass to get arguments before
   * expansion.
   */
  virtual bool InvokeInitialPass(std::vector<cmListFileArgument> const&,
                                 cmExecutionStatus &);

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const&,
                           cmExecutionStatus &) {return false;}

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "endfunction";}

  cmTypeMacro(cmEndFunctionCommand, cmCommand);
};


#endif
