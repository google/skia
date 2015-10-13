/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDependsFortran.h"

#include "cmSystemTools.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"

#include "cmDependsFortranParser.h" /* Interface to parser object.  */
#include <cmsys/FStream.hxx>
#include <assert.h>
#include <stack>

// TODO: Test compiler for the case of the mod file.  Some always
// use lower case and some always use upper case.  I do not know if any
// use the case from the source code.

//----------------------------------------------------------------------------
// Information about a single source file.
class cmDependsFortranSourceInfo
{
public:
  // The name of the source file.
  std::string Source;

  // Set of provided and required modules.
  std::set<std::string> Provides;
  std::set<std::string> Requires;

  // Set of files included in the translation unit.
  std::set<std::string> Includes;
};

//----------------------------------------------------------------------------
// Parser methods not included in generated interface.

// Get the current buffer processed by the lexer.
YY_BUFFER_STATE cmDependsFortranLexer_GetCurrentBuffer(yyscan_t yyscanner);

// The parser entry point.
int cmDependsFortran_yyparse(yyscan_t);

//----------------------------------------------------------------------------
// Define parser object internal structure.
struct cmDependsFortranFile
{
  cmDependsFortranFile(FILE* file, YY_BUFFER_STATE buffer,
                       const std::string& dir):
    File(file), Buffer(buffer), Directory(dir) {}
  FILE* File;
  YY_BUFFER_STATE Buffer;
  std::string Directory;
};

struct cmDependsFortranParser_s
{
  cmDependsFortranParser_s(cmDependsFortran* self,
                           std::set<std::string>& ppDefines,
                           cmDependsFortranSourceInfo& info);
  ~cmDependsFortranParser_s();

  // Pointer back to the main class.
  cmDependsFortran* Self;

  // Lexical scanner instance.
  yyscan_t Scanner;

  // Stack of open files in the translation unit.
  std::stack<cmDependsFortranFile> FileStack;

  // Buffer for string literals.
  std::string TokenString;

  // Flag for whether lexer is reading from inside an interface.
  bool InInterface;

  int OldStartcond;
  std::set<std::string> PPDefinitions;
  size_t InPPFalseBranch;
  std::stack<bool> SkipToEnd;

  // Information about the parsed source.
  cmDependsFortranSourceInfo& Info;
};

//----------------------------------------------------------------------------
class cmDependsFortranInternals
{
public:
  // The set of modules provided by this target.
  std::set<std::string> TargetProvides;

  // Map modules required by this target to locations.
  typedef std::map<std::string, std::string> TargetRequiresMap;
  TargetRequiresMap TargetRequires;

  // Information about each object file.
  typedef std::map<std::string, cmDependsFortranSourceInfo> ObjectInfoMap;
  ObjectInfoMap ObjectInfo;

  cmDependsFortranSourceInfo& CreateObjectInfo(const char* obj,
                                               const char* src)
    {
    std::map<std::string, cmDependsFortranSourceInfo>::iterator i =
      this->ObjectInfo.find(obj);
    if(i == this->ObjectInfo.end())
      {
      std::map<std::string, cmDependsFortranSourceInfo>::value_type
        entry(obj, cmDependsFortranSourceInfo());
      i = this->ObjectInfo.insert(entry).first;
      i->second.Source = src;
      }
    return i->second;
    }
};

//----------------------------------------------------------------------------
cmDependsFortran::cmDependsFortran():
  PPDefinitions(0), Internal(0)
{
}

//----------------------------------------------------------------------------
cmDependsFortran
::cmDependsFortran(cmLocalGenerator* lg):
  cmDepends(lg),
  Internal(new cmDependsFortranInternals)
{
  // Configure the include file search path.
  this->SetIncludePathFromLanguage("Fortran");

  // Get the list of definitions.
  std::vector<std::string> definitions;
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  if(const char* c_defines =
     mf->GetDefinition("CMAKE_TARGET_DEFINITIONS_Fortran"))
    {
    cmSystemTools::ExpandListArgument(c_defines, definitions);
    }

  // translate i.e. FOO=BAR to FOO and add it to the list of defined
  // preprocessor symbols
  for(std::vector<std::string>::const_iterator
      it = definitions.begin(); it != definitions.end(); ++it)
    {
    std::string def = *it;
    std::string::size_type assignment = def.find("=");
    if(assignment != std::string::npos)
      {
      def = it->substr(0, assignment);
      }
    this->PPDefinitions.push_back(def);
    }
}

