/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTryRunCommand.h"
#include "cmTryCompileCommand.h"
#include <cmsys/FStream.hxx>

// cmTryRunCommand
bool cmTryRunCommand
::InitialPass(std::vector<std::string> const& argv, cmExecutionStatus &)
{
  if(argv.size() < 4)
    {
    return false;
    }

  if(this->Makefile->GetCMakeInstance()->GetWorkingMode() ==
                                                      cmake::FIND_PACKAGE_MODE)
    {
    this->Makefile->IssueMessage(cmake::FATAL_ERROR,
            "The TRY_RUN() command is not supported in --find-package mode.");
    return false;
    }

  // build an arg list for TryCompile and extract the runArgs,
  std::vector<std::string> tryCompile;

  this->CompileResultVariable = "";
  this->RunResultVariable = "";
  this->OutputVariable = "";
  this->RunOutputVariable = "";
  this->CompileOutputVariable = "";

  std::string runArgs;
  unsigned int i;
  for (i = 1; i < argv.size(); ++i)
    {
    if (argv[i] == "ARGS")
      {
      ++i;
      while (i < argv.size() && argv[i] != "COMPILE_DEFINITIONS" &&
             argv[i] != "CMAKE_FLAGS" &&
             argv[i] != "LINK_LIBRARIES")
        {
        runArgs += " ";
        runArgs += argv[i];
        ++i;
        }
      if (i < argv.size())
        {
        tryCompile.push_back(argv[i]);
        }
      }
    else
      {
      if (argv[i] == "OUTPUT_VARIABLE")
        {
        if ( argv.size() <= (i+1) )
          {
          cmSystemTools::Error(
            "OUTPUT_VARIABLE specified but there is no variable");
          return false;
          }
        i++;
        this->OutputVariable = argv[i];
        }
      else if (argv[i] == "RUN_OUTPUT_VARIABLE")
        {
        if (argv.size() <= (i + 1))
          {
          cmSystemTools::Error(
            "RUN_OUTPUT_VARIABLE specified but there is no variable");
          return false;
          }
        i++;
        this->RunOutputVariable = argv[i];
        }
      else if (argv[i] == "COMPILE_OUTPUT_VARIABLE")
        {
        if (argv.size() <= (i + 1))
          {
          cmSystemTools::Error(
            "COMPILE_OUTPUT_VARIABLE specified but there is no variable");
          return false;
          }
        i++;
        this->CompileOutputVariable = argv[i];
        }
      else
        {
        tryCompile.push_back(argv[i]);
        }
      }
    }

  // although they could be used together, don't allow it, because
  // using OUTPUT_VARIABLE makes crosscompiling harder
  if (this->OutputVariable.size()
      && (!this->RunOutputVariable.empty()
       || !this->CompileOutputVariable.empty()))
    {
    cmSystemTools::Error(
      "You cannot use OUTPUT_VARIABLE together with COMPILE_OUTPUT_VARIABLE "
      "or RUN_OUTPUT_VARIABLE. Please use only COMPILE_OUTPUT_VARIABLE and/or "
      "RUN_OUTPUT_VARIABLE.");
    return false;
    }

  bool captureRunOutput = false;
  if (!this->OutputVariable.empty())
    {
    captureRunOutput = true;
    tryCompile.push_back("OUTPUT_VARIABLE");
    tryCompile.push_back(this->OutputVariable);
    }
  if (!this->CompileOutputVariable.empty())
    {
    tryCompile.push_back("OUTPUT_VARIABLE");
    tryCompile.push_back(this->CompileOutputVariable);
    }
  if (!this->RunOutputVariable.empty())
    {
    captureRunOutput = true;
    }

  this->RunResultVariable = argv[0];
  this->CompileResultVariable = argv[1];

  // do the try compile
  int res = this->TryCompileCode(tryCompile);

  // now try running the command if it compiled
  if (!res)
    {
    if (this->OutputFile.empty())
      {
      cmSystemTools::Error(this->FindErrorMessage.c_str());
      }
    else
      {
      // "run" it and capture the output
      std::string runOutputContents;
      if (this->Makefile->IsOn("CMAKE_CROSSCOMPILING") &&
          !this->Makefile->IsDefinitionSet("CMAKE_CROSSCOMPILING_EMULATOR"))
        {
        this->DoNotRunExecutable(runArgs,
                                 argv[3],
                                 captureRunOutput ? &runOutputContents : 0);
        }
      else
        {
        this->RunExecutable(runArgs, &runOutputContents);
        }

      // now put the output into the variables
      if(!this->RunOutputVariable.empty())
        {
        this->Makefile->AddDefinition(this->RunOutputVariable,
                                      runOutputContents.c_str());
        }

      if(!this->OutputVariable.empty())
        {
        // if the TryCompileCore saved output in this outputVariable then
        // prepend that output to this output
        const char* compileOutput
                 = this->Makefile->GetDefinition(this->OutputVariable);
        if (compileOutput)
          {
          runOutputContents = std::string(compileOutput) + runOutputContents;
          }
        this->Makefile->AddDefinition(this->OutputVariable,
                                      runOutputContents.c_str());
        }
      }
    }

  // if we created a directory etc, then cleanup after ourselves
  if(!this->Makefile->GetCMakeInstance()->GetDebugTryCompile())
    {
    this->CleanupFiles(this->BinaryDirectory.c_str());
    }
  return true;
}

