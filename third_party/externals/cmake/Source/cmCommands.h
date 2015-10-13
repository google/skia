/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCommands_h
#define cmCommands_h
#include "cmStandardIncludes.h"

#include <vector>

class cmCommand;
/**
 * Global function to return all compiled in commands.
 * To add a new command edit cmCommands.cxx or cmBootstrapCommands[12].cxx
 * and add your command.
 * It is up to the caller to delete the commands created by this
 * call.
 */
void GetBootstrapCommands1(std::vector<cmCommand*>& commands);
void GetBootstrapCommands2(std::vector<cmCommand*>& commands);
void GetPredefinedCommands(std::vector<cmCommand*>& commands);


#endif
