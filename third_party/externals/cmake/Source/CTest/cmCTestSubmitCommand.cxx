/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestSubmitCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestSubmitHandler.h"

cmCTestGenericHandler* cmCTestSubmitCommand::InitializeHandler()
{
  const char* ctestDropMethod
    = this->Makefile->GetDefinition("CTEST_DROP_METHOD");
  const char* ctestDropSite
    = this->Makefile->GetDefinition("CTEST_DROP_SITE");
  const char* ctestDropLocation
    = this->Makefile->GetDefinition("CTEST_DROP_LOCATION");
  const char* ctestTriggerSite
    = this->Makefile->GetDefinition("CTEST_TRIGGER_SITE");
  bool ctestDropSiteCDash
    = this->Makefile->IsOn("CTEST_DROP_SITE_CDASH");
  const char* ctestProjectName
    = this->Makefile->GetDefinition("CTEST_PROJECT_NAME");
  if ( !ctestDropMethod )
    {
    ctestDropMethod = "http";
    }

  if ( !ctestDropSite )
    {
    // error: CDash requires CTEST_DROP_SITE definition
    // in CTestConfig.cmake
    }
  if ( !ctestDropLocation )
    {
    // error: CDash requires CTEST_DROP_LOCATION definition
    // in CTestConfig.cmake
    }
  this->CTest->SetCTestConfiguration("ProjectName", ctestProjectName,
    this->Quiet);
  this->CTest->SetCTestConfiguration("DropMethod", ctestDropMethod,
    this->Quiet);
  this->CTest->SetCTestConfiguration("DropSite", ctestDropSite, this->Quiet);
  this->CTest->SetCTestConfiguration("DropLocation", ctestDropLocation,
    this->Quiet);

  this->CTest->SetCTestConfiguration("IsCDash",
    ctestDropSiteCDash ? "TRUE" : "FALSE", this->Quiet);

  // Only propagate TriggerSite for non-CDash projects:
  //
  if ( !ctestDropSiteCDash )
    {
    this->CTest->SetCTestConfiguration("TriggerSite",  ctestTriggerSite,
      this->Quiet);
    }

  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "CurlOptions", "CTEST_CURL_OPTIONS", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "DropSiteUser", "CTEST_DROP_SITE_USER", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "DropSitePassword", "CTEST_DROP_SITE_PASSWORD", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "ScpCommand", "CTEST_SCP_COMMAND", this->Quiet);

  const char* notesFilesVariable
    = this->Makefile->GetDefinition("CTEST_NOTES_FILES");
  if (notesFilesVariable)
    {
    std::vector<std::string> notesFiles;
    cmCTest::VectorOfStrings newNotesFiles;
    cmSystemTools::ExpandListArgument(notesFilesVariable,notesFiles);
    newNotesFiles.insert(newNotesFiles.end(),
                         notesFiles.begin(), notesFiles.end());
    this->CTest->GenerateNotesFile(newNotesFiles);
    }

  const char* extraFilesVariable
    = this->Makefile->GetDefinition("CTEST_EXTRA_SUBMIT_FILES");
  if (extraFilesVariable)
    {
    std::vector<std::string> extraFiles;
    cmCTest::VectorOfStrings newExtraFiles;
    cmSystemTools::ExpandListArgument(extraFilesVariable,extraFiles);
    newExtraFiles.insert(newExtraFiles.end(),
                         extraFiles.begin(), extraFiles.end());
    if ( !this->CTest->SubmitExtraFiles(newExtraFiles))
      {
      this->SetError("problem submitting extra files.");
      return 0;
      }
    }

  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("submit");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate submit handler");
    return 0;
    }

  // If no FILES or PARTS given, *all* PARTS are submitted by default.
  //
  // If FILES are given, but not PARTS, only the FILES are submitted
  // and *no* PARTS are submitted.
  //  (This is why we select the empty "noParts" set in the
  //   FilesMentioned block below...)
  //
  // If PARTS are given, only the selected PARTS are submitted.
  //
  // If both PARTS and FILES are given, only the selected PARTS *and*
  // all the given FILES are submitted.

  // If given explicit FILES to submit, pass them to the handler.
  //
  if(this->FilesMentioned)
    {
    // Intentionally select *no* PARTS. (Pass an empty set.) If PARTS
    // were also explicitly mentioned, they will be selected below...
    // But FILES with no PARTS mentioned should just submit the FILES
    // without any of the default parts.
    //
    std::set<cmCTest::Part> noParts;
    static_cast<cmCTestSubmitHandler*>(handler)->SelectParts(noParts);

    static_cast<cmCTestSubmitHandler*>(handler)->SelectFiles(this->Files);
    }

  // If a PARTS option was given, select only the named parts for submission.
  //
  if(this->PartsMentioned)
    {
    static_cast<cmCTestSubmitHandler*>(handler)->SelectParts(this->Parts);
    }

  static_cast<cmCTestSubmitHandler*>(handler)->SetOption("RetryDelay",
    this->RetryDelay.c_str());
  static_cast<cmCTestSubmitHandler*>(handler)->SetOption("RetryCount",
    this->RetryCount.c_str());
  static_cast<cmCTestSubmitHandler*>(handler)->SetOption("InternalTest",
    this->InternalTest ? "ON" : "OFF");

  handler->SetQuiet(this->Quiet);

  if (this->CDashUpload)
    {
    static_cast<cmCTestSubmitHandler*>(handler)->
      SetOption("CDashUploadFile", this->CDashUploadFile.c_str());
    static_cast<cmCTestSubmitHandler*>(handler)->
      SetOption("CDashUploadType", this->CDashUploadType.c_str());
    }
  return handler;
}

