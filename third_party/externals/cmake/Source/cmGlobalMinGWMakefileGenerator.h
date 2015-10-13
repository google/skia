/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalMinGWMakefileGenerator_h
#define cmGlobalMinGWMakefileGenerator_h

#include "cmGlobalUnixMakefileGenerator3.h"

/** \class cmGlobalMinGWMakefileGenerator
 * \brief Write a NMake makefiles.
 *
 * cmGlobalMinGWMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalMinGWMakefileGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalMinGWMakefileGenerator(cmake* cm);
  static cmGlobalGeneratorFactory* NewFactory() {
    return new cmGlobalGeneratorSimpleFactory
      <cmGlobalMinGWMakefileGenerator>(); }
  ///! Get the name for the generator.
  virtual std::string GetName() const {
    return cmGlobalMinGWMakefileGenerator::GetActualName();}
  static std::string GetActualName() {return "MinGW Makefiles";}

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);
};

#endif
