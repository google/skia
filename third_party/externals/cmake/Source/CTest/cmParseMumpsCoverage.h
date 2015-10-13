/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmParseMumpsCoverage_h
#define cmParseMumpsCoverage_h

#include "cmStandardIncludes.h"
#include "cmCTestCoverageHandler.h"

/** \class cmParseMumpsCoverage
 * \brief Parse Mumps coverage information
 *
 * This class is used as the base class for Mumps coverage
 * parsing.
 */
class cmParseMumpsCoverage
{
public:
  cmParseMumpsCoverage(cmCTestCoverageHandlerContainer& cont,
    cmCTest* ctest);
  virtual ~cmParseMumpsCoverage();
  // This is the toplevel coverage file locating the coverage files
  // and the mumps source code package tree.
  bool ReadCoverageFile(const char* file);
protected:
  // sub classes will use this to
  // load all coverage files found in the given directory
  virtual bool LoadCoverageData(const char* d) = 0;
  // search the package directory for mumps files and fill
  // in the RoutineToDirectory map
  bool LoadPackages(const char* dir);
  // initialize the coverage information for a single mumps file
  void InitializeMumpsFile(std::string& file);
  // Find mumps file for routine
  bool FindMumpsFile(std::string const& routine,
                     std::string& filepath);
protected:
  std::map<std::string, std::string> RoutineToDirectory;
  cmCTestCoverageHandlerContainer& Coverage;
  cmCTest* CTest;
};

#endif
