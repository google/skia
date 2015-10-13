/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmMakeDepend.h"
#include "cmSystemTools.h"
#include "cmGeneratorExpression.h"
#include "cmAlgorithms.h"

#include <cmsys/RegularExpression.hxx>
#include <cmsys/FStream.hxx>

void cmDependInformation::AddDependencies(cmDependInformation* info)
{
  if(this != info)
    {
    this->DependencySet.insert(info);
    }
}

cmMakeDepend::cmMakeDepend()
{
  this->Verbose = false;
  this->IncludeFileRegularExpression.compile("^.*$");
  this->ComplainFileRegularExpression.compile("^$");
}


cmMakeDepend::~cmMakeDepend()
{
  cmDeleteAll(this->DependInformationMap);
}


// Set the makefile that depends will be made from.
// The pointer is kept so the cmSourceFile array can
// be updated with the depend information in the cmMakefile.

void cmMakeDepend::SetMakefile(cmMakefile* makefile)
{
  this->Makefile = makefile;

  // Now extract the include file regular expression from the makefile.
  this->IncludeFileRegularExpression.compile(
    this->Makefile->IncludeFileRegularExpression.c_str());
  this->ComplainFileRegularExpression.compile(
    this->Makefile->ComplainFileRegularExpression.c_str());

  // Now extract any include paths from the targets
  std::set<std::string> uniqueIncludes;
  std::vector<std::string> orderedAndUniqueIncludes;
  cmTargets &targets = this->Makefile->GetTargets();
  for (cmTargets::iterator l = targets.begin();
       l != targets.end(); ++l)
    {
    const char *incDirProp = l->second.GetProperty("INCLUDE_DIRECTORIES");
    if (!incDirProp)
      {
      continue;
      }

    std::string incDirs = cmGeneratorExpression::Preprocess(incDirProp,
                      cmGeneratorExpression::StripAllGeneratorExpressions);

    std::vector<std::string> includes;
    cmSystemTools::ExpandListArgument(incDirs, includes);

    for(std::vector<std::string>::const_iterator j = includes.begin();
        j != includes.end(); ++j)
      {
      std::string path = *j;
      this->Makefile->ExpandVariablesInString(path);
      if(uniqueIncludes.insert(path).second)
        {
        orderedAndUniqueIncludes.push_back(path);
        }
      }
    }

  for(std::vector<std::string>::const_iterator
    it = orderedAndUniqueIncludes.begin();
    it != orderedAndUniqueIncludes.end();
    ++it)
    {
    this->AddSearchPath(*it);
    }
}


const cmDependInformation* cmMakeDepend::FindDependencies(const char* file)
{
  cmDependInformation* info = this->GetDependInformation(file,0);
  this->GenerateDependInformation(info);
  return info;
}

void cmMakeDepend::GenerateDependInformation(cmDependInformation* info)
{
  // If dependencies are already done, stop now.
  if(info->DependDone)
    {
    return;
    }
  else
    {
    // Make sure we don't visit the same file more than once.
    info->DependDone = true;
    }
  const char* path = info->FullPath.c_str();
  if(!path)
    {
    cmSystemTools::Error(
      "Attempt to find dependencies for file without path!");
    return;
    }

  bool found = false;

  // If the file exists, use it to find dependency information.
  if(cmSystemTools::FileExists(path, true))
    {
    // Use the real file to find its dependencies.
    this->DependWalk(info);
    found = true;
    }


  // See if the cmSourceFile for it has any files specified as
  // dependency hints.
  if(info->SourceFile != 0)
    {

    // Get the cmSourceFile corresponding to this.
    const cmSourceFile& cFile = *(info->SourceFile);
    // See if there are any hints for finding dependencies for the missing
    // file.
    if(!cFile.GetDepends().empty())
      {
      // Dependency hints have been given.  Use them to begin the
      // recursion.
      for(std::vector<std::string>::const_iterator file =
            cFile.GetDepends().begin(); file != cFile.GetDepends().end();
          ++file)
        {
        this->AddDependency(info, file->c_str());
        }

      // Found dependency information.  We are done.
      found = true;
      }
    }

  if(!found)
    {
    // Try to find the file amongst the sources
    cmSourceFile *srcFile = this->Makefile->GetSource
      (cmSystemTools::GetFilenameWithoutExtension(path));
    if (srcFile)
      {
      if (srcFile->GetFullPath() == path)
        {
        found=true;
        }
      else
        {
        //try to guess which include path to use
        for(std::vector<std::string>::iterator t =
              this->IncludeDirectories.begin();
            t != this->IncludeDirectories.end(); ++t)
          {
          std::string incpath = *t;
          if (!incpath.empty() && incpath[incpath.size() - 1] != '/')
            {
            incpath = incpath + "/";
            }
          incpath = incpath + path;
          if (srcFile->GetFullPath() == incpath)
            {
            // set the path to the guessed path
            info->FullPath = incpath;
            found=true;
            }
          }
        }
      }
    }

  if(!found)
    {
    // Couldn't find any dependency information.
    if(this->ComplainFileRegularExpression.find(info->IncludeName.c_str()))
      {
      cmSystemTools::Error("error cannot find dependencies for ", path);
      }
    else
      {
      // Destroy the name of the file so that it won't be output as a
      // dependency.
      info->FullPath = "";
      }
    }
}