//----------------------------------------------------------------------------
cmDependsFortran::~cmDependsFortran()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
bool cmDependsFortran::WriteDependencies(
    const std::set<std::string>& sources, const std::string& obj,
    std::ostream&, std::ostream&)
{
  // Make sure this is a scanning instance.
  if(sources.empty() || sources.begin()->empty())
    {
    cmSystemTools::Error("Cannot scan dependencies without a source file.");
    return false;
    }
  if(obj.empty())
    {
    cmSystemTools::Error("Cannot scan dependencies without an object file.");
    return false;
    }

  bool okay = true;
  for(std::set<std::string>::const_iterator it = sources.begin();
      it != sources.end(); ++it)
    {
    const std::string& src = *it;
    // Get the information object for this source.
    cmDependsFortranSourceInfo& info =
      this->Internal->CreateObjectInfo(obj.c_str(), src.c_str());

    // Make a copy of the macros defined via ADD_DEFINITIONS
    std::set<std::string> ppDefines(this->PPDefinitions.begin(),
                                    this->PPDefinitions.end());

    // Create the parser object. The constructor takes ppMacro and info per
    // reference, so we may look into the resulting objects later.
    cmDependsFortranParser parser(this, ppDefines, info);

    // Push on the starting file.
    cmDependsFortranParser_FilePush(&parser, src.c_str());

    // Parse the translation unit.
    if(cmDependsFortran_yyparse(parser.Scanner) != 0)
      {
      // Failed to parse the file.  Report failure to write dependencies.
      okay = false;
      }
    }
  return okay;
}

//----------------------------------------------------------------------------
bool cmDependsFortran::Finalize(std::ostream& makeDepends,
                                std::ostream& internalDepends)
{
  // Prepare the module search process.
  this->LocateModules();

  // Get the directory in which stamp files will be stored.
  const char* stamp_dir = this->TargetDirectory.c_str();

  // Get the directory in which module files will be created.
  const char* mod_dir;
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  if(const char* target_mod_dir =
     mf->GetDefinition("CMAKE_Fortran_TARGET_MODULE_DIR"))
    {
    mod_dir = target_mod_dir;
    }
  else
    {
    mod_dir =
      this->LocalGenerator->GetMakefile()->GetCurrentBinaryDirectory();
    }

  // Actually write dependencies to the streams.
  typedef cmDependsFortranInternals::ObjectInfoMap ObjectInfoMap;
  ObjectInfoMap const& objInfo = this->Internal->ObjectInfo;
  for(ObjectInfoMap::const_iterator i = objInfo.begin();
      i != objInfo.end(); ++i)
    {
    if(!this->WriteDependenciesReal(i->first.c_str(), i->second,
                                    mod_dir, stamp_dir,
                                    makeDepends, internalDepends))
      {
      return false;
      }
    }

  // Store the list of modules provided by this target.
  std::string fiName = this->TargetDirectory;
  fiName += "/fortran.internal";
  cmGeneratedFileStream fiStream(fiName.c_str());
  fiStream << "# The fortran modules provided by this target.\n";
  fiStream << "provides\n";
  std::set<std::string> const& provides = this->Internal->TargetProvides;
  for(std::set<std::string>::const_iterator i = provides.begin();
      i != provides.end(); ++i)
    {
    fiStream << " " << *i << "\n";
    }

  // Create a script to clean the modules.
  if(!provides.empty())
    {
    std::string fcName = this->TargetDirectory;
    fcName += "/cmake_clean_Fortran.cmake";
    cmGeneratedFileStream fcStream(fcName.c_str());
    fcStream << "# Remove fortran modules provided by this target.\n";
    fcStream << "FILE(REMOVE";
    for(std::set<std::string>::const_iterator i = provides.begin();
        i != provides.end(); ++i)
      {
      std::string mod_upper = mod_dir;
      mod_upper += "/";
      mod_upper += cmSystemTools::UpperCase(*i);
      mod_upper += ".mod";
      std::string mod_lower = mod_dir;
      mod_lower += "/";
      mod_lower += *i;
      mod_lower += ".mod";
      std::string stamp = stamp_dir;
      stamp += "/";
      stamp += *i;
      stamp += ".mod.stamp";
      fcStream << "\n";
      fcStream << "  \"" <<
        this->LocalGenerator->Convert(mod_lower,
                                      cmLocalGenerator::START_OUTPUT)
               << "\"\n";
      fcStream << "  \"" <<
        this->LocalGenerator->Convert(mod_upper,
                                      cmLocalGenerator::START_OUTPUT)
               << "\"\n";
      fcStream << "  \"" <<
        this->LocalGenerator->Convert(stamp,
                                      cmLocalGenerator::START_OUTPUT)
               << "\"\n";
      }
    fcStream << "  )\n";
    }
  return true;
}

