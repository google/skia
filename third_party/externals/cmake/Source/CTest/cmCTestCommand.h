/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestCommand_h
#define cmCTestCommand_h

#include "cmCommand.h"

class cmCTest;
class cmCTestScriptHandler;

/** \class cmCTestCommand
 * \brief A superclass for all commands added to the CTestScriptHandler
 *
 * cmCTestCommand is the superclass for all commands that will be added to
 * the ctest script handlers parser.
 *
 */
class cmCTestCommand : public cmCommand
{
public:

  cmCTestCommand() {this->CTest = 0; this->CTestScriptHandler = 0;}

  cmCTest *CTest;
  cmCTestScriptHandler *CTestScriptHandler;

  cmTypeMacro(cmCTestCommand, cmCommand);
};


#endif
