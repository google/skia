/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestConfigureCommand_h
#define cmCTestConfigureCommand_h

#include "cmCTestHandlerCommand.h"

/** \class cmCTestConfigure
 * \brief Run a ctest script
 *
 * cmCTestConfigureCommand defineds the command to configures the project.
 */
class cmCTestConfigureCommand : public cmCTestHandlerCommand
{
public:
  cmCTestConfigureCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestConfigureCommand* ni = new cmCTestConfigureCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "ctest_configure";}

  cmTypeMacro(cmCTestConfigureCommand, cmCTestHandlerCommand);

protected:
  cmCTestGenericHandler* InitializeHandler();

  enum {
    ctc_FIRST = ct_LAST,
    ctc_OPTIONS,
    ctc_LAST
  };
};


#endif