void cmTryRunCommand::RunExecutable(const std::string& runArgs,
                                    std::string* out)
{
  int retVal = -1;

  std::string finalCommand;
  const std::string emulator =
  this->Makefile->GetSafeDefinition("CMAKE_CROSSCOMPILING_EMULATOR");
  if (!emulator.empty())
    {
    std::vector<std::string> emulatorWithArgs;
    cmSystemTools::ExpandListArgument(emulator, emulatorWithArgs);
    finalCommand += cmSystemTools::ConvertToRunCommandPath(
                                 emulatorWithArgs[0].c_str());
    finalCommand += " ";
    for (std::vector<std::string>::const_iterator ei =
         emulatorWithArgs.begin()+1;
         ei != emulatorWithArgs.end(); ++ei)
      {
      finalCommand += "\"";
      finalCommand += *ei;
      finalCommand += "\"";
      finalCommand += " ";
      }
    }
  finalCommand += cmSystemTools::ConvertToRunCommandPath(
                               this->OutputFile.c_str());
  if (!runArgs.empty())
    {
    finalCommand += runArgs;
    }
  int timeout = 0;
  bool worked = cmSystemTools::RunSingleCommand(finalCommand.c_str(),
                out, out, &retVal,
                0, cmSystemTools::OUTPUT_NONE, timeout);
  // set the run var
  char retChar[1000];
  if (worked)
    {
    sprintf(retChar, "%i", retVal);
    }
  else
    {
    strcpy(retChar, "FAILED_TO_RUN");
    }
  this->Makefile->AddCacheDefinition(this->RunResultVariable, retChar,
                                     "Result of TRY_RUN",
                                     cmState::INTERNAL);
}

