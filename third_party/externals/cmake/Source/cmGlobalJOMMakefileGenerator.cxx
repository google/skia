/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalJOMMakefileGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"

cmGlobalJOMMakefileGenerator::cmGlobalJOMMakefileGenerator(cmake* cm)
  : cmGlobalUnixMakefileGenerator3(cm)
{
  this->FindMakeProgramFile = "CMakeJOMFindMake.cmake";
  this->ForceUnixPaths = false;
  this->ToolSupportsColor = true;
  this->UseLinkScript = false;
  cm->GetState()->SetWindowsShell(true);
  cm->GetState()->SetNMake(true);
  this->DefineWindowsNULL = true;
  this->PassMakeflags = true;
  this->UnixCD = false;
  this->MakeSilentFlag = "/nologo";
}

void cmGlobalJOMMakefileGenerator
::EnableLanguage(std::vector<std::string>const& l,
                 cmMakefile *mf,
                 bool optional)
{
  // pick a default
  mf->AddDefinition("CMAKE_GENERATOR_CC", "cl");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "cl");
  if(!(cmSystemTools::GetEnv("INCLUDE") &&
       cmSystemTools::GetEnv("LIB"))
    )
    {
    std::string message = "To use the JOM generator, cmake must be run "
      "from a shell that can use the compiler cl from the command line. "
      "This environment does not contain INCLUDE, LIB, or LIBPATH, and "
      "these must be set for the cl compiler to work. ";
    mf->IssueMessage(cmake::WARNING,
                     message);
    }

  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf, optional);
}

//----------------------------------------------------------------------------
void cmGlobalJOMMakefileGenerator
::GetDocumentation(cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalJOMMakefileGenerator::GetActualName();
  entry.Brief = "Generates JOM makefiles.";
}
