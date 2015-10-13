/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackCygwinSourceGenerator_h
#define cmCPackCygwinSourceGenerator_h

#include "cmCPackTarBZip2Generator.h"

/** \class cmCPackCygwinSourceGenerator
 * \brief A generator for cygwin source files
 */
class cmCPackCygwinSourceGenerator : public cmCPackTarBZip2Generator
{
public:
  cmCPackTypeMacro(cmCPackCygwinSourceGenerator, cmCPackTarBZip2Generator);

  /**
   * Construct generator
   */
  cmCPackCygwinSourceGenerator();
  virtual ~cmCPackCygwinSourceGenerator();
protected:
  const char* GetPackagingInstallPrefix();
  virtual int InitializeInternal();
  int PackageFiles();
  virtual const char* GetOutputExtension();
  std::string InstallPrefix;
  std::string OutputExtension;
};

#endif
