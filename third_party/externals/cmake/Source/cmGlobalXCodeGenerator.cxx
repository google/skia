/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalXCodeGenerator.h"
#include "cmLocalXCodeGenerator.h"
#include "cmMakefile.h"
#include "cmXCodeObject.h"
#include "cmXCode21Object.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"
#include "cmComputeLinkInformation.h"
#include "cmSourceFile.h"
#include "cmCustomCommandGenerator.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmAlgorithms.h"

#include <cmsys/auto_ptr.hxx>

//----------------------------------------------------------------------------
#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmXMLParser.h"

// parse the xml file storing the installed version of Xcode on
// the machine
class cmXcodeVersionParser : public cmXMLParser
{
public:
  cmXcodeVersionParser(): Version("1.5") {}
  void StartElement(const std::string&, const char**)
    {
      this->Data = "";
    }
  void EndElement(const std::string& name)
    {
      if(name == "key")
        {
        this->Key = this->Data;
        }
      else if(name == "string")
        {
        if(this->Key == "CFBundleShortVersionString")
          {
          this->Version = this->Data;
          }
        }
    }
  void CharacterDataHandler(const char* data, int length)
    {
      this->Data.append(data, length);
    }
  std::string Version;
  std::string Key;
  std::string Data;
};
#endif

// Builds either an object list or a space-separated string from the
// given inputs.
class cmGlobalXCodeGenerator::BuildObjectListOrString
{
  cmGlobalXCodeGenerator *Generator;
  cmXCodeObject *Group;
  bool Empty;
  std::string String;

public:
  BuildObjectListOrString(cmGlobalXCodeGenerator *gen, bool buildObjectList)
    : Generator(gen), Group(0), Empty(true)
    {
    if (buildObjectList)
      {
      this->Group = this->Generator->CreateObject(cmXCodeObject::OBJECT_LIST);
      }
    }

  bool IsEmpty() const { return this->Empty; }

  void Add(const char *newString)
    {
    this->Empty = false;

    if (this->Group)
      {
      this->Group->AddObject(this->Generator->CreateString(newString));
      }
    else
      {
      this->String += newString;
      this->String += ' ';
      }
    }

  const std::string &GetString() const { return this->String; }

  cmXCodeObject *CreateList()
    {
    if (this->Group)
      {
      return this->Group;
      }
    else
      {
      return this->Generator->CreateString(this->String.c_str());
      }
    }
};

class cmGlobalXCodeGenerator::Factory : public cmGlobalGeneratorFactory
{
public:
  virtual cmGlobalGenerator*
  CreateGlobalGenerator(const std::string& name, cmake* cm) const;

  virtual void GetDocumentation(cmDocumentationEntry& entry) const {
    cmGlobalXCodeGenerator::GetDocumentation(entry); }

  virtual void GetGenerators(std::vector<std::string>& names) const {
    names.push_back(cmGlobalXCodeGenerator::GetActualName()); }
};

//----------------------------------------------------------------------------
cmGlobalXCodeGenerator::cmGlobalXCodeGenerator(cmake* cm,
                                               std::string const& version)
  : cmGlobalGenerator(cm)
{
  this->VersionString = version;

  // Compute an integer form of the version number.
  unsigned int v[2] = {0,0};
  sscanf(this->VersionString.c_str(), "%u.%u", &v[0], &v[1]);
  this->XcodeVersion = 10*v[0] + v[1];

  this->RootObject = 0;
  this->MainGroupChildren = 0;
  this->SourcesGroupChildren = 0;
  this->ResourcesGroupChildren = 0;
  this->CurrentMakefile = 0;
  this->CurrentLocalGenerator = 0;
  this->XcodeBuildCommandInitialized = false;
}

//----------------------------------------------------------------------------
cmGlobalGeneratorFactory* cmGlobalXCodeGenerator::NewFactory()
{
  return new Factory;
}

