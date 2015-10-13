/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTestGenerator_h
#define cmTestGenerator_h

#include "cmScriptGenerator.h"

class cmTest;

/** \class cmTestGenerator
 * \brief Support class for generating install scripts.
 *
 */
class cmTestGenerator: public cmScriptGenerator
{
public:
  cmTestGenerator(cmTest* test,
                  std::vector<std::string> const&
                  configurations = std::vector<std::string>());
  virtual ~cmTestGenerator();

protected:
  virtual void GenerateScriptConfigs(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptActions(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptForConfig(std::ostream& os,
                                       const std::string& config,
                                       Indent const& indent);
  virtual void GenerateScriptNoConfig(std::ostream& os, Indent const& indent);
  virtual bool NeedsScriptNoConfig() const;
  void GenerateOldStyle(std::ostream& os, Indent const& indent);

  cmTest* Test;
  bool TestGenerated;
};

#endif
