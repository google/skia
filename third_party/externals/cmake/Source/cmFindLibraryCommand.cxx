/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFindLibraryCommand.h"
#include <cmsys/Directory.hxx>
#include <cmsys/stl/algorithm>

cmFindLibraryCommand::cmFindLibraryCommand()
{
  this->EnvironmentPath = "LIB";
  this->NamesPerDirAllowed = true;
}

// cmFindLibraryCommand
bool cmFindLibraryCommand
::InitialPass(std::vector<std::string> const& argsIn, cmExecutionStatus &)
{
  this->VariableDocumentation = "Path to a library.";
  this->CMakePathName = "LIBRARY";
  if(!this->ParseArguments(argsIn))
    {
    return false;
    }
  if(this->AlreadyInCache)
    {
    // If the user specifies the entry on the command line without a
    // type we should add the type and docstring but keep the original
    // value.
    if(this->AlreadyInCacheWithoutMetaInfo)
      {
      this->Makefile->AddCacheDefinition(this->VariableName, "",
                                         this->VariableDocumentation.c_str(),
                                         cmState::FILEPATH);
      }
    return true;
    }

  if(const char* abi_name =
     this->Makefile->GetDefinition("CMAKE_INTERNAL_PLATFORM_ABI"))
    {
    std::string abi = abi_name;
    if(abi.find("ELF N32") != abi.npos)
      {
      // Convert lib to lib32.
      this->AddArchitecturePaths("32");
      }
    }

  if(this->Makefile->GetState()
         ->GetGlobalPropertyAsBool("FIND_LIBRARY_USE_LIB64_PATHS"))
    {
    // add special 64 bit paths if this is a 64 bit compile.
    if(this->Makefile->PlatformIs64Bit())
      {
      this->AddArchitecturePaths("64");
      }
    }

  std::string library = this->FindLibrary();
  if(library != "")
    {
    // Save the value in the cache
    this->Makefile->AddCacheDefinition(this->VariableName,
                                       library.c_str(),
                                       this->VariableDocumentation.c_str(),
                                       cmState::FILEPATH);
    return true;
    }
  std::string notfound = this->VariableName + "-NOTFOUND";
  this->Makefile->AddCacheDefinition(this->VariableName,
                                     notfound.c_str(),
                                     this->VariableDocumentation.c_str(),
                                     cmState::FILEPATH);
  return true;
}

//----------------------------------------------------------------------------
void cmFindLibraryCommand::AddArchitecturePaths(const char* suffix)
{
  std::vector<std::string> original;
  original.swap(this->SearchPaths);
  for(std::vector<std::string>::const_iterator i = original.begin();
      i != original.end(); ++i)
    {
    this->AddArchitecturePath(*i, 0, suffix);
    }
}