//----------------------------------------------------------------------------
void cmDependsFortran::LocateModules()
{
  // Collect the set of modules provided and required by all sources.
  typedef cmDependsFortranInternals::ObjectInfoMap ObjectInfoMap;
  ObjectInfoMap const& objInfo = this->Internal->ObjectInfo;
  for(ObjectInfoMap::const_iterator infoI = objInfo.begin();
      infoI != objInfo.end(); ++infoI)
    {
    cmDependsFortranSourceInfo const& info = infoI->second;
    // Include this module in the set provided by this target.
    this->Internal->TargetProvides.insert(info.Provides.begin(),
                                          info.Provides.end());

    for(std::set<std::string>::const_iterator i = info.Requires.begin();
        i != info.Requires.end(); ++i)
      {
      this->Internal->TargetRequires[*i] = "";
      }
    }

  // Short-circuit for simple targets.
  if(this->Internal->TargetRequires.empty())
    {
    return;
    }

  // Match modules provided by this target to those it requires.
  this->MatchLocalModules();

  // Load information about other targets.
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  std::vector<std::string> infoFiles;
  if(const char* infoFilesValue =
     mf->GetDefinition("CMAKE_TARGET_LINKED_INFO_FILES"))
    {
    cmSystemTools::ExpandListArgument(infoFilesValue, infoFiles);
    }
  for(std::vector<std::string>::const_iterator i = infoFiles.begin();
      i != infoFiles.end(); ++i)
    {
    std::string targetDir = cmSystemTools::GetFilenamePath(*i);
    std::string fname = targetDir + "/fortran.internal";
    cmsys::ifstream fin(fname.c_str());
    if(fin)
      {
      this->MatchRemoteModules(fin, targetDir.c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmDependsFortran::MatchLocalModules()
{
  const char* stampDir = this->TargetDirectory.c_str();
  std::set<std::string> const& provides = this->Internal->TargetProvides;
  for(std::set<std::string>::const_iterator i = provides.begin();
      i != provides.end(); ++i)
    {
    this->ConsiderModule(i->c_str(), stampDir);
    }
}

//----------------------------------------------------------------------------
void cmDependsFortran::MatchRemoteModules(std::istream& fin,
                                          const char* stampDir)
{
  std::string line;
  bool doing_provides = false;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    // Ignore comments and empty lines.
    if(line.empty() || line[0] == '#' || line[0] == '\r')
      {
      continue;
      }

    if(line[0] == ' ')
      {
      if(doing_provides)
        {
        this->ConsiderModule(line.c_str()+1, stampDir);
        }
      }
    else if(line == "provides")
      {
      doing_provides = true;
      }
    else
      {
      doing_provides = false;
      }
    }
}

//----------------------------------------------------------------------------
void cmDependsFortran::ConsiderModule(const char* name,
                                      const char* stampDir)
{
  // Locate each required module.
  typedef cmDependsFortranInternals::TargetRequiresMap TargetRequiresMap;
  TargetRequiresMap::iterator required =
    this->Internal->TargetRequires.find(name);
  if(required != this->Internal->TargetRequires.end() &&
     required->second.empty())
    {
    // The module is provided by a CMake target.  It will have a stamp file.
    std::string stampFile = stampDir;
    stampFile += "/";
    stampFile += name;
    stampFile += ".mod.stamp";
    required->second = stampFile;
    }
}

//----------------------------------------------------------------------------
bool
cmDependsFortran
::WriteDependenciesReal(const char *obj,
                        cmDependsFortranSourceInfo const& info,
                        const char* mod_dir, const char* stamp_dir,
                        std::ostream& makeDepends,
                        std::ostream& internalDepends)
{
  typedef cmDependsFortranInternals::TargetRequiresMap TargetRequiresMap;

  // Get the source file for this object.
  const char* src = info.Source.c_str();

  // Write the include dependencies to the output stream.
  std::string obj_i =
    this->LocalGenerator->Convert(obj, cmLocalGenerator::HOME_OUTPUT);
  std::string obj_m =
    this->LocalGenerator->ConvertToOutputFormat(obj_i,
                                                cmLocalGenerator::MAKERULE);
  internalDepends << obj_i << std::endl;
  internalDepends << " " << src << std::endl;
  for(std::set<std::string>::const_iterator i = info.Includes.begin();
      i != info.Includes.end(); ++i)
    {
    makeDepends << obj_m << ": " <<
      this->LocalGenerator->Convert(*i,
                                    cmLocalGenerator::HOME_OUTPUT,
                                    cmLocalGenerator::MAKERULE)
                << std::endl;
    internalDepends << " " << *i << std::endl;
    }
  makeDepends << std::endl;

  // Write module requirements to the output stream.
  for(std::set<std::string>::const_iterator i = info.Requires.begin();
      i != info.Requires.end(); ++i)
    {
    // Require only modules not provided in the same source.
    if(std::set<std::string>::const_iterator(info.Provides.find(*i)) !=
       info.Provides.end())
      {
      continue;
      }

    // If the module is provided in this target special handling is
    // needed.
    if(this->Internal->TargetProvides.find(*i) !=
       this->Internal->TargetProvides.end())
      {
      // The module is provided by a different source in the same
      // target.  Add the proxy dependency to make sure the other
      // source builds first.
      std::string proxy = stamp_dir;
      proxy += "/";
      proxy += *i;
      proxy += ".mod.proxy";
      proxy = this->LocalGenerator->Convert(proxy,
                                            cmLocalGenerator::HOME_OUTPUT,
                                            cmLocalGenerator::MAKERULE);

      // since we require some things add them to our list of requirements
      makeDepends << obj_m << ".requires: " << proxy << std::endl;
      }

    // The object file should depend on timestamped files for the
    // modules it uses.
    TargetRequiresMap::const_iterator required =
      this->Internal->TargetRequires.find(*i);
    if(required == this->Internal->TargetRequires.end()) { abort(); }
    if(!required->second.empty())
      {
      // This module is known.  Depend on its timestamp file.
      std::string stampFile =
        this->LocalGenerator->Convert(required->second,
                                      cmLocalGenerator::HOME_OUTPUT,
                                      cmLocalGenerator::MAKERULE);
      makeDepends << obj_m << ": " << stampFile << "\n";
      }
    else
      {
      // This module is not known to CMake.  Try to locate it where
      // the compiler will and depend on that.
      std::string module;
      if(this->FindModule(*i, module))
        {
        module =
          this->LocalGenerator->Convert(module,
                                        cmLocalGenerator::HOME_OUTPUT,
                                        cmLocalGenerator::MAKERULE);
        makeDepends << obj_m << ": " << module << "\n";
        }
      }
    }

  // Write provided modules to the output stream.
  for(std::set<std::string>::const_iterator i = info.Provides.begin();
      i != info.Provides.end(); ++i)
    {
    std::string proxy = stamp_dir;
    proxy += "/";
    proxy += *i;
    proxy += ".mod.proxy";
    proxy = this->LocalGenerator->Convert(proxy,
                                          cmLocalGenerator::HOME_OUTPUT,
                                          cmLocalGenerator::MAKERULE);
    makeDepends << proxy << ": " << obj_m << ".provides" << std::endl;
    }

  // If any modules are provided then they must be converted to stamp files.
  if(!info.Provides.empty())
    {
    // Create a target to copy the module after the object file
    // changes.
    makeDepends << obj_m << ".provides.build:\n";
    for(std::set<std::string>::const_iterator i = info.Provides.begin();
        i != info.Provides.end(); ++i)
      {
      // Include this module in the set provided by this target.
      this->Internal->TargetProvides.insert(*i);

      // Always use lower case for the mod stamp file name.  The
      // cmake_copy_f90_mod will call back to this class, which will
      // try various cases for the real mod file name.
      std::string m = cmSystemTools::LowerCase(*i);
      std::string modFile = mod_dir;
      modFile += "/";
      modFile += *i;
      modFile =
        this->LocalGenerator->Convert(modFile,
                                      cmLocalGenerator::HOME_OUTPUT,
                                      cmLocalGenerator::SHELL);
      std::string stampFile = stamp_dir;
      stampFile += "/";
      stampFile += m;
      stampFile += ".mod.stamp";
      stampFile =
        this->LocalGenerator->Convert(stampFile,
                                      cmLocalGenerator::HOME_OUTPUT,
                                      cmLocalGenerator::SHELL);
      makeDepends << "\t$(CMAKE_COMMAND) -E cmake_copy_f90_mod "
                  << modFile << " " << stampFile;
      cmMakefile* mf = this->LocalGenerator->GetMakefile();
      const char* cid = mf->GetDefinition("CMAKE_Fortran_COMPILER_ID");
      if(cid && *cid)
        {
        makeDepends << " " << cid;
        }
      makeDepends << "\n";
      }
    // After copying the modules update the timestamp file so that
    // copying will not be done again until the source rebuilds.
    makeDepends << "\t$(CMAKE_COMMAND) -E touch " << obj_m
                << ".provides.build\n";

    // Make sure the module timestamp rule is evaluated by the time
    // the target finishes building.
    std::string driver = this->TargetDirectory;
    driver += "/build";
    driver = this->LocalGenerator->Convert(driver,
                                           cmLocalGenerator::HOME_OUTPUT,
                                           cmLocalGenerator::MAKERULE);
    makeDepends << driver << ": " << obj_m << ".provides.build\n";
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmDependsFortran::FindModule(std::string const& name,
                                  std::string& module)
{
  // Construct possible names for the module file.
  std::string mod_upper = cmSystemTools::UpperCase(name);
  std::string mod_lower = name;
  mod_upper += ".mod";
  mod_lower += ".mod";

  // Search the include path for the module.
  std::string fullName;
  for(std::vector<std::string>::const_iterator i =
        this->IncludePath.begin(); i != this->IncludePath.end(); ++i)
    {
    // Try the lower-case name.
    fullName = *i;
    fullName += "/";
    fullName += mod_lower;
    if(cmSystemTools::FileExists(fullName.c_str(), true))
      {
      module = fullName;
      return true;
      }

    // Try the upper-case name.
    fullName = *i;
    fullName += "/";
    fullName += mod_upper;
    if(cmSystemTools::FileExists(fullName.c_str(), true))
      {
      module = fullName;
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmDependsFortran::CopyModule(const std::vector<std::string>& args)
{
  // Implements
  //
  //   $(CMAKE_COMMAND) -E cmake_copy_f90_mod input.mod output.mod.stamp
  //                                          [compiler-id]
  //
  // Note that the case of the .mod file depends on the compiler.  In
  // the future this copy could also account for the fact that some
  // compilers include a timestamp in the .mod file so it changes even
  // when the interface described in the module does not.

  std::string mod = args[2];
  std::string stamp = args[3];
  std::string compilerId;
  if(args.size() >= 5)
    {
    compilerId = args[4];
    }
  std::string mod_dir = cmSystemTools::GetFilenamePath(mod);
  if(!mod_dir.empty()) { mod_dir += "/"; }
  std::string mod_upper = mod_dir;
  mod_upper += cmSystemTools::UpperCase(cmSystemTools::GetFilenameName(mod));
  std::string mod_lower = mod_dir;
  mod_lower += cmSystemTools::LowerCase(cmSystemTools::GetFilenameName(mod));
  mod += ".mod";
  mod_upper += ".mod";
  mod_lower += ".mod";
  if(cmSystemTools::FileExists(mod_upper.c_str(), true))
    {
    if(cmDependsFortran::ModulesDiffer(mod_upper.c_str(), stamp.c_str(),
                                       compilerId.c_str()))
      {
      if(!cmSystemTools::CopyFileAlways(mod_upper, stamp))
        {
        std::cerr << "Error copying Fortran module from \""
                  << mod_upper << "\" to \"" << stamp
                  << "\".\n";
        return false;
        }
      }
    return true;
    }
  else if(cmSystemTools::FileExists(mod_lower.c_str(), true))
    {
    if(cmDependsFortran::ModulesDiffer(mod_lower.c_str(), stamp.c_str(),
                                       compilerId.c_str()))
      {
      if(!cmSystemTools::CopyFileAlways(mod_lower, stamp))
        {
        std::cerr << "Error copying Fortran module from \""
                  << mod_lower << "\" to \"" << stamp
                  << "\".\n";
        return false;
        }
      }
    return true;
    }

  std::cerr << "Error copying Fortran module \"" << args[2]
            << "\".  Tried \"" << mod_upper
            << "\" and \"" << mod_lower << "\".\n";
  return false;
}

//----------------------------------------------------------------------------
// Helper function to look for a short sequence in a stream.  If this
// is later used for longer sequences it should be re-written using an
// efficient string search algorithm such as Boyer-Moore.
static
bool cmDependsFortranStreamContainsSequence(std::istream& ifs,
                                            const char* seq, int len)
{
  assert(len > 0);

  int cur = 0;
  while(cur < len)
    {
    // Get the next character.
    int token = ifs.get();
    if(!ifs)
      {
      return false;
      }

    // Check the character.
    if(token == static_cast<int>(seq[cur]))
      {
      ++cur;
      }
    else
      {
      // Assume the sequence has no repeating subsequence.
      cur = 0;
      }
    }

  // The entire sequence was matched.
  return true;
}

//----------------------------------------------------------------------------
// Helper function to compare the remaining content in two streams.
static bool cmDependsFortranStreamsDiffer(std::istream& ifs1,
                                          std::istream& ifs2)
{
  // Compare the remaining content.
  for(;;)
    {
    int ifs1_c = ifs1.get();
    int ifs2_c = ifs2.get();
    if(!ifs1 && !ifs2)
      {
      // We have reached the end of both streams simultaneously.
      // The streams are identical.
      return false;
      }

    if(!ifs1 || !ifs2 || ifs1_c != ifs2_c)
      {
      // We have reached the end of one stream before the other or
      // found differing content.  The streams are different.
      break;
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmDependsFortran::ModulesDiffer(const char* modFile,
                                     const char* stampFile,
                                     const char* compilerId)
{
  /*
  gnu >= 4.9:
    A mod file is an ascii file compressed with gzip.
    Compiling twice produces identical modules.

  gnu < 4.9:
    A mod file is an ascii file.
    <bar.mod>
    FORTRAN module created from /path/to/foo.f90 on Sun Dec 30 22:47:58 2007
    If you edit this, you'll get what you deserve.
    ...
    </bar.mod>
    As you can see the first line contains the date.

  intel:
    A mod file is a binary file.
    However, looking into both generated bar.mod files with a hex editor
    shows that they differ only before a sequence linefeed-zero (0x0A 0x00)
    which is located some bytes in front of the absoulte path to the source
    file.

  sun:
    A mod file is a binary file.  Compiling twice produces identical modules.

  others:
    TODO ...
  */


  /* Compilers which do _not_ produce different mod content when the same
   * source is compiled twice
   *   -SunPro
   */
  if(strcmp(compilerId, "SunPro") == 0)
    {
    return cmSystemTools::FilesDiffer(modFile, stampFile);
    }

#if defined(_WIN32) || defined(__CYGWIN__)
  cmsys::ifstream finModFile(modFile, std::ios::in | std::ios::binary);
  cmsys::ifstream finStampFile(stampFile, std::ios::in | std::ios::binary);
#else
  cmsys::ifstream finModFile(modFile, std::ios::in);
  cmsys::ifstream finStampFile(stampFile, std::ios::in);
#endif
  if(!finModFile || !finStampFile)
    {
    // At least one of the files does not exist.  The modules differ.
    return true;
    }

  /* Compilers which _do_ produce different mod content when the same
   * source is compiled twice
   *   -GNU
   *   -Intel
   *
   * Eat the stream content until all recompile only related changes
   * are left behind.
   */
  if (strcmp(compilerId, "GNU") == 0 )
    {
    // GNU Fortran 4.9 and later compress .mod files with gzip
    // but also do not include a date so we can fall through to
    // compare them without skipping any prefix.
    unsigned char hdr[2];
    bool okay = finModFile.read(reinterpret_cast<char*>(hdr), 2)? true:false;
    finModFile.seekg(0);
    if(!(okay && hdr[0] == 0x1f && hdr[1] == 0x8b))
      {
      const char seq[1] = {'\n'};
      const int seqlen = 1;

      if(!cmDependsFortranStreamContainsSequence(finModFile, seq, seqlen))
        {
        // The module is of unexpected format.  Assume it is different.
        std::cerr << compilerId << " fortran module " << modFile
                  << " has unexpected format." << std::endl;
        return true;
        }

      if(!cmDependsFortranStreamContainsSequence(finStampFile, seq, seqlen))
        {
        // The stamp must differ if the sequence is not contained.
        return true;
        }
      }
    }
  else if(strcmp(compilerId, "Intel") == 0)
    {
    const char seq[2] = {'\n', '\0'};
    const int seqlen = 2;

    if(!cmDependsFortranStreamContainsSequence(finModFile, seq, seqlen))
      {
      // The module is of unexpected format.  Assume it is different.
      std::cerr << compilerId << " fortran module " << modFile
                << " has unexpected format." << std::endl;
      return true;
      }

    if(!cmDependsFortranStreamContainsSequence(finStampFile, seq, seqlen))
      {
      // The stamp must differ if the sequence is not contained.
      return true;
      }
    }

  // Compare the remaining content.  If no compiler id matched above,
  // including the case none was given, this will compare the whole
  // content.
  if(!cmDependsFortranStreamsDiffer(finModFile, finStampFile))
    {
    return false;
    }

   // The modules are different.
   return true;
}

//----------------------------------------------------------------------------
bool cmDependsFortran::FindIncludeFile(const char* dir,
                                       const char* includeName,
                                       std::string& fileName)
{
  // If the file is a full path, include it directly.
  if(cmSystemTools::FileIsFullPath(includeName))
    {
    fileName = includeName;
    return cmSystemTools::FileExists(fileName.c_str(), true);
    }
  else
    {
    // Check for the file in the directory containing the including
    // file.
    std::string fullName = dir;
    fullName += "/";
    fullName += includeName;
    if(cmSystemTools::FileExists(fullName.c_str(), true))
      {
      fileName = fullName;
      return true;
      }

    // Search the include path for the file.
    for(std::vector<std::string>::const_iterator i =
          this->IncludePath.begin(); i != this->IncludePath.end(); ++i)
      {
      fullName = *i;
      fullName += "/";
      fullName += includeName;
      if(cmSystemTools::FileExists(fullName.c_str(), true))
        {
        fileName = fullName;
        return true;
        }
      }
    }
  return false;
}

//----------------------------------------------------------------------------
cmDependsFortranParser_s
::cmDependsFortranParser_s(cmDependsFortran* self,
                           std::set<std::string>& ppDefines,
                           cmDependsFortranSourceInfo& info):
  Self(self), PPDefinitions(ppDefines), Info(info)
{
  this->InInterface = 0;
  this->InPPFalseBranch = 0;

  // Initialize the lexical scanner.
  cmDependsFortran_yylex_init(&this->Scanner);
  cmDependsFortran_yyset_extra(this, this->Scanner);

  // Create a dummy buffer that is never read but is the fallback
  // buffer when the last file is popped off the stack.
  YY_BUFFER_STATE buffer =
    cmDependsFortran_yy_create_buffer(0, 4, this->Scanner);
  cmDependsFortran_yy_switch_to_buffer(buffer, this->Scanner);
}

//----------------------------------------------------------------------------
cmDependsFortranParser_s::~cmDependsFortranParser_s()
{
  cmDependsFortran_yylex_destroy(this->Scanner);
}

//----------------------------------------------------------------------------
bool cmDependsFortranParser_FilePush(cmDependsFortranParser* parser,
                                    const char* fname)
{
  // Open the new file and push it onto the stack.  Save the old
  // buffer with it on the stack.
  if(FILE* file = cmsys::SystemTools::Fopen(fname, "rb"))
    {
    YY_BUFFER_STATE current =
      cmDependsFortranLexer_GetCurrentBuffer(parser->Scanner);
    std::string dir = cmSystemTools::GetParentDirectory(fname);
    cmDependsFortranFile f(file, current, dir);
    YY_BUFFER_STATE buffer =
      cmDependsFortran_yy_create_buffer(0, 16384, parser->Scanner);
    cmDependsFortran_yy_switch_to_buffer(buffer, parser->Scanner);
    parser->FileStack.push(f);
    return 1;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
bool cmDependsFortranParser_FilePop(cmDependsFortranParser* parser)
{
  // Pop one file off the stack and close it.  Switch the lexer back
  // to the next one on the stack.
  if(parser->FileStack.empty())
    {
    return 0;
    }
  else
    {
    cmDependsFortranFile f = parser->FileStack.top(); parser->FileStack.pop();
    fclose(f.File);
    YY_BUFFER_STATE current =
      cmDependsFortranLexer_GetCurrentBuffer(parser->Scanner);
    cmDependsFortran_yy_delete_buffer(current, parser->Scanner);
    cmDependsFortran_yy_switch_to_buffer(f.Buffer, parser->Scanner);
    return 1;
    }
}

//----------------------------------------------------------------------------
int cmDependsFortranParser_Input(cmDependsFortranParser* parser,
                                 char* buffer, size_t bufferSize)
{
  // Read from the file on top of the stack.  If the stack is empty,
  // the end of the translation unit has been reached.
  if(!parser->FileStack.empty())
    {
    FILE* file = parser->FileStack.top().File;
    return (int)fread(buffer, 1, bufferSize, file);
    }
  return 0;
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_StringStart(cmDependsFortranParser* parser)
{
  parser->TokenString = "";
}

//----------------------------------------------------------------------------
const char* cmDependsFortranParser_StringEnd(cmDependsFortranParser* parser)
{
  return parser->TokenString.c_str();
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_StringAppend(cmDependsFortranParser* parser,
                                         char c)
{
  parser->TokenString += c;
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_SetInInterface(cmDependsFortranParser* parser,
                                           bool in)
{
  if(parser->InPPFalseBranch)
    {
    return;
    }

  parser->InInterface = in;
}

//----------------------------------------------------------------------------
bool cmDependsFortranParser_GetInInterface(cmDependsFortranParser* parser)
{
  return parser->InInterface;
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_SetOldStartcond(cmDependsFortranParser* parser,
                                            int arg)
{
  parser->OldStartcond = arg;
}

//----------------------------------------------------------------------------
int cmDependsFortranParser_GetOldStartcond(cmDependsFortranParser* parser)
{
  return parser->OldStartcond;
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_Error(cmDependsFortranParser*, const char*)
{
  // If there is a parser error just ignore it.  The source will not
  // compile and the user will edit it.  Then dependencies will have
  // to be regenerated anyway.
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleUse(cmDependsFortranParser* parser,
                                    const char* name)
{
  if(!parser->InPPFalseBranch)
    {
    parser->Info.Requires.insert(cmSystemTools::LowerCase(name) );
    }
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleInclude(cmDependsFortranParser* parser,
                                        const char* name)
{
  if(parser->InPPFalseBranch)
    {
    return;
    }

  // If processing an include statement there must be an open file.
  assert(!parser->FileStack.empty());

  // Get the directory containing the source in which the include
  // statement appears.  This is always the first search location for
  // Fortran include files.
  std::string dir = parser->FileStack.top().Directory;

  // Find the included file.  If it cannot be found just ignore the
  // problem because either the source will not compile or the user
  // does not care about depending on this included source.
  std::string fullName;
  if(parser->Self->FindIncludeFile(dir.c_str(), name, fullName))
    {
    // Found the included file.  Save it in the set of included files.
    parser->Info.Includes.insert(fullName);

    // Parse it immediately to translate the source inline.
    cmDependsFortranParser_FilePush(parser, fullName.c_str());
    }
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleModule(cmDependsFortranParser* parser,
                                       const char* name)
{
  if(!parser->InPPFalseBranch && !parser->InInterface)
    {
    parser->Info.Provides.insert(cmSystemTools::LowerCase(name));
    }
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleDefine(cmDependsFortranParser* parser,
                                       const char* macro)
{
  if(!parser->InPPFalseBranch)
    {
    parser->PPDefinitions.insert(macro);
    }
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleUndef(cmDependsFortranParser* parser,
                                      const char* macro)
{
  if(!parser->InPPFalseBranch)
    {
    std::set<std::string>::iterator match;
    match = parser->PPDefinitions.find(macro);
    if(match != parser->PPDefinitions.end())
      {
      parser->PPDefinitions.erase(match);
      }
    }
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleIfdef(cmDependsFortranParser* parser,
                                      const char* macro)
{
  // A new PP branch has been opened
  parser->SkipToEnd.push(false);

  if (parser->InPPFalseBranch)
    {
    parser->InPPFalseBranch++;
    }
  else if(parser->PPDefinitions.find(macro) == parser->PPDefinitions.end())
    {
    parser->InPPFalseBranch=1;
    }
  else
    {
    parser->SkipToEnd.top() = true;
    }
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleIfndef(cmDependsFortranParser* parser,
  const char* macro)
{
  // A new PP branch has been opened
  parser->SkipToEnd.push(false);

  if (parser->InPPFalseBranch)
    {
    parser->InPPFalseBranch++;
    }
  else if(parser->PPDefinitions.find(macro) != parser->PPDefinitions.end())
    {
    parser->InPPFalseBranch = 1;
    }
  else
    {
    // ignore other branches
    parser->SkipToEnd.top() = true;
    }
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleIf(cmDependsFortranParser* parser)
{
  /* Note: The current parser is _not_ able to get statements like
   *   #if 0
   *   #if 1
   *   #if MYSMBOL
   *   #if defined(MYSYMBOL)
   *   #if defined(MYSYMBOL) && ...
   * right.  The same for #elif.  Thus in
   *   #if SYMBOL_1
   *     ..
   *   #elif SYMBOL_2
   *     ...
   *     ...
   *   #elif SYMBOL_N
   *     ..
   *   #else
   *     ..
   *   #endif
   * _all_ N+1 branches are considered.  If you got something like this
   *   #if defined(MYSYMBOL)
   *   #if !defined(MYSYMBOL)
   * use
   *   #ifdef MYSYMBOL
   *   #ifndef MYSYMBOL
   * instead.
   */

  // A new PP branch has been opened
  // Never skip!  See note above.
  parser->SkipToEnd.push(false);
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleElif(cmDependsFortranParser* parser)
{
  /* Note: There are parser limitations.  See the note at
   * cmDependsFortranParser_RuleIf(..)
   */

  // Always taken unless an #ifdef or #ifndef-branch has been taken
  // already.  If the second condition isn't meet already
  // (parser->InPPFalseBranch == 0) correct it.
  if(!parser->SkipToEnd.empty() &&
     parser->SkipToEnd.top() && !parser->InPPFalseBranch)
    {
    parser->InPPFalseBranch = 1;
    }
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleElse(cmDependsFortranParser* parser)
{
  // if the parent branch is false do nothing!
  if(parser->InPPFalseBranch > 1)
    {
    return;
    }

  // parser->InPPFalseBranch is either 0 or 1.  We change it depending on
  // parser->SkipToEnd.top()
  if(!parser->SkipToEnd.empty() &&
     parser->SkipToEnd.top())
    {
    parser->InPPFalseBranch = 1;
    }
  else
    {
    parser->InPPFalseBranch = 0;
    }
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleEndif(cmDependsFortranParser* parser)
{
  if(!parser->SkipToEnd.empty())
    {
    parser->SkipToEnd.pop();
    }

  // #endif doesn't know if there was a "#else" in before, so it
  // always decreases InPPFalseBranch
  if(parser->InPPFalseBranch)
    {
    parser->InPPFalseBranch--;
    }
}
