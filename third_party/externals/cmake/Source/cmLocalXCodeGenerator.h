/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalXCodeGenerator_h
#define cmLocalXCodeGenerator_h

#include "cmLocalGenerator.h"

/** \class cmLocalXCodeGenerator
 * \brief Write a local Xcode project
 *
 * cmLocalXCodeGenerator produces a LocalUnix makefile from its
 * member Makefile.
 */
class cmLocalXCodeGenerator : public cmLocalGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalXCodeGenerator(cmGlobalGenerator* gg, cmLocalGenerator* parent,
                        cmState::Snapshot snapshot);

  virtual ~cmLocalXCodeGenerator();
  virtual std::string GetTargetDirectory(cmTarget const& target) const;
  virtual void AppendFlagEscape(std::string& flags,
                                const std::string& rawFlag);
  virtual void Generate();
  virtual void GenerateInstallRules();
  virtual void ComputeObjectFilenames(
                        std::map<cmSourceFile const*, std::string>& mapping,
                        cmGeneratorTarget const* gt = 0);
private:

};

#endif

