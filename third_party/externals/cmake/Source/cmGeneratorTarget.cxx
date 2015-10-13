/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGeneratorTarget.h"

#include "cmTarget.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmSourceFile.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmComputeLinkInformation.h"
#include "cmCustomCommandGenerator.h"
#include "cmAlgorithms.h"

#include <queue>

#include "assert.h"

//----------------------------------------------------------------------------
void reportBadObjLib(std::vector<cmSourceFile*> const& badObjLib,
                     cmTarget *target, cmake *cm)
{
  if(!badObjLib.empty())
    {
    std::ostringstream e;
    e << "OBJECT library \"" << target->GetName() << "\" contains:\n";
    for(std::vector<cmSourceFile*>::const_iterator i = badObjLib.begin();
        i != badObjLib.end(); ++i)
      {
      e << "  " << (*i)->GetLocation().GetName() << "\n";
      }
    e << "but may contain only sources that compile, header files, and "
         "other files that would not affect linking of a normal library.";
    cm->IssueMessage(cmake::FATAL_ERROR, e.str(),
                     target->GetBacktrace());
    }
}

struct ObjectSourcesTag {};
struct CustomCommandsTag {};
struct ExtraSourcesTag {};
struct HeaderSourcesTag {};
struct ExternalObjectsTag {};
struct IDLSourcesTag {};
struct ResxTag {};
struct ModuleDefinitionFileTag {};
struct AppManifestTag{};
struct CertificatesTag{};
struct XamlTag{};

template<typename Tag, typename OtherTag>
struct IsSameTag
{
  enum {
    Result = false
  };
};

template<typename Tag>
struct IsSameTag<Tag, Tag>
{
  enum {
    Result = true
  };
};

template<bool>
struct DoAccept
{
  template <typename T> static void Do(T&, cmSourceFile*) {}
};

template<>
struct DoAccept<true>
{
  static void Do(std::vector<cmSourceFile const*>& files, cmSourceFile* f)
    {
    files.push_back(f);
    }
  static void Do(cmGeneratorTarget::ResxData& data, cmSourceFile* f)
    {
    // Build and save the name of the corresponding .h file
    // This relationship will be used later when building the project files.
    // Both names would have been auto generated from Visual Studio
    // where the user supplied the file name and Visual Studio
    // appended the suffix.
    std::string resx = f->GetFullPath();
    std::string hFileName = resx.substr(0, resx.find_last_of(".")) + ".h";
    data.ExpectedResxHeaders.insert(hFileName);
    data.ResxSources.push_back(f);
    }
  static void Do(cmGeneratorTarget::XamlData& data, cmSourceFile* f)
    {
    // Build and save the name of the corresponding .h and .cpp file
    // This relationship will be used later when building the project files.
    // Both names would have been auto generated from Visual Studio
    // where the user supplied the file name and Visual Studio
    // appended the suffix.
    std::string xaml = f->GetFullPath();
    std::string hFileName = xaml + ".h";
    std::string cppFileName = xaml + ".cpp";
    data.ExpectedXamlHeaders.insert(hFileName);
    data.ExpectedXamlSources.insert(cppFileName);
    data.XamlSources.push_back(f);
    }
  static void Do(std::string& data, cmSourceFile* f)
    {
    data = f->GetFullPath();
    }
};

//----------------------------------------------------------------------------
template<typename Tag, typename DataType = std::vector<cmSourceFile const*> >
struct TagVisitor
{
  DataType& Data;
  std::vector<cmSourceFile*> BadObjLibFiles;
  cmTarget *Target;
  cmGlobalGenerator *GlobalGenerator;
  cmsys::RegularExpression Header;
  bool IsObjLib;

  TagVisitor(cmTarget *target, DataType& data)
    : Data(data), Target(target),
    GlobalGenerator(target->GetMakefile()->GetGlobalGenerator()),
    Header(CM_HEADER_REGEX),
    IsObjLib(target->GetType() == cmTarget::OBJECT_LIBRARY)
  {
  }

  ~TagVisitor()
  {
    reportBadObjLib(this->BadObjLibFiles, this->Target,
                    this->GlobalGenerator->GetCMakeInstance());
  }

