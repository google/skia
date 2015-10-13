/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackBundleGenerator_h
#define cmCPackBundleGenerator_h

#include "cmCPackDragNDropGenerator.h"

/** \class cmCPackBundleGenerator
 * \brief A generator for OSX bundles
 *
 * Based on Gimp.app
 */
class cmCPackBundleGenerator : public cmCPackDragNDropGenerator
{
public:
  cmCPackTypeMacro(cmCPackBundleGenerator, cmCPackDragNDropGenerator);

  cmCPackBundleGenerator();
  virtual ~cmCPackBundleGenerator();

protected:
  virtual int InitializeInternal();
  virtual const char* GetPackagingInstallPrefix();
  int ConstructBundle();
  int SignBundle(const std::string& src_dir);
  int PackageFiles();
  bool SupportsComponentInstallation() const;

  std::string InstallPrefix;
};

#endif
