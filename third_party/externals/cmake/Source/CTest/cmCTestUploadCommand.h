/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestUploadCommand_h
#define cmCTestUploadCommand_h

#include "cmCTestHandlerCommand.h"
#include "cmCTest.h"

/** \class cmCTestUpload
 * \brief Run a ctest script
 *
 * cmCTestUploadCommand defines the command to upload result files for
 * the project.
 */
class cmCTestUploadCommand : public cmCTestHandlerCommand
{
public:

  cmCTestUploadCommand()
    {
    }

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestUploadCommand* ni = new cmCTestUploadCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "ctest_upload";}

  cmTypeMacro(cmCTestUploadCommand, cmCTestHandlerCommand);

protected:
  cmCTestGenericHandler* InitializeHandler();

  virtual bool CheckArgumentKeyword(std::string const& arg);
  virtual bool CheckArgumentValue(std::string const& arg);

  enum
  {
    ArgumentDoingFiles = Superclass::ArgumentDoingLast1,
    ArgumentDoingLast2
  };

  cmCTest::SetOfStrings Files;
};


#endif