  void Accept(cmSourceFile *sf)
  {
    std::string ext = cmSystemTools::LowerCase(sf->GetExtension());
    if(sf->GetCustomCommand())
      {
      DoAccept<IsSameTag<Tag, CustomCommandsTag>::Result>::Do(this->Data, sf);
      }
    else if(this->Target->GetType() == cmTarget::UTILITY)
      {
      DoAccept<IsSameTag<Tag, ExtraSourcesTag>::Result>::Do(this->Data, sf);
      }
    else if(sf->GetPropertyAsBool("HEADER_FILE_ONLY"))
      {
      DoAccept<IsSameTag<Tag, HeaderSourcesTag>::Result>::Do(this->Data, sf);
      }
    else if(sf->GetPropertyAsBool("EXTERNAL_OBJECT"))
      {
      DoAccept<IsSameTag<Tag, ExternalObjectsTag>::Result>::Do(this->Data, sf);
      if(this->IsObjLib)
        {
        this->BadObjLibFiles.push_back(sf);
        }
      }
    else if(!sf->GetLanguage().empty())
      {
      DoAccept<IsSameTag<Tag, ObjectSourcesTag>::Result>::Do(this->Data, sf);
      }
    else if(ext == "def")
      {
      DoAccept<IsSameTag<Tag, ModuleDefinitionFileTag>::Result>::Do(this->Data,
                                                                    sf);
      if(this->IsObjLib)
        {
        this->BadObjLibFiles.push_back(sf);
        }
      }
    else if(ext == "idl")
      {
      DoAccept<IsSameTag<Tag, IDLSourcesTag>::Result>::Do(this->Data, sf);
      if(this->IsObjLib)
        {
        this->BadObjLibFiles.push_back(sf);
        }
      }
    else if(ext == "resx")
      {
      DoAccept<IsSameTag<Tag, ResxTag>::Result>::Do(this->Data, sf);
      }
    else if (ext == "appxmanifest")
      {
      DoAccept<IsSameTag<Tag, AppManifestTag>::Result>::Do(this->Data, sf);
      }
    else if (ext == "pfx")
      {
      DoAccept<IsSameTag<Tag, CertificatesTag>::Result>::Do(this->Data, sf);
      }
    else if (ext == "xaml")
      {
      DoAccept<IsSameTag<Tag, XamlTag>::Result>::Do(this->Data, sf);
      }
    else if(this->Header.find(sf->GetFullPath().c_str()))
      {
      DoAccept<IsSameTag<Tag, HeaderSourcesTag>::Result>::Do(this->Data, sf);
      }
    else if(this->GlobalGenerator->IgnoreFile(sf->GetExtension().c_str()))
      {
      DoAccept<IsSameTag<Tag, ExtraSourcesTag>::Result>::Do(this->Data, sf);
      }
    else
      {
      DoAccept<IsSameTag<Tag, ExtraSourcesTag>::Result>::Do(this->Data, sf);
      }
  }
};

//----------------------------------------------------------------------------
cmGeneratorTarget::cmGeneratorTarget(cmTarget* t): Target(t),
  SourceFileFlagsConstructed(false)
{
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator = this->Makefile->GetLocalGenerator();
  this->GlobalGenerator = this->Makefile->GetGlobalGenerator();
}

