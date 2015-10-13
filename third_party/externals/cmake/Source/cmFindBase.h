/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFindBase_h
#define cmFindBase_h

#include "cmFindCommon.h"

/** \class cmFindBase
 * \brief Base class for most FIND_XXX commands.
 *
 * cmFindBase is a parent class for cmFindProgramCommand, cmFindPathCommand,
 * and cmFindLibraryCommand, cmFindFileCommand
 */
class cmFindBase : public cmFindCommon
{
public:
  cmFindBase();
  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool ParseArguments(std::vector<std::string> const& args);
  cmTypeMacro(cmFindBase, cmFindCommon);

protected:
  void PrintFindStuff();
  void ExpandPaths();

  // see if the VariableName is already set in the cache,
  // also copy the documentation from the cache to VariableDocumentation
  // if it has documentation in the cache
  bool CheckForVariableInCache();

  // use by command during find
  std::string VariableDocumentation;
  std::string VariableName;
  std::vector<std::string> Names;
  bool NamesPerDir;
  bool NamesPerDirAllowed;

  // CMAKE_*_PATH CMAKE_SYSTEM_*_PATH FRAMEWORK|LIBRARY|INCLUDE|PROGRAM
  std::string EnvironmentPath; // LIB,INCLUDE

  bool AlreadyInCache;
  bool AlreadyInCacheWithoutMetaInfo;
private:
  // Add pieces of the search.
  void FillCMakeVariablePath();
  void FillCMakeEnvironmentPath();
  void FillUserHintsPath();
  void FillSystemEnvironmentPath();
  void FillCMakeSystemVariablePath();
  void FillUserGuessPath();
};



#endif
