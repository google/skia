/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmParsePHPCoverage_h
#define cmParsePHPCoverage_h

#include "cmStandardIncludes.h"
#include "cmCTestCoverageHandler.h"

/** \class cmParsePHPCoverage
 * \brief Parse xdebug PHP coverage information
 *
 * This class is used to parse php coverage information produced
 * by xdebug.  The data is stored as a php dump of the array
 * return by xdebug coverage.  It is an array of arrays.
 */
class cmParsePHPCoverage
{
public:
  cmParsePHPCoverage(cmCTestCoverageHandlerContainer& cont,
    cmCTest* ctest);
  bool ReadPHPCoverageDirectory(const char* dir);
  void PrintCoverage();
private:
  bool ReadPHPData(const char* file);
  bool ReadArraySize(std::istream& in, int& size);
  bool ReadFileInformation(std::istream& in);
  bool ReadInt(std::istream& in, int& v);
  bool ReadCoverageArray(std::istream& in, std::string const&);
  bool ReadUntil(std::istream& in, char until);
  cmCTestCoverageHandlerContainer& Coverage;
  cmCTest* CTest;
};


#endif
