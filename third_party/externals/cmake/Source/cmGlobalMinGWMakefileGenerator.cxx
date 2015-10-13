/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalMinGWMakefileGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"

cmGlobalMinGWMakefileGenerator::cmGlobalMinGWMakefileGenerator(cmake* cm)
  : cmGlobalUnixMakefileGenerator3(cm)
{
  this->FindMakeProgramFile = "CMakeMinGWFindMake.cmake";
  this->ForceUnixPaths = true;
  this->ToolSupportsColor = true;
  this->UseLinkScript = true;
  cm->GetState()->SetWindowsShell(true);
  cm->GetState()->SetMinGWMake(true);
}

void cmGlobalMinGWMakefileGenerator
::EnableLanguage(std::vector<std::string>const& l,
                 cmMakefile *mf,
                 bool optional)
{
  this->FindMakeProgram(mf);
  std::string makeProgram = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::vector<std::string> locations;
  locations.push_back(cmSystemTools::GetProgramPath(makeProgram));
  locations.push_back("/mingw/bin");
  locations.push_back("c:/mingw/bin");
  std::string tgcc = cmSystemTools::FindProgram("gcc", locations);
  std::string gcc = "gcc.exe";
  if(!tgcc.empty())
    {
    gcc = tgcc;
    }
  std::string tgxx = cmSystemTools::FindProgram("g++", locations);
  std::string gxx = "g++.exe";
  if(!tgxx.empty())
    {
    gxx = tgxx;
    }
  std::string trc = cmSystemTools::FindProgram("windres", locations);
  std::string rc = "windres.exe";
  if(!trc.empty())
    {
    rc = trc;
    }
  mf->AddDefinition("CMAKE_GENERATOR_CC", gcc.c_str());
  mf->AddDefinition("CMAKE_GENERATOR_CXX", gxx.c_str());
  mf->AddDefinition("CMAKE_GENERATOR_RC", rc.c_str());
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf, optional);
}

//----------------------------------------------------------------------------
void cmGlobalMinGWMakefileGenerator
::GetDocumentation(cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalMinGWMakefileGenerator::GetActualName();
  entry.Brief = "Generates a make file for use with mingw32-make.";
}
