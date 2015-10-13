/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestBuildCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestBuildHandler.h"
#include "cmake.h"
#include "cmGlobalGenerator.h"


//----------------------------------------------------------------------------
cmCTestBuildCommand::cmCTestBuildCommand()
{
  this->GlobalGenerator = 0;
  this->Arguments[ctb_NUMBER_ERRORS] = "NUMBER_ERRORS";
  this->Arguments[ctb_NUMBER_WARNINGS] = "NUMBER_WARNINGS";
  this->Arguments[ctb_TARGET] = "TARGET";
  this->Arguments[ctb_CONFIGURATION] = "CONFIGURATION";
  this->Arguments[ctb_FLAGS] = "FLAGS";
  this->Arguments[ctb_PROJECT_NAME] = "PROJECT_NAME";
  this->Arguments[ctb_LAST] = 0;
  this->Last = ctb_LAST;
}

//----------------------------------------------------------------------------
cmCTestBuildCommand::~cmCTestBuildCommand()
{
  if ( this->GlobalGenerator )
    {
    delete this->GlobalGenerator;
    this->GlobalGenerator = 0;
    }
}

//----------------------------------------------------------------------------
cmCTestGenericHandler* cmCTestBuildCommand::InitializeHandler()
{
  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("build");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate build handler");
    return 0;
    }
  this->Handler =  (cmCTestBuildHandler*)handler;

  const char* ctestBuildCommand
    = this->Makefile->GetDefinition("CTEST_BUILD_COMMAND");
  if ( ctestBuildCommand && *ctestBuildCommand )
    {
    this->CTest->SetCTestConfiguration("MakeCommand", ctestBuildCommand,
      this->Quiet);
    }
  else
    {
    const char* cmakeGeneratorName
      = this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR");
    const char* cmakeProjectName
      = (this->Values[ctb_PROJECT_NAME] && *this->Values[ctb_PROJECT_NAME])
      ? this->Values[ctb_PROJECT_NAME]
      : this->Makefile->GetDefinition("CTEST_PROJECT_NAME");

    // Build configuration is determined by: CONFIGURATION argument,
    // or CTEST_BUILD_CONFIGURATION script variable, or
    // CTEST_CONFIGURATION_TYPE script variable, or ctest -C command
    // line argument... in that order.
    //
    const char* ctestBuildConfiguration
      = this->Makefile->GetDefinition("CTEST_BUILD_CONFIGURATION");
    const char* cmakeBuildConfiguration
      = (this->Values[ctb_CONFIGURATION] && *this->Values[ctb_CONFIGURATION])
      ? this->Values[ctb_CONFIGURATION]
      : ((ctestBuildConfiguration && *ctestBuildConfiguration)
        ? ctestBuildConfiguration
        : this->CTest->GetConfigType().c_str());

    const char* cmakeBuildAdditionalFlags
      = (this->Values[ctb_FLAGS] && *this->Values[ctb_FLAGS])
      ? this->Values[ctb_FLAGS]
      : this->Makefile->GetDefinition("CTEST_BUILD_FLAGS");
    const char* cmakeBuildTarget
      = (this->Values[ctb_TARGET] && *this->Values[ctb_TARGET])
      ? this->Values[ctb_TARGET]
      : this->Makefile->GetDefinition("CTEST_BUILD_TARGET");

    if ( cmakeGeneratorName && *cmakeGeneratorName &&
      cmakeProjectName && *cmakeProjectName )
      {
      if ( !cmakeBuildConfiguration )
        {
        cmakeBuildConfiguration = "Release";
        }
      if ( this->GlobalGenerator )
        {
        if ( this->GlobalGenerator->GetName() != cmakeGeneratorName )
          {
          delete this->GlobalGenerator;
          this->GlobalGenerator = 0;
          }
        }
      if ( !this->GlobalGenerator )
        {
        this->GlobalGenerator =
          this->Makefile->GetCMakeInstance()->CreateGlobalGenerator(
            cmakeGeneratorName);
        if(!this->GlobalGenerator)
          {
          std::string e = "could not create generator named \"";
          e += cmakeGeneratorName;
          e += "\"";
          this->Makefile->IssueMessage(cmake::FATAL_ERROR, e);
          cmSystemTools::SetFatalErrorOccured();
          return 0;
          }
        }
      if(strlen(cmakeBuildConfiguration) == 0)
        {
        const char* config = 0;
#ifdef CMAKE_INTDIR
        config = CMAKE_INTDIR;
#endif
        if(!config)
          {
          config = "Debug";
          }
        cmakeBuildConfiguration = config;
        }

      std::string dir = this->CTest->GetCTestConfiguration("BuildDirectory");
      std::string buildCommand
        = this->GlobalGenerator->
        GenerateCMakeBuildCommand(cmakeBuildTarget ? cmakeBuildTarget : "",
          cmakeBuildConfiguration,
          cmakeBuildAdditionalFlags ? cmakeBuildAdditionalFlags : "",
          this->Makefile->IgnoreErrorsCMP0061());
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
        "SetMakeCommand:" << buildCommand << "\n", this->Quiet);
      this->CTest->SetCTestConfiguration("MakeCommand", buildCommand.c_str(),
        this->Quiet);
      }
    else
      {
      std::ostringstream ostr;
      ostr << "has no project to build. If this is a "
        "\"built with CMake\" project, verify that CTEST_CMAKE_GENERATOR "
        "and CTEST_PROJECT_NAME are set."
        "\n"
        "CTEST_PROJECT_NAME is usually set in CTestConfig.cmake. Verify "
        "that CTestConfig.cmake exists, or CTEST_PROJECT_NAME "
        "is set in the script, or PROJECT_NAME is passed as an argument "
        "to ctest_build."
        "\n"
        "Alternatively, set CTEST_BUILD_COMMAND to build the project "
        "with a custom command line.";
      this->SetError(ostr.str());
      return 0;
      }
    }

  if(const char* useLaunchers =
     this->Makefile->GetDefinition("CTEST_USE_LAUNCHERS"))
    {
    this->CTest->SetCTestConfiguration("UseLaunchers", useLaunchers,
      this->Quiet);
    }

  handler->SetQuiet(this->Quiet);
  return handler;
}


bool cmCTestBuildCommand::InitialPass(std::vector<std::string> const& args,
                                      cmExecutionStatus &status)
{
  bool ret =  cmCTestHandlerCommand::InitialPass(args, status);
  if ( this->Values[ctb_NUMBER_ERRORS] && *this->Values[ctb_NUMBER_ERRORS])
    {
    std::ostringstream str;
    str << this->Handler->GetTotalErrors();
    this->Makefile->AddDefinition(
      this->Values[ctb_NUMBER_ERRORS], str.str().c_str());
    }
  if ( this->Values[ctb_NUMBER_WARNINGS]
       && *this->Values[ctb_NUMBER_WARNINGS])
    {
    std::ostringstream str;
    str << this->Handler->GetTotalWarnings();
    this->Makefile->AddDefinition(
      this->Values[ctb_NUMBER_WARNINGS], str.str().c_str());
    }
  return ret;
}
