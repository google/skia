/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestSVN_h
#define cmCTestSVN_h

#include "cmCTestGlobalVC.h"

#include <list>

/** \class cmCTestSVN
 * \brief Interaction with subversion command-line tool
 *
 */
class cmCTestSVN: public cmCTestGlobalVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestSVN(cmCTest* ctest, std::ostream& log);

  virtual ~cmCTestSVN();

private:
  // Implement cmCTestVC internal API.
  virtual void CleanupImpl();
  virtual void NoteOldRevision();
  virtual void NoteNewRevision();
  virtual bool UpdateImpl();

  bool RunSVNCommand(std::vector<char const*> const& parameters,
    OutputParser* out, OutputParser* err);

  // Information about an SVN repository (root repository or external)
  struct SVNInfo {

    SVNInfo(const char* path) : LocalPath(path) {}
    // Remove base from the filename
    std::string BuildLocalPath(std::string const& path) const;

    // LocalPath relative to the main source directory.
    std::string LocalPath;

    // URL of repository directory checked out in the working tree.
    std::string URL;

    // URL of repository root directory.
    std::string Root;

    // Directory under repository root checked out in working tree.
    std::string Base;

    // Old and new repository revisions.
    std::string OldRevision;
    std::string NewRevision;

  };

  // Extended revision structure to include info about external it refers to.
  struct Revision;
  friend struct Revision;

  // Info of all the repositories (root, externals and nested ones).
  std::list<SVNInfo> Repositories;

  // Pointer to the infos of the root repository.
  SVNInfo* RootInfo;

  std::string LoadInfo(SVNInfo& svninfo);
  void LoadExternals();
  void LoadModifications();
  void LoadRevisions();
  void LoadRevisions(SVNInfo& svninfo);

  void GuessBase(SVNInfo &svninfo, std::vector<Change> const& changes);

  void DoRevisionSVN(Revision const& revision,
                     std::vector<Change> const& changes);

  void WriteXMLGlobal(cmXMLWriter& xml);

  // Parsing helper classes.
  class InfoParser;
  class LogParser;
  class StatusParser;
  class UpdateParser;
  class ExternalParser;
  friend class InfoParser;
  friend class LogParser;
  friend class StatusParser;
  friend class UpdateParser;
  friend class ExternalParser;
};

#endif
