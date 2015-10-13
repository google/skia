/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio14Generator_h
#define cmGlobalVisualStudio14Generator_h

#include "cmGlobalVisualStudio12Generator.h"


/** \class cmGlobalVisualStudio14Generator  */
class cmGlobalVisualStudio14Generator:
  public cmGlobalVisualStudio12Generator
{
public:
  cmGlobalVisualStudio14Generator(cmake* cm, const std::string& name,
    const std::string& platformName);
  static cmGlobalGeneratorFactory* NewFactory();

  virtual bool MatchesGeneratorName(const std::string& name) const;

  virtual void WriteSLNHeader(std::ostream& fout);

  virtual const char* GetToolsVersion() { return "14.0"; }
protected:
  virtual const char* GetIDEVersion() { return "14.0"; }
private:
  class Factory;
};
#endif
