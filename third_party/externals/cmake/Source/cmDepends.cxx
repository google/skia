/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDepends.h"

#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"
#include "cmFileTimeComparison.h"
#include <string.h>
#include <cmsys/FStream.hxx>

//----------------------------------------------------------------------------
cmDepends::cmDepends(cmLocalGenerator* lg, const char* targetDir):
  CompileDirectory(),
  LocalGenerator(lg),
  Verbose(false),
  FileComparison(0),
  TargetDirectory(targetDir),
  MaxPath(16384),
  Dependee(new char[MaxPath]),
  Depender(new char[MaxPath])
{
}

//----------------------------------------------------------------------------
cmDepends::~cmDepends()
{
  delete [] this->Dependee;
  delete [] this->Depender;
}

//----------------------------------------------------------------------------
bool cmDepends::Write(std::ostream &makeDepends,
                      std::ostream &internalDepends)
{
  // Lookup the set of sources to scan.
  std::string srcLang = "CMAKE_DEPENDS_CHECK_";
  srcLang += this->Language;
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  const char* srcStr = mf->GetSafeDefinition(srcLang);
  std::vector<std::string> pairs;
  cmSystemTools::ExpandListArgument(srcStr, pairs);

  std::map<std::string, std::set<std::string> > dependencies;
  for(std::vector<std::string>::iterator si = pairs.begin();
      si != pairs.end();)
    {
    // Get the source and object file.
    std::string const& src = *si++;
    if(si == pairs.end()) { break; }
    std::string const& obj = *si++;
    dependencies[obj].insert(src);
    }
  for(std::map<std::string, std::set<std::string> >::const_iterator
      it = dependencies.begin(); it != dependencies.end(); ++it)
    {

    // Write the dependencies for this pair.
    if(!this->WriteDependencies(it->second, it->first,
                                makeDepends, internalDepends))
      {
      return false;
      }
    }

  return this->Finalize(makeDepends, internalDepends);
}

//----------------------------------------------------------------------------
bool cmDepends::Finalize(std::ostream&,
                         std::ostream&)
{
  return true;
}

//----------------------------------------------------------------------------
bool cmDepends::Check(const char *makeFile, const char *internalFile,
                      std::map<std::string, DependencyVector>& validDeps)
{
  // Dependency checks must be done in proper working directory.
  std::string oldcwd = ".";
  if(this->CompileDirectory != ".")
    {
    // Get the CWD but do not call CollapseFullPath because
    // we only need it to cd back, and the form does not matter
    oldcwd = cmSystemTools::GetCurrentWorkingDirectory(false);
    cmSystemTools::ChangeDirectory(this->CompileDirectory);
    }

  // Check whether dependencies must be regenerated.
  bool okay = true;
  cmsys::ifstream fin(internalFile);
  if(!(fin && this->CheckDependencies(fin, internalFile, validDeps)))
    {
    // Clear all dependencies so they will be regenerated.
    this->Clear(makeFile);
    cmSystemTools::RemoveFile(internalFile);
    okay = false;
    }

  // Restore working directory.
  if(oldcwd != ".")
    {
    cmSystemTools::ChangeDirectory(oldcwd);
    }

  return okay;
}

//----------------------------------------------------------------------------
void cmDepends::Clear(const char *file)
{
  // Print verbose output.
  if(this->Verbose)
    {
    std::ostringstream msg;
    msg << "Clearing dependencies in \"" << file << "\"." << std::endl;
    cmSystemTools::Stdout(msg.str().c_str());
    }

  // Write an empty dependency file.
  cmGeneratedFileStream depFileStream(file);
  depFileStream
    << "# Empty dependencies file\n"
    << "# This may be replaced when dependencies are built." << std::endl;
}

//----------------------------------------------------------------------------
bool cmDepends::WriteDependencies(
    const std::set<std::string>&, const std::string&,
    std::ostream&, std::ostream&)
{
  // This should be implemented by the subclass.
  return false;
}

