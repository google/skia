/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestCVS_h
#define cmCTestCVS_h

#include "cmCTestVC.h"

/** \class cmCTestCVS
 * \brief Interaction with cvs command-line tool
 *
 */
class cmCTestCVS: public cmCTestVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestCVS(cmCTest* ctest, std::ostream& log);

  virtual ~cmCTestCVS();

private:
  // Implement cmCTestVC internal API.
  virtual bool UpdateImpl();
  virtual bool WriteXMLUpdates(cmXMLWriter& xml);

  // Update status for files in each directory.
  class Directory: public std::map<std::string, PathStatus> {};
  std::map<std::string, Directory> Dirs;

  std::string ComputeBranchFlag(std::string const& dir);
  void LoadRevisions(std::string const& file, const char* branchFlag,
                     std::vector<Revision>& revisions);
  void WriteXMLDirectory(cmXMLWriter& xml, std::string const& path,
                         Directory const& dir);

  // Parsing helper classes.
  class UpdateParser;
  class LogParser;
  friend class UpdateParser;
  friend class LogParser;
};

#endif
