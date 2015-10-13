/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio12Generator_h
#define cmGlobalVisualStudio12Generator_h

#include "cmGlobalVisualStudio11Generator.h"


/** \class cmGlobalVisualStudio12Generator  */
class cmGlobalVisualStudio12Generator:
  public cmGlobalVisualStudio11Generator
{
public:
  cmGlobalVisualStudio12Generator(cmake* cm, const std::string& name,
    const std::string& platformName);
  static cmGlobalGeneratorFactory* NewFactory();

  virtual bool MatchesGeneratorName(const std::string& name) const;

  virtual void WriteSLNHeader(std::ostream& fout);

  //in Visual Studio 2013 they detached the MSBuild tools version
  //from the .Net Framework version and instead made it have it's own
  //version number
  virtual const char* GetToolsVersion() { return "12.0"; }
protected:
  virtual bool InitializeWindowsPhone(cmMakefile* mf);
  virtual bool InitializeWindowsStore(cmMakefile* mf);
  virtual bool SelectWindowsPhoneToolset(std::string& toolset) const;
  virtual bool SelectWindowsStoreToolset(std::string& toolset) const;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsDesktopToolsetInstalled() const;
  bool IsWindowsPhoneToolsetInstalled() const;
  bool IsWindowsStoreToolsetInstalled() const;
  virtual const char* GetIDEVersion() { return "12.0"; }
private:
  class Factory;
};
#endif
