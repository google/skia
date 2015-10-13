/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmParseJacocoCoverage_h
#define cmParseJacocoCoverage_h

#include "cmStandardIncludes.h"
#include "cmCTestCoverageHandler.h"


/** \class cmParseJacocoCoverage
 * \brief Parse JaCoCO coverage information
 *
 * This class is used to parse coverage information for
 * java using the JaCoCo tool:
 *
 * http://www.eclemma.org/jacoco/trunk/index.html
 */
class cmParseJacocoCoverage
{
public:
  cmParseJacocoCoverage(cmCTestCoverageHandlerContainer& cont,
    cmCTest* ctest);
  bool LoadCoverageData(const std::vector<std::string> files);

  std::string PackageName;
  std::string FileName;
  std::string ModuleName;
  std::string CurFileName;
private:
  // implement virtual from parent
  // remove files with no coverage
  void RemoveUnCoveredFiles();
  // Read a single mcov file
  bool ReadJacocoXML(const char* f);
  // split a string based on ,
  bool SplitString(std::vector<std::string>& args,
    std::string const& line);
  bool FindJavaFile(std::string const& routine,
    std::string& filepath);
  void InitializeJavaFile(std::string& file);
  bool LoadSource(std::string d);

  class XMLParser;
  std::map<std::string, std::string> RoutineToDirectory;
  cmCTestCoverageHandlerContainer& Coverage;
  cmCTest* CTest;
};

#endif
