/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackGeneratorFactory_h
#define cmCPackGeneratorFactory_h

#include "cmObject.h"

class cmCPackLog;
class cmCPackGenerator;

/** \class cmCPackGeneratorFactory
 * \brief A container for CPack generators
 *
 */
class cmCPackGeneratorFactory : public cmObject
{
public:
  cmTypeMacro(cmCPackGeneratorFactory, cmObject);

  cmCPackGeneratorFactory();
  ~cmCPackGeneratorFactory();

  //! Get the generator
  cmCPackGenerator* NewGenerator(const std::string& name);
  void DeleteGenerator(cmCPackGenerator* gen);

  typedef cmCPackGenerator* CreateGeneratorCall();

  void RegisterGenerator(const std::string& name,
    const char* generatorDescription,
    CreateGeneratorCall* createGenerator);

  void SetLogger(cmCPackLog* logger) { this->Logger = logger; }

  typedef std::map<std::string, std::string> DescriptionsMap;
  const DescriptionsMap& GetGeneratorsList() const
    { return this->GeneratorDescriptions; }

private:
  cmCPackGenerator* NewGeneratorInternal(const std::string& name);
  std::vector<cmCPackGenerator*> Generators;

  typedef std::map<std::string, CreateGeneratorCall*> t_GeneratorCreatorsMap;
  t_GeneratorCreatorsMap GeneratorCreators;
  DescriptionsMap GeneratorDescriptions;
  cmCPackLog* Logger;
};

#endif
