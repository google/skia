/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmWIXFeaturesSourceWriter_h
#define cmWIXFeaturesSourceWriter_h

#include "cmWIXSourceWriter.h"
#include <CPack/cmCPackGenerator.h>

/** \class cmWIXFeaturesSourceWriter
 * \brief Helper class to generate features.wxs
 */
class cmWIXFeaturesSourceWriter : public cmWIXSourceWriter
{
public:
  cmWIXFeaturesSourceWriter(cmCPackLog* logger,
    std::string const& filename);

  void CreateCMakePackageRegistryEntry(
    std::string const& package,
    std::string const& upgradeGuid);

  void EmitFeatureForComponentGroup(const cmCPackComponentGroup& group);

  void EmitFeatureForComponent(const cmCPackComponent& component);

  void EmitComponentRef(std::string const& id);
};

#endif