//----------------------------------------------------------------------------
cmGlobalGenerator* cmGlobalXCodeGenerator::Factory
::CreateGlobalGenerator(const std::string& name, cmake* cm) const
{
  if (name != GetActualName())
    return 0;
#if defined(CMAKE_BUILD_WITH_CMAKE)
  cmXcodeVersionParser parser;
  std::string versionFile;
  {
  std::string out;
  std::string::size_type pos;
  if(cmSystemTools::RunSingleCommand("xcode-select --print-path", &out, 0,
                                     0, 0, cmSystemTools::OUTPUT_NONE) &&
     (pos = out.find(".app/"), pos != out.npos))
    {
    versionFile = out.substr(0, pos+5)+"Contents/version.plist";
    }
  }
  if(!versionFile.empty() && cmSystemTools::FileExists(versionFile.c_str()))
    {
    parser.ParseFile(versionFile.c_str());
    }
  else if (cmSystemTools::FileExists(
             "/Applications/Xcode.app/Contents/version.plist"))
    {
    parser.ParseFile
      ("/Applications/Xcode.app/Contents/version.plist");
    }
  else
    {
    parser.ParseFile
      ("/Developer/Applications/Xcode.app/Contents/version.plist");
    }
  cmsys::auto_ptr<cmGlobalXCodeGenerator>
    gg(new cmGlobalXCodeGenerator(cm, parser.Version));
  if (gg->XcodeVersion == 20)
    {
    cmSystemTools::Message("Xcode 2.0 not really supported by cmake, "
                           "using Xcode 15 generator\n");
    gg->XcodeVersion = 15;
    }
  return gg.release();
#else
  std::cerr << "CMake should be built with cmake to use Xcode, "
    "default to Xcode 1.5\n";
  return new cmGlobalXCodeGenerator(cm);
#endif
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::FindMakeProgram(cmMakefile* mf)
{
  // The Xcode generator knows how to lookup its build tool
  // directly instead of needing a helper module to do it, so we
  // do not actually need to put CMAKE_MAKE_PROGRAM into the cache.
  if(cmSystemTools::IsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM")))
    {
    mf->AddDefinition("CMAKE_MAKE_PROGRAM",
                      this->GetXcodeBuildCommand().c_str());
    }
}

//----------------------------------------------------------------------------
std::string const& cmGlobalXCodeGenerator::GetXcodeBuildCommand()
{
  if(!this->XcodeBuildCommandInitialized)
    {
    this->XcodeBuildCommandInitialized = true;
    this->XcodeBuildCommand = this->FindXcodeBuildCommand();
    }
  return this->XcodeBuildCommand;
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::FindXcodeBuildCommand()
{
  if (this->XcodeVersion >= 40)
    {
    std::string makeProgram = cmSystemTools::FindProgram("xcodebuild");
    if (makeProgram.empty())
      {
      makeProgram = "xcodebuild";
      }
    return makeProgram;
    }
  else
    {
    // Use cmakexbuild wrapper to suppress environment dump from output.
    return cmSystemTools::GetCMakeCommand() + "xbuild";
    }
}

//----------------------------------------------------------------------------
bool cmGlobalXCodeGenerator::SetGeneratorToolset(std::string const& ts,
                                                 cmMakefile* mf)
{
  if(this->XcodeVersion >= 30)
    {
    this->GeneratorToolset = ts;
    if(!this->GeneratorToolset.empty())
      {
      mf->AddDefinition("CMAKE_XCODE_PLATFORM_TOOLSET",
                        this->GeneratorToolset.c_str());
      }
    return true;
    }
  else
    {
    return cmGlobalGenerator::SetGeneratorToolset(ts, mf);
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::EnableLanguage(std::vector<std::string>const&
                                            lang,
                                            cmMakefile * mf, bool optional)
{
  mf->AddDefinition("XCODE","1");
  mf->AddDefinition("XCODE_VERSION", this->VersionString.c_str());
  if(this->XcodeVersion == 15)
    {
    }
  else
    {
    if(!mf->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
      {
      mf->AddCacheDefinition(
        "CMAKE_CONFIGURATION_TYPES",
        "Debug;Release;MinSizeRel;RelWithDebInfo",
        "Semicolon separated list of supported configuration types, "
        "only supports Debug, Release, MinSizeRel, and RelWithDebInfo, "
        "anything else will be ignored.",
        cmState::STRING);
      }
    }
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  this->cmGlobalGenerator::EnableLanguage(lang, mf, optional);
    const char* osxArch =
      mf->GetDefinition("CMAKE_OSX_ARCHITECTURES");
  const char* sysroot =
      mf->GetDefinition("CMAKE_OSX_SYSROOT");
  if(osxArch && sysroot)
    {
    this->Architectures.clear();
    cmSystemTools::ExpandListArgument(std::string(osxArch),
                                      this->Architectures);
    }
}

//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator::GenerateBuildCommand(
  std::vector<std::string>& makeCommand,
  const std::string& makeProgram,
  const std::string& projectName,
  const std::string& /*projectDir*/,
  const std::string& targetName,
  const std::string& config,
  bool /*fast*/, bool /*verbose*/,
  std::vector<std::string> const& makeOptions)
{
  // now build the test
  makeCommand.push_back(
    this->SelectMakeProgram(makeProgram, this->GetXcodeBuildCommand())
    );

  makeCommand.push_back("-project");
  std::string projectArg = projectName;
  projectArg += ".xcode";
  if(this->XcodeVersion > 20)
    {
    projectArg += "proj";
    }
  makeCommand.push_back(projectArg);

  bool clean = false;
  std::string realTarget = targetName;
  if ( realTarget == "clean" )
    {
    clean = true;
    realTarget = "ALL_BUILD";
    }
  if(clean)
    {
    makeCommand.push_back("clean");
    }
  else
    {
    makeCommand.push_back("build");
    }
  makeCommand.push_back("-target");
  if (!realTarget.empty())
    {
    makeCommand.push_back(realTarget);
    }
  else
    {
    makeCommand.push_back("ALL_BUILD");
    }
  if(this->XcodeVersion == 15)
    {
    makeCommand.push_back("-buildstyle");
    makeCommand.push_back("Development");
    }
  else
    {
    makeCommand.push_back("-configuration");
    makeCommand.push_back(!config.empty()?config:"Debug");
    }
  makeCommand.insert(makeCommand.end(),
                     makeOptions.begin(), makeOptions.end());
}

//----------------------------------------------------------------------------
///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *
cmGlobalXCodeGenerator::CreateLocalGenerator(cmLocalGenerator* parent,
                                             cmState::Snapshot snapshot)
{
  return new cmLocalXCodeGenerator(this, parent, snapshot);
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::Generate()
{
  std::map<std::string, std::vector<cmLocalGenerator*> >::iterator it;
  // make sure extra targets are added before calling
  // the parent generate which will call trace depends
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    {
    cmLocalGenerator* root = it->second[0];
    this->SetGenerationRoot(root);
    // add ALL_BUILD, INSTALL, etc
    this->AddExtraTargets(root, it->second);
    }
  this->cmGlobalGenerator::Generate();
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return;
    }
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    {
    cmLocalGenerator* root = it->second[0];
    this->SetGenerationRoot(root);
    // now create the project
    this->OutputXCodeProject(root, it->second);
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::SetGenerationRoot(cmLocalGenerator* root)
{
  this->CurrentProject = root->GetMakefile()->GetProjectName();
  this->SetCurrentLocalGenerator(root);
  cmSystemTools::SplitPath(this->CurrentMakefile->GetCurrentSourceDirectory(),
                           this->ProjectSourceDirectoryComponents);
  cmSystemTools::SplitPath(this->CurrentMakefile->GetCurrentBinaryDirectory(),
                           this->ProjectOutputDirectoryComponents);

  this->CurrentXCodeHackMakefile =
    root->GetMakefile()->GetCurrentBinaryDirectory();
  this->CurrentXCodeHackMakefile += "/CMakeScripts";
  cmSystemTools::MakeDirectory(this->CurrentXCodeHackMakefile.c_str());
  this->CurrentXCodeHackMakefile += "/XCODE_DEPEND_HELPER.make";
}

//----------------------------------------------------------------------------
std::string
cmGlobalXCodeGenerator::PostBuildMakeTarget(std::string const& tName,
                                            std::string const& configName)
{
  std::string target = tName;
  cmSystemTools::ReplaceString(target, " ", "_");
  std::string out = "PostBuild." + target;
  if(this->XcodeVersion > 20)
    {
    out += "." + configName;
    }
  return out;
}

//----------------------------------------------------------------------------
#define CMAKE_CHECK_BUILD_SYSTEM_TARGET "ZERO_CHECK"

//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator::AddExtraTargets(cmLocalGenerator* root,
                                        std::vector<cmLocalGenerator*>& gens)
{
  cmMakefile* mf = root->GetMakefile();

  // Add ALL_BUILD
  const char* no_working_directory = 0;
  std::vector<std::string> no_depends;
  mf->AddUtilityCommand("ALL_BUILD", true, no_depends,
                        no_working_directory,
                        "echo", "Build all projects");
  cmTarget* allbuild = mf->FindTarget("ALL_BUILD");

  // Refer to the main build configuration file for easy editing.
  std::string listfile = mf->GetCurrentSourceDirectory();
  listfile += "/";
  listfile += "CMakeLists.txt";
  allbuild->AddSourceCMP0049(listfile.c_str());

  // Add XCODE depend helper
  std::string dir = mf->GetCurrentBinaryDirectory();
  cmCustomCommandLine makeHelper;
  if(this->XcodeVersion < 50)
    {
    makeHelper.push_back("make");
    makeHelper.push_back("-C");
    makeHelper.push_back(dir.c_str());
    makeHelper.push_back("-f");
    makeHelper.push_back(this->CurrentXCodeHackMakefile.c_str());
    makeHelper.push_back(""); // placeholder, see below
    }

  // Add ZERO_CHECK
  bool regenerate = !mf->IsOn("CMAKE_SUPPRESS_REGENERATION");
  if (regenerate)
    {
    this->CreateReRunCMakeFile(root, gens);
    std::string file = this->ConvertToRelativeForMake(
      this->CurrentReRunCMakeMakefile.c_str());
    cmSystemTools::ReplaceString(file, "\\ ", " ");
    mf->AddUtilityCommand(CMAKE_CHECK_BUILD_SYSTEM_TARGET, true, no_depends,
                          no_working_directory,
                          "make", "-f", file.c_str());
    }

  // now make the allbuild depend on all the non-utility targets
  // in the project
  for(std::vector<cmLocalGenerator*>::iterator i = gens.begin();
      i != gens.end(); ++i)
    {
    cmLocalGenerator* lg = *i;
    if(this->IsExcluded(root, *i))
      {
      continue;
      }

    cmTargets& tgts = lg->GetMakefile()->GetTargets();
    for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
      {
      cmTarget& target = l->second;

      if (regenerate && (l->first != CMAKE_CHECK_BUILD_SYSTEM_TARGET))
        {
        target.AddUtility(CMAKE_CHECK_BUILD_SYSTEM_TARGET);
        }

      // make all exe, shared libs and modules
      // run the depend check makefile as a post build rule
      // this will make sure that when the next target is built
      // things are up-to-date
      if(!makeHelper.empty() &&
         (target.GetType() == cmTarget::EXECUTABLE ||
// Nope - no post-build for OBJECT_LIRBRARY
//          target.GetType() == cmTarget::OBJECT_LIBRARY ||
          target.GetType() == cmTarget::STATIC_LIBRARY ||
          target.GetType() == cmTarget::SHARED_LIBRARY ||
          target.GetType() == cmTarget::MODULE_LIBRARY))
        {
        makeHelper[makeHelper.size()-1] = // fill placeholder
          this->PostBuildMakeTarget(target.GetName(), "$(CONFIGURATION)");
        cmCustomCommandLines commandLines;
        commandLines.push_back(makeHelper);
        std::vector<std::string> no_byproducts;
        lg->GetMakefile()->AddCustomCommandToTarget(target.GetName(),
                                                    no_byproducts,
                                                    no_depends,
                                                    commandLines,
                                                    cmTarget::POST_BUILD,
                                                    "Depend check for xcode",
                                                    dir.c_str());
        }

      if(target.GetType() != cmTarget::INTERFACE_LIBRARY
          && !target.GetPropertyAsBool("EXCLUDE_FROM_ALL"))
        {
        allbuild->AddUtility(target.GetName());
        }

      // Refer to the build configuration file for easy editing.
      listfile = lg->GetMakefile()->GetCurrentSourceDirectory();
      listfile += "/";
      listfile += "CMakeLists.txt";
      target.AddSourceCMP0049(listfile.c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateReRunCMakeFile(
  cmLocalGenerator* root, std::vector<cmLocalGenerator*> const& gens)
{
  cmMakefile* mf = root->GetMakefile();
  std::vector<std::string> lfiles;
  for(std::vector<cmLocalGenerator*>::const_iterator gi = gens.begin();
      gi != gens.end(); ++gi)
    {
    std::vector<std::string> const& lf = (*gi)->GetMakefile()->GetListFiles();
    lfiles.insert(lfiles.end(), lf.begin(), lf.end());
    }

  // sort the array
  std::sort(lfiles.begin(), lfiles.end(), std::less<std::string>());
  std::vector<std::string>::iterator new_end =
    std::unique(lfiles.begin(), lfiles.end());
  lfiles.erase(new_end, lfiles.end());
  this->CurrentReRunCMakeMakefile = mf->GetCurrentBinaryDirectory();
  this->CurrentReRunCMakeMakefile += "/CMakeScripts";
  cmSystemTools::MakeDirectory(this->CurrentReRunCMakeMakefile.c_str());
  this->CurrentReRunCMakeMakefile += "/ReRunCMake.make";
  cmGeneratedFileStream makefileStream
    (this->CurrentReRunCMakeMakefile.c_str());
  makefileStream.SetCopyIfDifferent(true);
  makefileStream << "# Generated by CMake, DO NOT EDIT\n";
  std::string checkCache = mf->GetHomeOutputDirectory();
  checkCache += "/";
  checkCache += cmake::GetCMakeFilesDirectoryPostSlash();
  checkCache += "cmake.check_cache";
  makefileStream << this->ConvertToRelativeForMake(checkCache.c_str())
                 << ": ";
  for(std::vector<std::string>::const_iterator i = lfiles.begin();
      i !=  lfiles.end(); ++i)
    {
    makefileStream << "\\\n" << this->ConvertToRelativeForMake(i->c_str());
    }
  makefileStream << "\n\t" <<
    this->ConvertToRelativeForMake(cmSystemTools::GetCMakeCommand().c_str())
                 << " -H" << this->ConvertToRelativeForMake(
                   mf->GetHomeDirectory())
                 << " -B" << this->ConvertToRelativeForMake(
                   mf->GetHomeOutputDirectory()) << "\n";
}

//----------------------------------------------------------------------------

static bool objectIdLessThan(cmXCodeObject* l, cmXCodeObject* r)
{
  return l->GetId() < r->GetId();
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::SortXCodeObjects()
{
  std::sort(this->XCodeObjects.begin(), this->XCodeObjects.end(),
            objectIdLessThan);
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::ClearXCodeObjects()
{
  this->TargetDoneSet.clear();
  for(unsigned int i = 0; i < this->XCodeObjects.size(); ++i)
    {
    delete this->XCodeObjects[i];
    }
  this->XCodeObjects.clear();
  this->XCodeObjectIDs.clear();
  this->XCodeObjectMap.clear();
  this->GroupMap.clear();
  this->GroupNameMap.clear();
  this->TargetGroup.clear();
  this->FileRefs.clear();
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::addObject(cmXCodeObject *obj)
{
  if(obj->GetType() == cmXCodeObject::OBJECT)
    {
    std::string id = obj->GetId();

    // If this is a duplicate id, it's an error:
    //
    if(this->XCodeObjectIDs.count(id))
      {
      cmSystemTools::Error(
        "Xcode generator: duplicate object ids not allowed");
      }

    this->XCodeObjectIDs.insert(id);
    }

  this->XCodeObjects.push_back(obj);
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::PBXType ptype)
{
  cmXCodeObject* obj;
  if(this->XcodeVersion == 15)
    {
    obj = new cmXCodeObject(ptype, cmXCodeObject::OBJECT);
    }
  else
    {
    obj = new cmXCode21Object(ptype, cmXCodeObject::OBJECT);
    }
  this->addObject(obj);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::Type type)
{
  cmXCodeObject* obj = new cmXCodeObject(cmXCodeObject::None, type);
  this->addObject(obj);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateString(const std::string& s)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::STRING);
  obj->SetString(s);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator
::CreateObjectReference(cmXCodeObject* ref)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::OBJECT_REF);
  obj->SetObject(ref);
  return obj;
}

//----------------------------------------------------------------------------
std::string
GetGroupMapKeyFromPath(cmTarget& cmtarget, const std::string& fullpath)
{
  std::string key(cmtarget.GetName());
  key += "-";
  key += fullpath;
  return key;
}

//----------------------------------------------------------------------------
std::string
GetGroupMapKey(cmTarget& cmtarget, cmSourceFile* sf)
{
  return GetGroupMapKeyFromPath(cmtarget, sf->GetFullPath());
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateXCodeSourceFileFromPath(
  const std::string &fullpath,
  cmTarget& cmtarget,
  const std::string &lang,
  cmSourceFile* sf)
{
  // Using a map and the full path guarantees that we will always get the same
  // fileRef object for any given full path.
  //
  cmXCodeObject* fileRef =
    this->CreateXCodeFileReferenceFromPath(fullpath, cmtarget, lang, sf);

  cmXCodeObject* buildFile = this->CreateObject(cmXCodeObject::PBXBuildFile);
  buildFile->SetComment(fileRef->GetComment());
  buildFile->AddAttribute("fileRef", this->CreateObjectReference(fileRef));

  return buildFile;
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateXCodeSourceFile(cmLocalGenerator* lg,
                                              cmSourceFile* sf,
                                              cmTarget& cmtarget)
{
  // Add flags from target and source file properties.
  std::string flags;
  const char* srcfmt = sf->GetProperty("Fortran_FORMAT");
  switch(this->CurrentLocalGenerator->GetFortranFormat(srcfmt))
    {
    case cmLocalGenerator::FortranFormatFixed: flags="-fixed "+flags; break;
    case cmLocalGenerator::FortranFormatFree: flags="-free "+flags; break;
    default: break;
    }
  lg->AppendFlags(flags, sf->GetProperty("COMPILE_FLAGS"));

  // Add per-source definitions.
  BuildObjectListOrString flagsBuild(this, false);
  this->AppendDefines(flagsBuild,
                      sf->GetProperty("COMPILE_DEFINITIONS"), true);
  if (!flagsBuild.IsEmpty())
    {
    if (flags.size())
      {
      flags += ' ';
      }
    flags += flagsBuild.GetString();
    }

  std::string lang =
    this->CurrentLocalGenerator->GetSourceFileLanguage(*sf);

  cmXCodeObject* buildFile =
    this->CreateXCodeSourceFileFromPath(sf->GetFullPath(), cmtarget, lang, sf);
  cmXCodeObject* fileRef = buildFile->GetObject("fileRef")->GetObject();

  cmXCodeObject* settings =
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  settings->AddAttribute("COMPILER_FLAGS", this->CreateString(flags.c_str()));

  // Is this a resource file in this target? Add it to the resources group...
  //
  cmGeneratorTarget::SourceFileFlags tsFlags =
            this->GetGeneratorTarget(&cmtarget)->GetTargetSourceFileFlags(sf);
  bool isResource = tsFlags.Type == cmGeneratorTarget::SourceFileTypeResource;

  // Is this a "private" or "public" framework header file?
  // Set the ATTRIBUTES attribute appropriately...
  //
  if(cmtarget.IsFrameworkOnApple())
    {
    if(tsFlags.Type == cmGeneratorTarget::SourceFileTypePrivateHeader)
      {
      cmXCodeObject* attrs = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      attrs->AddObject(this->CreateString("Private"));
      settings->AddAttribute("ATTRIBUTES", attrs);
      isResource = true;
      }
    else if(tsFlags.Type == cmGeneratorTarget::SourceFileTypePublicHeader)
      {
      cmXCodeObject* attrs = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      attrs->AddObject(this->CreateString("Public"));
      settings->AddAttribute("ATTRIBUTES", attrs);
      isResource = true;
      }
    }

  // Add the fileRef to the top level Resources group/folder if it is not
  // already there.
  //
  if(isResource && this->ResourcesGroupChildren &&
    !this->ResourcesGroupChildren->HasObject(fileRef))
    {
    this->ResourcesGroupChildren->AddObject(fileRef);
    }

  buildFile->AddAttribute("settings", settings);
  return buildFile;
}

//----------------------------------------------------------------------------
std::string
GetSourcecodeValueFromFileExtension(const std::string& _ext,
                                    const std::string& lang,
                                    bool& keepLastKnownFileType)
{
  std::string ext = cmSystemTools::LowerCase(_ext);
  std::string sourcecode = "sourcecode";

  if(ext == "o")
    {
    sourcecode = "compiled.mach-o.objfile";
    }
  else if(ext == "xctest")
    {
    sourcecode = "wrapper.cfbundle";
    }
  else if(ext == "xib")
    {
    keepLastKnownFileType = true;
    sourcecode = "file.xib";
    }
  else if(ext == "storyboard")
    {
    keepLastKnownFileType = true;
    sourcecode = "file.storyboard";
    }
  else if(ext == "mm")
    {
    sourcecode += ".cpp.objcpp";
    }
  else if(ext == "m")
    {
    sourcecode += ".c.objc";
    }
  else if(ext == "plist")
    {
    sourcecode += ".text.plist";
    }
  else if(ext == "h")
    {
    sourcecode += ".c.h";
    }
  else if(ext == "hxx" || ext == "hpp" || ext == "txx"
    || ext == "pch" || ext == "hh")
    {
    sourcecode += ".cpp.h";
    }
  else if(ext == "png" || ext == "gif" || ext == "jpg")
    {
    keepLastKnownFileType = true;
    sourcecode = "image";
    }
  else if(ext == "txt")
    {
    sourcecode += ".text";
    }
  else if(lang == "CXX")
    {
    sourcecode += ".cpp.cpp";
    }
  else if(lang == "C")
    {
    sourcecode += ".c.c";
    }
  else if(lang == "Fortran")
    {
    sourcecode += ".fortran.f90";
    }
  else if(lang == "ASM")
    {
    sourcecode += ".asm";
    }
  else if (ext == "metal")
    {
    sourcecode += ".metal";
    }
  //else
  //  {
  //  // Already specialized above or we leave sourcecode == "sourcecode"
  //  // which is probably the most correct choice. Extensionless headers,
  //  // for example... Or file types unknown to Xcode that do not map to a
  //  // valid explicitFileType value.
  //  }

  return sourcecode;
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateXCodeFileReferenceFromPath(
  const std::string &fullpath,
  cmTarget& cmtarget,
  const std::string &lang,
  cmSourceFile* sf)
{
  std::string key = GetGroupMapKeyFromPath(cmtarget, fullpath);
  cmXCodeObject* fileRef = this->FileRefs[key];
  if(!fileRef)
    {
    fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
    fileRef->SetComment(fullpath);
    this->FileRefs[key] = fileRef;
    }
  cmXCodeObject* group = this->GroupMap[key];
  cmXCodeObject* children = group->GetObject("children");
  if (!children->HasObject(fileRef))
    {
    children->AddObject(fileRef);
    }
  fileRef->AddAttribute("fileEncoding", this->CreateString("4"));

  bool useLastKnownFileType = false;
  std::string fileType;
  if(sf)
    {
    if(const char* e = sf->GetProperty("XCODE_EXPLICIT_FILE_TYPE"))
      {
      fileType = e;
      }
    else if(const char* l = sf->GetProperty("XCODE_LAST_KNOWN_FILE_TYPE"))
      {
      useLastKnownFileType = true;
      fileType = l;
      }
    }
  if(fileType.empty())
    {
    // Compute the extension without leading '.'.
    std::string ext = cmSystemTools::GetFilenameLastExtension(fullpath);
    if(!ext.empty())
      {
      ext = ext.substr(1);
      }

    // If fullpath references a directory, then we need to specify
    // lastKnownFileType as folder in order for Xcode to be able to
    // open the contents of the folder.
    // (Xcode 4.6 does not like explicitFileType=folder).
    if(cmSystemTools::FileIsDirectory(fullpath.c_str()))
      {
      fileType = (ext == "xcassets"? "folder.assetcatalog" : "folder");
      useLastKnownFileType = true;
      }
    else
      {
      fileType = GetSourcecodeValueFromFileExtension(
        ext, lang, useLastKnownFileType);
      }
    }

  fileRef->AddAttribute(useLastKnownFileType? "lastKnownFileType"
                                            : "explicitFileType",
                        this->CreateString(fileType));

  // Store the file path relative to the top of the source tree.
  std::string path = this->RelativeToSource(fullpath.c_str());
  std::string name = cmSystemTools::GetFilenameName(path.c_str());
  const char* sourceTree = (cmSystemTools::FileIsFullPath(path.c_str())?
                            "<absolute>" : "SOURCE_ROOT");
  fileRef->AddAttribute("name", this->CreateString(name.c_str()));
  fileRef->AddAttribute("path", this->CreateString(path.c_str()));
  fileRef->AddAttribute("sourceTree", this->CreateString(sourceTree));
  if(this->XcodeVersion == 15)
    {
    fileRef->AddAttribute("refType", this->CreateString("4"));
    }
  return fileRef;
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateXCodeFileReference(cmSourceFile* sf,
                                                 cmTarget& cmtarget)
{
  std::string lang =
    this->CurrentLocalGenerator->GetSourceFileLanguage(*sf);

  return this->CreateXCodeFileReferenceFromPath(
    sf->GetFullPath(), cmtarget, lang, sf);
}

//----------------------------------------------------------------------------
bool cmGlobalXCodeGenerator::SpecialTargetEmitted(std::string const& tname)
{
  if(tname == "ALL_BUILD" || tname == "XCODE_DEPEND_HELPER" ||
     tname == "install" || tname == "package" || tname == "RUN_TESTS" ||
     tname == CMAKE_CHECK_BUILD_SYSTEM_TARGET )
    {
    if(this->TargetDoneSet.find(tname) != this->TargetDoneSet.end())
      {
      return true;
      }
    this->TargetDoneSet.insert(tname);
    return false;
    }
  return false;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::SetCurrentLocalGenerator(cmLocalGenerator* gen)
{
  this->CurrentLocalGenerator = gen;
  this->CurrentMakefile = gen->GetMakefile();
  std::string outdir =
    cmSystemTools::CollapseFullPath(this->CurrentMakefile->
                                    GetCurrentBinaryDirectory());
  cmSystemTools::SplitPath(outdir.c_str(),
                           this->CurrentOutputDirectoryComponents);

  // Select the current set of configuration types.
  this->CurrentConfigurationTypes.clear();
  this->CurrentMakefile->GetConfigurations(this->CurrentConfigurationTypes);
  if(this->CurrentConfigurationTypes.empty())
    {
    this->CurrentConfigurationTypes.push_back("");
    }
}

//----------------------------------------------------------------------------
struct cmSourceFilePathCompare
{
  bool operator()(cmSourceFile* l, cmSourceFile* r)
  {
    return l->GetFullPath() < r->GetFullPath();
  }
};

//----------------------------------------------------------------------------
struct cmCompareTargets
{
  bool operator () (std::string const& a, std::string const& b) const
  {
    if (a == "ALL_BUILD")
      {
      return true;
      }
    if (b == "ALL_BUILD")
      {
      return false;
      }
    return strcmp(a.c_str(), b.c_str()) < 0;
  }
};

//----------------------------------------------------------------------------
bool
cmGlobalXCodeGenerator::CreateXCodeTargets(cmLocalGenerator* gen,
                                           std::vector<cmXCodeObject*>&
                                           targets)
{
  this->SetCurrentLocalGenerator(gen);
  cmTargets &tgts = this->CurrentMakefile->GetTargets();
  typedef std::map<std::string, cmTarget*, cmCompareTargets> cmSortedTargets;
  cmSortedTargets sortedTargets;
  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
    {
    sortedTargets[l->first] = &l->second;
    }
  for(cmSortedTargets::iterator l = sortedTargets.begin();
      l != sortedTargets.end(); l++)
    {
    cmTarget& cmtarget = *l->second;
    cmGeneratorTarget* gtgt = this->GetGeneratorTarget(&cmtarget);

    // make sure ALL_BUILD, INSTALL, etc are only done once
    if(this->SpecialTargetEmitted(l->first.c_str()))
      {
      continue;
      }

    if(cmtarget.GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      continue;
      }

    if(cmtarget.GetType() == cmTarget::UTILITY ||
       cmtarget.GetType() == cmTarget::GLOBAL_TARGET)
      {
      cmXCodeObject* t = this->CreateUtilityTarget(cmtarget);
      if (!t)
        {
        return false;
        }
      targets.push_back(t);
      continue;
      }

    // organize the sources
    std::vector<cmSourceFile*> classes;
    if (!cmtarget.GetConfigCommonSourceFiles(classes))
      {
      return false;
      }
    std::sort(classes.begin(), classes.end(), cmSourceFilePathCompare());

    gtgt->ComputeObjectMapping();

    std::vector<cmXCodeObject*> externalObjFiles;
    std::vector<cmXCodeObject*> headerFiles;
    std::vector<cmXCodeObject*> resourceFiles;
    std::vector<cmXCodeObject*> sourceFiles;
    for(std::vector<cmSourceFile*>::const_iterator i = classes.begin();
        i != classes.end(); ++i)
      {
      cmXCodeObject* xsf =
        this->CreateXCodeSourceFile(this->CurrentLocalGenerator,
                                    *i, cmtarget);
      cmXCodeObject* fr = xsf->GetObject("fileRef");
      cmXCodeObject* filetype =
        fr->GetObject()->GetObject("explicitFileType");

      cmGeneratorTarget::SourceFileFlags tsFlags =
        gtgt->GetTargetSourceFileFlags(*i);

      if(filetype &&
         filetype->GetString() == "compiled.mach-o.objfile")
        {
        if ((*i)->GetObjectLibrary().empty())
          {
          externalObjFiles.push_back(xsf);
          }
        }
      else if(this->IsHeaderFile(*i) ||
        (tsFlags.Type == cmGeneratorTarget::SourceFileTypePrivateHeader) ||
        (tsFlags.Type == cmGeneratorTarget::SourceFileTypePublicHeader))
        {
        headerFiles.push_back(xsf);
        }
      else if(tsFlags.Type == cmGeneratorTarget::SourceFileTypeResource)
        {
        resourceFiles.push_back(xsf);
        }
      else if(!(*i)->GetPropertyAsBool("HEADER_FILE_ONLY"))
        {
        // Include this file in the build if it has a known language
        // and has not been listed as an ignored extension for this
        // generator.
        if(!this->CurrentLocalGenerator->GetSourceFileLanguage(**i).empty() &&
           !this->IgnoreFile((*i)->GetExtension().c_str()))
          {
          sourceFiles.push_back(xsf);
          }
        }
      }

    if(this->XcodeVersion < 50)
      {
      // Add object library contents as external objects. (Equivalent to
      // the externalObjFiles above, except each one is not a cmSourceFile
      // within the target.)
      std::vector<std::string> objs;
      gtgt->UseObjectLibraries(objs, "");
      for(std::vector<std::string>::const_iterator
            oi = objs.begin(); oi != objs.end(); ++oi)
        {
        std::string obj = *oi;
        cmXCodeObject* xsf =
          this->CreateXCodeSourceFileFromPath(obj, cmtarget, "", 0);
        externalObjFiles.push_back(xsf);
        }
      }

    // some build phases only apply to bundles and/or frameworks
    bool isFrameworkTarget = cmtarget.IsFrameworkOnApple();
    bool isBundleTarget = cmtarget.GetPropertyAsBool("MACOSX_BUNDLE");
    bool isCFBundleTarget = cmtarget.IsCFBundleOnApple();

    cmXCodeObject* buildFiles = 0;

    // create source build phase
    cmXCodeObject* sourceBuildPhase = 0;
    if (!sourceFiles.empty())
      {
      sourceBuildPhase =
        this->CreateObject(cmXCodeObject::PBXSourcesBuildPhase);
      sourceBuildPhase->SetComment("Sources");
      sourceBuildPhase->AddAttribute("buildActionMask",
                                     this->CreateString("2147483647"));
      buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      for(std::vector<cmXCodeObject*>::iterator i = sourceFiles.begin();
          i != sourceFiles.end(); ++i)
        {
        buildFiles->AddObject(*i);
        }
      sourceBuildPhase->AddAttribute("files", buildFiles);
      sourceBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                     this->CreateString("0"));
      }

    // create header build phase - only for framework targets
    cmXCodeObject* headerBuildPhase = 0;
    if (!headerFiles.empty() && isFrameworkTarget)
      {
      headerBuildPhase =
        this->CreateObject(cmXCodeObject::PBXHeadersBuildPhase);
      headerBuildPhase->SetComment("Headers");
      headerBuildPhase->AddAttribute("buildActionMask",
                                     this->CreateString("2147483647"));
      buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      for(std::vector<cmXCodeObject*>::iterator i = headerFiles.begin();
          i != headerFiles.end(); ++i)
        {
        buildFiles->AddObject(*i);
        }
      headerBuildPhase->AddAttribute("files", buildFiles);
      headerBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                     this->CreateString("0"));
      }

    // create resource build phase - only for framework or bundle targets
    cmXCodeObject* resourceBuildPhase = 0;
    if (!resourceFiles.empty() &&
        (isFrameworkTarget || isBundleTarget || isCFBundleTarget))
      {
      resourceBuildPhase =
        this->CreateObject(cmXCodeObject::PBXResourcesBuildPhase);
      resourceBuildPhase->SetComment("Resources");
      resourceBuildPhase->AddAttribute("buildActionMask",
                                       this->CreateString("2147483647"));
      buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      for(std::vector<cmXCodeObject*>::iterator i = resourceFiles.begin();
          i != resourceFiles.end(); ++i)
        {
        buildFiles->AddObject(*i);
        }
      resourceBuildPhase->AddAttribute("files", buildFiles);
      resourceBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                     this->CreateString("0"));
      }

    // create vector of "non-resource content file" build phases - only for
    // framework or bundle targets
    std::vector<cmXCodeObject*> contentBuildPhases;
    if (isFrameworkTarget || isBundleTarget || isCFBundleTarget)
      {
      typedef std::map<std::string, std::vector<cmSourceFile*> >
        mapOfVectorOfSourceFiles;
      mapOfVectorOfSourceFiles bundleFiles;
      for(std::vector<cmSourceFile*>::const_iterator i = classes.begin();
          i != classes.end(); ++i)
        {
        cmGeneratorTarget::SourceFileFlags tsFlags =
          gtgt->GetTargetSourceFileFlags(*i);
        if(tsFlags.Type == cmGeneratorTarget::SourceFileTypeMacContent)
          {
          bundleFiles[tsFlags.MacFolder].push_back(*i);
          }
        }
      mapOfVectorOfSourceFiles::iterator mit;
      for ( mit = bundleFiles.begin(); mit != bundleFiles.end(); ++ mit )
        {
        cmXCodeObject* copyFilesBuildPhase =
          this->CreateObject(cmXCodeObject::PBXCopyFilesBuildPhase);
        copyFilesBuildPhase->SetComment("Copy files");
        copyFilesBuildPhase->AddAttribute("buildActionMask",
          this->CreateString("2147483647"));
        copyFilesBuildPhase->AddAttribute("dstSubfolderSpec",
          this->CreateString("6"));
        std::ostringstream ostr;
        if (cmtarget.IsFrameworkOnApple())
          {
          // dstPath in frameworks is relative to Versions/<version>
          ostr << mit->first;
          }
        else if ( mit->first != "MacOS" )
          {
          // dstPath in bundles is relative to Contents/MacOS
          ostr << "../" << mit->first.c_str();
          }
        copyFilesBuildPhase->AddAttribute("dstPath",
          this->CreateString(ostr.str().c_str()));
        copyFilesBuildPhase->AddAttribute(
          "runOnlyForDeploymentPostprocessing", this->CreateString("0"));
        buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
        copyFilesBuildPhase->AddAttribute("files", buildFiles);
        std::vector<cmSourceFile*>::iterator sfIt;
        for ( sfIt = mit->second.begin(); sfIt != mit->second.end(); ++ sfIt )
          {
          cmXCodeObject* xsf =
            this->CreateXCodeSourceFile(this->CurrentLocalGenerator,
                                        *sfIt, cmtarget);
          buildFiles->AddObject(xsf);
          }
        contentBuildPhases.push_back(copyFilesBuildPhase);
        }
      }

    // create framework build phase
    cmXCodeObject* frameworkBuildPhase = 0;
    if (!externalObjFiles.empty())
      {
      frameworkBuildPhase =
        this->CreateObject(cmXCodeObject::PBXFrameworksBuildPhase);
      frameworkBuildPhase->SetComment("Frameworks");
      frameworkBuildPhase->AddAttribute("buildActionMask",
                                        this->CreateString("2147483647"));
      buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      frameworkBuildPhase->AddAttribute("files", buildFiles);
      for(std::vector<cmXCodeObject*>::iterator i =  externalObjFiles.begin();
          i != externalObjFiles.end(); ++i)
        {
        buildFiles->AddObject(*i);
        }
      frameworkBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                        this->CreateString("0"));
      }

    // create list of build phases and create the Xcode target
    cmXCodeObject* buildPhases =
      this->CreateObject(cmXCodeObject::OBJECT_LIST);

    this->CreateCustomCommands(buildPhases, sourceBuildPhase,
                               headerBuildPhase, resourceBuildPhase,
                               contentBuildPhases,
                               frameworkBuildPhase, cmtarget);

    targets.push_back(this->CreateXCodeTarget(cmtarget, buildPhases));
    }
  return true;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::ForceLinkerLanguages()
{
  // This makes sure all targets link using the proper language.
  for(TargetMap::const_iterator
        ti = this->TotalTargets.begin(); ti != this->TotalTargets.end(); ++ti)
    {
    this->ForceLinkerLanguage(*ti->second);
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::ForceLinkerLanguage(cmTarget& cmtarget)
{
  // This matters only for targets that link.
  if(cmtarget.GetType() != cmTarget::EXECUTABLE &&
     cmtarget.GetType() != cmTarget::SHARED_LIBRARY &&
     cmtarget.GetType() != cmTarget::MODULE_LIBRARY)
    {
    return;
    }

  std::string llang = cmtarget.GetLinkerLanguage("NOCONFIG");
  if(llang.empty()) { return; }

  // If the language is compiled as a source trust Xcode to link with it.
  cmTarget::LinkImplementation const* impl =
    cmtarget.GetLinkImplementation("NOCONFIG");
  for(std::vector<std::string>::const_iterator li = impl->Languages.begin();
      li != impl->Languages.end(); ++li)
    {
    if(*li == llang) { return; }
    }

  // Add an empty source file to the target that compiles with the
  // linker language.  This should convince Xcode to choose the proper
  // language.
  cmMakefile* mf = cmtarget.GetMakefile();
  std::string fname = mf->GetCurrentBinaryDirectory();
  fname += cmake::GetCMakeFilesDirectory();
  fname += "/";
  fname += cmtarget.GetName();
  fname += "-CMakeForceLinker";
  fname += ".";
  fname += cmSystemTools::LowerCase(llang);
  {
  cmGeneratedFileStream fout(fname.c_str());
  fout << "\n";
  }
  if(cmSourceFile* sf = mf->GetOrCreateSource(fname.c_str()))
    {
    sf->SetProperty("LANGUAGE", llang.c_str());
    cmtarget.AddSource(fname);
    }
}

//----------------------------------------------------------------------------
bool cmGlobalXCodeGenerator::IsHeaderFile(cmSourceFile* sf)
{
  const std::vector<std::string>& hdrExts =
    this->CurrentMakefile->GetHeaderExtensions();
  return (std::find(hdrExts.begin(), hdrExts.end(), sf->GetExtension()) !=
          hdrExts.end());
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateBuildPhase(const char* name,
                                         const char* name2,
                                         cmTarget& cmtarget,
                                         const std::vector<cmCustomCommand>&
                                         commands)
{
  if(commands.size() == 0 && strcmp(name, "CMake ReRun") != 0)
    {
    return 0;
    }
  cmXCodeObject* buildPhase =
    this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
  buildPhase->AddAttribute("buildActionMask",
                           this->CreateString("2147483647"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  buildPhase->AddAttribute("files", buildFiles);
  buildPhase->AddAttribute("name",
                           this->CreateString(name));
  buildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                           this->CreateString("0"));
  buildPhase->AddAttribute("shellPath",
                           this->CreateString("/bin/sh"));
  this->AddCommandsToBuildPhase(buildPhase, cmtarget, commands,
                                name2);
  return buildPhase;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateCustomCommands(cmXCodeObject* buildPhases,
                                                  cmXCodeObject*
                                                  sourceBuildPhase,
                                                  cmXCodeObject*
                                                  headerBuildPhase,
                                                  cmXCodeObject*
                                                  resourceBuildPhase,
                                                  std::vector<cmXCodeObject*>
                                                  contentBuildPhases,
                                                  cmXCodeObject*
                                                  frameworkBuildPhase,
                                                  cmTarget& cmtarget)
{
  std::vector<cmCustomCommand> const & prebuild
    = cmtarget.GetPreBuildCommands();
  std::vector<cmCustomCommand> const & prelink
    = cmtarget.GetPreLinkCommands();
  std::vector<cmCustomCommand> postbuild
    = cmtarget.GetPostBuildCommands();

  if(cmtarget.GetType() == cmTarget::SHARED_LIBRARY &&
    !cmtarget.IsFrameworkOnApple())
    {
    cmCustomCommandLines cmd;
    cmd.resize(1);
    cmd[0].push_back(cmSystemTools::GetCMakeCommand());
    cmd[0].push_back("-E");
    cmd[0].push_back("cmake_symlink_library");
    std::string str_file = "$<TARGET_FILE:";
    str_file += cmtarget.GetName();
    str_file += ">";
    std::string str_so_file = "$<TARGET_SONAME_FILE:";
    str_so_file += cmtarget.GetName();
    str_so_file += ">";
    std::string str_link_file = "$<TARGET_LINKER_FILE:";
    str_link_file += cmtarget.GetName();
    str_link_file += ">";
    cmd[0].push_back(str_file);
    cmd[0].push_back(str_so_file);
    cmd[0].push_back(str_link_file);

    cmCustomCommand command(this->CurrentMakefile,
      std::vector<std::string>(),
      std::vector<std::string>(),
      std::vector<std::string>(),
      cmd,
      "Creating symlinks",
      "");

    postbuild.push_back(command);
    }

  std::vector<cmSourceFile*> classes;
  if (!cmtarget.GetConfigCommonSourceFiles(classes))
    {
    return;
    }
  // add all the sources
  std::vector<cmCustomCommand> commands;
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin();
      i != classes.end(); ++i)
    {
    if((*i)->GetCustomCommand())
      {
      commands.push_back(*(*i)->GetCustomCommand());
      }
    }
  // create prebuild phase
  cmXCodeObject* cmakeRulesBuildPhase =
    this->CreateBuildPhase("CMake Rules",
                           "cmakeRulesBuildPhase",
                           cmtarget, commands);
  // create prebuild phase
  cmXCodeObject* preBuildPhase =
    this->CreateBuildPhase("CMake PreBuild Rules", "preBuildCommands",
                           cmtarget, prebuild);
  // create prelink phase
  cmXCodeObject* preLinkPhase =
    this->CreateBuildPhase("CMake PreLink Rules", "preLinkCommands",
                           cmtarget, prelink);
  // create postbuild phase
  cmXCodeObject* postBuildPhase =
    this->CreateBuildPhase("CMake PostBuild Rules", "postBuildPhase",
                           cmtarget, postbuild);

  // The order here is the order they will be built in.
  // The order "headers, resources, sources" mimics a native project generated
  // from an xcode template...
  //
  if(preBuildPhase)
    {
    buildPhases->AddObject(preBuildPhase);
    }
  if(cmakeRulesBuildPhase)
    {
    buildPhases->AddObject(cmakeRulesBuildPhase);
    }
  if(headerBuildPhase)
    {
    buildPhases->AddObject(headerBuildPhase);
    }
  if(resourceBuildPhase)
    {
    buildPhases->AddObject(resourceBuildPhase);
    }
  std::vector<cmXCodeObject*>::iterator cit;
  for (cit = contentBuildPhases.begin(); cit != contentBuildPhases.end();
       ++cit)
    {
    buildPhases->AddObject(*cit);
    }
  if(sourceBuildPhase)
    {
    buildPhases->AddObject(sourceBuildPhase);
    }
  if(preLinkPhase)
    {
    buildPhases->AddObject(preLinkPhase);
    }
  if(frameworkBuildPhase)
    {
    buildPhases->AddObject(frameworkBuildPhase);
    }
  if(postBuildPhase)
    {
    buildPhases->AddObject(postBuildPhase);
    }
}

//----------------------------------------------------------------------------
// This function removes each occurrence of the flag and returns the last one
// (i.e., the dominant flag in GCC)
std::string cmGlobalXCodeGenerator::ExtractFlag(const char* flag,
                                                std::string& flags)
{
  std::string retFlag;
  std::string::size_type lastOccurancePos = flags.rfind(flag);
  bool saved = false;
  while(lastOccurancePos != flags.npos)
    {
    //increment pos, we use lastOccurancePos to reduce search space on next inc
    std::string::size_type pos = lastOccurancePos;
    if(pos == 0 || flags[pos-1]==' ')
      {
      while(pos < flags.size() && flags[pos] != ' ')
        {
        if(!saved)
          {
          retFlag += flags[pos];
          }
        flags[pos] = ' ';
        pos++;
        }
      saved = true;
      }
    //decrement lastOccurancePos while making sure we don't loop around
    //and become a very large positive number since size_type is unsigned
    lastOccurancePos = lastOccurancePos == 0 ? 0 : lastOccurancePos-1;
    lastOccurancePos = flags.rfind(flag,lastOccurancePos);
    }
  return retFlag;
}

//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator::AddCommandsToBuildPhase(cmXCodeObject* buildphase,
                                                cmTarget& target,
                                                std::vector<cmCustomCommand>
                                                const & commands,
                                                const char* name)
{
  std::string dir = this->CurrentMakefile->GetCurrentBinaryDirectory();
  dir += "/CMakeScripts";
  cmSystemTools::MakeDirectory(dir.c_str());
  std::string makefile = dir;
  makefile += "/";
  makefile += target.GetName();
  makefile += "_";
  makefile += name;
  makefile += ".make";

  for (std::vector<std::string>::const_iterator currentConfig=
            this->CurrentConfigurationTypes.begin();
       currentConfig!=this->CurrentConfigurationTypes.end();
       currentConfig++ )
    {
    this->CreateCustomRulesMakefile(makefile.c_str(),
                                    target,
                                    commands,
                                    currentConfig->c_str());
    }

  std::string cdir = this->CurrentMakefile->GetCurrentBinaryDirectory();
  cdir = this->ConvertToRelativeForXCode(cdir.c_str());
  std::string makecmd = "make -C ";
  makecmd += cdir;
  makecmd += " -f ";
  makecmd += this->ConvertToRelativeForMake(
                                          (makefile+"$CONFIGURATION").c_str());
  makecmd += " all";
  cmSystemTools::ReplaceString(makecmd, "\\ ", "\\\\ ");
  buildphase->AddAttribute("shellScript",
                           this->CreateString(makecmd.c_str()));
  buildphase->AddAttribute("showEnvVarsInLog",
                           this->CreateString("0"));
}

//----------------------------------------------------------------------------
void  cmGlobalXCodeGenerator
::CreateCustomRulesMakefile(const char* makefileBasename,
                            cmTarget& target,
                            std::vector<cmCustomCommand>
                            const & commands,
                            const std::string& configName)
{
  std::string makefileName=makefileBasename;
  if(this->XcodeVersion > 20)
    {
    makefileName+=configName;
    }
  cmGeneratedFileStream makefileStream(makefileName.c_str());
  if(!makefileStream)
    {
    return;
    }
  makefileStream.SetCopyIfDifferent(true);
  makefileStream << "# Generated by CMake, DO NOT EDIT\n";
  makefileStream << "# Custom rules for " << target.GetName() << "\n";

  // disable the implicit rules
  makefileStream << ".SUFFIXES: " << "\n";

  // have all depend on all outputs
  makefileStream << "all: ";
  std::map<const cmCustomCommand*, std::string> tname;
  int count = 0;
  for(std::vector<cmCustomCommand>::const_iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    cmCustomCommandGenerator ccg(*i, configName, this->CurrentMakefile);
    if(ccg.GetNumberOfCommands() > 0)
      {
      const std::vector<std::string>& outputs = ccg.GetOutputs();
      if(!outputs.empty())
        {
        for(std::vector<std::string>::const_iterator o = outputs.begin();
            o != outputs.end(); ++o)
          {
          makefileStream
              << "\\\n\t" << this->ConvertToRelativeForMake(o->c_str());
          }
        }
      else
        {
        std::ostringstream str;
        str << "_buildpart_" << count++ ;
        tname[&ccg.GetCC()] = std::string(target.GetName()) + str.str();
        makefileStream << "\\\n\t" << tname[&ccg.GetCC()];
        }
      }
    }
  makefileStream << "\n\n";
  for(std::vector<cmCustomCommand>::const_iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    cmCustomCommandGenerator ccg(*i, configName, this->CurrentMakefile);
    if(ccg.GetNumberOfCommands() > 0)
      {
      makefileStream << "\n";
      const std::vector<std::string>& outputs = ccg.GetOutputs();
      if(!outputs.empty())
        {
        // There is at least one output, start the rule for it
        const char* sep = "";
        for(std::vector<std::string>::const_iterator oi = outputs.begin();
            oi != outputs.end(); ++oi)
          {
          makefileStream << sep <<
            this->ConvertToRelativeForMake(oi->c_str());
          sep = " ";
          }
        makefileStream << ": ";
        }
      else
        {
        // There are no outputs.  Use the generated force rule name.
        makefileStream << tname[&ccg.GetCC()] << ": ";
        }
      for(std::vector<std::string>::const_iterator d =
          ccg.GetDepends().begin();
          d != ccg.GetDepends().end(); ++d)
        {
        std::string dep;
        if(this->CurrentLocalGenerator
           ->GetRealDependency(d->c_str(), configName, dep))
          {
          makefileStream << "\\\n" <<
            this->ConvertToRelativeForMake(dep.c_str());
          }
        }
      makefileStream << "\n";

      if(const char* comment = ccg.GetComment())
        {
        std::string echo_cmd = "echo ";
        echo_cmd += (this->CurrentLocalGenerator->
          EscapeForShell(comment, ccg.GetCC().GetEscapeAllowMakeVars()));
        makefileStream << "\t" << echo_cmd.c_str() << "\n";
        }

      // Add each command line to the set of commands.
      for(unsigned int c = 0; c < ccg.GetNumberOfCommands(); ++c)
        {
        // Build the command line in a single string.
        std::string cmd2 = ccg.GetCommand(c);
        cmSystemTools::ReplaceString(cmd2, "/./", "/");
        cmd2 = this->ConvertToRelativeForMake(cmd2.c_str());
        std::string cmd;
        std::string wd = ccg.GetWorkingDirectory();
        if(!wd.empty())
          {
          cmd += "cd ";
          cmd += this->ConvertToRelativeForMake(wd.c_str());
          cmd += " && ";
          }
        cmd += cmd2;
        ccg.AppendArguments(c, cmd);
        makefileStream << "\t" << cmd.c_str() << "\n";
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateBuildSettings(cmTarget& target,
                                                cmXCodeObject* buildSettings,
                                                const std::string& configName)
{
  if(target.GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return;
    }

  std::string defFlags;
  bool shared = ((target.GetType() == cmTarget::SHARED_LIBRARY) ||
                 (target.GetType() == cmTarget::MODULE_LIBRARY));
  bool binary = ((target.GetType() == cmTarget::OBJECT_LIBRARY) ||
                 (target.GetType() == cmTarget::STATIC_LIBRARY) ||
                 (target.GetType() == cmTarget::EXECUTABLE) ||
                 shared);

  // Compute the compilation flags for each language.
  std::set<std::string> languages;
  target.GetLanguages(languages, configName);
  std::map<std::string, std::string> cflags;
  for (std::set<std::string>::iterator li = languages.begin();
       li != languages.end(); ++li)
    {
    std::string const& lang = *li;
    std::string& flags = cflags[lang];

    // Add language-specific flags.
    this->CurrentLocalGenerator->AddLanguageFlags(flags, lang, configName);

    // Add shared-library flags if needed.
    this->CurrentLocalGenerator->AddCMP0018Flags(flags, &target,
                                                 lang, configName);

    this->CurrentLocalGenerator->AddVisibilityPresetFlags(flags, &target,
                                                   lang);

    this->CurrentLocalGenerator->
      AddCompileOptions(flags, &target, lang, configName);
    }

  std::string llang = target.GetLinkerLanguage(configName);
  if(binary && llang.empty())
    {
    cmSystemTools::Error
      ("CMake can not determine linker language for target: ",
       target.GetName().c_str());
    return;
    }

  // Add define flags
  this->CurrentLocalGenerator->
    AppendFlags(defFlags,
                this->CurrentMakefile->GetDefineFlags());

  // Add preprocessor definitions for this target and configuration.
  BuildObjectListOrString ppDefs(this, this->XcodeVersion >= 30);
  if(this->XcodeVersion > 15)
    {
    this->AppendDefines(ppDefs,
      "CMAKE_INTDIR=\"$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)\"");
    }
  if(const char* exportMacro = target.GetExportMacro())
    {
    // Add the export symbol definition for shared library objects.
    this->AppendDefines(ppDefs, exportMacro);
    }
  cmGeneratorTarget *gtgt = this->GetGeneratorTarget(&target);
  std::vector<std::string> targetDefines;
  target.GetCompileDefinitions(targetDefines, configName, "C");
  this->AppendDefines(ppDefs, targetDefines);
  buildSettings->AddAttribute
    ("GCC_PREPROCESSOR_DEFINITIONS", ppDefs.CreateList());

  std::string extraLinkOptionsVar;
  std::string extraLinkOptions;
  if(target.GetType() == cmTarget::EXECUTABLE)
    {
    extraLinkOptionsVar = "CMAKE_EXE_LINKER_FLAGS";
    }
  else if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    extraLinkOptionsVar = "CMAKE_SHARED_LINKER_FLAGS";
    }
  else if(target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    extraLinkOptionsVar = "CMAKE_MODULE_LINKER_FLAGS";
    }
  if(extraLinkOptionsVar.size())
    {
    this->CurrentLocalGenerator
      ->AddConfigVariableFlags(extraLinkOptions,
                               extraLinkOptionsVar.c_str(),
                               configName);
    }

  if(target.GetType() == cmTarget::OBJECT_LIBRARY ||
     target.GetType() == cmTarget::STATIC_LIBRARY)
    {
    this->CurrentLocalGenerator
      ->GetStaticLibraryFlags(extraLinkOptions,
                              cmSystemTools::UpperCase(configName),
                              &target);
    }
  else
    {
    const char* targetLinkFlags = target.GetProperty("LINK_FLAGS");
    if(targetLinkFlags)
      {
      this->CurrentLocalGenerator->
        AppendFlags(extraLinkOptions, targetLinkFlags);
      }
    if(!configName.empty())
      {
      std::string linkFlagsVar = "LINK_FLAGS_";
      linkFlagsVar += cmSystemTools::UpperCase(configName);
      if(const char* linkFlags = target.GetProperty(linkFlagsVar.c_str()))
        {
        this->CurrentLocalGenerator->
          AppendFlags(extraLinkOptions, linkFlags);
        }
      }
    }

  // Set target-specific architectures.
  std::vector<std::string> archs;
  gtgt->GetAppleArchs(configName, archs);

  if(!archs.empty())
    {
    // Enable ARCHS attribute.
    buildSettings->AddAttribute("ONLY_ACTIVE_ARCH",
                                this->CreateString("NO"));

    // Store ARCHS value.
    if(archs.size() == 1)
      {
      buildSettings->AddAttribute("ARCHS",
                                  this->CreateString(archs[0].c_str()));
      }
    else
      {
      cmXCodeObject* archObjects =
        this->CreateObject(cmXCodeObject::OBJECT_LIST);
      for(std::vector<std::string>::iterator i = archs.begin();
          i != archs.end(); i++)
        {
        archObjects->AddObject(this->CreateString((*i).c_str()));
        }
      buildSettings->AddAttribute("ARCHS", archObjects);
      }
    }

  // Get the product name components.
  std::string pnprefix;
  std::string pnbase;
  std::string pnsuffix;
  target.GetFullNameComponents(pnprefix, pnbase, pnsuffix, configName);

  const char* version = target.GetProperty("VERSION");
  const char* soversion = target.GetProperty("SOVERSION");
  if(!target.HasSOName(configName) || target.IsFrameworkOnApple())
    {
    version = 0;
    soversion = 0;
    }
  if(version && !soversion)
    {
    soversion = version;
    }
  if(!version && soversion)
    {
    version = soversion;
    }

  std::string realName = pnbase;
  std::string soName = pnbase;
  if(version && soversion)
    {
    realName += ".";
    realName += version;
    soName += ".";
    soName += soversion;
    }

  // Set attributes to specify the proper name for the target.
  std::string pndir = this->CurrentMakefile->GetCurrentBinaryDirectory();
  if(target.GetType() == cmTarget::STATIC_LIBRARY ||
     target.GetType() == cmTarget::SHARED_LIBRARY ||
     target.GetType() == cmTarget::MODULE_LIBRARY ||
     target.GetType() == cmTarget::EXECUTABLE)
    {
    if(this->XcodeVersion >= 21)
      {
      if(!target.UsesDefaultOutputDir(configName, false))
        {
        std::string pncdir = target.GetDirectory(configName);
        buildSettings->AddAttribute("CONFIGURATION_BUILD_DIR",
                                    this->CreateString(pncdir.c_str()));
        }
      }
    else
      {
      buildSettings->AddAttribute("OBJROOT",
                                  this->CreateString(pndir.c_str()));
      pndir = target.GetDirectory(configName);
      }

    if(target.IsFrameworkOnApple() || target.IsCFBundleOnApple())
      {
      pnprefix = "";
      }

    buildSettings->AddAttribute("EXECUTABLE_PREFIX",
                                this->CreateString(pnprefix.c_str()));
    buildSettings->AddAttribute("EXECUTABLE_SUFFIX",
                                this->CreateString(pnsuffix.c_str()));
    }
  else if(target.GetType() == cmTarget::OBJECT_LIBRARY)
    {
    pnprefix = "lib";
    pnbase = target.GetName();
    pnsuffix = ".a";

    if(this->XcodeVersion >= 21)
      {
      std::string pncdir = this->GetObjectsNormalDirectory(
        this->CurrentProject, configName, &target);
      buildSettings->AddAttribute("CONFIGURATION_BUILD_DIR",
                                  this->CreateString(pncdir.c_str()));
      }
    else
      {
      buildSettings->AddAttribute("OBJROOT",
                                  this->CreateString(pndir.c_str()));
      pndir = this->GetObjectsNormalDirectory(
        this->CurrentProject, configName, &target);
      }
    }

  // Store the product name for all target types.
  buildSettings->AddAttribute("PRODUCT_NAME",
                              this->CreateString(realName.c_str()));
  buildSettings->AddAttribute("SYMROOT",
                              this->CreateString(pndir.c_str()));

  // Handle settings for each target type.
  switch(target.GetType())
    {
    case cmTarget::OBJECT_LIBRARY:
    case cmTarget::STATIC_LIBRARY:
    {
    buildSettings->AddAttribute("LIBRARY_STYLE",
                                this->CreateString("STATIC"));
    break;
    }

    case cmTarget::MODULE_LIBRARY:
    {
    buildSettings->AddAttribute("LIBRARY_STYLE",
                                this->CreateString("BUNDLE"));
    if (target.IsCFBundleOnApple())
      {
      // It turns out that a BUNDLE is basically the same
      // in many ways as an application bundle, as far as
      // link flags go
      std::string createFlags =
        this->LookupFlags("CMAKE_SHARED_MODULE_CREATE_", llang, "_FLAGS",
                          "-bundle");
      if(!createFlags.empty())
        {
        extraLinkOptions += " ";
        extraLinkOptions += createFlags;
        }
      std::string plist = this->ComputeInfoPListLocation(target);
      // Xcode will create the final version of Info.plist at build time,
      // so let it replace the cfbundle name. This avoids creating
      // a per-configuration Info.plist file. The cfbundle plist
      // is very similar to the application bundle plist
      this->CurrentLocalGenerator
        ->GenerateAppleInfoPList(&target, "$(EXECUTABLE_NAME)",
                                 plist.c_str());
      std::string path =
        this->ConvertToRelativeForXCode(plist.c_str());
      buildSettings->AddAttribute("INFOPLIST_FILE",
                                  this->CreateString(path.c_str()));
      }
    else if(this->XcodeVersion >= 22)
      {
      buildSettings->AddAttribute("MACH_O_TYPE",
                                  this->CreateString("mh_bundle"));
      buildSettings->AddAttribute("GCC_DYNAMIC_NO_PIC",
                                  this->CreateString("NO"));
      // Add the flags to create an executable.
      std::string createFlags =
        this->LookupFlags("CMAKE_", llang, "_LINK_FLAGS", "");
      if(!createFlags.empty())
        {
        extraLinkOptions += " ";
        extraLinkOptions += createFlags;
        }
      }
    else
      {
      // Add the flags to create a module.
      std::string createFlags =
        this->LookupFlags("CMAKE_SHARED_MODULE_CREATE_", llang, "_FLAGS",
                          "-bundle");
      if(!createFlags.empty())
        {
        extraLinkOptions += " ";
        extraLinkOptions += createFlags;
        }
      }
    break;
    }
    case cmTarget::SHARED_LIBRARY:
    {
    if(target.GetPropertyAsBool("FRAMEWORK"))
      {
      std::string fw_version = target.GetFrameworkVersion();
      buildSettings->AddAttribute("FRAMEWORK_VERSION",
                                  this->CreateString(fw_version.c_str()));

      std::string plist = this->ComputeInfoPListLocation(target);
      // Xcode will create the final version of Info.plist at build time,
      // so let it replace the framework name. This avoids creating
      // a per-configuration Info.plist file.
      this->CurrentLocalGenerator
        ->GenerateFrameworkInfoPList(&target, "$(EXECUTABLE_NAME)",
                                     plist.c_str());
      std::string path =
        this->ConvertToRelativeForXCode(plist.c_str());
      buildSettings->AddAttribute("INFOPLIST_FILE",
                                  this->CreateString(path.c_str()));
      }
    else
      {
      // Add the flags to create a shared library.
      std::string createFlags =
        this->LookupFlags("CMAKE_SHARED_LIBRARY_CREATE_", llang, "_FLAGS",
                          "-dynamiclib");
      if(!createFlags.empty())
        {
        extraLinkOptions += " ";
        extraLinkOptions += createFlags;
        }
      }

    buildSettings->AddAttribute("LIBRARY_STYLE",
                                this->CreateString("DYNAMIC"));
    break;
    }
    case cmTarget::EXECUTABLE:
    {
    // Add the flags to create an executable.
    std::string createFlags =
      this->LookupFlags("CMAKE_", llang, "_LINK_FLAGS", "");
    if(!createFlags.empty())
      {
      extraLinkOptions += " ";
      extraLinkOptions += createFlags;
      }

    // Handle bundles and normal executables separately.
    if(target.GetPropertyAsBool("MACOSX_BUNDLE"))
      {
      std::string plist = this->ComputeInfoPListLocation(target);
      // Xcode will create the final version of Info.plist at build time,
      // so let it replace the executable name.  This avoids creating
      // a per-configuration Info.plist file.
      this->CurrentLocalGenerator
        ->GenerateAppleInfoPList(&target, "$(EXECUTABLE_NAME)",
                                 plist.c_str());
      std::string path =
        this->ConvertToRelativeForXCode(plist.c_str());
      buildSettings->AddAttribute("INFOPLIST_FILE",
                                  this->CreateString(path.c_str()));

      }
    }
    break;
    default:
      break;
    }
  if(this->XcodeVersion >= 22 && this->XcodeVersion < 40)
    {
    buildSettings->AddAttribute("PREBINDING",
                                this->CreateString("NO"));
    }

  BuildObjectListOrString dirs(this, this->XcodeVersion >= 30);
  BuildObjectListOrString fdirs(this, this->XcodeVersion >= 30);
  std::vector<std::string> includes;
  this->CurrentLocalGenerator->GetIncludeDirectories(includes, gtgt,
                                                     "C", configName);
  std::set<std::string> emitted;
  emitted.insert("/System/Library/Frameworks");
  for(std::vector<std::string>::iterator i = includes.begin();
      i != includes.end(); ++i)
    {
    if(this->NameResolvesToFramework(i->c_str()))
      {
      std::string frameworkDir = *i;
      frameworkDir += "/../";
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir.c_str());
      if(emitted.insert(frameworkDir).second)
        {
        fdirs.Add(this->XCodeEscapePath(frameworkDir.c_str()).c_str());
        }
      }
    else
      {
      std::string incpath =
        this->XCodeEscapePath(i->c_str());
      dirs.Add(incpath.c_str());
      }
    }
  // Add framework search paths needed for linking.
  if(cmComputeLinkInformation* cli = target.GetLinkInformation(configName))
    {
    std::vector<std::string> const& fwDirs = cli->GetFrameworkPaths();
    for(std::vector<std::string>::const_iterator fdi = fwDirs.begin();
        fdi != fwDirs.end(); ++fdi)
      {
      if(emitted.insert(*fdi).second)
        {
        fdirs.Add(this->XCodeEscapePath(fdi->c_str()).c_str());
        }
      }
    }
  if(!fdirs.IsEmpty())
    {
    buildSettings->AddAttribute("FRAMEWORK_SEARCH_PATHS",
                                fdirs.CreateList());
    }
  if(!dirs.IsEmpty())
    {
    buildSettings->AddAttribute("HEADER_SEARCH_PATHS",
                                dirs.CreateList());
    }

  bool same_gflags = true;
  std::map<std::string, std::string> gflags;
  std::string const* last_gflag = 0;
  char optLevel[2];
  optLevel[0] = '0';
  optLevel[1] = 0;

  // Minimal map of flags to build settings.
  for (std::set<std::string>::iterator li = languages.begin();
       li != languages.end(); ++li)
    {
    std::string& flags = cflags[*li];
    std::string& gflag = gflags[*li];
    std::string oflag = this->ExtractFlag("-O", flags);
    if(oflag.size() == 3)
      {
      optLevel[0] = oflag[2];
      }
    if(oflag.size() == 2)
      {
      optLevel[0] = '1';
      }
    gflag = this->ExtractFlag("-g", flags);
    // put back gdwarf-2 if used since there is no way
    // to represent it in the gui, but we still want debug yes
    if(gflag == "-gdwarf-2")
      {
      flags += " ";
      flags += gflag;
      }
    if (last_gflag && *last_gflag != gflag)
      {
      same_gflags = false;
      }
    last_gflag = &gflag;
    }

  const char* debugStr = "YES";
  if (!same_gflags)
    {
    // We can't set the Xcode flag differently depending on the language,
    // so put them back in this case.
    for (std::set<std::string>::iterator li = languages.begin();
         li != languages.end(); ++li)
      {
      cflags[*li] += " ";
      cflags[*li] += gflags[*li];
      }
    debugStr = "NO";
    }
  else if (last_gflag && (last_gflag->empty() || *last_gflag == "-g0"))
    {
    debugStr = "NO";
    }

  buildSettings->AddAttribute("COMBINE_HIDPI_IMAGES",
                              this->CreateString("YES"));
  buildSettings->AddAttribute("GCC_GENERATE_DEBUGGING_SYMBOLS",
                              this->CreateString(debugStr));
  buildSettings->AddAttribute("GCC_OPTIMIZATION_LEVEL",
                              this->CreateString(optLevel));
  buildSettings->AddAttribute("GCC_SYMBOLS_PRIVATE_EXTERN",
                              this->CreateString("NO"));
  buildSettings->AddAttribute("GCC_INLINES_ARE_PRIVATE_EXTERN",
                              this->CreateString("NO"));
  for (std::set<std::string>::iterator li = languages.begin();
       li != languages.end(); ++li)
    {
    std::string flags = cflags[*li] + " " + defFlags;
    if (*li == "CXX")
      {
      buildSettings->AddAttribute("OTHER_CPLUSPLUSFLAGS",
                                  this->CreateString(flags.c_str()));
      }
    else if (*li == "Fortran")
      {
      buildSettings->AddAttribute("IFORT_OTHER_FLAGS",
                                  this->CreateString(flags.c_str()));
      }
    else if (*li == "C")
      {
      buildSettings->AddAttribute("OTHER_CFLAGS",
                                  this->CreateString(flags.c_str()));
      }
    }

  // Add Fortran source format attribute if property is set.
  const char* format = 0;
  const char* tgtfmt = target.GetProperty("Fortran_FORMAT");
  switch(this->CurrentLocalGenerator->GetFortranFormat(tgtfmt))
    {
    case cmLocalGenerator::FortranFormatFixed: format = "fixed"; break;
    case cmLocalGenerator::FortranFormatFree: format = "free"; break;
    default: break;
    }
  if(format)
    {
    buildSettings->AddAttribute("IFORT_LANG_SRCFMT",
                                this->CreateString(format));
    }

  // Create the INSTALL_PATH attribute.
  std::string install_name_dir;
  if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    // Get the install_name directory for the build tree.
    install_name_dir = target.GetInstallNameDirForBuildTree(configName);
    // Xcode doesn't create the correct install_name in some cases.
    // That is, if the INSTALL_PATH is empty, or if we have versioning
    // of dylib libraries, we want to specify the install_name.
    // This is done by adding a link flag to create an install_name
    // with just the library soname.
    std::string install_name;
    if(!install_name_dir.empty())
      {
      // Convert to a path for the native build tool.
      cmSystemTools::ConvertToUnixSlashes(install_name_dir);
      install_name += install_name_dir;
      install_name += "/";
      }
    install_name += target.GetSOName(configName);

    if((realName != soName) || install_name_dir.empty())
      {
      install_name_dir = "";
      extraLinkOptions += " -install_name ";
      extraLinkOptions += XCodeEscapePath(install_name.c_str());
      }
    }
  buildSettings->AddAttribute("INSTALL_PATH",
                              this->CreateString(install_name_dir.c_str()));

  // Create the LD_RUNPATH_SEARCH_PATHS
  cmComputeLinkInformation* pcli = target.GetLinkInformation(configName);
  if(pcli)
    {
    std::string search_paths;
    std::vector<std::string> runtimeDirs;
    pcli->GetRPath(runtimeDirs, false);
    // runpath dirs needs to be unique to prevent corruption
    std::set<std::string> unique_dirs;

    for(std::vector<std::string>::const_iterator i = runtimeDirs.begin();
        i != runtimeDirs.end(); ++i)
      {
      std::string runpath = *i;
      runpath = this->ExpandCFGIntDir(runpath, configName);

      if(unique_dirs.find(runpath) == unique_dirs.end())
        {
        unique_dirs.insert(runpath);
        if(!search_paths.empty())
          {
          search_paths += " ";
          }
        search_paths += this->XCodeEscapePath(runpath.c_str());
        }
      }
    if(!search_paths.empty())
      {
      buildSettings->AddAttribute("LD_RUNPATH_SEARCH_PATHS",
                                  this->CreateString(search_paths.c_str()));
      }
    }

  buildSettings->AddAttribute(this->GetTargetLinkFlagsVar(target),
                              this->CreateString(extraLinkOptions.c_str()));
  buildSettings->AddAttribute("OTHER_REZFLAGS",
                              this->CreateString(""));
  buildSettings->AddAttribute("SECTORDER_FLAGS",
                              this->CreateString(""));
  buildSettings->AddAttribute("USE_HEADERMAP",
                              this->CreateString("NO"));
  if (this->XcodeVersion >= 30)
    {
    cmXCodeObject *group = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    group->AddObject(this->CreateString("-Wmost"));
    group->AddObject(this->CreateString("-Wno-four-char-constants"));
    group->AddObject(this->CreateString("-Wno-unknown-pragmas"));
    group->AddObject(this->CreateString("$(inherited)"));
    buildSettings->AddAttribute("WARNING_CFLAGS", group);
    }
  else
    {
    buildSettings->AddAttribute("WARNING_CFLAGS",
                                this->CreateString(
                                  "-Wmost -Wno-four-char-constants"
                                  " -Wno-unknown-pragmas"));
    }

  // Runtime version information.
  if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    int major;
    int minor;
    int patch;

    // VERSION -> current_version
    target.GetTargetVersion(false, major, minor, patch);
    std::ostringstream v;

    // Xcode always wants at least 1.0.0 or nothing
    if(!(major == 0 && minor == 0 && patch == 0))
      {
      v << major << "." << minor << "." << patch;
      }
    buildSettings->AddAttribute("DYLIB_CURRENT_VERSION",
                                this->CreateString(v.str().c_str()));

    // SOVERSION -> compatibility_version
    target.GetTargetVersion(true, major, minor, patch);
    std::ostringstream vso;

    // Xcode always wants at least 1.0.0 or nothing
    if(!(major == 0 && minor == 0 && patch == 0))
      {
      vso << major << "." << minor << "." << patch;
      }
    buildSettings->AddAttribute("DYLIB_COMPATIBILITY_VERSION",
                                this->CreateString(vso.str().c_str()));
    }
  // put this last so it can override existing settings
  // Convert "XCODE_ATTRIBUTE_*" properties directly.
  {
  cmPropertyMap const& props = target.GetProperties();
  for(cmPropertyMap::const_iterator i = props.begin();
      i != props.end(); ++i)
    {
    if(i->first.find("XCODE_ATTRIBUTE_") == 0)
      {
      std::string attribute = i->first.substr(16);
      // Handle [variant=<config>] condition explicitly here.
      std::string::size_type beginVariant =
        attribute.find("[variant=");
      if (beginVariant != std::string::npos)
        {
        std::string::size_type endVariant =
          attribute.find("]", beginVariant+9);
        if (endVariant != std::string::npos)
          {
          // Compare the variant to the configuration.
          std::string variant =
            attribute.substr(beginVariant+9, endVariant-beginVariant-9);
          if (variant == configName)
            {
            // The variant matches the configuration so use this
            // attribute but drop the [variant=<config>] condition.
            attribute.erase(beginVariant, endVariant-beginVariant+1);
            }
          else
            {
            // The variant does not match the configuration so
            // do not use this attribute.
            attribute.clear();
            }
          }
        }

      if (!attribute.empty())
        {
        cmGeneratorExpression ge;
        std::string processed = ge.Parse(i->second.GetValue())
          ->Evaluate(this->CurrentMakefile, configName);
        buildSettings->AddAttribute(attribute.c_str(),
                                    this->CreateString(processed));
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateUtilityTarget(cmTarget& cmtarget)
{
  cmXCodeObject* shellBuildPhase =
    this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
  shellBuildPhase->AddAttribute("buildActionMask",
                                this->CreateString("2147483647"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("files", buildFiles);
  cmXCodeObject* inputPaths = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("inputPaths", inputPaths);
  cmXCodeObject* outputPaths = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("outputPaths", outputPaths);
  shellBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                this->CreateString("0"));
  shellBuildPhase->AddAttribute("shellPath",
                                this->CreateString("/bin/sh"));
  shellBuildPhase->AddAttribute("shellScript",
                                this->CreateString(
                                  "# shell script goes here\nexit 0"));
  shellBuildPhase->AddAttribute("showEnvVarsInLog",
                                this->CreateString("0"));

  cmXCodeObject* target =
    this->CreateObject(cmXCodeObject::PBXAggregateTarget);
  target->SetComment(cmtarget.GetName().c_str());
  cmXCodeObject* buildPhases =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  std::vector<cmXCodeObject*> emptyContentVector;
  this->CreateCustomCommands(buildPhases, 0, 0, 0, emptyContentVector, 0,
                             cmtarget);
  target->AddAttribute("buildPhases", buildPhases);
  if(this->XcodeVersion > 20)
    {
    this->AddConfigurations(target, cmtarget);
    }
  else
    {
    std::string theConfig =
      this->CurrentMakefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
    cmXCodeObject* buildSettings =
      this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    this->CreateBuildSettings(cmtarget, buildSettings, theConfig);
    target->AddAttribute("buildSettings", buildSettings);
    }
  cmXCodeObject* dependencies =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(cmtarget.GetName()));
  target->AddAttribute("productName",this->CreateString(cmtarget.GetName()));
  target->SetTarget(&cmtarget);
  this->XCodeObjectMap[&cmtarget] = target;

  // Add source files without build rules for editing convenience.
  if(cmtarget.GetType() == cmTarget::UTILITY)
    {
    std::vector<cmSourceFile*> sources;
    if (!cmtarget.GetConfigCommonSourceFiles(sources))
      {
      return 0;
      }

    for(std::vector<cmSourceFile*>::const_iterator i = sources.begin();
        i != sources.end(); ++i)
      {
      if(!(*i)->GetPropertyAsBool("GENERATED"))
        {
        this->CreateXCodeFileReference(*i, cmtarget);
        }
      }
    }

  target->SetId(this->GetOrCreateId(
    cmtarget.GetName(), target->GetId()).c_str());

  return target;
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::AddConfigurations(cmXCodeObject* target,
                                                      cmTarget& cmtarget)
{
  std::string configTypes =
    this->CurrentMakefile->GetRequiredDefinition("CMAKE_CONFIGURATION_TYPES");
  std::vector<std::string> configVectorIn;
  std::vector<std::string> configVector;
  configVectorIn.push_back(configTypes);
  cmSystemTools::ExpandList(configVectorIn, configVector);
  cmXCodeObject* configlist =
    this->CreateObject(cmXCodeObject::XCConfigurationList);
  cmXCodeObject* buildConfigurations =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  configlist->AddAttribute("buildConfigurations", buildConfigurations);
  std::string comment = "Build configuration list for ";
  comment += cmXCodeObject::PBXTypeNames[target->GetIsA()];
  comment += " \"";
  comment += cmtarget.GetName();
  comment += "\"";
  configlist->SetComment(comment.c_str());
  target->AddAttribute("buildConfigurationList",
                       this->CreateObjectReference(configlist));
  for(unsigned int i = 0; i < configVector.size(); ++i)
    {
    cmXCodeObject* config =
      this->CreateObject(cmXCodeObject::XCBuildConfiguration);
    buildConfigurations->AddObject(config);
    cmXCodeObject* buildSettings =
      this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    this->CreateBuildSettings(cmtarget, buildSettings,
                              configVector[i].c_str());
    config->AddAttribute("name", this->CreateString(configVector[i].c_str()));
    config->SetComment(configVector[i].c_str());
    config->AddAttribute("buildSettings", buildSettings);
    }
  if(configVector.size())
    {
    configlist->AddAttribute("defaultConfigurationName",
                             this->CreateString(configVector[0].c_str()));
    configlist->AddAttribute("defaultConfigurationIsVisible",
                             this->CreateString("0"));
    return configVector[0];
    }
  return "";
}

//----------------------------------------------------------------------------
const char*
cmGlobalXCodeGenerator::GetTargetLinkFlagsVar(cmTarget const& cmtarget) const
{
  if(this->XcodeVersion >= 60 &&
     (cmtarget.GetType() == cmTarget::STATIC_LIBRARY ||
      cmtarget.GetType() == cmTarget::OBJECT_LIBRARY))
    {
    return "OTHER_LIBTOOLFLAGS";
    }
  else
    {
    return "OTHER_LDFLAGS";
    }
}

//----------------------------------------------------------------------------
const char* cmGlobalXCodeGenerator::GetTargetFileType(cmTarget& cmtarget)
{
  switch(cmtarget.GetType())
    {
    case cmTarget::OBJECT_LIBRARY:
    case cmTarget::STATIC_LIBRARY:
      return "archive.ar";
    case cmTarget::MODULE_LIBRARY:
      if (cmtarget.IsXCTestOnApple())
        return "wrapper.cfbundle";
      else if (cmtarget.IsCFBundleOnApple())
        return "wrapper.plug-in";
      else
        return ((this->XcodeVersion >= 22)?
              "compiled.mach-o.executable" : "compiled.mach-o.dylib");
    case cmTarget::SHARED_LIBRARY:
      return (cmtarget.GetPropertyAsBool("FRAMEWORK")?
              "wrapper.framework" : "compiled.mach-o.dylib");
    case cmTarget::EXECUTABLE:
      return "compiled.mach-o.executable";
    default: break;
    }
  return 0;
}

//----------------------------------------------------------------------------
const char* cmGlobalXCodeGenerator::GetTargetProductType(cmTarget& cmtarget)
{
  switch(cmtarget.GetType())
    {
    case cmTarget::OBJECT_LIBRARY:
    case cmTarget::STATIC_LIBRARY:
      return "com.apple.product-type.library.static";
    case cmTarget::MODULE_LIBRARY:
      if (cmtarget.IsXCTestOnApple())
        return "com.apple.product-type.bundle.unit-test";
      else if (cmtarget.IsCFBundleOnApple())
        return "com.apple.product-type.bundle";
      else
        return ((this->XcodeVersion >= 22)?
                "com.apple.product-type.tool" :
                "com.apple.product-type.library.dynamic");
    case cmTarget::SHARED_LIBRARY:
      return (cmtarget.GetPropertyAsBool("FRAMEWORK")?
              "com.apple.product-type.framework" :
              "com.apple.product-type.library.dynamic");
    case cmTarget::EXECUTABLE:
      return (cmtarget.GetPropertyAsBool("MACOSX_BUNDLE")?
              "com.apple.product-type.application" :
              "com.apple.product-type.tool");
    default: break;
    }
  return 0;
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateXCodeTarget(cmTarget& cmtarget,
                                          cmXCodeObject* buildPhases)
{
  if(cmtarget.GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return 0;
    }
  cmXCodeObject* target =
    this->CreateObject(cmXCodeObject::PBXNativeTarget);
  target->AddAttribute("buildPhases", buildPhases);
  cmXCodeObject* buildRules = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("buildRules", buildRules);
  std::string defConfig;
  if(this->XcodeVersion > 20)
    {
    defConfig = this->AddConfigurations(target, cmtarget);
    }
  else
    {
    cmXCodeObject* buildSettings =
      this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    defConfig = this->CurrentMakefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
    this->CreateBuildSettings(cmtarget, buildSettings, defConfig.c_str());
    target->AddAttribute("buildSettings", buildSettings);
    }
  cmXCodeObject* dependencies =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(cmtarget.GetName()));
  target->AddAttribute("productName",this->CreateString(cmtarget.GetName()));

  cmXCodeObject* fileRef =
    this->CreateObject(cmXCodeObject::PBXFileReference);
  if(const char* fileType = this->GetTargetFileType(cmtarget))
    {
    fileRef->AddAttribute("explicitFileType", this->CreateString(fileType));
    }
  std::string fullName;
  if(cmtarget.GetType() == cmTarget::OBJECT_LIBRARY)
    {
    fullName = "lib";
    fullName += cmtarget.GetName();
    fullName += ".a";
    }
  else
    {
    fullName = cmtarget.GetFullName(defConfig.c_str());
    }
  fileRef->AddAttribute("path", this->CreateString(fullName.c_str()));
  fileRef->AddAttribute("refType", this->CreateString("0"));
  fileRef->AddAttribute("sourceTree",
                        this->CreateString("BUILT_PRODUCTS_DIR"));
  fileRef->SetComment(cmtarget.GetName().c_str());
  target->AddAttribute("productReference",
                       this->CreateObjectReference(fileRef));
  if(const char* productType = this->GetTargetProductType(cmtarget))
    {
    target->AddAttribute("productType", this->CreateString(productType));
    }
  target->SetTarget(&cmtarget);
  this->XCodeObjectMap[&cmtarget] = target;
  target->SetId(this->GetOrCreateId(
    cmtarget.GetName(), target->GetId()).c_str());
  return target;
}

//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator::FindXCodeTarget(cmTarget const* t)
{
  if(!t)
    {
    return 0;
    }

  std::map<cmTarget const*, cmXCodeObject*>::const_iterator const i =
    this->XCodeObjectMap.find(t);
  if (i == this->XCodeObjectMap.end())
    {
    return 0;
    }
  return i->second;
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::GetOrCreateId(const std::string& name,
                                                  const std::string& id)
{
  std::string guidStoreName = name;
  guidStoreName += "_GUID_CMAKE";
  const char* storedGUID =
    this->CMakeInstance->GetCacheDefinition(guidStoreName.c_str());

  if(storedGUID)
    {
    return storedGUID;
    }

  this->CMakeInstance->AddCacheEntry(guidStoreName.c_str(),
    id.c_str(), "Stored Xcode object GUID", cmState::INTERNAL);

  return id;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AddDependTarget(cmXCodeObject* target,
                                             cmXCodeObject* dependTarget)
{
  // This is called once for every edge in the target dependency graph.
  cmXCodeObject* container =
    this->CreateObject(cmXCodeObject::PBXContainerItemProxy);
  container->SetComment("PBXContainerItemProxy");
  container->AddAttribute("containerPortal",
                          this->CreateObjectReference(this->RootObject));
  container->AddAttribute("proxyType", this->CreateString("1"));
  container->AddAttribute("remoteGlobalIDString",
                          this->CreateObjectReference(dependTarget));
  container->AddAttribute("remoteInfo",
                          this->CreateString(
                            dependTarget->GetTarget()->GetName()));
  cmXCodeObject* targetdep =
    this->CreateObject(cmXCodeObject::PBXTargetDependency);
  targetdep->SetComment("PBXTargetDependency");
  targetdep->AddAttribute("target",
                          this->CreateObjectReference(dependTarget));
  targetdep->AddAttribute("targetProxy",
                          this->CreateObjectReference(container));

  cmXCodeObject* depends = target->GetObject("dependencies");
  if(!depends)
    {
    cmSystemTools::
      Error("target does not have dependencies attribute error..");

    }
  else
    {
    depends->AddUniqueObject(targetdep);
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AppendOrAddBuildSetting(cmXCodeObject* settings,
                                                     const char* attribute,
                                                     const char* value)
{
  if(settings)
    {
    cmXCodeObject* attr = settings->GetObject(attribute);
    if(!attr)
      {
      settings->AddAttribute(attribute, this->CreateString(value));
      }
    else
      {
      std::string oldValue = attr->GetString();
      oldValue += " ";
      oldValue += value;
      attr->SetString(oldValue.c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator
::AppendBuildSettingAttribute(cmXCodeObject* target,
                              const char* attribute,
                              const char* value,
                              const std::string& configName)
{
  if(this->XcodeVersion < 21)
    {
    // There is only one configuration.  Add the setting to the buildSettings
    // of the target.
    this->AppendOrAddBuildSetting(target->GetObject("buildSettings"),
                                  attribute, value);
    }
  else
    {
    // There are multiple configurations.  Add the setting to the
    // buildSettings of the configuration name given.
    cmXCodeObject* configurationList =
      target->GetObject("buildConfigurationList")->GetObject();
    cmXCodeObject* buildConfigs =
      configurationList->GetObject("buildConfigurations");
    std::vector<cmXCodeObject*> list = buildConfigs->GetObjectList();
    // each configuration and the target itself has a buildSettings in it
    //list.push_back(target);
    for(std::vector<cmXCodeObject*>::iterator i = list.begin();
        i != list.end(); ++i)
      {
      if(!configName.empty())
        {
        if((*i)->GetObject("name")->GetString() == configName)
          {
          cmXCodeObject* settings = (*i)->GetObject("buildSettings");
          this->AppendOrAddBuildSetting(settings, attribute, value);
          }
        }
      else
        {
        cmXCodeObject* settings = (*i)->GetObject("buildSettings");
        this->AppendOrAddBuildSetting(settings, attribute, value);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator
::AddDependAndLinkInformation(cmXCodeObject* target)
{
  cmTarget* cmtarget = target->GetTarget();
  if(cmtarget->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return;
    }
  if(!cmtarget)
    {
    cmSystemTools::Error("Error no target on xobject\n");
    return;
    }

  // Add dependencies on other CMake targets.
  TargetDependSet const& deps = this->GetTargetDirectDepends(*cmtarget);
  for(TargetDependSet::const_iterator i = deps.begin(); i != deps.end(); ++i)
    {
    if(cmXCodeObject* dptarget = this->FindXCodeTarget(*i))
      {
      this->AddDependTarget(target, dptarget);
      }
    }

  // Loop over configuration types and set per-configuration info.
  for(std::vector<std::string>::iterator i =
        this->CurrentConfigurationTypes.begin();
      i != this->CurrentConfigurationTypes.end(); ++i)
    {
    // Get the current configuration name.
    std::string configName = *i;

    if(this->XcodeVersion >= 50)
      {
      // Add object library contents as link flags.
      std::string linkObjs;
      const char* sep = "";
      std::vector<std::string> objs;
      this->GetGeneratorTarget(cmtarget)->UseObjectLibraries(objs, "");
      for(std::vector<std::string>::const_iterator
            oi = objs.begin(); oi != objs.end(); ++oi)
        {
        linkObjs += sep;
        sep = " ";
        linkObjs += this->XCodeEscapePath(oi->c_str());
        }
      this->AppendBuildSettingAttribute(
        target, this->GetTargetLinkFlagsVar(*cmtarget),
        linkObjs.c_str(), configName);
      }

    // Skip link information for object libraries.
    if(cmtarget->GetType() == cmTarget::OBJECT_LIBRARY ||
       cmtarget->GetType() == cmTarget::STATIC_LIBRARY)
      {
      continue;
      }

    // Compute the link library and directory information.
    cmComputeLinkInformation* pcli = cmtarget->GetLinkInformation(configName);
    if(!pcli)
      {
      continue;
      }
    cmComputeLinkInformation& cli = *pcli;

    // Add dependencies directly on library files.
    {
    std::vector<std::string> const& libDeps = cli.GetDepends();
    for(std::vector<std::string>::const_iterator j = libDeps.begin();
        j != libDeps.end(); ++j)
      {
      target->AddDependLibrary(configName, j->c_str());
      }
    }

    // add the library search paths
    {
    std::vector<std::string> const& libDirs = cli.GetDirectories();
    std::string linkDirs;
    for(std::vector<std::string>::const_iterator libDir = libDirs.begin();
        libDir != libDirs.end(); ++libDir)
      {
      if(libDir->size() && *libDir != "/usr/lib")
        {
        if(this->XcodeVersion > 15)
          {
          // Now add the same one but append
          // $(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME) to it:
          linkDirs += " ";
          linkDirs += this->XCodeEscapePath(
            (*libDir + "/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)").c_str());
          }
        linkDirs += " ";
        linkDirs += this->XCodeEscapePath(libDir->c_str());
        }
      }
    this->AppendBuildSettingAttribute(target, "LIBRARY_SEARCH_PATHS",
                                      linkDirs.c_str(), configName);
    }

    // now add the link libraries
    {
    std::string linkLibs;
    const char* sep = "";
    typedef cmComputeLinkInformation::ItemVector ItemVector;
    ItemVector const& libNames = cli.GetItems();
    for(ItemVector::const_iterator li = libNames.begin();
        li != libNames.end(); ++li)
      {
      linkLibs += sep;
      sep = " ";
      if(li->IsPath)
        {
        linkLibs += this->XCodeEscapePath(li->Value.c_str());
        }
      else if (!li->Target
          || li->Target->GetType() != cmTarget::INTERFACE_LIBRARY)
        {
        linkLibs += li->Value;
        }
      if(li->Target && !li->Target->IsImported())
        {
        target->AddDependTarget(configName, li->Target->GetName());
        }
      }
    this->AppendBuildSettingAttribute(
      target, this->GetTargetLinkFlagsVar(*cmtarget),
      linkLibs.c_str(), configName);
    }
    }
}

//----------------------------------------------------------------------------
bool cmGlobalXCodeGenerator::CreateGroups(cmLocalGenerator* root,
                                          std::vector<cmLocalGenerator*>&
                                          generators)
{
  for(std::vector<cmLocalGenerator*>::iterator i = generators.begin();
      i != generators.end(); ++i)
    {
    if(this->IsExcluded(root, *i))
      {
      continue;
      }
    cmMakefile* mf = (*i)->GetMakefile();
    std::vector<cmSourceGroup> sourceGroups = mf->GetSourceGroups();
    cmTargets &tgts = mf->GetTargets();
    for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
      {
      cmTarget& cmtarget = l->second;

      // Same skipping logic here as in CreateXCodeTargets so that we do not
      // end up with (empty anyhow) ALL_BUILD and XCODE_DEPEND_HELPER source
      // groups:
      //
      if(cmtarget.GetType() == cmTarget::GLOBAL_TARGET)
        {
        continue;
        }
      if(cmtarget.GetType() == cmTarget::INTERFACE_LIBRARY)
        {
        continue;
        }

      // add the soon to be generated Info.plist file as a source for a
      // MACOSX_BUNDLE file
      if(cmtarget.GetPropertyAsBool("MACOSX_BUNDLE"))
        {
        std::string plist = this->ComputeInfoPListLocation(cmtarget);
        mf->GetOrCreateSource(plist, true);
        cmtarget.AddSource(plist);
        }

      std::vector<cmSourceFile*> classes;
      if (!cmtarget.GetConfigCommonSourceFiles(classes))
        {
        return false;
        }
      // Put cmSourceFile instances in proper groups:
      for(std::vector<cmSourceFile*>::const_iterator s = classes.begin();
          s != classes.end(); s++)
        {
        cmSourceFile* sf = *s;
        // Add the file to the list of sources.
        std::string const& source = sf->GetFullPath();
        cmSourceGroup* sourceGroup =
          mf->FindSourceGroup(source.c_str(), sourceGroups);
        cmXCodeObject* pbxgroup =
          this->CreateOrGetPBXGroup(cmtarget, sourceGroup);
        std::string key = GetGroupMapKey(cmtarget, sf);
        this->GroupMap[key] = pbxgroup;
        }

      // Put OBJECT_LIBRARY objects in proper groups:
      std::vector<std::string> objs;
      this->GetGeneratorTarget(&cmtarget)->UseObjectLibraries(objs, "");
      for(std::vector<std::string>::const_iterator
            oi = objs.begin(); oi != objs.end(); ++oi)
        {
        std::string const& source = *oi;
        cmSourceGroup* sourceGroup =
          mf->FindSourceGroup(source.c_str(), sourceGroups);
        cmXCodeObject* pbxgroup =
          this->CreateOrGetPBXGroup(cmtarget, sourceGroup);
        std::string key = GetGroupMapKeyFromPath(cmtarget, source);
        this->GroupMap[key] = pbxgroup;
        }
      }
    }
  return true;
}

cmXCodeObject *cmGlobalXCodeGenerator
::CreatePBXGroup(cmXCodeObject *parent, std::string name)
{
  cmXCodeObject* parentChildren = NULL;
  if(parent)
    parentChildren = parent->GetObject("children");
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::PBXGroup);
  cmXCodeObject* groupChildren =
  this->CreateObject(cmXCodeObject::OBJECT_LIST);
  group->AddAttribute("name", this->CreateString(name.c_str()));
  group->AddAttribute("children", groupChildren);
  if(this->XcodeVersion == 15)
  {
    group->AddAttribute("refType", this->CreateString("4"));
  }
  group->AddAttribute("sourceTree", this->CreateString("<group>"));
  if(parentChildren)
    parentChildren->AddObject(group);
  return group;
}

//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator
::CreateOrGetPBXGroup(cmTarget& cmtarget, cmSourceGroup* sg)
{
  std::string s;
  std::string target;
  const char *targetFolder= cmtarget.GetProperty("FOLDER");
  if(targetFolder) {
    target = targetFolder;
    target += "/";
  }
  target += cmtarget.GetName();
  s = target + "/";
  s += sg->GetFullName();
  std::map<std::string, cmXCodeObject* >::iterator it =
    this->GroupNameMap.find(s);
  if(it != this->GroupNameMap.end())
    {
    return it->second;
    }

  it = this->TargetGroup.find(target);
  cmXCodeObject* tgroup = 0;
  if(it != this->TargetGroup.end())
    {
    tgroup = it->second;
    }
  else
    {
    std::vector<std::string> tgt_folders =
      cmSystemTools::tokenize(target, "/");
    std::string curr_tgt_folder;
    for(std::vector<std::string>::size_type i = 0; i < tgt_folders.size();i++)
      {
      if (i != 0)
        {
        curr_tgt_folder += "/";
        }
      curr_tgt_folder += tgt_folders[i];
      it = this->TargetGroup.find(curr_tgt_folder);
      if(it != this->TargetGroup.end())
        {
        tgroup = it->second;
        continue;
        }
      tgroup = this->CreatePBXGroup(tgroup,tgt_folders[i]);
      this->TargetGroup[curr_tgt_folder] = tgroup;
      if(i == 0)
        {
        this->SourcesGroupChildren->AddObject(tgroup);
        }
      }
    }
  this->TargetGroup[target] = tgroup;

  // If it's the default source group (empty name) then put the source file
  // directly in the tgroup...
  //
  if (std::string(sg->GetFullName()) == "")
    {
    this->GroupNameMap[s] = tgroup;
    return tgroup;
    }

  //It's a recursive folder structure, let's find the real parent group
  if(std::string(sg->GetFullName()) != std::string(sg->GetName()))
    {
    std::vector<std::string> folders =
      cmSystemTools::tokenize(sg->GetFullName(), "\\");
    std::string curr_folder = target;
    curr_folder += "/";
    for(std::vector<std::string>::size_type i = 0; i < folders.size();i++)
      {
      curr_folder += folders[i];
      std::map<std::string, cmXCodeObject* >::iterator i_folder =
        this->GroupNameMap.find(curr_folder);
      //Create new folder
      if(i_folder == this->GroupNameMap.end())
        {
        cmXCodeObject *group = this->CreatePBXGroup(tgroup,folders[i]);
        this->GroupNameMap[curr_folder] = group;
        tgroup = group;
        }
      else
        {
        tgroup = i_folder->second;
        }
      curr_folder = curr_folder + "\\";
      }
    return tgroup;
    }
  cmXCodeObject *group = this->CreatePBXGroup(tgroup,sg->GetName());
  this->GroupNameMap[s] = group;
  return group;
}

//----------------------------------------------------------------------------
bool cmGlobalXCodeGenerator
::CreateXCodeObjects(cmLocalGenerator* root,
                     std::vector<cmLocalGenerator*>&
                     generators)
{
  this->ClearXCodeObjects();
  this->RootObject = 0;
  this->SourcesGroupChildren = 0;
  this->ResourcesGroupChildren = 0;
  this->MainGroupChildren = 0;
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("NO"));
  cmXCodeObject* developBuildStyle =
    this->CreateObject(cmXCodeObject::PBXBuildStyle);
  cmXCodeObject* listObjs = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  if(this->XcodeVersion == 15)
    {
    developBuildStyle->AddAttribute("name",
                                    this->CreateString("Development"));
    developBuildStyle->AddAttribute("buildSettings", group);
    listObjs->AddObject(developBuildStyle);
    group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("YES"));
    cmXCodeObject* deployBuildStyle =
    this->CreateObject(cmXCodeObject::PBXBuildStyle);
    deployBuildStyle->AddAttribute("name", this->CreateString("Deployment"));
    deployBuildStyle->AddAttribute("buildSettings", group);
    listObjs->AddObject(deployBuildStyle);
    }
  else
    {
    for(unsigned int i = 0; i < this->CurrentConfigurationTypes.size(); ++i)
      {
      cmXCodeObject* buildStyle =
        this->CreateObject(cmXCodeObject::PBXBuildStyle);
      const char* name = this->CurrentConfigurationTypes[i].c_str();
      buildStyle->AddAttribute("name", this->CreateString(name));
      buildStyle->SetComment(name);
      cmXCodeObject* sgroup =
        this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
      sgroup->AddAttribute("COPY_PHASE_STRIP", this->CreateString("NO"));
      buildStyle->AddAttribute("buildSettings", sgroup);
      listObjs->AddObject(buildStyle);
      }
    }

  cmXCodeObject* mainGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  this->MainGroupChildren =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  mainGroup->AddAttribute("children", this->MainGroupChildren);
  if(this->XcodeVersion == 15)
    {
    mainGroup->AddAttribute("refType", this->CreateString("4"));
    }
  mainGroup->AddAttribute("sourceTree", this->CreateString("<group>"));

  cmXCodeObject* sourcesGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  this->SourcesGroupChildren =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  sourcesGroup->AddAttribute("name", this->CreateString("Sources"));
  sourcesGroup->AddAttribute("children", this->SourcesGroupChildren);
  if(this->XcodeVersion == 15)
    {
    sourcesGroup->AddAttribute("refType", this->CreateString("4"));
    }
  sourcesGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  this->MainGroupChildren->AddObject(sourcesGroup);

  cmXCodeObject* resourcesGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  this->ResourcesGroupChildren =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  resourcesGroup->AddAttribute("name", this->CreateString("Resources"));
  resourcesGroup->AddAttribute("children", this->ResourcesGroupChildren);
  if(this->XcodeVersion == 15)
    {
    resourcesGroup->AddAttribute("refType", this->CreateString("4"));
    }
  resourcesGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  this->MainGroupChildren->AddObject(resourcesGroup);

  // now create the cmake groups
  if (!this->CreateGroups(root, generators))
    {
    return false;
    }

  cmXCodeObject* productGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  productGroup->AddAttribute("name", this->CreateString("Products"));
  if(this->XcodeVersion == 15)
    {
    productGroup->AddAttribute("refType", this->CreateString("4"));
    }
  productGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  cmXCodeObject* productGroupChildren =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  productGroup->AddAttribute("children", productGroupChildren);
  this->MainGroupChildren->AddObject(productGroup);


  this->RootObject = this->CreateObject(cmXCodeObject::PBXProject);
  this->RootObject->SetComment("Project object");

  std::string project_id = "PROJECT_";
  project_id += root->GetMakefile()->GetProjectName();
  this->RootObject->SetId(this->GetOrCreateId(
    project_id.c_str(), this->RootObject->GetId()).c_str());

  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  this->RootObject->AddAttribute("mainGroup",
                             this->CreateObjectReference(mainGroup));
  this->RootObject->AddAttribute("buildSettings", group);
  this->RootObject->AddAttribute("buildStyles", listObjs);
  this->RootObject->AddAttribute("hasScannedForEncodings",
                             this->CreateString("0"));
  if (this->XcodeVersion >= 30)
    {
    group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    group->AddAttribute("BuildIndependentTargetsInParallel",
                        this->CreateString("YES"));
    this->RootObject->AddAttribute("attributes", group);
    if (this->XcodeVersion >= 32)
      this->RootObject->AddAttribute("compatibilityVersion",
                                     this->CreateString("Xcode 3.2"));
    else if (this->XcodeVersion >= 31)
      this->RootObject->AddAttribute("compatibilityVersion",
                                     this->CreateString("Xcode 3.1"));
    else
      this->RootObject->AddAttribute("compatibilityVersion",
                                     this->CreateString("Xcode 3.0"));
    }
  // Point Xcode at the top of the source tree.
  {
  std::string pdir =
    this->RelativeToBinary(root->GetMakefile()->GetCurrentSourceDirectory());
  this->RootObject->AddAttribute("projectDirPath",
                                 this->CreateString(pdir.c_str()));
  this->RootObject->AddAttribute("projectRoot", this->CreateString(""));
  }
  cmXCodeObject* configlist =
    this->CreateObject(cmXCodeObject::XCConfigurationList);
  cmXCodeObject* buildConfigurations =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  std::vector<cmXCodeObject*> configs;
  const char *defaultConfigName = "Debug";
  if(this->XcodeVersion == 15)
    {
    cmXCodeObject* configDebug =
      this->CreateObject(cmXCodeObject::XCBuildConfiguration);
    configDebug->AddAttribute("name", this->CreateString("Debug"));
    configs.push_back(configDebug);
    cmXCodeObject* configRelease =
      this->CreateObject(cmXCodeObject::XCBuildConfiguration);
    configRelease->AddAttribute("name", this->CreateString("Release"));
    configs.push_back(configRelease);
    }
  else
    {
    for(unsigned int i = 0; i < this->CurrentConfigurationTypes.size(); ++i)
      {
      const char* name = this->CurrentConfigurationTypes[i].c_str();
      if (0 == i)
        {
        defaultConfigName = name;
        }
      cmXCodeObject* config =
        this->CreateObject(cmXCodeObject::XCBuildConfiguration);
      config->AddAttribute("name", this->CreateString(name));
      configs.push_back(config);
      }
    }
  for(std::vector<cmXCodeObject*>::iterator c = configs.begin();
      c != configs.end(); ++c)
    {
    buildConfigurations->AddObject(*c);
    }
  configlist->AddAttribute("buildConfigurations", buildConfigurations);

  std::string comment = "Build configuration list for PBXProject";
  comment += " \"";
  comment += this->CurrentProject;
  comment += "\"";
  configlist->SetComment(comment.c_str());
  configlist->AddAttribute("defaultConfigurationIsVisible",
                           this->CreateString("0"));
  configlist->AddAttribute("defaultConfigurationName",
                           this->CreateString(defaultConfigName));
  cmXCodeObject* buildSettings =
      this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  const char* osxArch =
      this->CurrentMakefile->GetDefinition("CMAKE_OSX_ARCHITECTURES");
  const char* sysroot =
      this->CurrentMakefile->GetDefinition("CMAKE_OSX_SYSROOT");
  const char* deploymentTarget =
    this->CurrentMakefile->GetDefinition("CMAKE_OSX_DEPLOYMENT_TARGET");
  std::string archs;
  if(sysroot)
    {
    if(osxArch)
      {
      // recompute this as it may have been changed since enable language
      this->Architectures.clear();
      cmSystemTools::ExpandListArgument(std::string(osxArch),
                                        this->Architectures);
      archs = cmJoin(this->Architectures, " ");
      }
    buildSettings->AddAttribute("SDKROOT",
                                this->CreateString(sysroot));
    }
  if (archs.empty())
    {
    // Tell Xcode to use NATIVE_ARCH instead of ARCHS.
    buildSettings->AddAttribute("ONLY_ACTIVE_ARCH", this->CreateString("YES"));
    }
  else
    {
    // Tell Xcode to use ARCHS (ONLY_ACTIVE_ARCH defaults to NO).
    buildSettings->AddAttribute("ARCHS", this->CreateString(archs.c_str()));
    }
  if(deploymentTarget && *deploymentTarget)
    {
    buildSettings->AddAttribute("MACOSX_DEPLOYMENT_TARGET",
                                this->CreateString(deploymentTarget));
    }
  if(!this->GeneratorToolset.empty())
    {
    buildSettings->AddAttribute("GCC_VERSION",
      this->CreateString(this->GeneratorToolset.c_str()));
    }

  // Put this last so it can override existing settings
  // Convert "CMAKE_XCODE_ATTRIBUTE_*" variables directly.
  {
    std::vector<std::string> vars = this->CurrentMakefile->GetDefinitions();
    for(std::vector<std::string>::const_iterator i = vars.begin();
        i != vars.end(); ++i)
    {
      if(i->find("CMAKE_XCODE_ATTRIBUTE_") == 0)
      {
        buildSettings->AddAttribute(i->substr(22).c_str(),
          this->CreateString(
            this->CurrentMakefile->GetDefinition(i->c_str())));
      }
    }
  }

  std::string symroot = root->GetMakefile()->GetCurrentBinaryDirectory();
  symroot += "/build";
  buildSettings->AddAttribute("SYMROOT", this->CreateString(symroot.c_str()));

  for( std::vector<cmXCodeObject*>::iterator i = configs.begin();
       i != configs.end(); ++i)
    {
    (*i)->AddAttribute("buildSettings", buildSettings);
    }

  this->RootObject->AddAttribute("buildConfigurationList",
                             this->CreateObjectReference(configlist));

  std::vector<cmXCodeObject*> targets;
  for(std::vector<cmLocalGenerator*>::iterator i = generators.begin();
      i != generators.end(); ++i)
    {
    if(!this->IsExcluded(root, *i))
      {
      if (!this->CreateXCodeTargets(*i, targets))
        {
        return false;
        }
      }
    }
  // loop over all targets and add link and depend info
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    {
    cmXCodeObject* t = *i;
    this->AddDependAndLinkInformation(t);
    }
  if(this->XcodeVersion < 50)
    {
    // now create xcode depend hack makefile
    this->CreateXCodeDependHackTarget(targets);
    }
  // now add all targets to the root object
  cmXCodeObject* allTargets = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    {
    cmXCodeObject* t = *i;
    allTargets->AddObject(t);
    cmXCodeObject* productRef = t->GetObject("productReference");
    if(productRef)
      {
      productGroupChildren->AddObject(productRef->GetObject());
      }
    }
  this->RootObject->AddAttribute("targets", allTargets);
  return true;
}

//----------------------------------------------------------------------------
std::string
cmGlobalXCodeGenerator::GetObjectsNormalDirectory(
  const std::string &projName,
  const std::string &configName,
  const cmTarget *t) const
{
  std::string dir =
    t->GetMakefile()->GetCurrentBinaryDirectory();
  dir += "/";
  dir += projName;
  dir += ".build/";
  dir += configName;
  dir += "/";
  dir += t->GetName();
  dir += ".build/Objects-normal/";

  return dir;
}

//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator::CreateXCodeDependHackTarget(
  std::vector<cmXCodeObject*>& targets)
{
  cmGeneratedFileStream
    makefileStream(this->CurrentXCodeHackMakefile.c_str());
  if(!makefileStream)
    {
    cmSystemTools::Error("Could not create",
                         this->CurrentXCodeHackMakefile.c_str());
    return;
    }
  makefileStream.SetCopyIfDifferent(true);
  // one more pass for external depend information not handled
  // correctly by xcode
  makefileStream << "# DO NOT EDIT\n";
  makefileStream << "# This makefile makes sure all linkable targets are\n";
  makefileStream << "# up-to-date with anything they link to\n"
    "default:\n"
    "\techo \"Do not invoke directly\"\n"
    "\n";
  makefileStream
    << "# For each target create a dummy rule "
    "so the target does not have to exist\n";
  std::set<std::string> emitted;
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    {
    cmXCodeObject* target = *i;
    std::map<std::string, cmXCodeObject::StringVec> const& deplibs =
      target->GetDependLibraries();
    for(std::map<std::string, cmXCodeObject::StringVec>::const_iterator ci
          = deplibs.begin(); ci != deplibs.end(); ++ci)
      {
      for(cmXCodeObject::StringVec::const_iterator d = ci->second.begin();
          d != ci->second.end(); ++d)
        {
        if(emitted.insert(*d).second)
          {
          makefileStream <<
            this->ConvertToRelativeForMake(d->c_str()) << ":\n";
          }
        }
      }
    }
  makefileStream << "\n\n";

  // Write rules to help Xcode relink things at the right time.
  makefileStream <<
    "# Rules to remove targets that are older than anything to which they\n"
    "# link.  This forces Xcode to relink the targets from scratch.  It\n"
    "# does not seem to check these dependencies itself.\n";
  for(std::vector<std::string>::const_iterator
        ct = this->CurrentConfigurationTypes.begin();
      ct != this->CurrentConfigurationTypes.end(); ++ct)
    {
    std::string configName = *ct;
    for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
        i != targets.end(); ++i)
      {
      cmXCodeObject* target = *i;
      cmTarget* t =target->GetTarget();

      if(t->GetType() == cmTarget::EXECUTABLE ||
// Nope - no post-build for OBJECT_LIRBRARY
//         t->GetType() == cmTarget::OBJECT_LIBRARY ||
         t->GetType() == cmTarget::STATIC_LIBRARY ||
         t->GetType() == cmTarget::SHARED_LIBRARY ||
         t->GetType() == cmTarget::MODULE_LIBRARY)
        {
        // Declare an entry point for the target post-build phase.
        makefileStream << this->PostBuildMakeTarget(t->GetName(), *ct)
                       << ":\n";
        }

      if(t->GetType() == cmTarget::EXECUTABLE ||
         t->GetType() == cmTarget::SHARED_LIBRARY ||
         t->GetType() == cmTarget::MODULE_LIBRARY)
        {
        std::string tfull = t->GetFullPath(configName);
        std::string trel = this->ConvertToRelativeForMake(tfull.c_str());

        // Add this target to the post-build phases of its dependencies.
        std::map<std::string, cmXCodeObject::StringVec>::const_iterator
          y = target->GetDependTargets().find(*ct);
        if(y != target->GetDependTargets().end())
          {
          std::vector<std::string> const& deptgts = y->second;
          for(std::vector<std::string>::const_iterator d = deptgts.begin();
              d != deptgts.end(); ++d)
            {
            makefileStream << this->PostBuildMakeTarget(*d, *ct) << ": "
                           << trel << "\n";
            }
          }

        // Create a rule for this target.
        makefileStream << trel << ":";

        // List dependencies if any exist.
        std::map<std::string, cmXCodeObject::StringVec>::const_iterator
          x = target->GetDependLibraries().find(*ct);
        if(x != target->GetDependLibraries().end())
          {
          std::vector<std::string> const& deplibs = x->second;
          for(std::vector<std::string>::const_iterator d = deplibs.begin();
              d != deplibs.end(); ++d)
            {
            makefileStream << "\\\n\t" <<
              this->ConvertToRelativeForMake(d->c_str());
            }
          }
        // Write the action to remove the target if it is out of date.
        makefileStream << "\n";
        makefileStream << "\t/bin/rm -f "
                       << this->ConvertToRelativeForMake(tfull.c_str())
                       << "\n";
        // if building for more than one architecture
        // then remove those exectuables as well
        if(this->Architectures.size() > 1)
          {
          std::string universal = this->GetObjectsNormalDirectory(
            this->CurrentProject, configName, t);
          for( std::vector<std::string>::iterator arch =
                 this->Architectures.begin();
               arch != this->Architectures.end(); ++arch)
            {
            std::string universalFile = universal;
            universalFile += *arch;
            universalFile += "/";
            universalFile += t->GetFullName(configName);
            makefileStream << "\t/bin/rm -f "
                           <<
              this->ConvertToRelativeForMake(universalFile.c_str())
                           << "\n";
            }
          }
        makefileStream << "\n\n";
        }
      }
    }
}

//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator::OutputXCodeProject(cmLocalGenerator* root,
                                           std::vector<cmLocalGenerator*>&
                                           generators)
{
  if(generators.size() == 0)
    {
    return;
    }
  // Skip local generators that are excluded from this project.
  for(std::vector<cmLocalGenerator*>::iterator g = generators.begin();
      g != generators.end(); ++g)
    {
    if(this->IsExcluded(root, *g))
      {
      continue;
      }
    }

  if (!this->CreateXCodeObjects(root, generators))
    {
    return;
    }
  std::string xcodeDir = root->GetMakefile()->GetCurrentBinaryDirectory();
  xcodeDir += "/";
  xcodeDir += root->GetMakefile()->GetProjectName();
  xcodeDir += ".xcode";
  if(this->XcodeVersion > 20)
    {
    xcodeDir += "proj";
    }
  cmSystemTools::MakeDirectory(xcodeDir.c_str());
  std::string xcodeProjFile = xcodeDir + "/project.pbxproj";
  cmGeneratedFileStream fout(xcodeProjFile.c_str());
  fout.SetCopyIfDifferent(true);
  if(!fout)
    {
    return;
    }
  this->WriteXCodePBXProj(fout, root, generators);
  this->ClearXCodeObjects();

  // Since this call may have created new cache entries, save the cache:
  //
  root->GetMakefile()->GetCMakeInstance()->SaveCache(
    root->GetMakefile()->GetHomeOutputDirectory());
}

//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator::WriteXCodePBXProj(std::ostream& fout,
                                          cmLocalGenerator* ,
                                          std::vector<cmLocalGenerator*>& )
{
  SortXCodeObjects();

  fout << "// !$*UTF8*$!\n";
  fout << "{\n";
  cmXCodeObject::Indent(1, fout);
  fout << "archiveVersion = 1;\n";
  cmXCodeObject::Indent(1, fout);
  fout << "classes = {\n";
  cmXCodeObject::Indent(1, fout);
  fout << "};\n";
  cmXCodeObject::Indent(1, fout);
  if(this->XcodeVersion >= 21)
    {
    if (this->XcodeVersion >= 32)
      fout << "objectVersion = 46;\n";
    else if (this->XcodeVersion >= 31)
      fout << "objectVersion = 45;\n";
    else if (this->XcodeVersion >= 30)
      fout << "objectVersion = 44;\n";
    else
      fout << "objectVersion = 42;\n";
    cmXCode21Object::PrintList(this->XCodeObjects, fout);
    }
  else
    {
    fout << "objectVersion = 39;\n";
    cmXCodeObject::PrintList(this->XCodeObjects, fout);
    }
  cmXCodeObject::Indent(1, fout);
  fout << "rootObject = " << this->RootObject->GetId()
       << " /* Project object */;\n";
  fout << "}\n";
}

//----------------------------------------------------------------------------
const char* cmGlobalXCodeGenerator::GetCMakeCFGIntDir() const
{
  return this->XcodeVersion >= 21 ?
    "$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)" : ".";
}

std::string cmGlobalXCodeGenerator::ExpandCFGIntDir(const std::string& str,
                                        const std::string& config) const
{
  std::string replace1 = "$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)";
  std::string replace2 = "$(CONFIGURATION)";

  std::string tmp = str;
  for(std::string::size_type i = tmp.find(replace1);
      i != std::string::npos;
      i = tmp.find(replace1, i))
    {
    tmp.replace(i, replace1.size(), config);
    i += config.size();
    }
  for(std::string::size_type i = tmp.find(replace2);
      i != std::string::npos;
      i = tmp.find(replace2, i))
    {
    tmp.replace(i, replace2.size(), config);
    i += config.size();
    }
  return tmp;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::GetDocumentation(cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalXCodeGenerator::GetActualName();
  entry.Brief = "Generate Xcode project files.";
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::ConvertToRelativeForMake(const char* p)
{
  if ( !this->CurrentMakefile->IsOn("CMAKE_USE_RELATIVE_PATHS") )
    {
    return cmSystemTools::ConvertToOutputPath(p);
    }
  else
    {
    std::string ret =
      this->CurrentLocalGenerator->
        ConvertToRelativePath(this->CurrentOutputDirectoryComponents, p);
    return cmSystemTools::ConvertToOutputPath(ret.c_str());
    }
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::ConvertToRelativeForXCode(const char* p)
{
  if ( !this->CurrentMakefile->IsOn("CMAKE_USE_RELATIVE_PATHS") )
    {
    return cmSystemTools::ConvertToOutputPath(p);
    }
  else
    {
    std::string ret =
      this->CurrentLocalGenerator->
        ConvertToRelativePath(this->ProjectOutputDirectoryComponents, p);
    return cmSystemTools::ConvertToOutputPath(ret.c_str());
    }
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::RelativeToSource(const char* p)
{
  // We force conversion because Xcode breakpoints do not work unless
  // they are in a file named relative to the source tree.
  return this->CurrentLocalGenerator->
    ConvertToRelativePath(this->ProjectSourceDirectoryComponents, p, true);
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::RelativeToBinary(const char* p)
{
  return this->CurrentLocalGenerator->
    ConvertToRelativePath(this->ProjectOutputDirectoryComponents, p);
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::XCodeEscapePath(const char* p)
{
  std::string ret = p;
  if(ret.find(' ') != ret.npos)
    {
    std::string t = ret;
    ret = "\"";
    ret += t;
    ret += "\"";
    }
  return ret;
}

//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator
::AppendDirectoryForConfig(const std::string& prefix,
                           const std::string& config,
                           const std::string& suffix,
                           std::string& dir)
{
  if(this->XcodeVersion > 20)
    {
    if(!config.empty())
      {
      dir += prefix;
      dir += config;
      dir += suffix;
      }
    }
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::LookupFlags(
                                              const std::string& varNamePrefix,
                                              const std::string& varNameLang,
                                              const std::string& varNameSuffix,
                                              const std::string& default_flags)
{
  if(!varNameLang.empty())
    {
    std::string varName = varNamePrefix;
    varName += varNameLang;
    varName += varNameSuffix;
    if(const char* varValue =
       this->CurrentMakefile->GetDefinition(varName.c_str()))
      {
      if(*varValue)
        {
        return varValue;
        }
      }
    }
  return default_flags;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AppendDefines(BuildObjectListOrString& defs,
                                           const char* defines_list,
                                           bool dflag)
{
  // Skip this if there are no definitions.
  if(!defines_list)
    {
    return;
    }

  // Expand the list of definitions.
  std::vector<std::string> defines;
  cmSystemTools::ExpandListArgument(defines_list, defines);

  // Store the definitions in the string.
  this->AppendDefines(defs, defines, dflag);
}

//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator::AppendDefines(BuildObjectListOrString& defs,
                                      std::vector<std::string> const& defines,
                                      bool dflag)
{
  // GCC_PREPROCESSOR_DEFINITIONS is a space-separated list of definitions.
  std::string def;
  for(std::vector<std::string>::const_iterator di = defines.begin();
      di != defines.end(); ++di)
    {
    // Start with -D if requested.
    def = dflag? "-D": "";
    def += *di;

    // Append the flag with needed escapes.
    std::string tmp;
    this->AppendFlag(tmp, def);
    defs.Add(tmp.c_str());
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AppendFlag(std::string& flags,
                                        std::string const& flag)
{
  // Short-circuit for an empty flag.
  if(flag.empty())
    {
    return;
    }

  // Separate from previous flags.
  if(!flags.empty())
    {
    flags += " ";
    }

  // Check if the flag needs quoting.
  bool quoteFlag =
    flag.find_first_of("`~!@#$%^&*()+={}[]|:;\"'<>,.? ") != flag.npos;

  // We escape a flag as follows:
  //   - Place each flag in single quotes ''
  //   - Escape a single quote as \\'
  //   - Escape a backslash as \\\\ since it itself is an escape
  // Note that in the code below we need one more level of escapes for
  // C string syntax in this source file.
  //
  // The final level of escaping is done when the string is stored
  // into the project file by cmXCodeObject::PrintString.

  if(quoteFlag)
    {
    // Open single quote.
    flags += "'";
    }

  // Flag value with escaped quotes and backslashes.
  for(const char* c = flag.c_str(); *c; ++c)
    {
    if(*c == '\'')
      {
      if (this->XcodeVersion >= 40)
        {
        flags += "'\\\\''";
        }
      else
        {
        flags += "\\\\'";
        }
      }
    else if(*c == '\\')
      {
      flags += "\\\\\\\\";
      }
    else
      {
      flags += *c;
      }
    }

  if(quoteFlag)
    {
    // Close single quote.
    flags += "'";
    }
}

//----------------------------------------------------------------------------
std::string
cmGlobalXCodeGenerator::ComputeInfoPListLocation(cmTarget& target)
{
  std::string plist = target.GetMakefile()->GetCurrentBinaryDirectory();
  plist += cmake::GetCMakeFilesDirectory();
  plist += "/";
  plist += target.GetName();
  plist += ".dir/Info.plist";
  return plist;
}

//----------------------------------------------------------------------------
// Return true if the generated build tree may contain multiple builds.
// i.e. "Can I build Debug and Release in the same tree?"
bool cmGlobalXCodeGenerator::IsMultiConfig()
{
  // Old Xcode 1.5 is single config:
  if(this->XcodeVersion == 15)
    {
    return false;
    }

  // Newer Xcode versions are multi config:
  return true;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator
::ComputeTargetObjectDirectory(cmGeneratorTarget* gt) const
{
  std::string configName = this->GetCMakeCFGIntDir();
  std::string dir = this->GetObjectsNormalDirectory(
    "$(PROJECT_NAME)", configName, gt->Target);
  if(this->XcodeVersion >= 21)
    {
    dir += "$(CURRENT_ARCH)/";
    }
  else
    {
#ifdef __ppc__
    dir += "ppc/";
#endif
#ifdef __i386
    dir += "i386/";
#endif
    }
  gt->ObjectDirectory = dir;
}
