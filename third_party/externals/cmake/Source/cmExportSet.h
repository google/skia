/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportSet_h
#define cmExportSet_h

#include "cmSystemTools.h"
class cmTargetExport;
class cmInstallExportGenerator;

/// A set of targets that were installed with the same EXPORT parameter.
class cmExportSet
{
public:
  /// Construct an empty export set named \a name
  cmExportSet(const std::string &name) : Name(name) {}
  /// Destructor
  ~cmExportSet();

  void AddTargetExport(cmTargetExport* tgt);

  void AddInstallation(cmInstallExportGenerator const* installation);

  std::string const& GetName() const { return this->Name; }

  std::vector<cmTargetExport*> const* GetTargetExports() const
     { return &this->TargetExports; }

  std::vector<cmInstallExportGenerator const*> const* GetInstallations() const
     { return &this->Installations; }

private:
  std::vector<cmTargetExport*> TargetExports;
  std::string Name;
  std::vector<cmInstallExportGenerator const*> Installations;
};

#endif
