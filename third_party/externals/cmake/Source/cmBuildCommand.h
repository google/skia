/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmBuildCommand_h
#define cmBuildCommand_h

#include "cmCommand.h"

/** \class cmBuildCommand
 * \brief build_command command
 *
 * cmBuildCommand implements the build_command CMake command
 */
class cmBuildCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmBuildCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The primary command signature with optional, KEYWORD-based args.
   */
  virtual bool MainSignature(std::vector<std::string> const& args);

  /**
   * Legacy "exactly 2 args required" signature.
   */
  virtual bool TwoArgsSignature(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const {return "build_command";}

  cmTypeMacro(cmBuildCommand, cmCommand);
private:
  bool IgnoreErrors() const;
};

#endif
