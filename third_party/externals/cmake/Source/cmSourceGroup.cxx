/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSourceGroup.h"

class cmSourceGroupInternals
{
public:
  std::vector<cmSourceGroup> GroupChildren;
};

//----------------------------------------------------------------------------
cmSourceGroup::cmSourceGroup(const char* name, const char* regex,
                             const char* parentName): Name(name)
{
  this->Internal = new cmSourceGroupInternals;
  this->SetGroupRegex(regex);
  if(parentName)
    {
    this->FullName = parentName;
    this->FullName += "\\";
    }
  this->FullName += this->Name;
}

//----------------------------------------------------------------------------
cmSourceGroup::~cmSourceGroup()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
cmSourceGroup::cmSourceGroup(cmSourceGroup const& r)
{
  this->Name = r.Name;
  this->FullName = r.FullName;
  this->GroupRegex = r.GroupRegex;
  this->GroupFiles = r.GroupFiles;
  this->SourceFiles = r.SourceFiles;
  this->Internal = new cmSourceGroupInternals(*r.Internal);
}

//----------------------------------------------------------------------------
cmSourceGroup& cmSourceGroup::operator=(cmSourceGroup const& r)
{
  this->Name = r.Name;
  this->GroupRegex = r.GroupRegex;
  this->GroupFiles = r.GroupFiles;
  this->SourceFiles = r.SourceFiles;
  *(this->Internal) = *(r.Internal);
  return *this;
}

//----------------------------------------------------------------------------
void cmSourceGroup::SetGroupRegex(const char* regex)
{
  if(regex)
    {
    this->GroupRegex.compile(regex);
    }
  else
    {
    this->GroupRegex.compile("^$");
    }
}

//----------------------------------------------------------------------------
void cmSourceGroup::AddGroupFile(const std::string& name)
{
  this->GroupFiles.insert(name);
}

//----------------------------------------------------------------------------
const char* cmSourceGroup::GetName() const
{
  return this->Name.c_str();
}

//----------------------------------------------------------------------------
const char* cmSourceGroup::GetFullName() const
{
  return this->FullName.c_str();
}

//----------------------------------------------------------------------------
bool cmSourceGroup::MatchesRegex(const char* name)
{
  return this->GroupRegex.find(name);
}

//----------------------------------------------------------------------------
bool cmSourceGroup::MatchesFiles(const char* name)
{
  std::set<std::string>::const_iterator i = this->GroupFiles.find(name);
  if(i != this->GroupFiles.end())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void cmSourceGroup::AssignSource(const cmSourceFile* sf)
{
  this->SourceFiles.push_back(sf);
}

//----------------------------------------------------------------------------
const std::vector<const cmSourceFile*>& cmSourceGroup::GetSourceFiles() const
{
  return this->SourceFiles;
}

//----------------------------------------------------------------------------
void cmSourceGroup::AddChild(cmSourceGroup child)
{
  this->Internal->GroupChildren.push_back(child);
}

//----------------------------------------------------------------------------
cmSourceGroup *cmSourceGroup::LookupChild(const char* name) const
{
  // initializing iterators
  std::vector<cmSourceGroup>::const_iterator iter =
    this->Internal->GroupChildren.begin();
  const std::vector<cmSourceGroup>::const_iterator end =
    this->Internal->GroupChildren.end();

  // st
  for(;iter!=end; ++iter)
    {
    std::string sgName = iter->GetName();

    // look if descenened is the one were looking for
    if(sgName == name)
      {
      return const_cast<cmSourceGroup*>(&(*iter)); // if it so return it
      }
    }

  // if no child with this name was found return NULL
  return NULL;
}

cmSourceGroup *cmSourceGroup::MatchChildrenFiles(const char *name)
{
  // initializing iterators
  std::vector<cmSourceGroup>::iterator iter =
    this->Internal->GroupChildren.begin();
  std::vector<cmSourceGroup>::iterator end =
    this->Internal->GroupChildren.end();

  if(this->MatchesFiles(name))
    {
    return this;
    }
  for(;iter!=end;++iter)
    {
    cmSourceGroup *result = iter->MatchChildrenFiles(name);
    if(result)
      {
      return result;
      }
    }
  return 0;
}


cmSourceGroup *cmSourceGroup::MatchChildrenRegex(const char *name)
{
  // initializing iterators
  std::vector<cmSourceGroup>::iterator iter =
    this->Internal->GroupChildren.begin();
  std::vector<cmSourceGroup>::iterator end =
    this->Internal->GroupChildren.end();

  for(;iter!=end; ++iter)
    {
    cmSourceGroup *result = iter->MatchChildrenRegex(name);
    if(result)
      {
      return result;
      }
    }
  if(this->MatchesRegex(name))
    {
    return this;
    }

  return 0;
}

std::vector<cmSourceGroup> const&
cmSourceGroup::GetGroupChildren() const
{
  return this->Internal->GroupChildren;
}
