/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExecuteProcessCommand_h
#define cmExecuteProcessCommand_h

#include "cmCommand.h"

/** \class cmExecuteProcessCommand
 * \brief Command that adds a target to the build system.
 *
 * cmExecuteProcessCommand is a CMake language interface to the KWSys
 * Process Execution implementation.
 */
class cmExecuteProcessCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmExecuteProcessCommand;
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
    {return "execute_process";}

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  cmTypeMacro(cmExecuteProcessCommand, cmCommand);
};

#endif
