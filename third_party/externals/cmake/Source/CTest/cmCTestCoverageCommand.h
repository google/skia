/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestCoverageCommand_h
#define cmCTestCoverageCommand_h

#include "cmCTestHandlerCommand.h"

/** \class cmCTestCoverage
 * \brief Run a ctest script
 *
 * cmCTestCoverageCommand defineds the command to test the project.
 */
class cmCTestCoverageCommand : public cmCTestHandlerCommand
{
public:

  cmCTestCoverageCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestCoverageCommand* ni = new cmCTestCoverageCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "ctest_coverage";}

  cmTypeMacro(cmCTestCoverageCommand, cmCTestHandlerCommand);

protected:
  cmCTestGenericHandler* InitializeHandler();

  virtual bool CheckArgumentKeyword(std::string const& arg);
  virtual bool CheckArgumentValue(std::string const& arg);

  enum
  {
    ArgumentDoingLabels = Superclass::ArgumentDoingLast1,
    ArgumentDoingLast2
  };

  bool LabelsMentioned;
  std::set<std::string> Labels;
};


#endif

