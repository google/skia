/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackDebGenerator_h
#define cmCPackDebGenerator_h


#include "cmCPackGenerator.h"

/** \class cmCPackDebGenerator
 * \brief A generator for Debian packages
 *
 */
class cmCPackDebGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackDebGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackDebGenerator();
  virtual ~cmCPackDebGenerator();

  static bool CanGenerate()
    {
#ifdef __APPLE__
    // on MacOS enable CPackDeb iff dpkg is found
    std::vector<std::string> locations;
    locations.push_back("/sw/bin");        // Fink
    locations.push_back("/opt/local/bin"); // MacPorts
    return cmSystemTools::FindProgram("dpkg",locations) != "" ? true : false;
#else
    // legacy behavior on other systems
    return true;
#endif
    }

protected:
  virtual int InitializeInternal();
  /**
   * This method factors out the work done in component packaging case.
   */
  int PackageOnePack(std::string initialToplevel, std::string packageName);
  /**
   * The method used to package files when component
   * install is used. This will create one
   * archive for each component group.
   */
  int PackageComponents(bool ignoreGroup);
  /**
   * Special case of component install where all
   * components will be put in a single installer.
   */
  int PackageComponentsAllInOne();
  virtual int PackageFiles();
  virtual const char* GetOutputExtension() { return ".deb"; }
  virtual bool SupportsComponentInstallation() const;
  virtual std::string GetComponentInstallDirNameSuffix(
      const std::string& componentName);

private:
  int createDeb();
  std::vector<std::string> packageFiles;

};

#endif
