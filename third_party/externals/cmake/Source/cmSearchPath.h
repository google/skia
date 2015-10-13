/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSearchPath_h
#define cmSearchPath_h

#include "cmStandardIncludes.h"

class cmFindCommon;

/** \class cmSearchPath
 * \brief Container for encapsulating a set of search paths
 *
 * cmSearchPath is a container that encapsulates search path construction and
 * management
 */
class cmSearchPath
{
public:
  // cmSearchPath must be initialized from a valid pointer.  The only reason
  // for teh default is to allow it to be easily used in stl containers.
  // Attempting to initialize with a NULL value will fail an assertion
  cmSearchPath(cmFindCommon* findCmd = 0);
  ~cmSearchPath();

  const std::vector<std::string>& GetPaths() const { return this->Paths; }

  void ExtractWithout(const std::set<std::string>& ignore,
                      std::vector<std::string>& outPaths,
                      bool clear = false) const;

  void AddPath(const std::string& path);
  void AddUserPath(const std::string& path);
  void AddCMakePath(const std::string& variable);
  void AddEnvPath(const std::string& variable);
  void AddCMakePrefixPath(const std::string& variable);
  void AddEnvPrefixPath(const std::string& variable, bool stripBin = false);
  void AddSuffixes(const std::vector<std::string>& suffixes);

protected:
  void AddPrefixPaths(const std::vector<std::string>& paths,
                      const char *base = 0);
  void AddPathInternal(const std::string& path, const char *base = 0);

  cmFindCommon *FC;
  std::vector<std::string> Paths;
};

#endif
