/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2013 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmVisualStudioSlnData_h
#define cmVisualStudioSlnData_h

#include "cmStandardIncludes.h"

class cmSlnProjectEntry
{
public:
  cmSlnProjectEntry() {}
  cmSlnProjectEntry(const std::string& guid,
                    const std::string& name,
                    const std::string& relativePath)
    : Guid(guid), Name(name), RelativePath(relativePath)
  {}

  std::string GetGUID() const { return Guid; }
  std::string GetName() const { return Name; }
  std::string GetRelativePath() const { return RelativePath; }

private:
  std::string Guid, Name, RelativePath;
};


class cmSlnData
{
public:
  const cmSlnProjectEntry*
  GetProjectByGUID(const std::string& projectGUID) const;

  const cmSlnProjectEntry*
  GetProjectByName(const std::string& projectName) const;

  std::vector<cmSlnProjectEntry> GetProjects() const;

  cmSlnProjectEntry* AddProject(const std::string& projectGUID,
                                const std::string& projectName,
                                const std::string& projectRelativePath);

private:
  typedef std::map<std::string, cmSlnProjectEntry> ProjectStorage;
  ProjectStorage ProjectsByGUID;
  typedef std::map<std::string, ProjectStorage::iterator> ProjectStringIndex;
  ProjectStringIndex ProjectNameIndex;
};

#endif