//----------------------------------------------------------------------------
int cmGeneratorTarget::GetType() const
{
  return this->Target->GetType();
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetName() const
{
  return this->Target->GetName();
}

//----------------------------------------------------------------------------
const char *cmGeneratorTarget::GetProperty(const std::string& prop) const
{
  return this->Target->GetProperty(prop);
}

//----------------------------------------------------------------------------
std::vector<cmSourceFile*> const*
cmGeneratorTarget::GetSourceDepends(cmSourceFile const* sf) const
{
  SourceEntriesType::const_iterator i = this->SourceEntries.find(sf);
  if(i != this->SourceEntries.end())
    {
    return &i->second.Depends;
    }
  return 0;
}

static void handleSystemIncludesDep(cmMakefile *mf, cmTarget const* depTgt,
                                  const std::string& config,
                                  cmTarget *headTarget,
                                  cmGeneratorExpressionDAGChecker *dagChecker,
                                  std::vector<std::string>& result,
                                  bool excludeImported)
{
  if (const char* dirs =
          depTgt->GetProperty("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES"))
    {
    cmGeneratorExpression ge;
    cmSystemTools::ExpandListArgument(ge.Parse(dirs)
                                      ->Evaluate(mf,
                                      config, false, headTarget,
                                      depTgt, dagChecker), result);
    }
  if (!depTgt->IsImported() || excludeImported)
    {
    return;
    }

  if (const char* dirs =
                depTgt->GetProperty("INTERFACE_INCLUDE_DIRECTORIES"))
    {
    cmGeneratorExpression ge;
    cmSystemTools::ExpandListArgument(ge.Parse(dirs)
                                      ->Evaluate(mf,
                                      config, false, headTarget,
                                      depTgt, dagChecker), result);
    }
}

#define IMPLEMENT_VISIT_IMPL(DATA, DATATYPE) \
  { \
  std::vector<cmSourceFile*> sourceFiles; \
  this->Target->GetSourceFiles(sourceFiles, config); \
  TagVisitor<DATA ## Tag DATATYPE> visitor(this->Target, data); \
  for(std::vector<cmSourceFile*>::const_iterator si = sourceFiles.begin(); \
      si != sourceFiles.end(); ++si) \
    { \
    visitor.Accept(*si); \
    } \
  } \


#define IMPLEMENT_VISIT(DATA) \
  IMPLEMENT_VISIT_IMPL(DATA, EMPTY) \

#define EMPTY
#define COMMA ,

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetObjectSources(std::vector<cmSourceFile const*> &data,
                   const std::string& config) const
{
  IMPLEMENT_VISIT(ObjectSources);

  if (!this->Objects.empty())
    {
    return;
    }

  for(std::vector<cmSourceFile const*>::const_iterator it = data.begin();
      it != data.end(); ++it)
    {
    this->Objects[*it];
    }

  this->LocalGenerator->ComputeObjectFilenames(this->Objects, this);
}

void cmGeneratorTarget::ComputeObjectMapping()
{
  if(!this->Objects.empty())
    {
    return;
    }

  std::vector<std::string> configs;
  this->Makefile->GetConfigurations(configs);
  if (configs.empty())
    {
    configs.push_back("");
    }
  for(std::vector<std::string>::const_iterator ci = configs.begin();
      ci != configs.end(); ++ci)
    {
    std::vector<cmSourceFile const*> sourceFiles;
    this->GetObjectSources(sourceFiles, *ci);
    }
}

//----------------------------------------------------------------------------
const std::string& cmGeneratorTarget::GetObjectName(cmSourceFile const* file)
{
  this->ComputeObjectMapping();
  return this->Objects[file];
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::AddExplicitObjectName(cmSourceFile const* sf)
{
  this->ExplicitObjectName.insert(sf);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::HasExplicitObjectName(cmSourceFile const* file) const
{
  const_cast<cmGeneratorTarget*>(this)->ComputeObjectMapping();
  std::set<cmSourceFile const*>::const_iterator it
                                        = this->ExplicitObjectName.find(file);
  return it != this->ExplicitObjectName.end();
}

//----------------------------------------------------------------------------
void cmGeneratorTarget
::GetIDLSources(std::vector<cmSourceFile const*>& data,
                const std::string& config) const
{
  IMPLEMENT_VISIT(IDLSources);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetHeaderSources(std::vector<cmSourceFile const*>& data,
                   const std::string& config) const
{
  IMPLEMENT_VISIT(HeaderSources);
}

//----------------------------------------------------------------------------
void cmGeneratorTarget
::GetExtraSources(std::vector<cmSourceFile const*>& data,
                  const std::string& config) const
{
  IMPLEMENT_VISIT(ExtraSources);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetCustomCommands(std::vector<cmSourceFile const*>& data,
                    const std::string& config) const
{
  IMPLEMENT_VISIT(CustomCommands);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetExternalObjects(std::vector<cmSourceFile const*>& data,
                     const std::string& config) const
{
  IMPLEMENT_VISIT(ExternalObjects);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetExpectedResxHeaders(std::set<std::string>& srcs,
                                          const std::string& config) const
{
  ResxData data;
  IMPLEMENT_VISIT_IMPL(Resx, COMMA cmGeneratorTarget::ResxData)
  srcs = data.ExpectedResxHeaders;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget
::GetResxSources(std::vector<cmSourceFile const*>& srcs,
                 const std::string& config) const
{
  ResxData data;
  IMPLEMENT_VISIT_IMPL(Resx, COMMA cmGeneratorTarget::ResxData)
  srcs = data.ResxSources;
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetAppManifest(std::vector<cmSourceFile const*>& data,
                 const std::string& config) const
{
  IMPLEMENT_VISIT(AppManifest);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetCertificates(std::vector<cmSourceFile const*>& data,
                  const std::string& config) const
{
  IMPLEMENT_VISIT(Certificates);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetExpectedXamlHeaders(std::set<std::string>& headers,
                                          const std::string& config) const
{
  XamlData data;
  IMPLEMENT_VISIT_IMPL(Xaml, COMMA cmGeneratorTarget::XamlData)
  headers = data.ExpectedXamlHeaders;
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetExpectedXamlSources(std::set<std::string>& srcs,
                                          const std::string& config) const
{
  XamlData data;
  IMPLEMENT_VISIT_IMPL(Xaml, COMMA cmGeneratorTarget::XamlData)
  srcs = data.ExpectedXamlSources;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget
::GetXamlSources(std::vector<cmSourceFile const*>& srcs,
                 const std::string& config) const
{
  XamlData data;
  IMPLEMENT_VISIT_IMPL(Xaml, COMMA cmGeneratorTarget::XamlData)
  srcs = data.XamlSources;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsSystemIncludeDirectory(const std::string& dir,
                                              const std::string& config) const
{
  assert(this->GetType() != cmTarget::INTERFACE_LIBRARY);
  std::string config_upper;
  if(!config.empty())
    {
    config_upper = cmSystemTools::UpperCase(config);
    }

  typedef std::map<std::string, std::vector<std::string> > IncludeCacheType;
  IncludeCacheType::const_iterator iter =
      this->SystemIncludesCache.find(config_upper);

  if (iter == this->SystemIncludesCache.end())
    {
    cmGeneratorExpressionDAGChecker dagChecker(
                                        this->GetName(),
                                        "SYSTEM_INCLUDE_DIRECTORIES", 0, 0);

    bool excludeImported
                = this->Target->GetPropertyAsBool("NO_SYSTEM_FROM_IMPORTED");

    std::vector<std::string> result;
    for (std::set<std::string>::const_iterator
        it = this->Target->GetSystemIncludeDirectories().begin();
        it != this->Target->GetSystemIncludeDirectories().end(); ++it)
      {
      cmGeneratorExpression ge;
      cmSystemTools::ExpandListArgument(ge.Parse(*it)
                                          ->Evaluate(this->Makefile,
                                          config, false, this->Target,
                                          &dagChecker), result);
      }

    std::vector<cmTarget const*> const& deps =
      this->Target->GetLinkImplementationClosure(config);
    for(std::vector<cmTarget const*>::const_iterator
          li = deps.begin(), le = deps.end(); li != le; ++li)
      {
      handleSystemIncludesDep(this->Makefile, *li, config, this->Target,
                              &dagChecker, result, excludeImported);
      }

    std::set<std::string> unique;
    for(std::vector<std::string>::iterator li = result.begin();
        li != result.end(); ++li)
      {
      cmSystemTools::ConvertToUnixSlashes(*li);
      unique.insert(*li);
      }
    result.clear();
    result.insert(result.end(), unique.begin(), unique.end());

    IncludeCacheType::value_type entry(config_upper, result);
    iter = this->SystemIncludesCache.insert(entry).first;
    }

  return std::binary_search(iter->second.begin(), iter->second.end(), dir);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::GetPropertyAsBool(const std::string& prop) const
{
  return this->Target->GetPropertyAsBool(prop);
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetSourceFiles(std::vector<cmSourceFile*> &files,
                                       const std::string& config) const
{
  this->Target->GetSourceFiles(files, config);
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::GetModuleDefinitionFile(const std::string& config) const
{
  std::string data;
  IMPLEMENT_VISIT_IMPL(ModuleDefinitionFile, COMMA std::string)
  return data;
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::UseObjectLibraries(std::vector<std::string>& objs,
                                      const std::string &config) const
{
  std::vector<cmSourceFile const*> objectFiles;
  this->GetExternalObjects(objectFiles, config);
  std::vector<cmTarget*> objectLibraries;
  for(std::vector<cmSourceFile const*>::const_iterator
      it = objectFiles.begin(); it != objectFiles.end(); ++it)
    {
    std::string objLib = (*it)->GetObjectLibrary();
    if (cmTarget* tgt = this->Makefile->FindTargetToUse(objLib))
      {
      objectLibraries.push_back(tgt);
      }
    }

  std::vector<cmTarget*>::const_iterator end
      = cmRemoveDuplicates(objectLibraries);

  for(std::vector<cmTarget*>::const_iterator
        ti = objectLibraries.begin();
      ti != end; ++ti)
    {
    cmTarget* objLib = *ti;
    cmGeneratorTarget* ogt =
      this->GlobalGenerator->GetGeneratorTarget(objLib);
    std::vector<cmSourceFile const*> objectSources;
    ogt->GetObjectSources(objectSources, config);
    for(std::vector<cmSourceFile const*>::const_iterator
          si = objectSources.begin();
        si != objectSources.end(); ++si)
      {
      std::string obj = ogt->ObjectDirectory;
      obj += ogt->Objects[*si];
      objs.push_back(obj);
      }
    }
}

//----------------------------------------------------------------------------
class cmTargetTraceDependencies
{
public:
  cmTargetTraceDependencies(cmGeneratorTarget* target);
  void Trace();
private:
  cmTarget* Target;
  cmGeneratorTarget* GeneratorTarget;
  cmMakefile* Makefile;
  cmGlobalGenerator const* GlobalGenerator;
  typedef cmGeneratorTarget::SourceEntry SourceEntry;
  SourceEntry* CurrentEntry;
  std::queue<cmSourceFile*> SourceQueue;
  std::set<cmSourceFile*> SourcesQueued;
  typedef std::map<std::string, cmSourceFile*> NameMapType;
  NameMapType NameMap;
  std::vector<std::string> NewSources;

  void QueueSource(cmSourceFile* sf);
  void FollowName(std::string const& name);
  void FollowNames(std::vector<std::string> const& names);
  bool IsUtility(std::string const& dep);
  void CheckCustomCommand(cmCustomCommand const& cc);
  void CheckCustomCommands(const std::vector<cmCustomCommand>& commands);
  void FollowCommandDepends(cmCustomCommand const& cc,
                            const std::string& config,
                            std::set<std::string>& emitted);
};

//----------------------------------------------------------------------------
cmTargetTraceDependencies
::cmTargetTraceDependencies(cmGeneratorTarget* target):
  Target(target->Target), GeneratorTarget(target)
{
  // Convenience.
  this->Makefile = this->Target->GetMakefile();
  this->GlobalGenerator = this->Makefile->GetGlobalGenerator();
  this->CurrentEntry = 0;

  // Queue all the source files already specified for the target.
  if (this->Target->GetType() != cmTarget::INTERFACE_LIBRARY)
    {
    std::vector<std::string> configs;
    this->Makefile->GetConfigurations(configs);
    if (configs.empty())
      {
      configs.push_back("");
      }
    std::set<cmSourceFile*> emitted;
    for(std::vector<std::string>::const_iterator ci = configs.begin();
        ci != configs.end(); ++ci)
      {
      std::vector<cmSourceFile*> sources;
      this->Target->GetSourceFiles(sources, *ci);
      for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
          si != sources.end(); ++si)
        {
        cmSourceFile* sf = *si;
        const std::set<cmTarget const*> tgts =
                          this->GlobalGenerator->GetFilenameTargetDepends(sf);
        if (tgts.find(this->Target) != tgts.end())
          {
          std::ostringstream e;
          e << "Evaluation output file\n  \"" << sf->GetFullPath()
            << "\"\ndepends on the sources of a target it is used in.  This "
              "is a dependency loop and is not allowed.";
          this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
          return;
          }
        if(emitted.insert(sf).second && this->SourcesQueued.insert(sf).second)
          {
          this->SourceQueue.push(sf);
          }
        }
      }
    }

  // Queue pre-build, pre-link, and post-build rule dependencies.
  this->CheckCustomCommands(this->Target->GetPreBuildCommands());
  this->CheckCustomCommands(this->Target->GetPreLinkCommands());
  this->CheckCustomCommands(this->Target->GetPostBuildCommands());
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::Trace()
{
  // Process one dependency at a time until the queue is empty.
  while(!this->SourceQueue.empty())
    {
    // Get the next source from the queue.
    cmSourceFile* sf = this->SourceQueue.front();
    this->SourceQueue.pop();
    this->CurrentEntry = &this->GeneratorTarget->SourceEntries[sf];

    // Queue dependencies added explicitly by the user.
    if(const char* additionalDeps = sf->GetProperty("OBJECT_DEPENDS"))
      {
      std::vector<std::string> objDeps;
      cmSystemTools::ExpandListArgument(additionalDeps, objDeps);
      for(std::vector<std::string>::iterator odi = objDeps.begin();
          odi != objDeps.end(); ++odi)
        {
        if (cmSystemTools::FileIsFullPath(*odi))
          {
          *odi = cmSystemTools::CollapseFullPath(*odi);
          }
        }
      this->FollowNames(objDeps);
      }

    // Queue the source needed to generate this file, if any.
    this->FollowName(sf->GetFullPath());

    // Queue dependencies added programatically by commands.
    this->FollowNames(sf->GetDepends());

    // Queue custom command dependencies.
    if(cmCustomCommand const* cc = sf->GetCustomCommand())
      {
      this->CheckCustomCommand(*cc);
      }
    }
  this->CurrentEntry = 0;

  this->Target->AddTracedSources(this->NewSources);
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::QueueSource(cmSourceFile* sf)
{
  if(this->SourcesQueued.insert(sf).second)
    {
    this->SourceQueue.push(sf);

    // Make sure this file is in the target at the end.
    this->NewSources.push_back(sf->GetFullPath());
    }
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::FollowName(std::string const& name)
{
  NameMapType::iterator i = this->NameMap.find(name);
  if(i == this->NameMap.end())
    {
    // Check if we know how to generate this file.
    cmSourceFile* sf = this->Makefile->GetSourceFileWithOutput(name);
    NameMapType::value_type entry(name, sf);
    i = this->NameMap.insert(entry).first;
    }
  if(cmSourceFile* sf = i->second)
    {
    // Record the dependency we just followed.
    if(this->CurrentEntry)
      {
      this->CurrentEntry->Depends.push_back(sf);
      }
    this->QueueSource(sf);
    }
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies::FollowNames(std::vector<std::string> const& names)
{
  for(std::vector<std::string>::const_iterator i = names.begin();
      i != names.end(); ++i)
    {
    this->FollowName(*i);
    }
}

//----------------------------------------------------------------------------
bool cmTargetTraceDependencies::IsUtility(std::string const& dep)
{
  // Dependencies on targets (utilities) are supposed to be named by
  // just the target name.  However for compatibility we support
  // naming the output file generated by the target (assuming there is
  // no output-name property which old code would not have set).  In
  // that case the target name will be the file basename of the
  // dependency.
  std::string util = cmSystemTools::GetFilenameName(dep);
  if(cmSystemTools::GetFilenameLastExtension(util) == ".exe")
    {
    util = cmSystemTools::GetFilenameWithoutLastExtension(util);
    }

  // Check for a target with this name.
  if(cmTarget* t = this->Makefile->FindTargetToUse(util))
    {
    // If we find the target and the dep was given as a full path,
    // then make sure it was not a full path to something else, and
    // the fact that the name matched a target was just a coincidence.
    if(cmSystemTools::FileIsFullPath(dep.c_str()))
      {
      if(t->GetType() >= cmTarget::EXECUTABLE &&
         t->GetType() <= cmTarget::MODULE_LIBRARY)
        {
        // This is really only for compatibility so we do not need to
        // worry about configuration names and output names.
        std::string tLocation = t->GetLocationForBuild();
        tLocation = cmSystemTools::GetFilenamePath(tLocation);
        std::string depLocation = cmSystemTools::GetFilenamePath(dep);
        depLocation = cmSystemTools::CollapseFullPath(depLocation);
        tLocation = cmSystemTools::CollapseFullPath(tLocation);
        if(depLocation == tLocation)
          {
          this->Target->AddUtility(util);
          return true;
          }
        }
      }
    else
      {
      // The original name of the dependency was not a full path.  It
      // must name a target, so add the target-level dependency.
      this->Target->AddUtility(util);
      return true;
      }
    }

  // The dependency does not name a target built in this project.
  return false;
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies
::CheckCustomCommand(cmCustomCommand const& cc)
{
  // Transform command names that reference targets built in this
  // project to corresponding target-level dependencies.
  cmGeneratorExpression ge(&cc.GetBacktrace());

  // Add target-level dependencies referenced by generator expressions.
  std::set<cmTarget*> targets;

  for(cmCustomCommandLines::const_iterator cit = cc.GetCommandLines().begin();
      cit != cc.GetCommandLines().end(); ++cit)
    {
    std::string const& command = *cit->begin();
    // Check for a target with this name.
    if(cmTarget* t = this->Makefile->FindTargetToUse(command))
      {
      if(t->GetType() == cmTarget::EXECUTABLE)
        {
        // The command refers to an executable target built in
        // this project.  Add the target-level dependency to make
        // sure the executable is up to date before this custom
        // command possibly runs.
        this->Target->AddUtility(command);
        }
      }

    // Check for target references in generator expressions.
    for(cmCustomCommandLine::const_iterator cli = cit->begin();
        cli != cit->end(); ++cli)
      {
      const cmsys::auto_ptr<cmCompiledGeneratorExpression> cge
                                                              = ge.Parse(*cli);
      cge->Evaluate(this->Makefile, "", true);
      std::set<cmTarget*> geTargets = cge->GetTargets();
      targets.insert(geTargets.begin(), geTargets.end());
      }
    }

  for(std::set<cmTarget*>::iterator ti = targets.begin();
      ti != targets.end(); ++ti)
    {
    this->Target->AddUtility((*ti)->GetName());
    }

  // Queue the custom command dependencies.
  std::vector<std::string> configs;
  std::set<std::string> emitted;
  this->Makefile->GetConfigurations(configs);
  if (configs.empty())
    {
    configs.push_back("");
    }
  for(std::vector<std::string>::const_iterator ci = configs.begin();
      ci != configs.end(); ++ci)
    {
    this->FollowCommandDepends(cc, *ci, emitted);
    }
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::FollowCommandDepends(cmCustomCommand const& cc,
                                              const std::string& config,
                                              std::set<std::string>& emitted)
{
  cmCustomCommandGenerator ccg(cc, config, this->Makefile);

  const std::vector<std::string>& depends = ccg.GetDepends();

  for(std::vector<std::string>::const_iterator di = depends.begin();
      di != depends.end(); ++di)
    {
    std::string const& dep = *di;
    if(emitted.insert(dep).second)
      {
      if(!this->IsUtility(dep))
        {
        // The dependency does not name a target and may be a file we
        // know how to generate.  Queue it.
        this->FollowName(dep);
        }
      }
    }
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies
::CheckCustomCommands(const std::vector<cmCustomCommand>& commands)
{
  for(std::vector<cmCustomCommand>::const_iterator cli = commands.begin();
      cli != commands.end(); ++cli)
    {
    this->CheckCustomCommand(*cli);
    }
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::TraceDependencies()
{
  // CMake-generated targets have no dependencies to trace.  Normally tracing
  // would find nothing anyway, but when building CMake itself the "install"
  // target command ends up referencing the "cmake" target but we do not
  // really want the dependency because "install" depend on "all" anyway.
  if(this->GetType() == cmTarget::GLOBAL_TARGET)
    {
    return;
    }

  // Use a helper object to trace the dependencies.
  cmTargetTraceDependencies tracer(this);
  tracer.Trace();
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetAppleArchs(const std::string& config,
                             std::vector<std::string>& archVec) const
{
  const char* archs = 0;
  if(!config.empty())
    {
    std::string defVarName = "OSX_ARCHITECTURES_";
    defVarName += cmSystemTools::UpperCase(config);
    archs = this->Target->GetProperty(defVarName);
    }
  if(!archs)
    {
    archs = this->Target->GetProperty("OSX_ARCHITECTURES");
    }
  if(archs)
    {
    cmSystemTools::ExpandListArgument(std::string(archs), archVec);
    }
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::GetCreateRuleVariable(std::string const& lang,
                                         std::string const& config) const
{
  switch(this->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      {
      std::string var = "CMAKE_" + lang + "_CREATE_STATIC_LIBRARY";
      if(this->Target->GetFeatureAsBool(
           "INTERPROCEDURAL_OPTIMIZATION", config))
        {
        std::string varIPO = var + "_IPO";
        if(this->Makefile->GetDefinition(varIPO))
          {
          return varIPO;
          }
        }
      return var;
      }
    case cmTarget::SHARED_LIBRARY:
      return "CMAKE_" + lang + "_CREATE_SHARED_LIBRARY";
    case cmTarget::MODULE_LIBRARY:
      return "CMAKE_" + lang + "_CREATE_SHARED_MODULE";
    case cmTarget::EXECUTABLE:
      return "CMAKE_" + lang + "_LINK_EXECUTABLE";
    default:
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
std::vector<std::string>
cmGeneratorTarget::GetIncludeDirectories(const std::string& config,
                                         const std::string& lang) const
{
  return this->Target->GetIncludeDirectories(config, lang);
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GenerateTargetManifest(
                                              const std::string& config) const
{
  if (this->Target->IsImported())
    {
    return;
    }
  cmMakefile* mf = this->Target->GetMakefile();
  cmGlobalGenerator* gg = mf->GetGlobalGenerator();

  // Get the names.
  std::string name;
  std::string soName;
  std::string realName;
  std::string impName;
  std::string pdbName;
  if(this->GetType() == cmTarget::EXECUTABLE)
    {
    this->Target->GetExecutableNames(name, realName, impName, pdbName,
                                     config);
    }
  else if(this->GetType() == cmTarget::STATIC_LIBRARY ||
          this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->GetType() == cmTarget::MODULE_LIBRARY)
    {
    this->Target->GetLibraryNames(name, soName, realName, impName, pdbName,
                                  config);
    }
  else
    {
    return;
    }

  // Get the directory.
  std::string dir = this->Target->GetDirectory(config, false);

  // Add each name.
  std::string f;
  if(!name.empty())
    {
    f = dir;
    f += "/";
    f += name;
    gg->AddToManifest(config, f);
    }
  if(!soName.empty())
    {
    f = dir;
    f += "/";
    f += soName;
    gg->AddToManifest(config, f);
    }
  if(!realName.empty())
    {
    f = dir;
    f += "/";
    f += realName;
    gg->AddToManifest(config, f);
    }
  if(!pdbName.empty())
    {
    f = dir;
    f += "/";
    f += pdbName;
    gg->AddToManifest(config, f);
    }
  if(!impName.empty())
    {
    f = this->Target->GetDirectory(config, true);
    f += "/";
    f += impName;
    gg->AddToManifest(config, f);
    }
}

bool cmStrictTargetComparison::operator()(cmTarget const* t1,
                                          cmTarget const* t2) const
{
  int nameResult = strcmp(t1->GetName().c_str(), t2->GetName().c_str());
  if (nameResult == 0)
    {
    return strcmp(t1->GetMakefile()->GetCurrentBinaryDirectory(),
                  t2->GetMakefile()->GetCurrentBinaryDirectory()) < 0;
    }
  return nameResult < 0;
}

//----------------------------------------------------------------------------
struct cmGeneratorTarget::SourceFileFlags
cmGeneratorTarget::GetTargetSourceFileFlags(const cmSourceFile* sf) const
{
  struct SourceFileFlags flags;
  this->ConstructSourceFileFlags();
  std::map<cmSourceFile const*, SourceFileFlags>::iterator si =
    this->SourceFlagsMap.find(sf);
  if(si != this->SourceFlagsMap.end())
    {
    flags = si->second;
    }
  else
    {
    // Handle the MACOSX_PACKAGE_LOCATION property on source files that
    // were not listed in one of the other lists.
    if(const char* location = sf->GetProperty("MACOSX_PACKAGE_LOCATION"))
      {
      flags.MacFolder = location;
      if(strcmp(location, "Resources") == 0)
        {
        flags.Type = cmGeneratorTarget::SourceFileTypeResource;
        }
      else
        {
        flags.Type = cmGeneratorTarget::SourceFileTypeMacContent;
        }
      }
    }
  return flags;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::ConstructSourceFileFlags() const
{
  if(this->SourceFileFlagsConstructed)
    {
    return;
    }
  this->SourceFileFlagsConstructed = true;

  // Process public headers to mark the source files.
  if(const char* files = this->Target->GetProperty("PUBLIC_HEADER"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(*it))
        {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "Headers";
        flags.Type = cmGeneratorTarget::SourceFileTypePublicHeader;
        }
      }
    }

  // Process private headers after public headers so that they take
  // precedence if a file is listed in both.
  if(const char* files = this->Target->GetProperty("PRIVATE_HEADER"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(*it))
        {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "PrivateHeaders";
        flags.Type = cmGeneratorTarget::SourceFileTypePrivateHeader;
        }
      }
    }

  // Mark sources listed as resources.
  if(const char* files = this->Target->GetProperty("RESOURCE"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(*it))
        {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "Resources";
        flags.Type = cmGeneratorTarget::SourceFileTypeResource;
        }
      }
    }
}
