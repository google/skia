/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmParseCacheCoverage_h
#define cmParseCacheCoverage_h

#include "cmParseMumpsCoverage.h"

/** \class cmParseCacheCoverage
 * \brief Parse Cache coverage information
 *
 * This class is used to parse Cache coverage information for
 * mumps.
 */
class cmParseCacheCoverage : public cmParseMumpsCoverage
{
public:
  cmParseCacheCoverage(cmCTestCoverageHandlerContainer& cont,
    cmCTest* ctest);
protected:
  // implement virtual from parent
  bool LoadCoverageData(const char* dir);
  // remove files with no coverage
  void RemoveUnCoveredFiles();
  // Read a single mcov file
  bool ReadCMCovFile(const char* f);
  // split a string based on ,
  bool SplitString(std::vector<std::string>& args,
                   std::string const& line);
};


#endif
