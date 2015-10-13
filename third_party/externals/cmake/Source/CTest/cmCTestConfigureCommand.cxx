/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestConfigureCommand.h"

#include "cmGlobalGenerator.h"
#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

cmCTestConfigureCommand::cmCTestConfigureCommand()
{
  this->Arguments[ctc_OPTIONS] = "OPTIONS";
  this->Arguments[ctc_LAST] = 0;
  this->Last = ctc_LAST;
}

cmCTestGenericHandler* cmCTestConfigureCommand::InitializeHandler()
{
  std::vector<std::string> options;

  if (this->Values[ctc_OPTIONS])
    {
    cmSystemTools::ExpandListArgument(this->Values[ctc_OPTIONS], options);
    }

  if ( this->CTest->GetCTestConfiguration("BuildDirectory").empty() )
    {
    this->SetError("Build directory not specified. Either use BUILD "
      "argument to CTEST_CONFIGURE command or set CTEST_BINARY_DIRECTORY "
      "variable");
    return 0;
    }

  const char* ctestConfigureCommand
    = this->Makefile->GetDefinition("CTEST_CONFIGURE_COMMAND");

  if ( ctestConfigureCommand && *ctestConfigureCommand )
    {
    this->CTest->SetCTestConfiguration("ConfigureCommand",
      ctestConfigureCommand, this->Quiet);
    }
  else
    {
    const char* cmakeGeneratorName
      = this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR");
    if ( cmakeGeneratorName && *cmakeGeneratorName )
      {
      const std::string& source_dir
        = this->CTest->GetCTestConfiguration("SourceDirectory");
      if ( source_dir.empty() )
        {
        this->SetError("Source directory not specified. Either use SOURCE "
          "argument to CTEST_CONFIGURE command or set CTEST_SOURCE_DIRECTORY "
          "variable");
        return 0;
        }

      const std::string cmakelists_file = source_dir + "/CMakeLists.txt";
      if ( !cmSystemTools::FileExists(cmakelists_file.c_str()) )
        {
        std::ostringstream e;
        e << "CMakeLists.txt file does not exist ["
          << cmakelists_file << "]";
        this->SetError(e.str());
        return 0;
        }

      bool multiConfig = false;
      bool cmakeBuildTypeInOptions = false;

      cmGlobalGenerator *gg =
        this->Makefile->GetCMakeInstance()->CreateGlobalGenerator(
          cmakeGeneratorName);
      if(gg)
        {
        multiConfig = gg->IsMultiConfig();
        delete gg;
        }

      std::string cmakeConfigureCommand = "\"";
      cmakeConfigureCommand += cmSystemTools::GetCMakeCommand();
      cmakeConfigureCommand += "\"";

      std::vector<std::string>::const_iterator it;
      std::string option;
      for (it= options.begin(); it!=options.end(); ++it)
        {
        option = *it;

        cmakeConfigureCommand += " \"";
        cmakeConfigureCommand += option;
        cmakeConfigureCommand += "\"";

        if ((0 != strstr(option.c_str(), "CMAKE_BUILD_TYPE=")) ||
           (0 != strstr(option.c_str(), "CMAKE_BUILD_TYPE:STRING=")))
          {
          cmakeBuildTypeInOptions = true;
          }
        }

      if (!multiConfig && !cmakeBuildTypeInOptions &&
          !this->CTest->GetConfigType().empty())
        {
        cmakeConfigureCommand += " \"-DCMAKE_BUILD_TYPE:STRING=";
        cmakeConfigureCommand += this->CTest->GetConfigType();
        cmakeConfigureCommand += "\"";
        }

      cmakeConfigureCommand += " \"-G";
      cmakeConfigureCommand += cmakeGeneratorName;
      cmakeConfigureCommand += "\"";

      const char* cmakeGeneratorPlatform =
        this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR_PLATFORM");
      if(cmakeGeneratorPlatform && *cmakeGeneratorPlatform)
        {
        cmakeConfigureCommand += " \"-A";
        cmakeConfigureCommand += cmakeGeneratorPlatform;
        cmakeConfigureCommand += "\"";
        }

      const char* cmakeGeneratorToolset =
        this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR_TOOLSET");
      if(cmakeGeneratorToolset && *cmakeGeneratorToolset)
        {
        cmakeConfigureCommand += " \"-T";
        cmakeConfigureCommand += cmakeGeneratorToolset;
        cmakeConfigureCommand += "\"";
        }

      cmakeConfigureCommand += " \"";
      cmakeConfigureCommand += source_dir;
      cmakeConfigureCommand += "\"";

      this->CTest->SetCTestConfiguration("ConfigureCommand",
        cmakeConfigureCommand.c_str(), this->Quiet);
      }
    else
      {
      this->SetError("Configure command is not specified. If this is a "
        "\"built with CMake\" project, set CTEST_CMAKE_GENERATOR. If not, "
        "set CTEST_CONFIGURE_COMMAND.");
      return 0;
      }
    }

  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("configure");
  if ( !handler )
    {
    this->SetError(
      "internal CTest error. Cannot instantiate configure handler");
    return 0;
    }
  handler->SetQuiet(this->Quiet);
  return handler;
}
