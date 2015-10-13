/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTargetExport_h
#define cmTargetExport_h

#include "cmStandardIncludes.h"

class cmTarget;
class cmInstallTargetGenerator;
class cmInstallFilesGenerator;

/** \brief A member of an ExportSet
 *
 * This struct holds pointers to target and all relevant generators.
 */
class cmTargetExport
{
public:
  cmTarget* Target; ///< The target

  ///@name Generators
  ///@{
  cmInstallTargetGenerator* ArchiveGenerator;
  cmInstallTargetGenerator* RuntimeGenerator;
  cmInstallTargetGenerator* LibraryGenerator;
  cmInstallTargetGenerator* FrameworkGenerator;
  cmInstallTargetGenerator* BundleGenerator;
  cmInstallFilesGenerator* HeaderGenerator;
  std::string InterfaceIncludeDirectories;
  ///@}
};

#endif
