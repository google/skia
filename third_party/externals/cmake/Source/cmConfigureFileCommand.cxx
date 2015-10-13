/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmConfigureFileCommand.h"

#include <cmsys/RegularExpression.hxx>

// cmConfigureFileCommand
bool cmConfigureFileCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments, expected 2");
    return false;
    }

  const char* inFile = args[0].c_str();
  if(!cmSystemTools::FileIsFullPath(inFile))
    {
    this->InputFile = this->Makefile->GetCurrentSourceDirectory();
    this->InputFile += "/";
    }
  this->InputFile += inFile;

  // If the input location is a directory, error out.
  if(cmSystemTools::FileIsDirectory(this->InputFile))
    {
    std::ostringstream e;
    e << "input location\n"
      << "  " << this->InputFile << "\n"
      << "is a directory but a file was expected.";
    this->SetError(e.str());
    return false;
    }

  const char* outFile = args[1].c_str();
  if(!cmSystemTools::FileIsFullPath(outFile))
    {
    this->OutputFile = this->Makefile->GetCurrentBinaryDirectory();
    this->OutputFile += "/";
    }
  this->OutputFile += outFile;

  // If the output location is already a directory put the file in it.
  if(cmSystemTools::FileIsDirectory(this->OutputFile))
    {
    this->OutputFile += "/";
    this->OutputFile += cmSystemTools::GetFilenameName(inFile);
    }

  if ( !this->Makefile->CanIWriteThisFile(this->OutputFile.c_str()) )
    {
    std::string e = "attempted to configure a file: " + this->OutputFile
      + " into a source directory.";
    this->SetError(e);
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }
  std::string errorMessage;
  if (!this->NewLineStyle.ReadFromArguments(args, errorMessage))
    {
    this->SetError(errorMessage);
    return false;
    }
  this->CopyOnly = false;
  this->EscapeQuotes = false;

  std::string unknown_args;
  this->AtOnly = false;
  for(unsigned int i=2;i < args.size();++i)
    {
    if(args[i] == "COPYONLY")
      {
      this->CopyOnly = true;
      if (this->NewLineStyle.IsValid())
        {
        this->SetError("COPYONLY could not be used in combination "
                       "with NEWLINE_STYLE");
        return false;
        }
      }
    else if(args[i] == "ESCAPE_QUOTES")
      {
      this->EscapeQuotes = true;
      }
    else if(args[i] == "@ONLY")
      {
      this->AtOnly = true;
      }
    else if(args[i] == "IMMEDIATE")
      {
      /* Ignore legacy option.  */
      }
    else if(args[i] == "NEWLINE_STYLE" ||
            args[i] == "LF" || args[i] == "UNIX" ||
            args[i] == "CRLF" || args[i] == "WIN32" ||
            args[i] == "DOS")
      {
      /* Options handled by NewLineStyle member above.  */
      }
    else
      {
      unknown_args += " ";
      unknown_args += args[i];
      unknown_args += "\n";
      }
    }
  if (!unknown_args.empty())
    {
    std::string msg = "configure_file called with unknown argument(s):\n";
    msg += unknown_args;
    this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, msg);
    }

  if ( !this->ConfigureFile() )
    {
    this->SetError("Problem configuring file");
    return false;
    }

  return true;
}

int cmConfigureFileCommand::ConfigureFile()
{
  return this->Makefile->ConfigureFile(
    this->InputFile.c_str(),
    this->OutputFile.c_str(),
    this->CopyOnly,
    this->AtOnly,
    this->EscapeQuotes,
    this->NewLineStyle);
}


