/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExecProgramCommand_h
#define cmExecProgramCommand_h

#include "cmCommand.h"

/** \class cmExecProgramCommand
 * \brief Command that adds a target to the build system.
 *
 * cmExecProgramCommand adds an extra target to the build system.
 * This is useful when you would like to add special
 * targets like "install,", "clean," and so on.
 */
class cmExecProgramCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmExecProgramCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const
    {return "exec_program";}

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged() const
    {
    return true;
    }

  cmTypeMacro(cmExecProgramCommand, cmCommand);
private:
  static bool RunCommand(const char* command, std::string& output,
                         int &retVal, const char* directory = 0,
                         bool verbose = true);
};

#endif
