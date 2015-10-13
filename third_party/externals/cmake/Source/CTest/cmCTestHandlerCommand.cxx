/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestHandlerCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

cmCTestHandlerCommand::cmCTestHandlerCommand()
{
  const size_t INIT_SIZE = 100;
  size_t cc;
  this->Arguments.reserve(INIT_SIZE);
  for ( cc = 0; cc < INIT_SIZE; ++ cc )
    {
    this->Arguments.push_back(0);
    }
  this->Arguments[ct_RETURN_VALUE] = "RETURN_VALUE";
  this->Arguments[ct_SOURCE] = "SOURCE";
  this->Arguments[ct_BUILD] = "BUILD";
  this->Arguments[ct_SUBMIT_INDEX] = "SUBMIT_INDEX";
  this->Last = ct_LAST;
  this->AppendXML = false;
  this->Quiet = false;
}

bool cmCTestHandlerCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  // Allocate space for argument values.
  this->Values.clear();
  this->Values.resize(this->Last, 0);

  // Process input arguments.
  this->ArgumentDoing = ArgumentDoingNone;
  for(unsigned int i=0; i < args.size(); ++i)
    {
    // Check this argument.
    if(!this->CheckArgumentKeyword(args[i]) &&
       !this->CheckArgumentValue(args[i]))
      {
      std::ostringstream e;
      e << "called with unknown argument \"" << args[i] << "\".";
      this->SetError(e.str());
      return false;
      }

    // Quit if an argument is invalid.
    if(this->ArgumentDoing == ArgumentDoingError)
      {
      return false;
      }
    }

  // Set the config type of this ctest to the current value of the
  // CTEST_CONFIGURATION_TYPE script variable if it is defined.
  // The current script value trumps the -C argument on the command
  // line.
  const char* ctestConfigType =
    this->Makefile->GetDefinition("CTEST_CONFIGURATION_TYPE");
  if (ctestConfigType)
    {
    this->CTest->SetConfigType(ctestConfigType);
    }

  if ( this->Values[ct_BUILD] )
    {
    this->CTest->SetCTestConfiguration("BuildDirectory",
      cmSystemTools::CollapseFullPath(
        this->Values[ct_BUILD]).c_str(), this->Quiet);
    }
  else
    {
    const char* bdir =
      this->Makefile->GetSafeDefinition("CTEST_BINARY_DIRECTORY");
    if(bdir)
      {
      this->
        CTest->SetCTestConfiguration("BuildDirectory",
          cmSystemTools::CollapseFullPath(bdir).c_str(), this->Quiet);
      }
    else
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "CTEST_BINARY_DIRECTORY not set" << std::endl;);
      }
    }
  if ( this->Values[ct_SOURCE] )
    {
    cmCTestLog(this->CTest, DEBUG,
      "Set source directory to: " << this->Values[ct_SOURCE] << std::endl);
    this->CTest->SetCTestConfiguration("SourceDirectory",
      cmSystemTools::CollapseFullPath(
        this->Values[ct_SOURCE]).c_str(), this->Quiet);
    }
  else
    {
    this->CTest->SetCTestConfiguration("SourceDirectory",
      cmSystemTools::CollapseFullPath(
        this->Makefile->GetSafeDefinition("CTEST_SOURCE_DIRECTORY")).c_str(),
      this->Quiet);
    }

  cmCTestLog(this->CTest, DEBUG, "Initialize handler" << std::endl;);
  cmCTestGenericHandler* handler = this->InitializeHandler();
  if ( !handler )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot instantiate test handler " << this->GetName()
               << std::endl);
    return false;
    }

  handler->SetAppendXML(this->AppendXML);

  handler->PopulateCustomVectors(this->Makefile);
  if ( this->Values[ct_SUBMIT_INDEX] )
    {
    if(!this->CTest->GetDropSiteCDash() && this->CTest->GetDartVersion() <= 1)
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Dart before version 2.0 does not support collecting submissions."
        << std::endl
        << "Please upgrade the server to Dart 2 or higher, or do not use "
        "SUBMIT_INDEX." << std::endl);
      }
    else
      {
      handler->SetSubmitIndex(atoi(this->Values[ct_SUBMIT_INDEX]));
      }
    }
  std::string current_dir = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(
    this->CTest->GetCTestConfiguration("BuildDirectory"));
  int res = handler->ProcessHandler();
  if ( this->Values[ct_RETURN_VALUE] && *this->Values[ct_RETURN_VALUE])
    {
    std::ostringstream str;
    str << res;
    this->Makefile->AddDefinition(
      this->Values[ct_RETURN_VALUE], str.str().c_str());
    }
  cmSystemTools::ChangeDirectory(current_dir);
  return true;
}

//----------------------------------------------------------------------------
bool cmCTestHandlerCommand::CheckArgumentKeyword(std::string const& arg)
{
  // Look for non-value arguments common to all commands.
  if(arg == "APPEND")
    {
    this->ArgumentDoing = ArgumentDoingNone;
    this->AppendXML = true;
    return true;
    }
  if(arg == "QUIET")
    {
    this->ArgumentDoing = ArgumentDoingNone;
    this->Quiet = true;
    return true;
    }

  // Check for a keyword in our argument/value table.
  for(unsigned int k=0; k < this->Arguments.size(); ++k)
    {
    if(this->Arguments[k] && arg == this->Arguments[k])
      {
      this->ArgumentDoing = ArgumentDoingKeyword;
      this->ArgumentIndex = k;
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmCTestHandlerCommand::CheckArgumentValue(std::string const& arg)
{
  if(this->ArgumentDoing == ArgumentDoingKeyword)
    {
    this->ArgumentDoing = ArgumentDoingNone;
    unsigned int k = this->ArgumentIndex;
    if(this->Values[k])
      {
      std::ostringstream e;
      e << "Called with more than one value for " << this->Arguments[k];
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      this->ArgumentDoing = ArgumentDoingError;
      return true;
      }
    this->Values[k] = arg.c_str();
    cmCTestLog(this->CTest, DEBUG, "Set " << this->Arguments[k]
               << " to " << arg << "\n");
    return true;
    }
  return false;
}
