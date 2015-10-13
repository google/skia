/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstalledFile_h
#define cmInstalledFile_h

#include "cmGeneratorExpression.h"

/** \class cmInstalledFile
 * \brief Represents a file intended for installation.
 *
 * cmInstalledFile represents a file intended for installation.
 */
class cmInstalledFile
{
public:

  typedef cmsys::auto_ptr<cmCompiledGeneratorExpression>
    CompiledGeneratorExpressionPtrType;

  typedef std::vector<cmCompiledGeneratorExpression*>
    ExpressionVectorType;

  struct Property
  {
    Property();
    ~Property();

    ExpressionVectorType ValueExpressions;
  };

  typedef std::map<std::string, Property> PropertyMapType;

  cmInstalledFile();

  ~cmInstalledFile();

  void RemoveProperty(const std::string& prop);

  void SetProperty(cmMakefile const* mf,
    const std::string& prop, const char *value);

  void AppendProperty(cmMakefile const* mf,
    const std::string& prop, const char* value,bool asString=false);

  bool HasProperty(const std::string& prop) const;

  bool GetProperty(const std::string& prop, std::string& value) const;

  bool GetPropertyAsBool(const std::string& prop) const;

  void GetPropertyAsList(const std::string& prop,
    std::vector<std::string>& list) const;

  void SetName(cmMakefile* mf, const std::string& name);

  std::string const& GetName() const;

  cmCompiledGeneratorExpression const& GetNameExpression() const;

  PropertyMapType const& GetProperties() const { return this->Properties; }

private:
  std::string Name;
  cmCompiledGeneratorExpression* NameExpression;
  PropertyMapType Properties;
};

#endif
