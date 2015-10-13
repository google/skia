/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalVisualStudio6Generator_h
#define cmLocalVisualStudio6Generator_h

#include "cmLocalVisualStudioGenerator.h"

class cmTarget;
class cmSourceFile;
class cmSourceGroup;
class cmCustomCommand;

/** \class cmLocalVisualStudio6Generator
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalVisualStudio6Generator produces a LocalUnix makefile from its
 * member this->Makefile.
 */
class cmLocalVisualStudio6Generator : public cmLocalVisualStudioGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalVisualStudio6Generator(cmGlobalGenerator* gg,
                                cmLocalGenerator* parent,
                                cmState::Snapshot snapshot);

  virtual ~cmLocalVisualStudio6Generator();

  virtual void AddHelperCommands();
  virtual void AddCMakeListsRules();

  /**
   * Generate the makefile for this directory.
   */
  virtual void Generate();

  void OutputDSPFile();

  enum BuildType {STATIC_LIBRARY, DLL, EXECUTABLE, WIN32_EXECUTABLE, UTILITY};

  /**
   * Specify the type of the build: static, dll, or executable.
   */
  void SetBuildType(BuildType, const std::string& libName, cmTarget&);

  virtual std::string GetTargetDirectory(cmTarget const& target) const;
  virtual std::string ComputeLongestObjectDirectory(cmTarget&) const;
private:
  std::string DSPHeaderTemplate;
  std::string DSPFooterTemplate;

  void CreateSingleDSP(const std::string& lname, cmTarget &tgt);
  void WriteDSPFile(std::ostream& fout, const std::string& libName,
                    cmTarget &tgt);
  void WriteDSPBeginGroup(std::ostream& fout,
                          const char* group,
                          const char* filter);
  void WriteDSPEndGroup(std::ostream& fout);

  void WriteDSPHeader(std::ostream& fout, const std::string& libName,
                      cmTarget &tgt, std::vector<cmSourceGroup> &sgs);

  void WriteDSPFooter(std::ostream& fout);
  void AddDSPBuildRule(cmTarget& tgt);
  void WriteCustomRule(std::ostream& fout,
                       const char* source,
                       const cmCustomCommand& command,
                       const char* flags);
  void AddUtilityCommandHack(cmTarget& target, int count,
                             std::vector<std::string>& depends,
                             const cmCustomCommand& origCommand);
  void WriteGroup(const cmSourceGroup *sg, cmTarget& target,
                  std::ostream &fout, const std::string& libName);
  class EventWriter;
  friend class EventWriter;
  cmsys::auto_ptr<cmCustomCommand>
  MaybeCreateOutputDir(cmTarget& target, const std::string& config);
  std::string CreateTargetRules(cmTarget &target,
                                const std::string& configName,
                                const std::string& libName);
  void ComputeLinkOptions(cmTarget& target, const std::string& configName,
                          const std::string extraOptions,
                          std::string& options);
  void OutputObjects(cmTarget& target, const char* tool,
                     std::string& options);
  std::string GetTargetIncludeOptions(cmTarget &target,
                                      const std::string& config);
  std::vector<std::string> Configurations;

  std::string GetConfigName(std::string const& configuration) const;

  // Special definition check for VS6.
  virtual bool CheckDefinition(std::string const& define) const;
};

#endif

