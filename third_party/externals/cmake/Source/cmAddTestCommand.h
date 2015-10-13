/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmAddTestCommand_h
#define cmAddTestCommand_h

#include "cmCommand.h"

/** \class cmAddTestCommand
 * \brief Add a test to the lists of tests to run.
 *
 * cmAddTestCommand adds a test to the list of tests to run .
 */
class cmAddTestCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmAddTestCommand;
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
  virtual std::string GetName() const { return "add_test";}

  cmTypeMacro(cmAddTestCommand, cmCommand);
private:
  bool HandleNameMode(std::vector<std::string> const& args);
};


#endif
