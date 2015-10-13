/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestTestCommand_h
#define cmCTestTestCommand_h

#include "cmCTestHandlerCommand.h"

/** \class cmCTestTest
 * \brief Run a ctest script
 *
 * cmCTestTestCommand defineds the command to test the project.
 */
class cmCTestTestCommand : public cmCTestHandlerCommand
{
public:

  cmCTestTestCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestTestCommand* ni = new cmCTestTestCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "ctest_test";}

  cmTypeMacro(cmCTestTestCommand, cmCTestHandlerCommand);

protected:
  virtual cmCTestGenericHandler* InitializeActualHandler();
  cmCTestGenericHandler* InitializeHandler();

  enum {
    ctt_BUILD = ct_LAST,
    ctt_RETURN_VALUE,
    ctt_START,
    ctt_END,
    ctt_STRIDE,
    ctt_EXCLUDE,
    ctt_INCLUDE,
    ctt_EXCLUDE_LABEL,
    ctt_INCLUDE_LABEL,
    ctt_PARALLEL_LEVEL,
    ctt_SCHEDULE_RANDOM,
    ctt_STOP_TIME,
    ctt_LAST
  };
};


#endif
