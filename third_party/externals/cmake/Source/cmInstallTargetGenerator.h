/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallTargetGenerator_h
#define cmInstallTargetGenerator_h

#include "cmInstallGenerator.h"
#include "cmTarget.h"
#include "cmGeneratorTarget.h"

/** \class cmInstallTargetGenerator
 * \brief Generate target installation rules.
 */
class cmInstallTargetGenerator: public cmInstallGenerator
{
public:
  cmInstallTargetGenerator(
    cmTarget& t, const char* dest, bool implib,
    const char* file_permissions,
    std::vector<std::string> const& configurations,
    const char* component,
    MessageLevel message,
    bool optional
    );
  virtual ~cmInstallTargetGenerator();

  /** Select the policy for installing shared library linkable name
      symlinks.  */
  enum NamelinkModeType
  {
    NamelinkModeNone,
    NamelinkModeOnly,
    NamelinkModeSkip
  };
  void SetNamelinkMode(NamelinkModeType mode) { this->NamelinkMode = mode; }
  NamelinkModeType GetNamelinkMode() const { return this->NamelinkMode; }

  std::string GetInstallFilename(const std::string& config) const;

  enum NameType
  {
    NameNormal,
    NameImplib,
    NameSO,
    NameReal
  };

  static std::string GetInstallFilename(cmTarget const* target,
                                        const std::string& config,
                                        NameType nameType = NameNormal);

  cmTarget* GetTarget() const { return this->Target; }
  bool IsImportLibrary() const { return this->ImportLibrary; }

  std::string GetDestination(std::string const& config) const;

protected:
  virtual void GenerateScript(std::ostream& os);
  virtual void GenerateScriptForConfig(std::ostream& os,
                                       const std::string& config,
                                       Indent const& indent);
  typedef void (cmInstallTargetGenerator::*TweakMethod)(
    std::ostream&, Indent const&, const std::string&, std::string const&
    );
  void AddTweak(std::ostream& os, Indent const& indent,
                const std::string& config, std::string const& file,
                TweakMethod tweak);
  void AddTweak(std::ostream& os, Indent const& indent,
                const std::string& config,
                std::vector<std::string> const& files,
                TweakMethod tweak);
  std::string GetDestDirPath(std::string const& file);
  void PreReplacementTweaks(std::ostream& os, Indent const& indent,
                            const std::string& config,
                            std::string const& file);
  void PostReplacementTweaks(std::ostream& os, Indent const& indent,
                             const std::string& config,
                             std::string const& file);
  void AddInstallNamePatchRule(std::ostream& os, Indent const& indent,
                               const std::string& config,
                               const std::string& toDestDirPath);
  void AddChrpathPatchRule(std::ostream& os, Indent const& indent,
                           const std::string& config,
                           std::string const& toDestDirPath);
  void AddRPathCheckRule(std::ostream& os, Indent const& indent,
                         const std::string& config,
                         std::string const& toDestDirPath);

  void AddStripRule(std::ostream& os, Indent const& indent,
                    const std::string& toDestDirPath);
  void AddRanlibRule(std::ostream& os, Indent const& indent,
                     const std::string& toDestDirPath);

  cmTarget* Target;
  bool ImportLibrary;
  std::string FilePermissions;
  bool Optional;
  NamelinkModeType NamelinkMode;
  cmGeneratorTarget* GeneratorTarget;
};

#endif