//----------------------------------------------------------------------------
void cmFindLibraryCommand::AddArchitecturePath(
  std::string const& dir, std::string::size_type start_pos,
  const char* suffix, bool fresh)
{
  std::string::size_type pos = dir.find("lib/", start_pos);
  if(pos != std::string::npos)
    {
    std::string cur_dir  = dir.substr(0,pos+3);

    // Follow "lib<suffix>".
    std::string next_dir = cur_dir + suffix;
    if(cmSystemTools::FileIsDirectory(next_dir))
      {
      next_dir += dir.substr(pos+3);
      std::string::size_type next_pos = pos+3+strlen(suffix)+1;
      this->AddArchitecturePath(next_dir, next_pos, suffix);
      }

    // Follow "lib".
    if(cmSystemTools::FileIsDirectory(cur_dir))
      {
      this->AddArchitecturePath(dir, pos+3+1, suffix, false);
      }
    }
  if(fresh)
    {
    // Check for <dir><suffix>/.
    std::string cur_dir  = dir + suffix + "/";
    if(cmSystemTools::FileIsDirectory(cur_dir))
      {
      this->SearchPaths.push_back(cur_dir);
      }

    // Now add the original unchanged path
    if(cmSystemTools::FileIsDirectory(dir))
      {
      this->SearchPaths.push_back(dir);
      }
    }
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindLibrary()
{
  std::string library;
  if(this->SearchFrameworkFirst || this->SearchFrameworkOnly)
    {
    library = this->FindFrameworkLibrary();
    }
  if(library.empty() && !this->SearchFrameworkOnly)
    {
    library = this->FindNormalLibrary();
    }
  if(library.empty() && this->SearchFrameworkLast)
    {
    library = this->FindFrameworkLibrary();
    }
  return library;
}

//----------------------------------------------------------------------------
struct cmFindLibraryHelper
{
  cmFindLibraryHelper(cmMakefile* mf);

  // Context information.
  cmMakefile* Makefile;
  cmGlobalGenerator* GG;

  // List of valid prefixes and suffixes.
  std::vector<std::string> Prefixes;
  std::vector<std::string> Suffixes;
  std::string PrefixRegexStr;
  std::string SuffixRegexStr;

  // Keep track of the best library file found so far.
  typedef std::vector<std::string>::size_type size_type;
  std::string BestPath;

  // Support for OpenBSD shared library naming: lib<name>.so.<major>.<minor>
  bool OpenBSD;

  // Current names under consideration.
  struct Name
  {
    bool TryRaw;
    std::string Raw;
    cmsys::RegularExpression Regex;
    Name(): TryRaw(false) {}
  };
  std::vector<Name> Names;

  // Current full path under consideration.
  std::string TestPath;

  void RegexFromLiteral(std::string& out, std::string const& in);
  void RegexFromList(std::string& out, std::vector<std::string> const& in);
  size_type GetPrefixIndex(std::string const& prefix)
    {
    return std::find(this->Prefixes.begin(), this->Prefixes.end(),
                           prefix) - this->Prefixes.begin();
    }
  size_type GetSuffixIndex(std::string const& suffix)
    {
    return std::find(this->Suffixes.begin(), this->Suffixes.end(),
                           suffix) - this->Suffixes.begin();
    }
  bool HasValidSuffix(std::string const& name);
  void AddName(std::string const& name);
  bool CheckDirectory(std::string const& path);
  bool CheckDirectoryForName(std::string const& path, Name& name);
};

//----------------------------------------------------------------------------
cmFindLibraryHelper::cmFindLibraryHelper(cmMakefile* mf):
  Makefile(mf)
{
  this->GG = this->Makefile->GetGlobalGenerator();

  // Collect the list of library name prefixes/suffixes to try.
  const char* prefixes_list =
    this->Makefile->GetRequiredDefinition("CMAKE_FIND_LIBRARY_PREFIXES");
  const char* suffixes_list =
    this->Makefile->GetRequiredDefinition("CMAKE_FIND_LIBRARY_SUFFIXES");
  cmSystemTools::ExpandListArgument(prefixes_list, this->Prefixes, true);
  cmSystemTools::ExpandListArgument(suffixes_list, this->Suffixes, true);
  this->RegexFromList(this->PrefixRegexStr, this->Prefixes);
  this->RegexFromList(this->SuffixRegexStr, this->Suffixes);

  // Check whether to use OpenBSD-style library version comparisons.
  this->OpenBSD =
    this->Makefile->GetState()
        ->GetGlobalPropertyAsBool("FIND_LIBRARY_USE_OPENBSD_VERSIONING");
}

//----------------------------------------------------------------------------
void cmFindLibraryHelper::RegexFromLiteral(std::string& out,
                                           std::string const& in)
{
  for(std::string::const_iterator ci = in.begin(); ci != in.end(); ++ci)
    {
    char ch = *ci;
    if(ch == '[' || ch == ']' || ch == '(' || ch == ')' || ch == '\\' ||
       ch == '.' || ch == '*' || ch == '+' || ch == '?' || ch == '-' ||
       ch == '^' || ch == '$')
      {
      out += "\\";
      }
#if defined(_WIN32) || defined(__APPLE__)
    out += tolower(ch);
#else
    out += ch;
#endif
    }
}

//----------------------------------------------------------------------------
void cmFindLibraryHelper::RegexFromList(std::string& out,
                                        std::vector<std::string> const& in)
{
  // Surround the list in parens so the '|' does not apply to anything
  // else and the result can be checked after matching.
  out += "(";
  const char* sep = "";
  for(std::vector<std::string>::const_iterator si = in.begin();
      si != in.end(); ++si)
    {
    // Separate from previous item.
    out += sep;
    sep = "|";

    // Append this item.
    this->RegexFromLiteral(out, *si);
    }
  out += ")";
}

//----------------------------------------------------------------------------
bool cmFindLibraryHelper::HasValidSuffix(std::string const& name)
{
  for(std::vector<std::string>::const_iterator si = this->Suffixes.begin();
      si != this->Suffixes.end(); ++si)
    {
    std::string suffix = *si;
    if(name.length() <= suffix.length())
      {
      continue;
      }
    // Check if the given name ends in a valid library suffix.
    if(name.substr(name.size()-suffix.length()) == suffix)
      {
      return true;
      }
    // Check if a valid library suffix is somewhere in the name,
    // this may happen e.g. for versioned shared libraries: libfoo.so.2
    suffix += ".";
    if(name.find(suffix) != name.npos)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void cmFindLibraryHelper::AddName(std::string const& name)
{
  Name entry;

  // Consider checking the raw name too.
  entry.TryRaw = this->HasValidSuffix(name);
  entry.Raw = name;

  // Build a regular expression to match library names.
  std::string regex = "^";
  regex += this->PrefixRegexStr;
  this->RegexFromLiteral(regex, name);
  regex += this->SuffixRegexStr;
  if(this->OpenBSD)
    {
    regex += "(\\.[0-9]+\\.[0-9]+)?";
    }
  regex += "$";
  entry.Regex.compile(regex.c_str());
  this->Names.push_back(entry);
}

//----------------------------------------------------------------------------
bool cmFindLibraryHelper::CheckDirectory(std::string const& path)
{
  for(std::vector<Name>::iterator i = this->Names.begin();
      i != this->Names.end(); ++i)
    {
    if(this->CheckDirectoryForName(path, *i))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmFindLibraryHelper::CheckDirectoryForName(std::string const& path,
                                                Name& name)
{
  // If the original library name provided by the user matches one of
  // the suffixes, try it first.  This allows users to search
  // specifically for a static library on some platforms (on MS tools
  // one cannot tell just from the library name whether it is a static
  // library or an import library).
  if(name.TryRaw)
    {
    this->TestPath = path;
    this->TestPath += name.Raw;
    if(cmSystemTools::FileExists(this->TestPath.c_str(), true))
      {
      this->BestPath =
        cmSystemTools::CollapseFullPath(this->TestPath);
      cmSystemTools::ConvertToUnixSlashes(this->BestPath);
      return true;
      }
    }

  // No library file has yet been found.
  size_type bestPrefix = this->Prefixes.size();
  size_type bestSuffix = this->Suffixes.size();
  unsigned int bestMajor = 0;
  unsigned int bestMinor = 0;

  // Search for a file matching the library name regex.
  std::string dir = path;
  cmSystemTools::ConvertToUnixSlashes(dir);
  std::set<std::string> const& files = this->GG->GetDirectoryContent(dir);
  for(std::set<std::string>::const_iterator fi = files.begin();
      fi != files.end(); ++fi)
    {
    std::string const& origName = *fi;
#if defined(_WIN32) || defined(__APPLE__)
    std::string testName = cmSystemTools::LowerCase(origName);
#else
    std::string const& testName = origName;
#endif
    if(name.Regex.find(testName))
      {
      this->TestPath = path;
      this->TestPath += origName;
      if(!cmSystemTools::FileIsDirectory(this->TestPath))
        {
        // This is a matching file.  Check if it is better than the
        // best name found so far.  Earlier prefixes are preferred,
        // followed by earlier suffixes.  For OpenBSD, shared library
        // version extensions are compared.
        size_type prefix = this->GetPrefixIndex(name.Regex.match(1));
        size_type suffix = this->GetSuffixIndex(name.Regex.match(2));
        unsigned int major = 0;
        unsigned int minor = 0;
        if(this->OpenBSD)
          {
          sscanf(name.Regex.match(3).c_str(), ".%u.%u", &major, &minor);
          }
        if(this->BestPath.empty() || prefix < bestPrefix ||
           (prefix == bestPrefix && suffix < bestSuffix) ||
           (prefix == bestPrefix && suffix == bestSuffix &&
            (major > bestMajor ||
             (major == bestMajor && minor > bestMinor))))
          {
          this->BestPath = this->TestPath;
          bestPrefix = prefix;
          bestSuffix = suffix;
          bestMajor = major;
          bestMinor = minor;
          }
        }
      }
    }

  // Use the best candidate found in this directory, if any.
  return !this->BestPath.empty();
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindNormalLibrary()
{
  if(this->NamesPerDir)
    {
    return this->FindNormalLibraryNamesPerDir();
    }
  else
    {
    return this->FindNormalLibraryDirsPerName();
    }
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindNormalLibraryNamesPerDir()
{
  // Search for all names in each directory.
  cmFindLibraryHelper helper(this->Makefile);
  for(std::vector<std::string>::const_iterator ni = this->Names.begin();
      ni != this->Names.end() ; ++ni)
    {
    helper.AddName(*ni);
    }
  // Search every directory.
  for(std::vector<std::string>::const_iterator
        p = this->SearchPaths.begin(); p != this->SearchPaths.end(); ++p)
    {
    if(helper.CheckDirectory(*p))
      {
      return helper.BestPath;
      }
    }
  // Couldn't find the library.
  return "";
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindNormalLibraryDirsPerName()
{
  // Search the entire path for each name.
  cmFindLibraryHelper helper(this->Makefile);
  for(std::vector<std::string>::const_iterator ni = this->Names.begin();
      ni != this->Names.end() ; ++ni)
    {
    // Switch to searching for this name.
    std::string const& name = *ni;
    helper.AddName(name);

    // Search every directory.
    for(std::vector<std::string>::const_iterator
          p = this->SearchPaths.begin();
        p != this->SearchPaths.end(); ++p)
      {
      if(helper.CheckDirectory(*p))
        {
        return helper.BestPath;
        }
      }
    }
  // Couldn't find the library.
  return "";
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindFrameworkLibrary()
{
  if(this->NamesPerDir)
    {
    return this->FindFrameworkLibraryNamesPerDir();
    }
  else
    {
    return this->FindFrameworkLibraryDirsPerName();
    }
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindFrameworkLibraryNamesPerDir()
{
  std::string fwPath;
  // Search for all names in each search path.
  for(std::vector<std::string>::const_iterator di = this->SearchPaths.begin();
      di != this->SearchPaths.end(); ++di)
    {
    for(std::vector<std::string>::const_iterator ni = this->Names.begin();
        ni != this->Names.end() ; ++ni)
      {
      fwPath = *di;
      fwPath += *ni;
      fwPath += ".framework";
      if(cmSystemTools::FileIsDirectory(fwPath))
        {
        return cmSystemTools::CollapseFullPath(fwPath);
        }
      }
    }

  // No framework found.
  return "";
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindFrameworkLibraryDirsPerName()
{
  std::string fwPath;
  // Search for each name in all search paths.
  for(std::vector<std::string>::const_iterator ni = this->Names.begin();
      ni != this->Names.end() ; ++ni)
    {
    for(std::vector<std::string>::const_iterator
          di = this->SearchPaths.begin();
        di != this->SearchPaths.end(); ++di)
      {
      fwPath = *di;
      fwPath += *ni;
      fwPath += ".framework";
      if(cmSystemTools::FileIsDirectory(fwPath))
        {
        return cmSystemTools::CollapseFullPath(fwPath);
        }
      }
    }

  // No framework found.
  return "";
}
