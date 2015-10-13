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
#include "cmConditionEvaluator.cxx"
#include "cmExpandedCommandArgument.cxx"
#include "cmGetCMakePropertyCommand.cxx"
#include "cmGetDirectoryPropertyCommand.cxx"
#include "cmGetFilenameComponentCommand.cxx"
#include "cmGetPropertyCommand.cxx"
#include "cmGetSourceFilePropertyCommand.cxx"
#include "cmGetTargetPropertyCommand.cxx"
#include "cmHexFileConverter.cxx"
#include "cmIfCommand.cxx"
#include "cmIncludeCommand.cxx"
#include "cmIncludeDirectoryCommand.cxx"
#include "cmIncludeRegularExpressionCommand.cxx"
#include "cmInstallFilesCommand.cxx"
#include "cmInstallCommandArguments.cxx"
#include "cmInstallCommand.cxx"
#include "cmInstallTargetsCommand.cxx"
#include "cmLinkDirectoriesCommand.cxx"
#include "cmListCommand.cxx"
#include "cmMacroCommand.cxx"
#include "cmMakeDirectoryCommand.cxx"
#include "cmMarkAsAdvancedCommand.cxx"
#include "cmMathCommand.cxx"
#include "cmMessageCommand.cxx"
#include "cmOptionCommand.cxx"
#include "cmProjectCommand.cxx"
#include "cmReturnCommand.cxx"
#include "cmSeparateArgumentsCommand.cxx"
#include "cmSetCommand.cxx"
#include "cmSetDirectoryPropertiesCommand.cxx"
#include "cmSetPropertyCommand.cxx"
#include "cmSetSourceFilesPropertiesCommand.cxx"
#include "cmSetTargetPropertiesCommand.cxx"
#include "cmSetTestsPropertiesCommand.cxx"
#include "cmGetTestPropertyCommand.cxx"
#include "cmSiteNameCommand.cxx"
#include "cmStringCommand.cxx"
#include "cmSubdirCommand.cxx"
#include "cmTargetLinkLibrariesCommand.cxx"
#include "cmTimestamp.cxx"
#include "cmTryCompileCommand.cxx"
#include "cmTryRunCommand.cxx"
#include "cmUnsetCommand.cxx"
#include "cmWhileCommand.cxx"

void GetBootstrapCommands2(std::vector<cmCommand*>& commands)
{
  commands.push_back(new cmGetCMakePropertyCommand);
  commands.push_back(new cmGetDirectoryPropertyCommand);
  commands.push_back(new cmGetFilenameComponentCommand);
  commands.push_back(new cmGetPropertyCommand);
  commands.push_back(new cmGetSourceFilePropertyCommand);
  commands.push_back(new cmGetTargetPropertyCommand);
  commands.push_back(new cmIfCommand);
  commands.push_back(new cmIncludeCommand);
  commands.push_back(new cmIncludeDirectoryCommand);
  commands.push_back(new cmIncludeRegularExpressionCommand);
  commands.push_back(new cmInstallCommand);
  commands.push_back(new cmInstallFilesCommand);
  commands.push_back(new cmInstallTargetsCommand);
  commands.push_back(new cmLinkDirectoriesCommand);
  commands.push_back(new cmListCommand);
  commands.push_back(new cmMacroCommand);
  commands.push_back(new cmMakeDirectoryCommand);
  commands.push_back(new cmMarkAsAdvancedCommand);
  commands.push_back(new cmMathCommand);
  commands.push_back(new cmMessageCommand);
  commands.push_back(new cmOptionCommand);
  commands.push_back(new cmProjectCommand);
  commands.push_back(new cmReturnCommand);
  commands.push_back(new cmSeparateArgumentsCommand);
  commands.push_back(new cmSetCommand);
  commands.push_back(new cmSetDirectoryPropertiesCommand);
  commands.push_back(new cmSetPropertyCommand);
  commands.push_back(new cmSetSourceFilesPropertiesCommand);
  commands.push_back(new cmSetTargetPropertiesCommand);
  commands.push_back(new cmGetTestPropertyCommand);
  commands.push_back(new cmSetTestsPropertiesCommand);
  commands.push_back(new cmSiteNameCommand);
  commands.push_back(new cmStringCommand);
  commands.push_back(new cmSubdirCommand);
  commands.push_back(new cmTargetLinkLibrariesCommand);
  commands.push_back(new cmTryCompileCommand);
  commands.push_back(new cmTryRunCommand);
  commands.push_back(new cmUnsetCommand);
  commands.push_back(new cmWhileCommand);
}
