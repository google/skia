/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmRemoveCommand_h
#define cmRemoveCommand_h

#include "cmCommand.h"

/** \class cmRemoveCommand
 * \brief remove command
 *
 * cmRemoveCommand implements the remove CMake command
 */
class cmRemoveCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmRemoveCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const {return "remove";}

  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged() const
    {
    return true;
    }

  cmTypeMacro(cmRemoveCommand, cmCommand);
};



#endif
