/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallScriptGenerator_h
#define cmInstallScriptGenerator_h

#include "cmInstallGenerator.h"

/** \class cmInstallScriptGenerator
 * \brief Generate target installation rules.
 */
class cmInstallScriptGenerator: public cmInstallGenerator
{
public:
  cmInstallScriptGenerator(const char* script, bool code,
    const char* component);
  virtual ~cmInstallScriptGenerator();

protected:
  virtual void GenerateScript(std::ostream& os);
  std::string Script;
  bool Code;
};

#endif
