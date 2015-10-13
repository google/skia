/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
// This file is used to compile all the commands
// that CMake knows about at compile time.
// This is sort of a boot strapping approach since you would
// like to have CMake to build CMake.
#include "cmCommands.h"
#include "cmAddCustomCommandCommand.cxx"
#include "cmAddCustomTargetCommand.cxx"
#include "cmAddDefinitionsCommand.cxx"
#include "cmAddDependenciesCommand.cxx"
#include "cmAddExecutableCommand.cxx"
#include "cmAddLibraryCommand.cxx"
#include "cmAddSubDirectoryCommand.cxx"
#include "cmAddTestCommand.cxx"
#include "cmBreakCommand.cxx"
#include "cmBuildCommand.cxx"
#include "cmCMakeMinimumRequired.cxx"
#include "cmCMakePolicyCommand.cxx"
#include "cmCommandArgumentsHelper.cxx"
#include "cmConfigureFileCommand.cxx"
#include "cmContinueCommand.cxx"
#include "cmCoreTryCompile.cxx"
#include "cmCreateTestSourceList.cxx"
#include "cmDefinePropertyCommand.cxx"
#include "cmElseCommand.cxx"
#include "cmEnableLanguageCommand.cxx"
#include "cmEnableTestingCommand.cxx"
#include "cmEndForEachCommand.cxx"
#include "cmEndFunctionCommand.cxx"
#include "cmEndIfCommand.cxx"
#include "cmEndMacroCommand.cxx"
#include "cmEndWhileCommand.cxx"
#include "cmExecProgramCommand.cxx"
#include "cmExecuteProcessCommand.cxx"
#include "cmFindBase.cxx"
#include "cmFindCommon.cxx"
#include "cmFileCommand.cxx"
#include "cmFindFileCommand.cxx"
#include "cmFindLibraryCommand.cxx"
#include "cmFindPackageCommand.cxx"
#include "cmFindPathCommand.cxx"
#include "cmFindProgramCommand.cxx"
#include "cmForEachCommand.cxx"
#include "cmFunctionCommand.cxx"
#include "cmPathLabel.cxx"
#include "cmSearchPath.cxx"

void GetBootstrapCommands1(std::vector<cmCommand*>& commands)
{
  commands.push_back(new cmAddCustomCommandCommand);
  commands.push_back(new cmAddCustomTargetCommand);
  commands.push_back(new cmAddDefinitionsCommand);
  commands.push_back(new cmAddDependenciesCommand);
  commands.push_back(new cmAddExecutableCommand);
  commands.push_back(new cmAddLibraryCommand);
  commands.push_back(new cmAddSubDirectoryCommand);
  commands.push_back(new cmAddTestCommand);
  commands.push_back(new cmBreakCommand);
  commands.push_back(new cmBuildCommand);
  commands.push_back(new cmCMakeMinimumRequired);
  commands.push_back(new cmCMakePolicyCommand);
  commands.push_back(new cmConfigureFileCommand);
  commands.push_back(new cmContinueCommand);
  commands.push_back(new cmCreateTestSourceList);
  commands.push_back(new cmDefinePropertyCommand);
  commands.push_back(new cmElseCommand);
  commands.push_back(new cmEnableLanguageCommand);
  commands.push_back(new cmEnableTestingCommand);
  commands.push_back(new cmEndForEachCommand);
  commands.push_back(new cmEndFunctionCommand);
  commands.push_back(new cmEndIfCommand);
  commands.push_back(new cmEndMacroCommand);
  commands.push_back(new cmEndWhileCommand);
  commands.push_back(new cmExecProgramCommand);
  commands.push_back(new cmExecuteProcessCommand);
  commands.push_back(new cmFileCommand);
  commands.push_back(new cmFindFileCommand);
  commands.push_back(new cmFindLibraryCommand);
  commands.push_back(new cmFindPackageCommand);
  commands.push_back(new cmFindPathCommand);
  commands.push_back(new cmFindProgramCommand);
  commands.push_back(new cmForEachCommand);
  commands.push_back(new cmFunctionCommand);
}
