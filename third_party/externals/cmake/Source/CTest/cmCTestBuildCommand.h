/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestBuildCommand_h
#define cmCTestBuildCommand_h

#include "cmCTestHandlerCommand.h"

class cmGlobalGenerator;
class cmCTestBuildHandler;

/** \class cmCTestBuild
 * \brief Run a ctest script
 *
 * cmCTestBuildCommand defineds the command to build the project.
 */
class cmCTestBuildCommand : public cmCTestHandlerCommand
{
public:

  cmCTestBuildCommand();
  ~cmCTestBuildCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestBuildCommand* ni = new cmCTestBuildCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "ctest_build";}

  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  cmTypeMacro(cmCTestBuildCommand, cmCTestHandlerCommand);

  cmGlobalGenerator* GlobalGenerator;

protected:
  cmCTestBuildHandler* Handler;
  enum {
    ctb_BUILD = ct_LAST,
    ctb_NUMBER_ERRORS,
    ctb_NUMBER_WARNINGS,
    ctb_TARGET,
    ctb_CONFIGURATION,
    ctb_FLAGS,
    ctb_PROJECT_NAME,
    ctb_LAST
  };

  cmCTestGenericHandler* InitializeHandler();
};


#endif
