/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio9Generator_h
#define cmGlobalVisualStudio9Generator_h

#include "cmGlobalVisualStudio8Generator.h"


/** \class cmGlobalVisualStudio9Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio9Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio9Generator :
  public cmGlobalVisualStudio8Generator
{
public:
  cmGlobalVisualStudio9Generator(cmake* cm, const std::string& name,
    const std::string& platformName);
  static cmGlobalGeneratorFactory* NewFactory();

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void WriteSLNHeader(std::ostream& fout);

  /**
   * Where does this version of Visual Studio look for macros for the
   * current user? Returns the empty string if this version of Visual
   * Studio does not implement support for VB macros.
   */
  virtual std::string GetUserMacrosDirectory();

  /**
   * What is the reg key path to "vsmacros" for this version of Visual
   * Studio?
   */
  virtual std::string GetUserMacrosRegKeyBase();
protected:
  virtual const char* GetIDEVersion() { return "9.0"; }
private:
  class Factory;
  friend class Factory;
};
#endif
