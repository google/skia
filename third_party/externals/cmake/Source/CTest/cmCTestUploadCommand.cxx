/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestUploadCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestUploadHandler.h"

cmCTestGenericHandler* cmCTestUploadCommand::InitializeHandler()
{
  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("upload");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate upload handler");
    return 0;
    }
  static_cast<cmCTestUploadHandler*>(handler)->SetFiles(this->Files);

  handler->SetQuiet(this->Quiet);
  return handler;
}


//----------------------------------------------------------------------------
bool cmCTestUploadCommand::CheckArgumentKeyword(std::string const& arg)
{
  if(arg == "FILES")
    {
    this->ArgumentDoing = ArgumentDoingFiles;
    return true;
    }
  if(arg == "QUIET")
    {
    this->ArgumentDoing = ArgumentDoingNone;
    this->Quiet = true;
    return true;
    }
  return false;
}


//----------------------------------------------------------------------------
bool cmCTestUploadCommand::CheckArgumentValue(std::string const& arg)
{
  if(this->ArgumentDoing == ArgumentDoingFiles)
    {
    std::string filename(arg);
    if(cmSystemTools::FileExists(filename.c_str()))
      {
      this->Files.insert(filename);
      return true;
      }
    else
      {
      std::ostringstream e;
      e << "File \"" << filename << "\" does not exist. Cannot submit "
          << "a non-existent file.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      this->ArgumentDoing = ArgumentDoingError;
      return false;
      }
    }

  // Look for other arguments.
  return this->Superclass::CheckArgumentValue(arg);
}
