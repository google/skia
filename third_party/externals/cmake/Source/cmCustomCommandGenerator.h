/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2010 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCustomCommandGenerator_h
#define cmCustomCommandGenerator_h

#include "cmStandardIncludes.h"

class cmCustomCommand;
class cmMakefile;
class cmLocalGenerator;
class cmGeneratorExpression;

class cmCustomCommandGenerator
{
  cmCustomCommand const& CC;
  std::string Config;
  cmMakefile* Makefile;
  cmLocalGenerator* LG;
  bool OldStyle;
  bool MakeVars;
  cmGeneratorExpression* GE;
  mutable bool DependsDone;
  mutable std::vector<std::string> Depends;
public:
  cmCustomCommandGenerator(cmCustomCommand const& cc,
                           const std::string& config,
                           cmMakefile* mf);
  ~cmCustomCommandGenerator();
  cmCustomCommand const& GetCC() const { return this->CC; }
  unsigned int GetNumberOfCommands() const;
  std::string GetCommand(unsigned int c) const;
  void AppendArguments(unsigned int c, std::string& cmd) const;
  const char* GetComment() const;
  std::string GetWorkingDirectory() const;
  std::vector<std::string> const& GetOutputs() const;
  std::vector<std::string> const& GetByproducts() const;
  std::vector<std::string> const& GetDepends() const;
};

#endif
