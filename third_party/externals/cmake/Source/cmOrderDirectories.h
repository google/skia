/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmOrderDirectories_h
#define cmOrderDirectories_h

#include "cmStandardIncludes.h"

#include <cmsys/RegularExpression.hxx>

class cmGlobalGenerator;
class cmOrderDirectoriesConstraint;
class cmOrderDirectoriesConstraintLibrary;
class cmTarget;

/** \class cmOrderDirectories
 * \brief Compute a safe runtime path order for a set of shared libraries.
 */
class cmOrderDirectories
{
public:
  cmOrderDirectories(cmGlobalGenerator* gg, cmTarget const* target,
                     const char* purpose);
  ~cmOrderDirectories();
  void AddRuntimeLibrary(std::string const& fullPath, const char* soname = 0);
  void AddLinkLibrary(std::string const& fullPath);
  void AddUserDirectories(std::vector<std::string> const& extra);
  void AddLanguageDirectories(std::vector<std::string> const& dirs);
  void SetImplicitDirectories(std::set<std::string> const& implicitDirs);
  void SetLinkExtensionInfo(std::vector<std::string> const& linkExtensions,
                            std::string const& removeExtRegex);

  std::vector<std::string> const& GetOrderedDirectories();
private:
  cmGlobalGenerator* GlobalGenerator;
  cmTarget const* Target;
  std::string Purpose;

  bool Computed;

  std::vector<std::string> OrderedDirectories;

  std::vector<cmOrderDirectoriesConstraint*> ConstraintEntries;
  std::vector<cmOrderDirectoriesConstraint*> ImplicitDirEntries;
  std::vector<std::string> UserDirectories;
  std::vector<std::string> LanguageDirectories;
  cmsys::RegularExpression RemoveLibraryExtension;
  std::vector<std::string> LinkExtensions;
  std::set<std::string> ImplicitDirectories;
  std::set<std::string> EmmittedConstraintSOName;
  std::set<std::string> EmmittedConstraintLibrary;
  std::vector<std::string> OriginalDirectories;
  std::map<std::string, int> DirectoryIndex;
  std::vector<int> DirectoryVisited;
  void CollectOriginalDirectories();
  int AddOriginalDirectory(std::string const& dir);
  void AddOriginalDirectories(std::vector<std::string> const& dirs);
  void FindConflicts();
  void FindImplicitConflicts();
  void OrderDirectories();
  void VisitDirectory(unsigned int i);
  void DiagnoseCycle();
  bool CycleDiagnosed;
  int WalkId;

  // Adjacency-list representation of runtime path ordering graph.
  // This maps from directory to those that must come *before* it.
  // Each entry that must come before is a pair.  The first element is
  // the index of the directory that must come first.  The second
  // element is the index of the runtime library that added the
  // constraint.
  typedef std::pair<int, int> ConflictPair;
  struct ConflictList: public std::vector<ConflictPair> {};
  std::vector<ConflictList> ConflictGraph;

  friend class cmOrderDirectoriesConstraint;
  friend class cmOrderDirectoriesConstraintLibrary;
};

#endif