/* This is only used when cross compiling. Instead of running the
 executable, two cache variables are created which will hold the results
 the executable would have produced.
*/
void cmTryRunCommand::DoNotRunExecutable(const std::string& runArgs,
                                    const std::string& srcFile,
                                    std::string* out
                                    )
{
  // copy the executable out of the CMakeFiles/ directory, so it is not
  // removed at the end of TRY_RUN and the user can run it manually
  // on the target platform.
  std::string copyDest =  this->Makefile->GetHomeOutputDirectory();
  copyDest += cmake::GetCMakeFilesDirectory();
  copyDest += "/";
  copyDest += cmSystemTools::GetFilenameWithoutExtension(
                                                     this->OutputFile);
  copyDest += "-";
  copyDest += this->RunResultVariable;
  copyDest += cmSystemTools::GetFilenameExtension(this->OutputFile);
  cmSystemTools::CopyFileAlways(this->OutputFile, copyDest);

  std::string resultFileName =  this->Makefile->GetHomeOutputDirectory();
  resultFileName += "/TryRunResults.cmake";

  std::string detailsString = "For details see ";
  detailsString += resultFileName;

  std::string internalRunOutputName=this->RunResultVariable+"__TRYRUN_OUTPUT";
  bool error = false;

  if (this->Makefile->GetDefinition(this->RunResultVariable) == 0)
    {
    // if the variables doesn't exist, create it with a helpful error text
    // and mark it as advanced
    std::string comment;
    comment += "Run result of TRY_RUN(), indicates whether the executable "
               "would have been able to run on its target platform.\n";
    comment += detailsString;
    this->Makefile->AddCacheDefinition(this->RunResultVariable,
                                       "PLEASE_FILL_OUT-FAILED_TO_RUN",
                                       comment.c_str(),
                                       cmState::STRING);

    cmState* state = this->Makefile->GetState();
    const char* existingValue
                        = state->GetCacheEntryValue(this->RunResultVariable);
    if (existingValue)
      {
      state->SetCacheEntryProperty(this->RunResultVariable, "ADVANCED", "1");
      }

    error = true;
    }

  // is the output from the executable used ?
  if (out!=0)
    {
    if (this->Makefile->GetDefinition(internalRunOutputName) == 0)
      {
      // if the variables doesn't exist, create it with a helpful error text
      // and mark it as advanced
      std::string comment;
      comment+="Output of TRY_RUN(), contains the text, which the executable "
           "would have printed on stdout and stderr on its target platform.\n";
      comment += detailsString;

      this->Makefile->AddCacheDefinition(internalRunOutputName,
                                         "PLEASE_FILL_OUT-NOTFOUND",
                                         comment.c_str(),
                                         cmState::STRING);
      cmState* state = this->Makefile->GetState();
      const char* existing =
          state->GetCacheEntryValue(internalRunOutputName);
      if (existing)
        {
        state->SetCacheEntryProperty(internalRunOutputName,
                                      "ADVANCED", "1");
        }

      error = true;
      }
    }

  if (error)
    {
    static bool firstTryRun = true;
    cmsys::ofstream file(resultFileName.c_str(),
                                  firstTryRun ? std::ios::out : std::ios::app);
    if ( file )
      {
      if (firstTryRun)
        {
        file << "# This file was generated by CMake because it detected "
                "TRY_RUN() commands\n"
                "# in crosscompiling mode. It will be overwritten by the next "
                "CMake run.\n"
                "# Copy it to a safe location, set the variables to "
                "appropriate values\n"
                "# and use it then to preset the CMake cache (using -C).\n\n";
        }

      std::string comment ="\n";
      comment += this->RunResultVariable;
      comment += "\n   indicates whether the executable would have been able "
                 "to run on its\n"
                 "   target platform. If so, set ";
      comment += this->RunResultVariable;
      comment += " to\n"
                 "   the exit code (in many cases 0 for success), otherwise "
                 "enter \"FAILED_TO_RUN\".\n";
      if (out!=0)
        {
        comment += internalRunOutputName;
        comment += "\n   contains the text the executable "
                   "would have printed on stdout and stderr.\n"
                  "   If the executable would not have been able to run, set ";
        comment += internalRunOutputName;
        comment += " empty.\n"
                   "   Otherwise check if the output is evaluated by the "
                   "calling CMake code. If so,\n"
                   "   check what the source file would have printed when "
                   "called with the given arguments.\n";
        }
      comment += "The ";
      comment += this->CompileResultVariable;
      comment += " variable holds the build result for this TRY_RUN().\n\n"
                 "Source file   : ";
      comment += srcFile + "\n";
      comment += "Executable    : ";
      comment += copyDest + "\n";
      comment += "Run arguments : ";
      comment += runArgs;
      comment += "\n";
      comment += "   Called from: " + this->Makefile->FormatListFileStack();
      cmsys::SystemTools::ReplaceString(comment, "\n", "\n# ");
      file << comment << "\n\n";

      file << "set( " << this->RunResultVariable << " \n     \""
           << this->Makefile->GetDefinition(this->RunResultVariable)
           << "\"\n     CACHE STRING \"Result from TRY_RUN\" FORCE)\n\n";

      if (out!=0)
        {
        file << "set( " << internalRunOutputName << " \n     \""
             << this->Makefile->GetDefinition(internalRunOutputName)
             << "\"\n     CACHE STRING \"Output from TRY_RUN\" FORCE)\n\n";
        }
      file.close();
      }
    firstTryRun = false;

    std::string errorMessage = "TRY_RUN() invoked in cross-compiling mode, "
                               "please set the following cache variables "
                               "appropriately:\n";
    errorMessage += "   " + this->RunResultVariable + " (advanced)\n";
    if (out!=0)
      {
      errorMessage += "   " + internalRunOutputName + " (advanced)\n";
      }
    errorMessage += detailsString;
    cmSystemTools::Error(errorMessage.c_str());
    return;
    }

  if (out!=0)
    {
    (*out) = this->Makefile->GetDefinition(internalRunOutputName);
    }
}
