/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDependsC_h
#define cmDependsC_h

#include "cmDepends.h"
#include <cmsys/RegularExpression.hxx>
#include <queue>

/** \class cmDependsC
 * \brief Dependency scanner for C and C++ object files.
 */
class cmDependsC: public cmDepends
{
public:
  /** Checking instances need to know the build directory name and the
      relative path from the build directory to the target file.  */
  cmDependsC();
  cmDependsC(cmLocalGenerator* lg, const char* targetDir,
             const std::string& lang,
             const std::map<std::string, DependencyVector>* validDeps);

  /** Virtual destructor to cleanup subclasses properly.  */
  virtual ~cmDependsC();

protected:
  // Implement writing/checking methods required by superclass.
  virtual bool WriteDependencies(const std::set<std::string>& sources,
                                 const std::string&           obj,
                                 std::ostream& makeDepends,
                                 std::ostream& internalDepends);

  // Method to scan a single file.
  void Scan(std::istream& is, const char* directory,
    const std::string& fullName);

  // Regular expression to identify C preprocessor include directives.
  cmsys::RegularExpression IncludeRegexLine;

  // Regular expressions to choose which include files to scan
  // recursively and which to complain about not finding.
  cmsys::RegularExpression IncludeRegexScan;
  cmsys::RegularExpression IncludeRegexComplain;
  std::string IncludeRegexLineString;
  std::string IncludeRegexScanString;
  std::string IncludeRegexComplainString;

  // Regex to transform #include lines.
  std::string IncludeRegexTransformString;
  cmsys::RegularExpression IncludeRegexTransform;
  typedef std::map<std::string, std::string> TransformRulesType;
  TransformRulesType TransformRules;
  void SetupTransforms();
  void ParseTransform(std::string const& xform);
  void TransformLine(std::string& line);

public:
  // Data structures for dependency graph walk.
  struct UnscannedEntry
  {
    std::string FileName;
    std::string QuotedLocation;
  };

  struct cmIncludeLines
  {
    cmIncludeLines(): Used(false) {}
    std::vector<UnscannedEntry> UnscannedEntries;
    bool Used;
  };
protected:
  const std::map<std::string, DependencyVector>* ValidDeps;
  std::set<std::string> Encountered;
  std::queue<UnscannedEntry> Unscanned;

  std::map<std::string, cmIncludeLines *> FileCache;
  std::map<std::string, std::string> HeaderLocationCache;

  std::string CacheFileName;

  void WriteCacheFile() const;
  void ReadCacheFile();
private:
  cmDependsC(cmDependsC const&); // Purposely not implemented.
  void operator=(cmDependsC const&); // Purposely not implemented.
};

#endif
