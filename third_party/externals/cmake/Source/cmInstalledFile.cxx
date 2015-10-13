/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmInstalledFile.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmAlgorithms.h"

//----------------------------------------------------------------------------
cmInstalledFile::cmInstalledFile():
  NameExpression(0)
{

}

//----------------------------------------------------------------------------
cmInstalledFile::~cmInstalledFile()
{
  if(NameExpression)
    {
    delete NameExpression;
    }
}

cmInstalledFile::Property::Property()
{

}

cmInstalledFile::Property::~Property()
{
  cmDeleteAll(this->ValueExpressions);
}

//----------------------------------------------------------------------------
void cmInstalledFile::SetName(cmMakefile* mf, const std::string& name)
{
  cmListFileBacktrace backtrace = mf->GetBacktrace();
  cmGeneratorExpression ge(&backtrace);

  this->Name = name;
  this->NameExpression = ge.Parse(name).release();
}

//----------------------------------------------------------------------------
std::string const& cmInstalledFile::GetName() const
{
  return this->Name;
}

//----------------------------------------------------------------------------
cmCompiledGeneratorExpression const& cmInstalledFile::GetNameExpression() const
{
  return *(this->NameExpression);
}

//----------------------------------------------------------------------------
void cmInstalledFile::RemoveProperty(const std::string& prop)
{
  this->Properties.erase(prop);
}

//----------------------------------------------------------------------------
void cmInstalledFile::SetProperty(cmMakefile const* mf,
  const std::string& prop, const char* value)
{
  this->RemoveProperty(prop);
  this->AppendProperty(mf, prop, value);
}

//----------------------------------------------------------------------------
void cmInstalledFile::AppendProperty(cmMakefile const* mf,
  const std::string& prop, const char* value, bool /*asString*/)
{
  cmListFileBacktrace backtrace = mf->GetBacktrace();
  cmGeneratorExpression ge(&backtrace);

  Property& property = this->Properties[prop];
  property.ValueExpressions.push_back(ge.Parse(value).release());
}

//----------------------------------------------------------------------------
bool cmInstalledFile::HasProperty(
  const std::string& prop) const
{
  return this->Properties.find(prop) != this->Properties.end();
}

//----------------------------------------------------------------------------
bool cmInstalledFile::GetProperty(
  const std::string& prop, std::string& value) const
{
  PropertyMapType::const_iterator i = this->Properties.find(prop);
  if(i == this->Properties.end())
    {
    return false;
    }

  Property const& property = i->second;

  std::string output;
  std::string separator;

  for(ExpressionVectorType::const_iterator
    j = property.ValueExpressions.begin();
    j != property.ValueExpressions.end(); ++j)
    {
    output += separator;
    output += (*j)->GetInput();
    separator = ";";
    }

  value = output;
  return true;
}

//----------------------------------------------------------------------------
bool cmInstalledFile::GetPropertyAsBool(const std::string& prop) const
{
  std::string value;
  bool isSet = this->GetProperty(prop, value);
  return isSet && cmSystemTools::IsOn(value.c_str());
}

//----------------------------------------------------------------------------
void cmInstalledFile::GetPropertyAsList(const std::string& prop,
  std::vector<std::string>& list) const
{
  std::string value;
  this->GetProperty(prop, value);

  list.clear();
  cmSystemTools::ExpandListArgument(value, list);
}