//----------------------------------------------------------------------------
bool cmCTestSubmitCommand::InitialPass(std::vector<std::string> const& args,
                                       cmExecutionStatus& status)
{
  this->CDashUpload = !args.empty() && args[0] == "CDASH_UPLOAD";
  return this->cmCTestHandlerCommand::InitialPass(args, status);
}

//----------------------------------------------------------------------------
bool cmCTestSubmitCommand::CheckArgumentKeyword(std::string const& arg)
{
  if (this->CDashUpload)
    {
    if(arg == "CDASH_UPLOAD")
      {
      this->ArgumentDoing = ArgumentDoingCDashUpload;
      return true;
      }

    if(arg == "CDASH_UPLOAD_TYPE")
      {
      this->ArgumentDoing = ArgumentDoingCDashUploadType;
      return true;
      }
    }
  else
    {
    // Look for arguments specific to this command.
    if(arg == "PARTS")
      {
      this->ArgumentDoing = ArgumentDoingParts;
      this->PartsMentioned = true;
      return true;
      }

    if(arg == "FILES")
      {
      this->ArgumentDoing = ArgumentDoingFiles;
      this->FilesMentioned = true;
      return true;
      }

    if(arg == "RETRY_COUNT")
      {
      this->ArgumentDoing = ArgumentDoingRetryCount;
      return true;
      }

    if(arg == "RETRY_DELAY")
      {
      this->ArgumentDoing = ArgumentDoingRetryDelay;
      return true;
      }

    if(arg == "INTERNAL_TEST_CHECKSUM")
      {
      this->InternalTest = true;
      return true;
      }
    }

  // Look for other arguments.
  return this->Superclass::CheckArgumentKeyword(arg);
}


//----------------------------------------------------------------------------
bool cmCTestSubmitCommand::CheckArgumentValue(std::string const& arg)
{
  // Handle states specific to this command.
  if(this->ArgumentDoing == ArgumentDoingParts)
    {
    cmCTest::Part p = this->CTest->GetPartFromName(arg.c_str());
    if(p != cmCTest::PartCount)
      {
      this->Parts.insert(p);
      }
    else
      {
      std::ostringstream e;
      e << "Part name \"" << arg << "\" is invalid.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      this->ArgumentDoing = ArgumentDoingError;
      }
    return true;
    }

  if(this->ArgumentDoing == ArgumentDoingFiles)
    {
    std::string filename(arg);
    if(cmSystemTools::FileExists(filename.c_str()))
      {
      this->Files.insert(filename);
      }
    else
      {
      std::ostringstream e;
      e << "File \"" << filename << "\" does not exist. Cannot submit "
          << "a non-existent file.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      this->ArgumentDoing = ArgumentDoingError;
      }
    return true;
    }

  if(this->ArgumentDoing == ArgumentDoingRetryCount)
    {
    this->RetryCount = arg;
    return true;
    }

  if(this->ArgumentDoing == ArgumentDoingRetryDelay)
    {
    this->RetryDelay = arg;
    return true;
    }

  if(this->ArgumentDoing == ArgumentDoingCDashUpload)
    {
    this->ArgumentDoing = ArgumentDoingNone;
    this->CDashUploadFile = arg;
    return true;
    }

  if(this->ArgumentDoing == ArgumentDoingCDashUploadType)
    {
    this->ArgumentDoing = ArgumentDoingNone;
    this->CDashUploadType = arg;
    return true;
    }

  // Look for other arguments.
  return this->Superclass::CheckArgumentValue(arg);
}
