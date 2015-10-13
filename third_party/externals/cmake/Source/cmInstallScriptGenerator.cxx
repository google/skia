/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmInstallScriptGenerator.h"

//----------------------------------------------------------------------------
cmInstallScriptGenerator
::cmInstallScriptGenerator(const char* script, bool code,
                           const char* component) :
  cmInstallGenerator(0, std::vector<std::string>(), component, MessageDefault),
  Script(script), Code(code)
{
}

//----------------------------------------------------------------------------
cmInstallScriptGenerator
::~cmInstallScriptGenerator()
{
}

//----------------------------------------------------------------------------
void cmInstallScriptGenerator::GenerateScript(std::ostream& os)
{
  Indent indent;
  std::string component_test =
    this->CreateComponentTest(this->Component.c_str());
  os << indent << "if(" << component_test << ")\n";

  if(this->Code)
    {
    os << indent.Next() << this->Script << "\n";
    }
  else
    {
    os << indent.Next() << "include(\"" << this->Script << "\")\n";
    }

  os << indent << "endif()\n\n";
}
