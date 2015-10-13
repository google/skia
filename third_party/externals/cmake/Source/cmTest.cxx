/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTest.h"
#include "cmSystemTools.h"

#include "cmake.h"
#include "cmMakefile.h"

//----------------------------------------------------------------------------
cmTest::cmTest(cmMakefile* mf)
  : Backtrace(mf->GetBacktrace())
{
  this->Makefile = mf;
  this->OldStyle = true;
  this->Properties.SetCMakeInstance(mf->GetCMakeInstance());
}

//----------------------------------------------------------------------------
cmTest::~cmTest()
{
}

//----------------------------------------------------------------------------
cmListFileBacktrace const& cmTest::GetBacktrace() const
{
  return this->Backtrace;
}

//----------------------------------------------------------------------------
void cmTest::SetName(const std::string& name)
{
  this->Name = name;
}

//----------------------------------------------------------------------------
void cmTest::SetCommand(std::vector<std::string> const& command)
{
  this->Command = command;
}

//----------------------------------------------------------------------------
const char *cmTest::GetProperty(const std::string& prop) const
{
  bool chain = false;
  const char *retVal =
    this->Properties.GetPropertyValue(prop, cmProperty::TEST, chain);
  if (chain)
    {
    return this->Makefile->GetProperty(prop,cmProperty::TEST);
    }
  return retVal;
}

//----------------------------------------------------------------------------
bool cmTest::GetPropertyAsBool(const std::string& prop) const
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

//----------------------------------------------------------------------------
void cmTest::SetProperty(const std::string& prop, const char* value)
{
  this->Properties.SetProperty(prop, value, cmProperty::TEST);
}

//----------------------------------------------------------------------------
void cmTest::AppendProperty(const std::string& prop,
                            const char* value, bool asString)
{
  this->Properties.AppendProperty(prop, value, cmProperty::TEST, asString);
}
