/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCTestBuildAndTestHandler_h
#define cmCTestBuildAndTestHandler_h


#include "cmCTestGenericHandler.h"
#include "cmListFileCache.h"

class cmake;

/** \class cmCTestBuildAndTestHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestBuildAndTestHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestBuildAndTestHandler, cmCTestGenericHandler);

  /*
   * The main entry point for this class
   */
  int ProcessHandler();

  //! Set all the build and test arguments
  virtual int ProcessCommandLineArguments(
    const std::string& currentArg, size_t& idx,
    const std::vector<std::string>& allArgs);

  /*
   * Get the output variable
   */
  const char* GetOutput();

  cmCTestBuildAndTestHandler();

  virtual void Initialize();

protected:
  ///! Run CMake and build a test and then run it as a single test.
  int RunCMakeAndTest(std::string* output);
  int RunCMake(std::string* outstring, std::ostringstream &out,
               std::string &cmakeOutString,
               std::string &cwd, cmake *cm);

  std::string  Output;

  std::string              BuildGenerator;
  std::string              BuildGeneratorPlatform;
  std::string              BuildGeneratorToolset;
  std::vector<std::string> BuildOptions;
  bool                     BuildTwoConfig;
  std::string              BuildMakeProgram;
  std::string              ConfigSample;
  std::string              SourceDir;
  std::string              BinaryDir;
  std::string              BuildProject;
  std::string              TestCommand;
  bool                     BuildNoClean;
  std::string              BuildRunDir;
  std::string              ExecutableDirectory;
  std::vector<std::string> TestCommandArgs;
  std::vector<std::string> BuildTargets;
  bool                     BuildNoCMake;
  double                   Timeout;
};

#endif

