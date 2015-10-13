/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackOSXX11Generator_h
#define cmCPackOSXX11Generator_h

#include "cmCPackGenerator.h"

/** \class cmCPackOSXX11Generator
 * \brief A generator for OSX X11 modules
 *
 * Based on Gimp.app
 */
class cmCPackOSXX11Generator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackOSXX11Generator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackOSXX11Generator();
  virtual ~cmCPackOSXX11Generator();

protected:
  virtual int InitializeInternal();
  int PackageFiles();
  virtual const char* GetPackagingInstallPrefix();
  virtual const char* GetOutputExtension() { return ".dmg"; }

  //bool CopyCreateResourceFile(const std::string& name,
  //                            const std::string& dir);
  bool CopyResourcePlistFile(const std::string& name, const std::string& dir,
    const char* outputFileName = 0, bool copyOnly = false);
  std::string InstallPrefix;
};

#endif
