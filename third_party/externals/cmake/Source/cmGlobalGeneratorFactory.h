/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmGlobalGeneratorFactory_h
#define cmGlobalGeneratorFactory_h

#include "cmStandardIncludes.h"

class cmake;
class cmGlobalGenerator;
struct cmDocumentationEntry;

/** \class cmGlobalGeneratorFactory
 * \brief Responable for creating cmGlobalGenerator instances
 *
 * Subclasses of this class generate instances of cmGlobalGenerator.
 */
class cmGlobalGeneratorFactory
{
public:
  virtual ~cmGlobalGeneratorFactory() {}

  /** Create a GlobalGenerator */
  virtual cmGlobalGenerator* CreateGlobalGenerator(
      const std::string& n, cmake* cm) const = 0;

  /** Get the documentation entry for this factory */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const = 0;

  /** Get the names of the current registered generators */
  virtual void GetGenerators(std::vector<std::string>& names) const = 0;
};

template<class T>
class cmGlobalGeneratorSimpleFactory : public cmGlobalGeneratorFactory
{
public:
  /** Create a GlobalGenerator */
  virtual cmGlobalGenerator*
  CreateGlobalGenerator(const std::string& name, cmake* cm) const {
    if (name != T::GetActualName()) return 0;
    return new T(cm); }

  /** Get the documentation entry for this factory */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const {
    T::GetDocumentation(entry); }

  /** Get the names of the current registered generators */
  virtual void GetGenerators(std::vector<std::string>& names) const {
    names.push_back(T::GetActualName()); }
};

#endif
