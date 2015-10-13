/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmParseGTMCoverage_h
#define cmParseGTMCoverage_h

#include "cmParseMumpsCoverage.h"

/** \class cmParseGTMCoverage
 * \brief Parse GTM coverage information
 *
 * This class is used to parse GTM coverage information for
 * mumps.
 */
class cmParseGTMCoverage : public cmParseMumpsCoverage
{
public:
  cmParseGTMCoverage(cmCTestCoverageHandlerContainer& cont,
    cmCTest* ctest);
protected:
  // implement virtual from parent
  bool LoadCoverageData(const char* dir);
  // Read a single mcov file
  bool ReadMCovFile(const char* f);
  // find out what line in a mumps file (filepath) the given entry point
  // or function is.  lineoffset is set by this method.
  bool FindFunctionInMumpsFile(std::string const& filepath,
                               std::string const& function,
                               int& lineoffset);
  // parse a line from a .mcov file, and fill in the
  // routine, function, linenumber and coverage count
  bool ParseMCOVLine(std::string const& line,
                 std::string& routine,
                 std::string& function,
                 int& linenumber,
                 int& count);
};


#endif
