#ifndef CMGRAPHVIZWRITER_H
#define CMGRAPHVIZWRITER_H
/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmStandardIncludes.h"
#include "cmLocalGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmTarget.h"
#include <cmsys/RegularExpression.hxx>


/** This class implements writing files for graphviz (dot) for graphs
 * representing the dependencies between the targets in the project. */
class cmGraphVizWriter
{
public:

  cmGraphVizWriter(const std::vector<cmLocalGenerator*>& localGenerators);

  void ReadSettings(const char* settingsFileName,
                    const char* fallbackSettingsFileName);

  void WritePerTargetFiles(const char* fileName);
  void WriteTargetDependersFiles(const char* fileName);

  void WriteGlobalFile(const char* fileName);

protected:

  void CollectTargetsAndLibs();

  int CollectAllTargets();

  int CollectAllExternalLibs(int cnt);

  void WriteHeader(cmGeneratedFileStream& str) const;

  void WriteConnections(const std::string& targetName,
                        std::set<std::string>& insertedNodes,
                        std::set<std::string>& insertedConnections,
                        cmGeneratedFileStream& str) const;

  void WriteDependerConnections(const std::string& targetName,
                                std::set<std::string>& insertedNodes,
                                std::set<std::string>& insertedConnections,
                                cmGeneratedFileStream& str) const;

  void WriteNode(const std::string& targetName, const cmTarget* target,
                 std::set<std::string>& insertedNodes,
                 cmGeneratedFileStream& str) const;

  void WriteFooter(cmGeneratedFileStream& str) const;

  bool IgnoreThisTarget(const std::string& name);

  bool GenerateForTargetType(cmTarget::TargetType targetType) const;

  std::string GraphType;
  std::string GraphName;
  std::string GraphHeader;
  std::string GraphNodePrefix;

  bool GenerateForExecutables;
  bool GenerateForStaticLibs;
  bool GenerateForSharedLibs;
  bool GenerateForModuleLibs;
  bool GenerateForExternals;
  bool GeneratePerTarget;
  bool GenerateDependers;

  std::vector<cmsys::RegularExpression> TargetsToIgnoreRegex;

  const std::vector<cmLocalGenerator*>& LocalGenerators;

  std::map<std::string, const cmTarget*> TargetPtrs;
  // maps from the actual target names to node names in dot:
  std::map<std::string, std::string> TargetNamesNodes;

  bool HaveTargetsAndLibs;
};

#endif
