/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestMemCheckCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"


cmCTestGenericHandler* cmCTestMemCheckCommand::InitializeActualHandler()
{
  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("memcheck");

  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "MemoryCheckType", "CTEST_MEMORYCHECK_TYPE", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "MemoryCheckSanitizerOptions", "CTEST_MEMORYCHECK_SANITIZER_OPTIONS",
    this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "MemoryCheckCommand", "CTEST_MEMORYCHECK_COMMAND", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "MemoryCheckCommandOptions", "CTEST_MEMORYCHECK_COMMAND_OPTIONS",
    this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "MemoryCheckSuppressionFile", "CTEST_MEMORYCHECK_SUPPRESSIONS_FILE",
    this->Quiet);

  handler->SetQuiet(this->Quiet);
  return handler;
}

