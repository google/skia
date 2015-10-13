/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Nicolas Despres <nicolas.despres@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmOSXBundleGenerator.h"
#include "cmMakefile.h"
#include "cmTarget.h"
#include "cmLocalGenerator.h"

#include <cassert>

//----------------------------------------------------------------------------
cmOSXBundleGenerator::
cmOSXBundleGenerator(cmGeneratorTarget* target,
                     const std::string& configName)
 : GT(target)
 , Makefile(target->Target->GetMakefile())
 , LocalGenerator(Makefile->GetLocalGenerator())
 , ConfigName(configName)
 , MacContentFolders(0)
{
  if (this->MustSkip())
    return;

}

//----------------------------------------------------------------------------
bool cmOSXBundleGenerator::MustSkip()
{
  return !this->GT->Target->HaveWellDefinedOutputFiles();
}

//----------------------------------------------------------------------------
void cmOSXBundleGenerator::CreateAppBundle(const std::string& targetName,
                                           std::string& outpath)
{
  if (this->MustSkip())
    return;

  // Compute bundle directory names.
  std::string out = outpath;
  out += "/";
  out += this->GT->Target->GetAppBundleDirectory(this->ConfigName, false);
  cmSystemTools::MakeDirectory(out.c_str());
  this->Makefile->AddCMakeOutputFile(out);

  std::string newoutpath = out;

  // Configure the Info.plist file.  Note that it needs the executable name
  // to be set.
  std::string plist = outpath;
  plist += "/";
  plist += this->GT->Target->GetAppBundleDirectory(this->ConfigName, true);
  plist += "/Info.plist";
  this->LocalGenerator->GenerateAppleInfoPList(this->GT->Target,
                                               targetName,
                                               plist.c_str());
  this->Makefile->AddCMakeOutputFile(plist);
  outpath = newoutpath;
}

//----------------------------------------------------------------------------
void cmOSXBundleGenerator::CreateFramework(
  const std::string& targetName, const std::string& outpath)
{
  if (this->MustSkip())
    return;

  assert(this->MacContentFolders);

  // Compute the location of the top-level foo.framework directory.
  std::string contentdir = outpath + "/" +
    this->GT->Target->GetFrameworkDirectory(this->ConfigName, true);
  contentdir += "/";

  std::string newoutpath = outpath + "/" +
    this->GT->Target->GetFrameworkDirectory(this->ConfigName, false);

  std::string frameworkVersion = this->GT->Target->GetFrameworkVersion();

  // Configure the Info.plist file into the Resources directory.
  this->MacContentFolders->insert("Resources");
  std::string plist = newoutpath;
  plist += "/Resources/Info.plist";
  std::string name = cmSystemTools::GetFilenameName(targetName);
  this->LocalGenerator->GenerateFrameworkInfoPList(this->GT->Target,
                                                   name,
                                                   plist.c_str());

  // TODO: Use the cmMakefileTargetGenerator::ExtraFiles vector to
  // drive rules to create these files at build time.
  std::string oldName;
  std::string newName;


  // Make foo.framework/Versions
  std::string versions = contentdir;
  versions += "Versions";
  cmSystemTools::MakeDirectory(versions.c_str());

  // Make foo.framework/Versions/version
  cmSystemTools::MakeDirectory(newoutpath.c_str());

  // Current -> version
  oldName = frameworkVersion;
  newName = versions;
  newName += "/Current";
  cmSystemTools::RemoveFile(newName);
  cmSystemTools::CreateSymlink(oldName, newName);
  this->Makefile->AddCMakeOutputFile(newName);

  // foo -> Versions/Current/foo
  oldName = "Versions/Current/";
  oldName += name;
  newName = contentdir;
  newName += name;
  cmSystemTools::RemoveFile(newName);
  cmSystemTools::CreateSymlink(oldName, newName);
  this->Makefile->AddCMakeOutputFile(newName);

  // Resources -> Versions/Current/Resources
  if(this->MacContentFolders->find("Resources") !=
     this->MacContentFolders->end())
    {
    oldName = "Versions/Current/Resources";
    newName = contentdir;
    newName += "Resources";
    cmSystemTools::RemoveFile(newName);
    cmSystemTools::CreateSymlink(oldName, newName);
    this->Makefile->AddCMakeOutputFile(newName);
    }

  // Headers -> Versions/Current/Headers
  if(this->MacContentFolders->find("Headers") !=
     this->MacContentFolders->end())
    {
    oldName = "Versions/Current/Headers";
    newName = contentdir;
    newName += "Headers";
    cmSystemTools::RemoveFile(newName);
    cmSystemTools::CreateSymlink(oldName, newName);
    this->Makefile->AddCMakeOutputFile(newName);
    }

  // PrivateHeaders -> Versions/Current/PrivateHeaders
  if(this->MacContentFolders->find("PrivateHeaders") !=
     this->MacContentFolders->end())
    {
    oldName = "Versions/Current/PrivateHeaders";
    newName = contentdir;
    newName += "PrivateHeaders";
    cmSystemTools::RemoveFile(newName);
    cmSystemTools::CreateSymlink(oldName, newName);
    this->Makefile->AddCMakeOutputFile(newName);
    }
}

//----------------------------------------------------------------------------
void cmOSXBundleGenerator::CreateCFBundle(const std::string& targetName,
                                          const std::string& root)
{
  if (this->MustSkip())
    return;

  // Compute bundle directory names.
  std::string out = root;
  out += "/";
  out += this->GT->Target->GetCFBundleDirectory(this->ConfigName, false);
  cmSystemTools::MakeDirectory(out.c_str());
  this->Makefile->AddCMakeOutputFile(out);

  // Configure the Info.plist file.  Note that it needs the executable name
  // to be set.
  std::string plist = root + "/" +
    this->GT->Target->GetCFBundleDirectory(this->ConfigName, true);
  plist += "/Info.plist";
  std::string name = cmSystemTools::GetFilenameName(targetName);
  this->LocalGenerator->GenerateAppleInfoPList(this->GT->Target,
                                               name,
                                               plist.c_str());
  this->Makefile->AddCMakeOutputFile(plist);
}

//----------------------------------------------------------------------------
void
cmOSXBundleGenerator::
GenerateMacOSXContentStatements(
                              std::vector<cmSourceFile const*> const& sources,
                              MacOSXContentGeneratorType* generator)
{
  if (this->MustSkip())
    return;

  for(std::vector<cmSourceFile const*>::const_iterator
        si = sources.begin(); si != sources.end(); ++si)
    {
    cmGeneratorTarget::SourceFileFlags tsFlags =
      this->GT->GetTargetSourceFileFlags(*si);
    if(tsFlags.Type != cmGeneratorTarget::SourceFileTypeNormal)
      {
      (*generator)(**si, tsFlags.MacFolder);
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmOSXBundleGenerator::InitMacOSXContentDirectory(const char* pkgloc)
{
  // Construct the full path to the content subdirectory.

  std::string macdir =
    this->GT->Target->GetMacContentDirectory(this->ConfigName,
                                         /*implib*/ false);
  macdir += "/";
  macdir += pkgloc;
  cmSystemTools::MakeDirectory(macdir.c_str());

  // Record use of this content location.  Only the first level
  // directory is needed.
  {
  std::string loc = pkgloc;
  loc = loc.substr(0, loc.find('/'));
  this->MacContentFolders->insert(loc);
  }

  return macdir;
}
