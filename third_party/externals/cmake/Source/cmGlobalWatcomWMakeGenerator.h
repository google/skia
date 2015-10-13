/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalWatcomWMakeGenerator_h
#define cmGlobalWatcomWMakeGenerator_h

#include "cmGlobalUnixMakefileGenerator3.h"

/** \class cmGlobalWatcomWMakeGenerator
 * \brief Write a NMake makefiles.
 *
 * cmGlobalWatcomWMakeGenerator manages nmake build process for a tree
 */
class cmGlobalWatcomWMakeGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalWatcomWMakeGenerator(cmake* cm);
  static cmGlobalGeneratorFactory* NewFactory() {
    return new cmGlobalGeneratorSimpleFactory
      <cmGlobalWatcomWMakeGenerator>(); }
  ///! Get the name for the generator.
  virtual std::string GetName() const {
    return cmGlobalWatcomWMakeGenerator::GetActualName();}
  static std::string GetActualName() {return "Watcom WMake";}

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);

  virtual bool AllowNotParallel() const { return false; }
  virtual bool AllowDeleteOnError() const { return false; }
};

#endif
