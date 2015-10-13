/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestMemCheckCommand_h
#define cmCTestMemCheckCommand_h

#include "cmCTestTestCommand.h"

class cmCTestGenericHandler;

/** \class cmCTestMemCheck
 * \brief Run a ctest script
 *
 * cmCTestMemCheckCommand defineds the command to test the project.
 */
class cmCTestMemCheckCommand : public cmCTestTestCommand
{
public:

  cmCTestMemCheckCommand() {}

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestMemCheckCommand* ni = new cmCTestMemCheckCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "ctest_memcheck";}

  cmTypeMacro(cmCTestMemCheckCommand, cmCTestTestCommand);

protected:
  cmCTestGenericHandler* InitializeActualHandler();
};


#endif