// This function actually reads the file specified and scans it for
// #include directives
void cmMakeDepend::DependWalk(cmDependInformation* info)
{
  cmsys::RegularExpression includeLine
    ("^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)[\">]");
  cmsys::ifstream fin(info->FullPath.c_str());
  if(!fin)
    {
    cmSystemTools::Error("Cannot open ", info->FullPath.c_str());
    return;
    }

  // TODO: Write real read loop (see cmSystemTools::CopyFile).
  std::string line;
  while( cmSystemTools::GetLineFromStream(fin, line) )
    {
    if(includeLine.find(line.c_str()))
      {
      // extract the file being included
      std::string includeFile = includeLine.match(1);
      // see if the include matches the regular expression
      if(!this->IncludeFileRegularExpression.find(includeFile))
        {
        if(this->Verbose)
          {
          std::string message = "Skipping ";
          message += includeFile;
          message += " for file ";
          message += info->FullPath.c_str();
          cmSystemTools::Error(message.c_str(), 0);
          }
        continue;
        }

      // Add this file and all its dependencies.
      this->AddDependency(info, includeFile.c_str());
      }
    }
}


void cmMakeDepend::AddDependency(cmDependInformation* info, const char* file)
{
  cmDependInformation* dependInfo =
    this->GetDependInformation(file, info->PathOnly.c_str());
  this->GenerateDependInformation(dependInfo);
  info->AddDependencies(dependInfo);
}

cmDependInformation* cmMakeDepend::GetDependInformation(const char* file,
                                                        const char *extraPath)
{
  // Get the full path for the file so that lookup is unambiguous.
  std::string fullPath = this->FullPath(file, extraPath);

  // Try to find the file's instance of cmDependInformation.
  DependInformationMapType::const_iterator result =
    this->DependInformationMap.find(fullPath);
  if(result != this->DependInformationMap.end())
    {
    // Found an instance, return it.
    return result->second;
    }
  else
    {
    // Didn't find an instance.  Create a new one and save it.
    cmDependInformation* info = new cmDependInformation;
    info->FullPath = fullPath;
    info->PathOnly = cmSystemTools::GetFilenamePath(fullPath);
    info->IncludeName = file;
    this->DependInformationMap[fullPath] = info;
    return info;
    }
}


// find the full path to fname by searching the this->IncludeDirectories array
std::string cmMakeDepend::FullPath(const char* fname, const char *extraPath)
{
  DirectoryToFileToPathMapType::iterator m;
  if(extraPath)
    {
    m = this->DirectoryToFileToPathMap.find(extraPath);
    }
  else
    {
    m = this->DirectoryToFileToPathMap.find("");
    }

  if(m != this->DirectoryToFileToPathMap.end())
    {
    FileToPathMapType& map = m->second;
    FileToPathMapType::iterator p = map.find(fname);
    if(p != map.end())
      {
      return p->second;
      }
    }

  if(cmSystemTools::FileExists(fname, true))
    {
    std::string fp = cmSystemTools::CollapseFullPath(fname);
    this->DirectoryToFileToPathMap[extraPath? extraPath: ""][fname] = fp;
    return fp;
    }

  for(std::vector<std::string>::iterator i = this->IncludeDirectories.begin();
      i != this->IncludeDirectories.end(); ++i)
    {
    std::string path = *i;
    if (!path.empty() && path[path.size() - 1] != '/')
      {
      path = path + "/";
      }
    path = path + fname;
    if(cmSystemTools::FileExists(path.c_str(), true)
       && !cmSystemTools::FileIsDirectory(path))
      {
      std::string fp = cmSystemTools::CollapseFullPath(path);
      this->DirectoryToFileToPathMap[extraPath? extraPath: ""][fname] = fp;
      return fp;
      }
    }

  if (extraPath)
    {
    std::string path = extraPath;
    if (!path.empty() && path[path.size() - 1] != '/')
      {
      path = path + "/";
      }
    path = path + fname;
    if(cmSystemTools::FileExists(path.c_str(), true)
       && !cmSystemTools::FileIsDirectory(path))
      {
      std::string fp = cmSystemTools::CollapseFullPath(path);
      this->DirectoryToFileToPathMap[extraPath][fname] = fp;
      return fp;
      }
    }

  // Couldn't find the file.
  return std::string(fname);
}

// Add a directory to the search path
void cmMakeDepend::AddSearchPath(const std::string& path)
{
  this->IncludeDirectories.push_back(path);
}
