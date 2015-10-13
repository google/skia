/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestGlobalVC_h
#define cmCTestGlobalVC_h

#include "cmCTestVC.h"

#include <list>

/** \class cmCTestGlobalVC
 * \brief Base class for handling globally-versioned trees
 *
 */
class cmCTestGlobalVC: public cmCTestVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestGlobalVC(cmCTest* ctest, std::ostream& log);

  virtual ~cmCTestGlobalVC();

protected:
  // Implement cmCTestVC internal API.
  virtual bool WriteXMLUpdates(cmXMLWriter& xml);

  /** Represent a vcs-reported action for one path in a revision.  */
  struct Change
  {
    char Action;
    std::string Path;
    Change(char a = '?'): Action(a) {}
  };

  // Update status for files in each directory.
  class Directory: public std::map<std::string, File> {};
  std::map<std::string, Directory> Dirs;

  // Old and new repository revisions.
  std::string OldRevision;
  std::string NewRevision;

  // Information known about old revision.
  Revision PriorRev;

  // Information about revisions from a svn log.
  std::list<Revision> Revisions;

  virtual const char* LocalPath(std::string const& path);

  virtual void DoRevision(Revision const& revision,
                          std::vector<Change> const& changes);
  virtual void DoModification(PathStatus status, std::string const& path);
  virtual void LoadModifications() = 0;
  virtual void LoadRevisions() = 0;

  virtual void WriteXMLGlobal(cmXMLWriter& xml);
  void WriteXMLDirectory(cmXMLWriter& xml, std::string const& path,
                         Directory const& dir);
};

#endif
