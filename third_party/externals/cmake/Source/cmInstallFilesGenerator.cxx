/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmInstallFilesGenerator.h"

#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"

//----------------------------------------------------------------------------
cmInstallFilesGenerator
::cmInstallFilesGenerator(cmMakefile* mf,
                          std::vector<std::string> const& files,
                          const char* dest, bool programs,
                          const char* file_permissions,
                          std::vector<std::string> const& configurations,
                          const char* component,
                          MessageLevel message,
                          const char* rename,
                          bool optional):
  cmInstallGenerator(dest, configurations, component, message),
  Makefile(mf),
  Files(files), Programs(programs),
  FilePermissions(file_permissions),
  Rename(rename), Optional(optional)
{
  // We need per-config actions if any files have generator expressions.
  for(std::vector<std::string>::const_iterator i = files.begin();
      !this->ActionsPerConfig && i != files.end(); ++i)
    {
    if(cmGeneratorExpression::Find(*i) != std::string::npos)
      {
      this->ActionsPerConfig = true;
      }
    }
}

//----------------------------------------------------------------------------
cmInstallFilesGenerator
::~cmInstallFilesGenerator()
{
}

//----------------------------------------------------------------------------
void cmInstallFilesGenerator::AddFilesInstallRule(
  std::ostream& os, Indent const& indent,
  std::vector<std::string> const& files)
{
  // Write code to install the files.
  const char* no_dir_permissions = 0;
  this->AddInstallRule(os,
                       this->Destination,
                       (this->Programs
                        ? cmInstallType_PROGRAMS
                        : cmInstallType_FILES),
                       files,
                       this->Optional,
                       this->FilePermissions.c_str(), no_dir_permissions,
                       this->Rename.c_str(), 0, indent);
}

//----------------------------------------------------------------------------
void cmInstallFilesGenerator::GenerateScriptActions(std::ostream& os,
                                                    Indent const& indent)
{
  if(this->ActionsPerConfig)
    {
    this->cmInstallGenerator::GenerateScriptActions(os, indent);
    }
  else
    {
    this->AddFilesInstallRule(os, indent, this->Files);
    }
}

//----------------------------------------------------------------------------
void cmInstallFilesGenerator::GenerateScriptForConfig(std::ostream& os,
                                                    const std::string& config,
                                                    Indent const& indent)
{
  std::vector<std::string> files;
  cmGeneratorExpression ge;
  for(std::vector<std::string>::const_iterator i = this->Files.begin();
      i != this->Files.end(); ++i)
    {
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(*i);
    cmSystemTools::ExpandListArgument(cge->Evaluate(this->Makefile, config),
                                      files);
    }
  this->AddFilesInstallRule(os, indent, files);
}
