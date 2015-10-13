/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmAuxSourceDirectoryCommand.h"
#include "cmSourceFile.h"

#include <cmsys/Directory.hxx>

// cmAuxSourceDirectoryCommand
bool cmAuxSourceDirectoryCommand::InitialPass
(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 || args.size() > 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string sourceListValue;
  std::string templateDirectory = args[0];
  std::string tdir;
  if(!cmSystemTools::FileIsFullPath(templateDirectory.c_str()))
    {
    tdir = this->Makefile->GetCurrentSourceDirectory();
    tdir += "/";
    tdir += templateDirectory;
    }
  else
    {
    tdir = templateDirectory;
    }

  // was the list already populated
  const char *def = this->Makefile->GetDefinition(args[1]);
  if (def)
    {
    sourceListValue = def;
    }

  // Load all the files in the directory
  cmsys::Directory dir;
  if(dir.Load(tdir.c_str()))
    {
    size_t numfiles = dir.GetNumberOfFiles();
    for(size_t i =0; i < numfiles; ++i)
      {
      std::string file = dir.GetFile(static_cast<unsigned long>(i));
      // Split the filename into base and extension
      std::string::size_type dotpos = file.rfind(".");
      if( dotpos != std::string::npos )
        {
        std::string ext = file.substr(dotpos+1);
        std::string base = file.substr(0, dotpos);
        // Process only source files
        if(!base.empty()
            && std::find( this->Makefile->GetSourceExtensions().begin(),
                          this->Makefile->GetSourceExtensions().end(), ext )
                 != this->Makefile->GetSourceExtensions().end() )
          {
          std::string fullname = templateDirectory;
          fullname += "/";
          fullname += file;
          // add the file as a class file so
          // depends can be done
          cmSourceFile* sf =
            this->Makefile->GetOrCreateSource(fullname);
          sf->SetProperty("ABSTRACT","0");
          if(!sourceListValue.empty())
            {
            sourceListValue += ";";
            }
          sourceListValue += fullname;
          }
        }
      }
    }
  this->Makefile->AddDefinition(args[1], sourceListValue.c_str());
  return true;
}

