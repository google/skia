/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmParseCoberturaCoverage_h
#define cmParseCoberturaCoverage_h

#include "cmStandardIncludes.h"
#include "cmCTestCoverageHandler.h"

/** \class cmParsePythonCoverage
 * \brief Parse coverage.py Python coverage information
 *
 * This class is used to parse the output of the coverage.py tool that
 * is currently maintained by Ned Batchelder. That tool has a command
 * that produces xml output in the format typically output by the common
 * Java-based Cobertura coverage application. This helper class parses
 * that XML file to fill the coverage-handler container.
 */
class cmParseCoberturaCoverage
{
public:

  //! Create the coverage parser by passing in the coverage handler
  //! container and the cmCTest object
  cmParseCoberturaCoverage(cmCTestCoverageHandlerContainer& cont,
    cmCTest* ctest);

  bool inSources;
  bool inSource;
  std::vector<std::string> filepaths;
  //! Read the XML produced by running `coverage xml`
  bool ReadCoverageXML(const char* xmlFile);

private:

  class XMLParser;
  cmCTestCoverageHandlerContainer& Coverage;
  cmCTest* CTest;
  std::string CurFileName;
};

#endif
