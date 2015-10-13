/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio11Generator_h
#define cmGlobalVisualStudio11Generator_h

#include "cmGlobalVisualStudio10Generator.h"


/** \class cmGlobalVisualStudio11Generator  */
class cmGlobalVisualStudio11Generator:
  public cmGlobalVisualStudio10Generator
{
public:
  cmGlobalVisualStudio11Generator(cmake* cm, const std::string& name,
    const std::string& platformName);
  static cmGlobalGeneratorFactory* NewFactory();

  virtual bool MatchesGeneratorName(const std::string& name) const;

  virtual void WriteSLNHeader(std::ostream& fout);

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

  virtual const char* GetIDEVersion() { return "11.0"; }
  bool UseFolderProperty();
  static std::set<std::string> GetInstalledWindowsCESDKs();

  /** Return true if the configuration needs to be deployed */
  virtual bool NeedsDeploy(cmTarget::TargetType type) const;
private:
  class Factory;
  friend class Factory;
};
#endif
