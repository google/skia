/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmMakeDepend_h
#define cmMakeDepend_h

#include "cmMakefile.h"
#include "cmSourceFile.h"

#include <cmsys/RegularExpression.hxx>

/** \class cmDependInformation
 * \brief Store dependency information for a single source file.
 *
 * This structure stores the depend information for a single source file.
 */
class cmDependInformation
{
public:
  /**
   * Construct with dependency generation marked not done; instance
   * not placed in cmMakefile's list.
   */
  cmDependInformation(): DependDone(false), SourceFile(0) {}

  /**
   * The set of files on which this one depends.
   */
  typedef std::set<cmDependInformation*> DependencySetType;
  DependencySetType DependencySet;

  /**
   * This flag indicates whether dependency checking has been
   * performed for this file.
   */
  bool DependDone;

  /**
   * If this object corresponds to a cmSourceFile instance, this points
   * to it.
   */
  const cmSourceFile *SourceFile;

  /**
   * Full path to this file.
   */
  std::string FullPath;

  /**
   * Full path not including file name.
   */
  std::string PathOnly;

  /**
   * Name used to #include this file.
   */
  std::string IncludeName;

  /**
   * This method adds the dependencies of another file to this one.
   */
  void AddDependencies(cmDependInformation*);
};


// cmMakeDepend is used to generate dependancy information for
// the classes in a makefile
class cmMakeDepend
{
public:
  /**
   * Construct the object with verbose turned off.
   */
  cmMakeDepend();

  /**
   * Destructor.
   */
  virtual ~cmMakeDepend();

  /**
   * Set the makefile that is used as a source of classes.
   */
  virtual void SetMakefile(cmMakefile* makefile);

  /**
   * Add a directory to the search path for include files.
   */
  virtual void AddSearchPath(const std::string&);

  /**
   * Generate dependencies for the file given.  Returns a pointer to
   * the cmDependInformation object for the file.
   */
  const cmDependInformation* FindDependencies(const char* file);

protected:
  /**
   * Compute the depend information for this class.
   */
  virtual void DependWalk(cmDependInformation* info);

  /**
   * Add a dependency.  Possibly walk it for more dependencies.
   */
  virtual void AddDependency(cmDependInformation* info, const char* file);

  /**
   * Fill in the given object with dependency information.  If the
   * information is already complete, nothing is done.
   */
  void GenerateDependInformation(cmDependInformation* info);

  /**
   * Get an instance of cmDependInformation corresponding to the given file
   * name.
   */
  cmDependInformation* GetDependInformation(const char* file,
                                            const char *extraPath);

  /**
   * Find the full path name for the given file name.
   * This uses the include directories.
   * TODO: Cache path conversions to reduce FileExists calls.
   */
  std::string FullPath(const char *filename, const char *extraPath);

  cmMakefile* Makefile;
  bool Verbose;
  cmsys::RegularExpression IncludeFileRegularExpression;
  cmsys::RegularExpression ComplainFileRegularExpression;
  std::vector<std::string> IncludeDirectories;
  typedef std::map<std::string, std::string> FileToPathMapType;
  typedef std::map<std::string, FileToPathMapType>
  DirectoryToFileToPathMapType;
  typedef std::map<std::string, cmDependInformation*>
  DependInformationMapType;
  DependInformationMapType DependInformationMap;
  DirectoryToFileToPathMapType DirectoryToFileToPathMap;
};

#endif