//----------------------------------------------------------------------------
bool cmDepends::CheckDependencies(std::istream& internalDepends,
                                  const char* internalDependsFileName,
                            std::map<std::string, DependencyVector>& validDeps)
{
  // Parse dependencies from the stream.  If any dependee is missing
  // or newer than the depender then dependencies should be
  // regenerated.
  bool okay = true;
  bool dependerExists = false;
  DependencyVector* currentDependencies = 0;

  while(internalDepends.getline(this->Dependee, this->MaxPath))
    {
    if ( this->Dependee[0] == 0 || this->Dependee[0] == '#' ||
         this->Dependee[0] == '\r' )
      {
      continue;
      }
    size_t len = internalDepends.gcount()-1;
    if ( this->Dependee[len-1] == '\r' )
      {
      len --;
      this->Dependee[len] = 0;
      }
    if ( this->Dependee[0] != ' ' )
      {
      memcpy(this->Depender, this->Dependee, len+1);
      // Calling FileExists() for the depender here saves in many cases 50%
      // of the calls to FileExists() further down in the loop. E.g. for
      // kdelibs/khtml this reduces the number of calls from 184k down to 92k,
      // or the time for cmake -E cmake_depends from 0.3 s down to 0.21 s.
      dependerExists = cmSystemTools::FileExists(this->Depender);
      // If we erase validDeps[this->Depender] by overwriting it with an empty
      // vector, we lose dependencies for dependers that have multiple
      // entries. No need to initialize the entry, std::map will do so on first
      // access.
      currentDependencies = &validDeps[this->Depender];
      continue;
      }
    /*
    // Parse the dependency line.
    if(!this->ParseDependency(line.c_str()))
      {
      continue;
      }
      */

    // Dependencies must be regenerated
    // * if the dependee does not exist
    // * if the depender exists and is older than the dependee.
    // * if the depender does not exist, but the dependee is newer than the
    //   depends file
    bool regenerate = false;
    const char* dependee = this->Dependee+1;
    const char* depender = this->Depender;
    if (currentDependencies != 0)
      {
      currentDependencies->push_back(dependee);
      }

    if(!cmSystemTools::FileExists(dependee))
      {
      // The dependee does not exist.
      regenerate = true;

      // Print verbose output.
      if(this->Verbose)
        {
        std::ostringstream msg;
        msg << "Dependee \"" << dependee
            << "\" does not exist for depender \""
            << depender << "\"." << std::endl;
        cmSystemTools::Stdout(msg.str().c_str());
        }
      }
    else
      {
      if(dependerExists)
        {
        // The dependee and depender both exist.  Compare file times.
        int result = 0;
        if((!this->FileComparison->FileTimeCompare(depender, dependee,
                                              &result) || result < 0))
          {
          // The depender is older than the dependee.
          regenerate = true;

          // Print verbose output.
          if(this->Verbose)
            {
            std::ostringstream msg;
            msg << "Dependee \"" << dependee
                << "\" is newer than depender \""
                << depender << "\"." << std::endl;
            cmSystemTools::Stdout(msg.str().c_str());
            }
          }
        }
      else
        {
        // The dependee exists, but the depender doesn't. Regenerate if the
        // internalDepends file is older than the dependee.
        int result = 0;
        if((!this->FileComparison->FileTimeCompare(internalDependsFileName,
                                             dependee, &result) || result < 0))
          {
          // The depends-file is older than the dependee.
          regenerate = true;

          // Print verbose output.
          if(this->Verbose)
            {
            std::ostringstream msg;
            msg << "Dependee \"" << dependee
                << "\" is newer than depends file \""
                << internalDependsFileName << "\"." << std::endl;
            cmSystemTools::Stdout(msg.str().c_str());
            }
          }
        }
      }
    if(regenerate)
      {
      // Dependencies must be regenerated.
      okay = false;

      // Remove the information of this depender from the map, it needs
      // to be rescanned
      if (currentDependencies != 0)
        {
        validDeps.erase(this->Depender);
        currentDependencies = 0;
        }

      // Remove the depender to be sure it is rebuilt.
      if (dependerExists)
        {
        cmSystemTools::RemoveFile(depender);
        dependerExists = false;
        }
      }
    }

  return okay;
}

//----------------------------------------------------------------------------
void cmDepends::SetIncludePathFromLanguage(const std::string& lang)
{
  // Look for the new per "TARGET_" variant first:
  const char * includePath = 0;
  std::string includePathVar = "CMAKE_";
  includePathVar += lang;
  includePathVar += "_TARGET_INCLUDE_PATH";
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  includePath = mf->GetDefinition(includePathVar);
  if(includePath)
    {
    cmSystemTools::ExpandListArgument(includePath, this->IncludePath);
    }
  else
    {
    // Fallback to the old directory level variable if no per-target var:
    includePathVar = "CMAKE_";
    includePathVar += lang;
    includePathVar += "_INCLUDE_PATH";
    includePath = mf->GetDefinition(includePathVar);
    if(includePath)
      {
      cmSystemTools::ExpandListArgument(includePath, this->IncludePath);
      }
    }
}
