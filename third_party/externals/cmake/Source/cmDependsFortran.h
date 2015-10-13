/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDependsFortran_h
#define cmDependsFortran_h

#include "cmDepends.h"

class cmDependsFortranInternals;
class cmDependsFortranSourceInfo;

/** \class cmDependsFortran
 * \brief Dependency scanner for Fortran object files.
 */
class cmDependsFortran: public cmDepends
{
public:
  /** Checking instances need to know the build directory name and the
      relative path from the build directory to the target file.  */
  cmDependsFortran();

  /** Scanning need to know the build directory name, the relative
      path from the build directory to the target file, the source
      file from which to start scanning, the include file search
      path, and the target directory.  */
  cmDependsFortran(cmLocalGenerator* lg);

  /** Virtual destructor to cleanup subclasses properly.  */
  virtual ~cmDependsFortran();

  /** Callback from build system after a .mod file has been generated
      by a Fortran90 compiler to copy the .mod file to the
      corresponding stamp file.  */
  static bool CopyModule(const std::vector<std::string>& args);

  /** Determine if a mod file and the corresponding mod.stamp file
      are representing  different module information. */
  static bool  ModulesDiffer(const char* modFile, const char* stampFile,
                             const char* compilerId);

  /** Method to find an included file in the include path.  Fortran
      always searches the directory containing the including source
      first.  */
  bool FindIncludeFile(const char* dir, const char* includeName,
                       std::string& fileName);

protected:
  // Finalize the dependency information for the target.
  virtual bool Finalize(std::ostream& makeDepends,
                        std::ostream& internalDepends);

  // Find all the modules required by the target.
  void LocateModules();
  void MatchLocalModules();
  void MatchRemoteModules(std::istream& fin, const char* stampDir);
  void ConsiderModule(const char* name, const char* stampDir);
  bool FindModule(std::string const& name, std::string& module);

  // Implement writing/checking methods required by superclass.
  virtual bool WriteDependencies(
    const std::set<std::string>& sources, const std::string& file,
    std::ostream& makeDepends, std::ostream& internalDepends);

  // Actually write the depenencies to the streams.
  bool WriteDependenciesReal(const char *obj,
                             cmDependsFortranSourceInfo const& info,
                             const char* mod_dir, const char* stamp_dir,
                             std::ostream& makeDepends,
                             std::ostream& internalDepends);

  // The source file from which to start scanning.
  std::string SourceFile;

  std::vector<std::string> PPDefinitions;

  // Internal implementation details.
  cmDependsFortranInternals* Internal;

private:
  cmDependsFortran(cmDependsFortran const&); // Purposely not implemented.
  void operator=(cmDependsFortran const&); // Purposely not implemented.
};

#endif
